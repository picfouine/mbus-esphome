#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/uart/uart.h"
#include "Kamstrup303WA02.h"
#include "Pwm.h"

namespace esphome {
namespace warmtemetermbus {

#define HEATMETERMBUS_METER_SENSOR(name) \
 protected: \
  sensor::Sensor *name##_sensor_{nullptr}; \
\
 public: \
  void set_##name##_sensor(sensor::Sensor *(name)) { this->name##_sensor_ = name; }

#define HEATMETERMBUS_METER_BINARYSENSOR(name) \
 protected: \
  binary_sensor::BinarySensor *name##_binary_sensor_{nullptr}; \
\
 public: \
  void set_##name##_binary_sensor(binary_sensor::BinarySensor *(name)) { this->name##_binary_sensor_ = name; }

class HeatMeterMbus : public Component, public uart::UARTDevice {
  public:  
    HeatMeterMbus() : kamstrup(this) {}
    
    HEATMETERMBUS_METER_SENSOR(heat_energy_e1)
    HEATMETERMBUS_METER_SENSOR(volume_v1)
    HEATMETERMBUS_METER_SENSOR(energy_e8_inlet)
    HEATMETERMBUS_METER_SENSOR(energy_e9_outlet)
    HEATMETERMBUS_METER_SENSOR(operating_hours)
    HEATMETERMBUS_METER_SENSOR(error_hour_counter)
    HEATMETERMBUS_METER_SENSOR(t1_actual)
    HEATMETERMBUS_METER_SENSOR(t2_actual)
    HEATMETERMBUS_METER_SENSOR(t1_minus_t2)
    HEATMETERMBUS_METER_SENSOR(power_e1_over_e3)
    HEATMETERMBUS_METER_SENSOR(power_max_month)
    HEATMETERMBUS_METER_SENSOR(flow_v1_actual)
    HEATMETERMBUS_METER_SENSOR(flow_v1_max_month)
    HEATMETERMBUS_METER_SENSOR(heat_energy_e1_old)
    HEATMETERMBUS_METER_SENSOR(volume_v1_old)
    HEATMETERMBUS_METER_SENSOR(energy_e8_inlet_old)
    HEATMETERMBUS_METER_SENSOR(energy_e9_outlet_old)
    HEATMETERMBUS_METER_SENSOR(power_max_year_old)
    HEATMETERMBUS_METER_SENSOR(flow_v1_max_year_old)
    HEATMETERMBUS_METER_SENSOR(log_year)
    HEATMETERMBUS_METER_SENSOR(log_month)
    HEATMETERMBUS_METER_SENSOR(log_day)
    HEATMETERMBUS_METER_SENSOR(bus_voltage)

    HEATMETERMBUS_METER_BINARYSENSOR(info_no_voltage_supply)
    HEATMETERMBUS_METER_BINARYSENSOR(info_t1_above_range_or_disconnected)
    HEATMETERMBUS_METER_BINARYSENSOR(info_t2_above_range_or_disconnected)
    HEATMETERMBUS_METER_BINARYSENSOR(info_t1_below_range_or_shorted)
    HEATMETERMBUS_METER_BINARYSENSOR(info_t2_below_range_or_shorted)
    HEATMETERMBUS_METER_BINARYSENSOR(info_invalid_temp_difference)
    HEATMETERMBUS_METER_BINARYSENSOR(info_v1_air)
    HEATMETERMBUS_METER_BINARYSENSOR(info_v1_wrong_flow_direction)
    HEATMETERMBUS_METER_BINARYSENSOR(info_v1_greater_than_qs_more_than_hour)

    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override;

    void enableMbus();
    void disableMbus();
    void readMbus();

  private:
    Pwm pwm;
    Kamstrup303WA02 kamstrup;
    bool updateRequested { false };
    bool mbusEnabled { true };

    static void read_mbus_task_loop(void* params);
    static esp_err_t initializeAndEnablePwm(Pwm* pwm);
};

} //namespace warmtemetermbus
} //namespace esphome