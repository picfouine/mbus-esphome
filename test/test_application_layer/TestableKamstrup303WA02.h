#ifndef TESTABLEKAMSTRUP303WA02_H_
#define TESTABLEKAMSTRUP303WA02_H_

#include <test_includes.h>
#include <Kamstrup303WA02.h>
#include "../test_data_link_layer/TestableDataLinkLayer.h"

class TestableKamstrup303WA02 : public esphome::warmtemetermbus::Kamstrup303WA02 {
  public:
    TestableKamstrup303WA02(FakeUartInterface* uart_interface) : esphome::warmtemetermbus::Kamstrup303WA02(uart_interface) {}
};

#endif // TESTABLEKAMSTRUP303WA02_H_