#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
//#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/uart/uart.h"
#include "IMbusSensor.h"
#include "MbusSensor.h"
#include "Kamstrup303WA02.h"
#include "UartInterface.h"
#include "Pwm.h"
#include <vector>

namespace esphome {
namespace warmtemetermbus {

class HeatMeterMbus : public Component, public uart::UARTDevice {
  public:  
    HeatMeterMbus();
    
    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override;
    
    MbusSensor* create_sensor(uint8_t index) {
      MbusSensor* sensor = new MbusSensor(index);
      this->sensors_.push_back(sensor);
      return sensor;
    }
    void enableMbus();
    void disableMbus();
    void readMbus();

  private:
    uint8_t address { 0x01 };
    bool have_dumped_data_blocks_ { false };
    Pwm pwm;
    Kamstrup303WA02* kamstrup;
    bool updateRequested { false };
    bool mbusEnabled { true };
    std::vector<IMbusSensor*> sensors_;

    static void read_mbus_task_loop(void* params);
    void dump_data_blocks(Kamstrup303WA02::MbusMeterData* meter_data);
    static esp_err_t initializeAndEnablePwm(Pwm* pwm);
};

} //namespace warmtemetermbus
} //namespace esphome