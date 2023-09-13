#ifndef UARTINTERFACE_H_
#define UARTINTERFACE_H_

#ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"
#endif // UNIT_TEST

namespace esphome {
namespace warmtemetermbus {

class UartInterface {
  public:
    virtual bool read_byte(uint8_t* data) = 0;
    virtual bool read_array(uint8_t* data, size_t len) = 0;
    virtual void write_array(const uint8_t* data, size_t len) = 0;
    virtual int available() const = 0;
    virtual void flush() = 0;
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // UARTINTERFACE_H_