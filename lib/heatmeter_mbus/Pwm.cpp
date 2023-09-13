#include <driver/ledc.h>
#include <math.h>
#include "Pwm.h"

using namespace std; 

namespace esphome
{
  namespace warmtemetermbus
  {
    esp_err_t Pwm::initialize(uint8_t gpioPin, uint32_t frequency, float dutyCycle)
    {
      // GPIO32
      timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
      timerConfig.duty_resolution = LEDC_TIMER_10_BIT;
      timerConfig.timer_num = LEDC_TIMER_0;
      timerConfig.freq_hz = frequency;
      timerConfig.clk_cfg = LEDC_USE_APB_CLK;

      esp_err_t configResult = ledc_timer_config(&timerConfig);
      if (ESP_OK != configResult)
      {
        return configResult;
      }

      this->dutyCycle = dutyCycle;
      uint32_t dutyCycleValue = floor((1 << 10) * this->dutyCycle);
      channelConfig.gpio_num = gpioPin;
      channelConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
      channelConfig.channel = LEDC_CHANNEL_0;
      channelConfig.intr_type = LEDC_INTR_DISABLE;
      channelConfig.timer_sel = LEDC_TIMER_0;
      channelConfig.duty = dutyCycleValue;
      channelConfig.hpoint = 0;
      channelConfig.flags.output_invert = 0;

      return ESP_OK;
    }

    esp_err_t Pwm::enable()
    {
      uint32_t dutyCycleValue = floor((1 << 10) * this->dutyCycle);
      channelConfig.duty = dutyCycleValue;
      return ledc_channel_config(&channelConfig);
    }

    esp_err_t Pwm::disable()
    {
      channelConfig.duty = 0;
      return ledc_channel_config(&channelConfig);
    }

    esp_err_t Pwm::updateDutyCycle(float dutyCycle)
    {
      this->dutyCycle = dutyCycle;
      uint32_t dutyCycleValue = floor((1 << 10) * dutyCycle);
      esp_err_t configResult = ledc_set_duty(channelConfig.speed_mode, channelConfig.channel, dutyCycleValue);
      if (ESP_OK != configResult)
      {
        return configResult;
      }
      return ledc_update_duty(channelConfig.speed_mode, channelConfig.channel);
    }
  } // namespace warmtemetermbus
} // namespace esphome
