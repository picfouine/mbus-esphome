#ifndef ESP32ARDUINOUARTINTERFACE_H_
#define ESP32ARDUINOUARTINTERFACE_H_

// #ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"
// #endif // UNIT_TEST

#include "UartInterface.h"

namespace esphome {
namespace warmtemetermbus {

class Esp32ArduinoUartInterface : public UartInterface {
  public:
    Esp32ArduinoUartInterface(uart::UARTDevice* uart_device) : uart_device_(uart_device) {}

    virtual bool read_byte(uint8_t* data) {
      return this->uart_device_->read_byte(data);
    }

    virtual bool read_array(uint8_t* data, size_t len) {
      return this->uart_device_->read_array(data, len);
    }

    virtual void write_array(const uint8_t* data, size_t len) {
      this->uart_device_->write_array(data, len);
    }

    virtual int available() const {
      return this->uart_device_->available();
    }

    virtual void flush() {
      this->uart_device_->flush();
    }
  
  protected:
    uart::UARTDevice* uart_device_;
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // ESP32ARDUINOUARTINTERFACE_H_