#ifndef HEATMETERMBUS_H_
#define HEATMETERMBUS_H_

#include <vector>

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/datatypes.h"

#include "i_mbus_sensor.h"
#include "kamstrup_303wa02.h"
#include "mbus_sensor.h"
#include "pwm.h"

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
    void enable_mbus();
    void disable_mbus();
    void read_mbus();

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

#endif // HEATMETERMBUS_H_