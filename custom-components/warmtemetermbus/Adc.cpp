#include "Adc.h"
#include <driver/adc.h>
#include <esp_adc_cal.h>

using namespace std; 

namespace esphome
{
  namespace warmtemetermbus
  {
    void Adc::configure()
    {
      adc1_config_width(ADC_WIDTH_BIT_12);
      adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
      esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1Characteristics);
    }

    uint32_t Adc::maxAdcValueOverNumberOfConversions(uint8_t numberOfConversions)
    {
      uint32_t maxAdcValue {0};
      for (uint8_t i {0}; i < numberOfConversions; ++i) {
        const uint32_t rawAdcValue = adc1_get_raw(ADC1_CHANNEL_0);
        if (rawAdcValue > maxAdcValue) {
          maxAdcValue = rawAdcValue;
        }
      }

      const uint32_t maxVoltageInMv = esp_adc_cal_raw_to_voltage(maxAdcValue, &adc1Characteristics);
      return maxVoltageInMv;
    }
  } // namespace warmtemetermbus
} // namespace esphome
