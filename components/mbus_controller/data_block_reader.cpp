#include "data_block_reader.h"

#include <vector>

#include <Arduino.h>
#ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

#ifdef UNIT_TEST
#include <test_includes.h>
#endif // UNIT_TEST

#include "mbus_reader.h"

namespace esphome {
namespace mbus_controller {

using std::vector;
using DataBlock = MbusReader::DataBlock;
using LongFrame = MbusReader::DataLinkLayer::LongFrame;

static const char * TAG {"DataBlockReader"};

vector<DataBlock*>* DataBlockReader::read_data_blocks_from_long_frame(const LongFrame* const long_frame) {
  auto *data_blocks = new vector<DataBlock*>();

  this->long_frame_ = long_frame;
  current_position_in_user_data_ = MbusReader::FIXED_DATA_HEADER_SIZE;
  const uint8_t USER_DATA_LENGTH = long_frame->l - 3;

  uint8_t data_block_index = 0;
  while (current_position_in_user_data_ < USER_DATA_LENGTH) {
     DataBlock *next_data_block = this->read_next_data_block();
     next_data_block->index = data_block_index++;
     data_blocks->push_back(next_data_block);
  }

  return data_blocks;
}

DataBlock* DataBlockReader::read_next_data_block() {
  auto *data_block = new DataBlock();

  this->read_dif_into_block(data_block);
  this->read_vif_into_block(data_block);
  this->read_data_into_block(data_block);

  return data_block;
}

void DataBlockReader::read_dif_into_block(DataBlock* data_block) {
  const uint8_t dif = this->read_next_byte();
  data_block->storage_number = (dif & (1 << DIF_BIT_LSB_STORAGE_NUMBER)) >> DIF_BIT_LSB_STORAGE_NUMBER;
  data_block->function = static_cast<MbusReader::Function>((dif & (0b11u << DIF_BIT_FUNCTION_FIELD_LOW_BIT)) >> DIF_BIT_FUNCTION_FIELD_LOW_BIT);
  const uint8_t data_field = dif & DIF_BITS_DATA_FIELD;
  switch (data_field) {
    case 0:
      data_block->data_length = 0;
      data_block->binary_data = nullptr;
      break;
    case 1:
      data_block->data_length = 1;
      data_block->binary_data = new uint8_t[1];
      break;
    case 2:
      data_block->data_length = 2;
      data_block->binary_data = new uint8_t[2];
      break;
    case 3:
      data_block->data_length = 3;
      data_block->binary_data = new uint8_t[3];
      break;
    case 4:
      data_block->data_length = 4;
      data_block->binary_data = new uint8_t[4];
      break;
    default:
      ESP_LOGW(TAG, "Data Field %x not supported", data_field);
      data_block->data_length = 0;
      data_block->binary_data = nullptr;
      break;
  }
  bool dif_extended = (dif & (1 << DIF_BIT_EXTENDED)) >> DIF_BIT_EXTENDED;
  uint8_t extension_byte_index = 0;
  while (dif_extended) {
    uint8_t dife = this->read_next_byte();
    
    uint8_t storage_number_shift = 4 * extension_byte_index + 1;
    uint64_t storage_number_extension = (dife & DIFE_BITS_STORAGE_NUMBER) << storage_number_shift;
    data_block->storage_number |= storage_number_extension;

    uint8_t tariff_shift = 2 * extension_byte_index;
    uint32_t tariff_extension = ((dife & DIFE_BITS_TARIFF) >> DIFE_BIT_TARIFF_LOW_BIT) << tariff_shift;
    data_block->tariff |= tariff_extension;
    
    dif_extended = (dife & (1 << DIF_BIT_EXTENDED)) >> DIF_BIT_EXTENDED;
    ++extension_byte_index;
  }
}

void DataBlockReader::read_vif_into_block(DataBlock* data_block) {
  uint8_t vif = this->read_next_byte();
  uint8_t unit_and_multiplier = vif & VIF_BITS_UNIT_AND_MULTIPLIER;
  
  bool vif_is_primary = (unit_and_multiplier <= 0b01111011); // See 6.3.2, Value Information Block (VIB)
  bool vif_is_manufacturer_specific = (unit_and_multiplier == 0b01111111);
  if (vif_is_primary) {
    if ((unit_and_multiplier & 0b1111000) == 0b0000000) {
      // Energy in Wh
      data_block->ten_power = (unit_and_multiplier & 0b111) - 3;
      data_block->unit = MbusReader::Unit::WH;
    } else if ((unit_and_multiplier & 0b1111000) == 0b0001000) {
      // Energy in J
      data_block->ten_power = unit_and_multiplier & 0b111;
      data_block->unit = MbusReader::Unit::J;
    } else if ((unit_and_multiplier & 0b1111000) == 0b0010000) {
      // Volume in m3
      data_block->ten_power = (unit_and_multiplier & 0b111) - 6;
      data_block->unit = MbusReader::Unit::CUBIC_METER;
    } else if ((unit_and_multiplier & 0b1111100) == 0b0100000) {
      // On Time
      data_block->ten_power = 0;
      const uint8_t unit_value = (unit_and_multiplier & 0b11);
      switch (unit_value) {
        case 0:
          data_block->unit = MbusReader::Unit::SECONDS;
          break;
        case 1:
          data_block->unit = MbusReader::Unit::MINUTES;
          break;
        case 2:
          data_block->unit = MbusReader::Unit::HOURS;
          break;
        case 3:
          data_block->unit = MbusReader::Unit::DAYS;
          break;
      }
    } else if ((unit_and_multiplier & 0b1111000) == 0b0101000) {
      // Power in W
      data_block->ten_power = (unit_and_multiplier & 0b111) - 3;
      data_block->unit = MbusReader::Unit::W;
    } else if ((unit_and_multiplier & 0b1111000) == 0b0110000) {
      // Power in J/h
      data_block->ten_power = unit_and_multiplier & 0b111;
      data_block->unit = MbusReader::Unit::J_PER_HOUR;
    } else if ((unit_and_multiplier & 0b1111000) == 0b0111000) {
      // Volume Flow in m3/h
      data_block->ten_power = (unit_and_multiplier & 0b111) - 6;
      data_block->unit = MbusReader::Unit::CUBIC_METER_PER_HOUR;
    } else if ((unit_and_multiplier & 0b1111000) == 0b1000000) {
      // Volume Flow in m3/min
      data_block->ten_power = (unit_and_multiplier & 0b111) - 7;
      data_block->unit = MbusReader::Unit::CUBIC_METER_PER_MINUTE;
    } else if ((unit_and_multiplier & 0b1111000) == 0b1001000) {
      // Volume Flow in m3/s
      data_block->ten_power = (unit_and_multiplier & 0b111) - 9;
      data_block->unit = MbusReader::Unit::CUBIC_METER_PER_SECOND;
    } else if ((unit_and_multiplier & 0b1111100) == 0b1011000) {
      // Flow Temperature in deg C
      data_block->ten_power = (unit_and_multiplier & 0b11) - 3;
      data_block->unit = MbusReader::Unit::DEGREES_CELSIUS;
    } else if ((unit_and_multiplier & 0b1111100) == 0b1011100) {
      // Return Temperature in deg C
      data_block->ten_power = (unit_and_multiplier & 0b11) - 3;
      data_block->unit = MbusReader::Unit::DEGREES_CELSIUS;
    } else if ((unit_and_multiplier & 0b1111100) == 0b1100000) {
      // Temperature Difference in K
      data_block->ten_power = (unit_and_multiplier & 0b11) - 3;
      data_block->unit = MbusReader::Unit::K;
    } else if ((unit_and_multiplier & 0b1111110) == 0b1101100) {
      // Time Point
      data_block->ten_power = 0;
      data_block->unit = MbusReader::Unit::DATE;
    } else {
      ESP_LOGW(TAG, "Primary VIF with unit and multiplier %x not yet supported", unit_and_multiplier);
    }
  } else if (vif_is_manufacturer_specific) {
    data_block->unit = MbusReader::Unit::MANUFACTURER_SPECIFIC;
    data_block->is_manufacturer_specific = true;
  } else {
    ESP_LOGW(TAG, "Only primary VIF or manufacturer specific supported. VIF %x unsupported.", vif);
  }

  // Ignore the VIF extension; just skip over it
  uint8_t vif_is_extend = (vif & (1 << VIF_BIT_EXTENDED)) >> VIF_BIT_EXTENDED;
  while (vif_is_extend) {
    uint8_t vif_extension = this->read_next_byte();
    vif_is_extend = (vif_extension & (1 << VIF_BIT_EXTENDED)) >> VIF_BIT_EXTENDED;
  }
}

void DataBlockReader::read_data_into_block(DataBlock* data_block) {
  for (uint8_t i = 0; i < data_block->data_length; ++i) {
    data_block->binary_data[i] = this->read_next_byte();
  }
}

uint8_t DataBlockReader::read_next_byte() {
  return this->long_frame_->user_data[this->current_position_in_user_data_++];
}

} //namespace mbus_controller
} //namespace esphome