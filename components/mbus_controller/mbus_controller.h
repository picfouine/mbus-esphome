#ifndef MBUS_CONTROLLER_H_
#define MBUS_CONTROLLER_H_

#include <map>
#include <string>
#include <vector>

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/datatypes.h"

#include "i_mbus_sensor.h"
#include "mbus_reader.h"
#include "mbus_sensor.h"
#include "pwm.h"

namespace esphome {
namespace mbus_controller {

class MbusController : public Component, public uart::UARTDevice {
 public:  
  MbusController(uint8_t address);
  
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

 protected:
  uint8_t address_ { 0x01 };
  bool have_dumped_data_blocks_ { false };
  Pwm pwm_;
  MbusReader* kamstrup_ { nullptr };
  bool update_requested_ { false };
  bool mbus_enabled_ { true };
  std::vector<IMbusSensor*> sensors_;
  static std::map<MbusReader::Unit, std::string> unit_names_;
  static std::map<MbusReader::Function, std::string> function_names_;

 private:
  static void read_mbus_task_loop(void* params);
  void dump_data_blocks(MbusReader::MbusMeterData* meter_data);
  static esp_err_t initialize_and_enable_pwm(Pwm* pwm);
};

} //namespace mbus_controller
} //namespace esphome

#endif // MBUS_CONTROLLER_H_