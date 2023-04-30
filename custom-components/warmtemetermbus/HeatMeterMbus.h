#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "Kamstrup303WA02.h"

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
    HeatMeterMbus() : PollingComponent(10000), kamstrup(this) {}
    
    HEATMETERMBUS_METER_SENSOR(heat_energy_e1)
    HEATMETERMBUS_METER_SENSOR(volume_v1)
    HEATMETERMBUS_METER_SENSOR(energy_e8_inlet)
    HEATMETERMBUS_METER_SENSOR(t1_actual)
//    HEATMETERMBUS_METER_BINARYSENSOR(info_v1_air)

    void setup() override;
    void update() override;
    void dump_config() override;
    float get_setup_priority() const override;

  private:
    Kamstrup303WA02 kamstrup;
};

} //namespace warmtemetermbus
} //namespace esphome