#include "mbus_reader.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#include "esphome/core/log.h"
#endif // UNIT_TEST

#include "data_block_reader.h"
#include "uart_interface.h"

namespace esphome {
namespace mbus_controller {

static const char * TAG {"MbusReader"};

MbusReader::MbusMeterData::~MbusMeterData() {
  if (this->data_blocks != nullptr) {
    this->deallocate_data_blocks();
  }
}

void MbusReader::MbusMeterData::deallocate_data_blocks() {
  for (auto data_block : *(this->data_blocks)) {
    if (data_block->binary_data != nullptr) {
      delete[] data_block->binary_data;
      data_block->binary_data = nullptr;
    }
    delete data_block;
  }
  delete this->data_blocks;
  this->data_blocks = nullptr;
}

MbusReader::MbusReader(IUartInterface* uart_interface) {
  this->data_link_layer_ = new DataLinkLayer(uart_interface);
}

bool MbusReader::read_meter_data(MbusReader::MbusMeterData* meter_data, const uint8_t address) {
  bool success { false };
  DataLinkLayer::LongFrame response_to_req_ud2;
  if (!this->data_link_layer_->req_ud2(address, &response_to_req_ud2)) {
    ESP_LOGW(TAG, "req_ud2: fail");
    if (response_to_req_ud2.user_data != nullptr) {
      // Deallocate user_data, allocated by data link layer
      delete[] response_to_req_ud2.user_data;
    }
    return false;
  }

  // Check upper nibble CI
  if ((response_to_req_ud2.ci & 0xF0) != 0x70) {
    ESP_LOGW(TAG, "CI not as expected");
  }

  // Check type of response
  switch (response_to_req_ud2.ci & 0x03) {
    case CiMeterToMasterCode::GENERAL_APPLICATION_ERRORS:
      ESP_LOGW(TAG, "General App Error");
      break;
    case CiMeterToMasterCode::ALARM_STATUS:
      ESP_LOGW(TAG, "Alarm Status");
      break;
    case CiMeterToMasterCode::VARIABLE_DATA_RESPOND: {
      ESP_LOGV(TAG, "Variable data response");
      DataBlockReader data_block_reader;
      auto data_blocks = data_block_reader.read_data_blocks_from_long_frame(&response_to_req_ud2);
      meter_data->data_blocks = data_blocks;
      success = true;
      break;
    }
    case CiMeterToMasterCode::FIXED_DATA_RESPOND:
      ESP_LOGW(TAG, "Fixed data response");
      break;
  }
  // Deallocate user_data, allocated by data link layer
  delete[] response_to_req_ud2.user_data;

  return success;
}

bool MbusReader::DataLinkLayer::req_ud2(const uint8_t address, LongFrame* response_frame) {
  bool success { false };

  if (!this->meter_is_initialized_ && !this->initialize_meter(address)) {
    ESP_LOGW(TAG, "Could not initialize meter");
    return false;
  }

  const uint8_t fcb = this->next_req_ud2_fcb_ ? 1u : 0u;
  const uint8_t c = (1 << C_FIELD_BIT_DIRECTION) | (fcb << C_FIELD_BIT_FCB) | (1 << C_FIELD_BIT_FCV) | C_FIELD_FUNCTION_REQ_UD2;
  bool received_response_to_request = this->try_send_short_frame(c, address);
  if (received_response_to_request) {
    const bool received_sane_response = this->parse_long_frame_response(response_frame);
    if (received_sane_response && response_frame->a == address) {
      success = true;
    }
  }

  if (success) {
    this->next_req_ud2_fcb_ = !this->next_req_ud2_fcb_;
  }
  return success;
}

bool MbusReader::DataLinkLayer::initialize_meter(const uint8_t address) {
  if (this->snd_nke(address)) {
    this->meter_is_initialized_ = true;
    return true;
  } else {
    this->meter_is_initialized_ = false;
    return false;
  }
}

bool MbusReader::DataLinkLayer::parse_long_frame_response(MbusReader::DataLinkLayer::LongFrame* long_frame) {
  long_frame->user_data = nullptr;

  uint8_t current_byte { 0 };

  // Check start byte
  if (!this->read_next_byte(&current_byte) || (current_byte != START_BYTE_CONTROL_AND_LONG_FRAME)) {
    this->flush_rx_buffer();
    return false;
  }

  // Check two identical L fields
  uint8_t first_l_field { 0 };
  uint8_t second_l_field { 0 };
  if (!this->read_next_byte(&first_l_field) || !this->read_next_byte(&second_l_field)) {
    this->flush_rx_buffer();
    return false;
  }
  if (first_l_field != second_l_field) {
    this->flush_rx_buffer();
    return false;
  }
  long_frame->l = first_l_field;

  // Check 2nd start byte
  if (!this->read_next_byte(&current_byte) || (current_byte != START_BYTE_CONTROL_AND_LONG_FRAME)) {
    this->flush_rx_buffer();
    return false;
  }
  
  // Check C field
  if (!this->read_next_byte(&long_frame->c) || ((long_frame->c & 0x0F) != 0x08) || ((long_frame->c & 0xC0) != 0x00)) {
    this->flush_rx_buffer();
    return false;
  }

  // Read A field
  if (!this->read_next_byte(&long_frame->a)) {
    this->flush_rx_buffer();
    return false;
  }

  // Read CI field
  if (!this->read_next_byte(&long_frame->ci)) {
    this->flush_rx_buffer();
    return false;
  }

  // Read user data
  // Expected amount of user data: L - 3 (3 for C, A, CI)
  const uint8_t user_data_len = long_frame->l - 3;
  long_frame->user_data = new uint8_t[user_data_len];
  for (uint8_t i { 0 }; i < user_data_len; ++i) {
    if (!this->read_next_byte(&current_byte)) {
      delete[] long_frame->user_data;
      long_frame->user_data = nullptr;
      this->flush_rx_buffer();
      return false;
    }
    long_frame->user_data[i] = current_byte;
  }

  // Calculate, read and check the check sum
  const uint8_t calculated_check_sum = this->calculate_checksum(long_frame);
  if (!this->read_next_byte(&long_frame->check_sum) || (long_frame->check_sum != calculated_check_sum)) {
    delete[] long_frame->user_data;
    long_frame->user_data = nullptr;
    this->flush_rx_buffer();
    return false;
  }

  // Check stop byte
  if (!this->read_next_byte(&current_byte) || (STOP_BYTE != current_byte)) {
    delete[] long_frame->user_data;
    long_frame->user_data = nullptr;
    this->flush_rx_buffer();
    return false;
  }

  // Flip the FCB bit to use for next REQ_UD2 message (see 5.5)
  this->next_req_ud2_fcb_ = !this->next_req_ud2_fcb_;
  return true;
}

bool MbusReader::DataLinkLayer::read_next_byte(uint8_t* received_byte) {
  const uint32_t time_before_starting_to_wait { millis() };
  while (this->uart_interface_->available() == 0) {
    delay(1);
    if (millis() - time_before_starting_to_wait > 150) {
      ESP_LOGW(TAG, "No data available after timeout");
      return false;
    }
  }
  this->uart_interface_->read_byte(received_byte);
	return true;
}

bool MbusReader::DataLinkLayer::snd_nke(const uint8_t address) {
  bool success { false };

  const uint8_t c = (1 << C_FIELD_BIT_DIRECTION) | (C_FIELD_FUNCTION_SND_NKE);
  bool received_response_to_short_frame = this->try_send_short_frame(c, address);
  if (received_response_to_short_frame) {
    uint8_t received_byte { 0 };
    this->uart_interface_->read_byte(&received_byte);
    if (START_BYTE_SINGLE_CHARACTER == received_byte) {
      success = true;
      this->next_req_ud2_fcb_ = true;
    } else {
      ESP_LOGW(TAG, "Wrong answer to SND_NKE: %X", received_byte);
    }
  } else {
    ESP_LOGW(TAG, "No response to SND_NKE");
  }

  return success;
}

// Meter must wait at least 11 bit times, and at max 330 bit times + 50ms before answering.
// In case no answer within that time, retry at most twice.
// (see 5.4 Communication Process)
bool MbusReader::DataLinkLayer::try_send_short_frame(const uint8_t c, const uint8_t a) {
  bool success { false };
  bool data_is_received { false };
  this->flush_rx_buffer();
  for (uint8_t transmit_attempt {0}; transmit_attempt < 3 && !data_is_received; ++transmit_attempt) {
    if (transmit_attempt > 0) {
      ESP_LOGD(TAG, "Retry transmit short frame");
    }
    this->send_short_frame(c, a);
    // Sending takes about 4,58ms per byte. Short frame takes about 23ms to send.
    vTaskDelay(25 / portTICK_PERIOD_MS);
    data_is_received = this->wait_for_incoming_data();
  }
  success = data_is_received;
  return success;
}

void MbusReader::DataLinkLayer::flush_rx_buffer() {
  while (this->uart_interface_->available()) {
    int32_t byte_count_in_buffer {this->uart_interface_->available()};
    if (byte_count_in_buffer > 255) {
      byte_count_in_buffer = 255;
    }
    uint8_t bytes_in_buffer[byte_count_in_buffer];
    this->uart_interface_->read_array(bytes_in_buffer, byte_count_in_buffer);
  }
}

void MbusReader::DataLinkLayer::send_short_frame(const uint8_t c, const uint8_t a) {
  const uint8_t data[] = { c, a };
  const uint8_t checksum { this->calculate_checksum(data, 2) };
  const uint8_t short_frame[] = { START_BYTE_SHORT_FRAME, c, a, checksum, STOP_BYTE };
  this->uart_interface_->write_array(short_frame, 5);
  delay(1);
  this->uart_interface_->flush();
  delay(1);
}

bool MbusReader::DataLinkLayer::wait_for_incoming_data() {
  bool data_received {false};
  // 330 bits + 50ms = 330 * 1000 / 2400 + 50 ms = 187,5 ms
  // Wait at least 11 bit times = 5ms
  // For some reason waiting 5ms, then try until 190ms passed, does not work.
  // Waiting for 138ms initially and then another 500ms at max, does work.
  delay(138);
  for (uint16_t i {0}; i < 500; ++i) {
    if (this->uart_interface_->available() > 0) {
      data_received = true;
      break;
    }
    delay(1);
  }
  if (!data_received) {
    ESP_LOGW(TAG, "waitForIncomingData - exit - No data received");
  }
  return data_received;
}

uint8_t MbusReader::DataLinkLayer::calculate_checksum(const LongFrame* long_frame) const {
  const uint8_t user_data_len = long_frame->l - 3;
  uint8_t checksum = this->calculate_checksum(long_frame->user_data, user_data_len);
  checksum += long_frame->c;
  checksum += long_frame->a;
  checksum += long_frame->ci;
  return checksum;
}

uint8_t MbusReader::DataLinkLayer::calculate_checksum(const uint8_t* data, size_t length) const {
  uint8_t checksum { 0 };
  for (size_t i = 0; i < length; ++i) {
    checksum += data[i];
  }
  return checksum;
}

} //namespace mbus_controller
} //namespace esphome