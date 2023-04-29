#include "esphome/core/log.h"
#include <math.h>
#include "HeatMeterMbus.h"
#include "Kamstrup303WA02.h"

//#include <iostream>
//#include <sstream>

using namespace std; 

namespace esphome
{
  namespace warmtemetermbus
  {

    static const char *TAG = "heatmetermbus.sensor";

    void HeatMeterMbus::setup()
    {
    }

    void HeatMeterMbus::update()
    {
      // Let's request data, and wait for its results :-)
      Kamstrup303WA02::MeterData meterData;
      ESP_LOGI(TAG, "About to readData");
      bool readSuccessful = kamstrup.readData(&meterData);

      if (readSuccessful)
      {
        ESP_LOGI(TAG, "Successfully read meter data");
        test_temperature_sensor_->publish_state(1);
        ESP_LOGD(TAG, "Raw t1Actual value (decimal): %d", meterData.t1Actual.value);
        ESP_LOGD(TAG, "t1Actual tenPower (decimal): %d", meterData.t1Actual.tenPower);
        float t1ActualValue = pow(10, meterData.t1Actual.tenPower) * meterData.t1Actual.value;
        ESP_LOGD(TAG, "Calculated t1Actual value (decimal): %f", t1ActualValue);
        t1_actual_sensor_->publish_state(t1ActualValue);
      }
      else
      {
        ESP_LOGE(TAG, "Did not successfully read meter data");
        test_temperature_sensor_->publish_state(2);
      }
    }

    float HeatMeterMbus::get_setup_priority() const
    {
      // After UART bus
      return setup_priority::BUS - 1.0f;
    }

    void HeatMeterMbus::dump_config()
    {
      ESP_LOGCONFIG(TAG, "HeatMeterMbus sensor");
      LOG_SENSOR("  ", "Test Temperature", this->test_temperature_sensor_);
    }

  } // namespace warmtemetermbus
} // namespace esphome