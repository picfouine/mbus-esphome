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
      // Let's send SND_NKE, and wait for its results :-)
      Kamstrup303WA02 k(this);
      bool receivedCorrectAnswer = k.sndNke(0x01);

      if (receivedCorrectAnswer)
      {
        ESP_LOGI(TAG, "Received correct answer to SND_NKE");
        test_temperature_sensor_->publish_state(1);
      }
      else
      {
        ESP_LOGI(TAG, "Did not receive an answer, or incorrect answer to SND_NKE");
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