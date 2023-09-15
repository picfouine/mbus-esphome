#ifndef TESTABLEKAMSTRUP303WA02_H_
#define TESTABLEKAMSTRUP303WA02_H_

#include <mbus_reader.h>
#include "../test_data_link_layer/testable_data_link_layer.h"

class TestableKamstrup303WA02 : public esphome::mbus_controller::MbusReader {
  public:
    TestableKamstrup303WA02(FakeUartInterface* uart_interface) : esphome::mbus_controller::MbusReader(uart_interface) {}
};

#endif // TESTABLEKAMSTRUP303WA02_H_