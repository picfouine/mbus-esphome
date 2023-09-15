#ifndef UNIT_TEST

#include "pwm.h"

#include <math.h>

#include <driver/ledc.h>
#include "esp_err.h"
#include "esphome/core/datatypes.h"
#include "hal/ledc_types.h"

using namespace std; 

namespace esphome
{
  namespace mbus_controller
  {
    esp_err_t Pwm::initialize(uint8_t gpio_pin, uint32_t frequency, float duty_cycle)
    {
      // GPIO32
      this->timer_config_.speed_mode = LEDC_HIGH_SPEED_MODE;
      this->timer_config_.duty_resolution = LEDC_TIMER_10_BIT;
      this->timer_config_.timer_num = LEDC_TIMER_0;
      this->timer_config_.freq_hz = frequency;
      this->timer_config_.clk_cfg = LEDC_USE_APB_CLK;

      esp_err_t config_result = ledc_timer_config(&timer_config_);
      if (ESP_OK != config_result)
      {
        return config_result;
      }

      this->duty_cycle_ = duty_cycle;
      uint32_t duty_cycle_value = floor((1 << 10) * this->duty_cycle_);
      this->channel_config_.gpio_num = gpio_pin;
      this->channel_config_.speed_mode = LEDC_HIGH_SPEED_MODE;
      this->channel_config_.channel = LEDC_CHANNEL_0;
      this->channel_config_.intr_type = LEDC_INTR_DISABLE;
      this->channel_config_.timer_sel = LEDC_TIMER_0;
      this->channel_config_.duty = duty_cycle_value;
      this->channel_config_.hpoint = 0;
      this->channel_config_.flags.output_invert = 0;

      return ESP_OK;
    }

    esp_err_t Pwm::enable()
    {
      uint32_t duty_cycle_value = floor((1 << 10) * this->duty_cycle_);
      this->channel_config_.duty = duty_cycle_value;
      return ledc_channel_config(&this->channel_config_);
    }

    esp_err_t Pwm::disable()
    {
      this->channel_config_.duty = 0;
      return ledc_channel_config(&this->channel_config_);
    }

    esp_err_t Pwm::update_duty_cycle(float duty_cycle)
    {
      this->duty_cycle_ = duty_cycle;
      uint32_t duty_cycle_value = floor((1 << 10) * duty_cycle);
      esp_err_t config_result = ledc_set_duty(this->channel_config_.speed_mode, this->channel_config_.channel, duty_cycle_value);
      if (ESP_OK != config_result)
      {
        return config_result;
      }
      return ledc_update_duty(this->channel_config_.speed_mode, this->channel_config_.channel);
    }
  } // namespace mbus_controller
} // namespace esphome

#endif // UNIT_TEST