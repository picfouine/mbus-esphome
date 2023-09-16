#ifndef TESTABLE_MBUS_READER_H_
#define TESTABLE_MBUS_READER_H_

#include <mbus_reader.h>
#include "../test_data_link_layer/testable_data_link_layer.h"

class TestableMbusReader : public esphome::mbus_controller::MbusReader {
  public:
    TestableMbusReader(FakeUartInterface* uart_interface) : esphome::mbus_controller::MbusReader(uart_interface) {}
};

#endif // TESTABLE_MBUS_READER_H_