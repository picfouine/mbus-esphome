#include "esphome/core/log.h"
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