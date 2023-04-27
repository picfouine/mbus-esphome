#include "esphome/core/log.h"
#include "HeatMeterMbus.h"

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
      uint8_t c = (1 << CFieldBitDirection) | (CFieldFunctionSndNke);
      uint8_t a = 0x01;
      uint8_t data[] = { c, a };
      uint8_t checksum = calculateChecksum(data, 2);
      uint8_t shortFrame[] = { StartByteShortFrame, c, a, checksum, StopByte };

      write_array(shortFrame, 5);
      const uint32_t startToWaitTime = millis();
      bool receivedAnAnswer = true;
      while (!available()) {
        delay(1);
        const uint32_t now = millis();
        if (now - startToWaitTime > 187) {
          // Time to wait is over... no answer!
          receivedAnAnswer = false;
          break;
        }
      }

      if (receivedAnAnswer)
      {
        uint8_t receivedByte {0x00};
        read_byte(&receivedByte);
        ESP_LOGV(TAG, "Received byte: %d", receivedByte);
        if (0xE5 == receivedByte)
        {
          test_temperature_sensor_->publish_state(1);
        }
        else
        {
          test_temperature_sensor_->publish_state(2);
        }

      }
      else
      {
        ESP_LOGV(TAG, "Did not receive an answer");
        test_temperature_sensor_->publish_state(3);
      }
    }

    uint8_t HeatMeterMbus::calculateChecksum(const uint8_t data[], uint8_t length) {
      uint8_t checksum = 0;
      for (uint8_t i = 0; i < length; ++i) {
        checksum += data[i];
      }
      return checksum;
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