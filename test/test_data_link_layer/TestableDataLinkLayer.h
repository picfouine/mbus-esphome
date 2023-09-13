#ifndef TESTABLEDATALINKLAYER_H_
#define TESTABLEDATALINKLAYER_H_

#include <test_includes.h>
#include <UartInterface.h>
#include <vector>
#include <queue>
#include <Kamstrup303WA02.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using std::vector;
using std::queue;

class FakeUartInterface : public esphome::warmtemetermbus::UartInterface {
  public:
    typedef struct WrittenArray {
      const uint8_t* data;
      size_t len;
    } WrittenArray;

    // UartInterface
    virtual bool read_byte(uint8_t* data) {
      if (0 == this->fake_data_to_return_.size()) {
        return false;
      } else {
        *data = this->fake_data_to_return_.front();
        this->fake_data_to_return_.pop();
        return true;
      }
    }

    virtual bool read_array(uint8_t* data, size_t len) {
      for (size_t i = 0; i < len; ++i) {
        if (this->fake_data_to_return_.empty()) {
          break;
        } else {
          *data++ = this->fake_data_to_return_.front();
          this->fake_data_to_return_.pop();
        }
      }
      return true;
    }

    virtual void write_array(const uint8_t* data, size_t len) {
      uint8_t *copied_data = new uint8_t[len];
      for (size_t i = 0; i < len; ++i) {
        copied_data[i] = data[i];
      }
      WrittenArray writtenArray = { .data = copied_data, .len = len };
      this->written_arrays_.push_back(writtenArray);
      ++(this->write_array_call_count_);
    }

    virtual int available() const {
      return this->fake_data_to_return_.size();
    }

    void flush() {}

    // Access / configure methods
    vector<WrittenArray> written_arrays_;

    void set_fake_data_to_return(const uint8_t* fake_data, size_t len) {
      for (size_t i = 0; i < len; ++i) {
        this->fake_data_to_return_.push(fake_data[i]);
      }
    }

    uint8_t write_array_call_count() const {
      return this->write_array_call_count_;
    }

  protected:
    queue<uint8_t> fake_data_to_return_;
    uint8_t write_array_call_count_ { 0 };
};

class TestableDataLinkLayer : public esphome::warmtemetermbus::Kamstrup303WA02::DataLinkLayer {
  public:
    TestableDataLinkLayer(FakeUartInterface* uartInterface) : esphome::warmtemetermbus::Kamstrup303WA02::DataLinkLayer(uartInterface) {}

    bool call_try_send_short_frame(const uint8_t c, const uint8_t a) {
      return this->try_send_short_frame(c, a);
    }

    uint8_t call_calculate_checksum(const uint8_t* data, size_t length) {
      return this->calculate_checksum(data, length);
    }

    uint8_t call_calculate_checksum(const LongFrame* long_frame) {
      return this->calculate_checksum(long_frame);
    }

    bool get_next_req_ud2_fcb() {
      return this->next_req_ud2_fcb_;
    }

    void set_meter_is_initialized(bool is_initialized) {
      this->meter_is_initialized_ = is_initialized;
    }
};

typedef struct FakeUartInterfaceTaskArgs {
  FakeUartInterface* uart_interface;
  const uint8_t respond_to_nth_write;
  const uint8_t delay_in_ms;
  const uint8_t* data_to_return;
  const size_t len_of_data_to_return;
} FakeUartInterfaceTaskArgs;

void fake_uart_interface_task(void* param) {
  FakeUartInterfaceTaskArgs *args = reinterpret_cast<FakeUartInterfaceTaskArgs*>(param);
  FakeUartInterface *uartInterface = args->uart_interface;
  while (uartInterface->write_array_call_count() < args->respond_to_nth_write) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  delay(args->delay_in_ms);
  uartInterface->set_fake_data_to_return(args->data_to_return, args->len_of_data_to_return);
  vTaskDelete(NULL);
}

#endif // TESTABLEDATALINKLAYER_H_