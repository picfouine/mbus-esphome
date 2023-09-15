#ifndef PWM_H_
#define PWM_H_

#include <driver/ledc.h>
#include "esp_err.h"
#include "esphome/core/datatypes.h"

namespace esphome {
namespace mbus_controller {

class Pwm {
 public:  
  Pwm() {}
  ~Pwm() {}

  esp_err_t initialize(uint8_t gpio_pin, uint32_t frequency, float duty_cycle);
  esp_err_t enable();
  esp_err_t disable();
  esp_err_t update_duty_cycle(float duty_cycle);

 protected:
  float duty_cycle_ { 85.0f };
  ledc_timer_config_t timer_config_;
  ledc_channel_config_t channel_config_;
  
 private:
  Pwm(const Pwm& pwm);
};

} //namespace mbus_controller
} //namespace esphome

#endif // PWM_H_