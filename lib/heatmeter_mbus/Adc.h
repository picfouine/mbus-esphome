#pragma once

#include <driver/adc.h>
#include <esp_adc_cal.h>

namespace esphome {
namespace warmtemetermbus {

class Adc {
public:  
  Adc() {}
  ~Adc() {}

  void configure();
  uint32_t maxAdcValueOverNumberOfConversions(uint8_t numberOfConversions);
  
private:
  Adc(const Adc& pwm);
  esp_adc_cal_characteristics_t adc1Characteristics;
};

} //namespace warmtemetermbus
} //namespace esphome