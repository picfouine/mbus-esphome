#pragma once

#include <driver/ledc.h>

namespace esphome {
namespace warmtemetermbus {

class Pwm {
public:  
  Pwm() {}
  ~Pwm() {}

  esp_err_t initialize(uint8_t gpioPin, uint32_t frequency, float dutyCycle);
  esp_err_t enable();
  esp_err_t disable();
  esp_err_t updateDutyCycle(float dutyCycle);
  
private:
  Pwm(const Pwm& pwm);

  float dutyCycle { 85.0f };
  ledc_timer_config_t timerConfig;
  ledc_channel_config_t channelConfig;
};

} //namespace warmtemetermbus
} //namespace esphome