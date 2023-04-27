#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace warmtemetermbus {

#define HEATMETERMBUS_METER_SENSOR(name) \
 protected: \
  sensor::Sensor *name##_sensor_{nullptr}; \
\
 public: \
  void set_##name##_sensor(sensor::Sensor *(name)) { this->name##_sensor_ = name; }

class HeatMeterMbus : public PollingComponent, public uart::UARTDevice {
  public:  
    HeatMeterMbus() : PollingComponent(10000) {}
    
    HEATMETERMBUS_METER_SENSOR(test_temperature)

//    HEATMETERMBUS_METER_BINARYSENSOR(info_v1_air)

    void setup() override;
    void update() override;
    void dump_config() override;
    float get_setup_priority() const override;
};

} //namespace warmtemetermbus
} //namespace esphome