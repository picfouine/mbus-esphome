#ifndef UARTINTERFACE_H_
#define UARTINTERFACE_H_

#ifdef UNIT_TEST
#include <test_includes.h>
#else
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

namespace esphome {
namespace mbus_controller {

class IUartInterface {
 public:
  virtual bool read_byte(uint8_t* data) = 0;
  virtual bool read_array(uint8_t* data, size_t len) = 0;
  virtual void write_array(const uint8_t* data, size_t len) = 0;
  virtual int available() const = 0;
  virtual void flush() = 0;
};

} //namespace mbus_controller
} //namespace esphome

#endif // UARTINTERFACE_H_