#ifndef UNIT_TEST

#include "esphome/core/log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "HeatMeterMbus.h"
#include "IMbusSensor.h"
#include "Kamstrup303WA02.h"
#include "Esp32ArduinoUartInterface.h"

using namespace std; 

namespace esphome
{
  namespace warmtemetermbus
  {
    static const char *TAG = "heatmetermbus.sensor";

    bool pwmInitialized { false };
    bool pwmEnabled { false };

    HeatMeterMbus::HeatMeterMbus() {
      Esp32ArduinoUartInterface *uart_interface = new Esp32ArduinoUartInterface(this);
      this->kamstrup = new Kamstrup303WA02(uart_interface);
    }

    void HeatMeterMbus::setup()
    {
      ESP_LOGI(TAG, "setup()");
      if (ESP_OK != initializeAndEnablePwm(&pwm))
      {
        ESP_LOGE(TAG, "Error initializing and enabling PWM");
        return;
      }

      xTaskCreatePinnedToCore(HeatMeterMbus::read_mbus_task_loop,
                        "mbus_task", // name
                        10000,       // stack size (in words)
                        this,        // input params
                        1,           // priority
                        nullptr,     // Handle, not needed
                        0            // core
      );
    }

    esp_err_t HeatMeterMbus::initializeAndEnablePwm(Pwm* pwm)
    {
      esp_err_t configResult = pwm->initialize(32, 18000, 0.85f);
      if (ESP_OK != configResult)
      {
        ESP_LOGE(TAG, "Error initializing PWM: %d", configResult);
        return configResult;
      }
      else
      {
        ESP_LOGI(TAG, "Initialized PWM");
        pwmInitialized = true;
      }

      esp_err_t pwmEnableResult = pwm->enable();
      if (ESP_OK != pwmEnableResult)
      {
        ESP_LOGE(TAG, "Error enabling PWM channel");
      }
      else
      {
        ESP_LOGI(TAG, "Enabled PWM channel");
        pwmEnabled = true;
      }
      return pwmEnableResult;
    }

    void HeatMeterMbus::read_mbus_task_loop(void* params)
    {
      HeatMeterMbus *heatMeterMbus = reinterpret_cast<HeatMeterMbus*>(params);

      while (true)
      {
        const bool shouldReadNow = heatMeterMbus->updateRequested && heatMeterMbus->mbusEnabled;
        if (heatMeterMbus->updateRequested && !heatMeterMbus->mbusEnabled)
        {
          ESP_LOGD(TAG, "Read Mbus requested but Mbus disabled");
        }
        if (shouldReadNow)
        {
          // Let's request data, and wait for its results :-)
          Kamstrup303WA02::MbusMeterData mbus_meter_data;
          bool read_is_successful { heatMeterMbus->kamstrup->read_meter_data(&mbus_meter_data, heatMeterMbus->address) };

          if (read_is_successful) {
            ESP_LOGI(TAG, "Successfully read meter data");

            if (!heatMeterMbus->have_dumped_data_blocks_) {
              heatMeterMbus->dump_data_blocks(&mbus_meter_data);
            }

            for (auto sensor : heatMeterMbus->sensors_) {
              for (auto data_block : *(mbus_meter_data.data_blocks)) {
                if (sensor->is_right_sensor_for_data_block(data_block)) {
                  ESP_LOGI(TAG, "Found matching data block");
                  sensor->transform_and_publish(data_block);
                  break;
                }
              }
            }
          }
          else {
            ESP_LOGE(TAG, "Did not successfully read meter data");
          }
          heatMeterMbus->updateRequested = false;
        }
        else
        {
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
      }
    }

    void HeatMeterMbus::dump_data_blocks(Kamstrup303WA02::MbusMeterData* meter_data) {
      for (auto data_block : *(meter_data->data_blocks)) {
        ESP_LOGI(TAG, "-- Index:\t\t\t%d --", data_block->index);
        ESP_LOGI(TAG, "Function:\t\t\t%d", data_block->function);
        ESP_LOGI(TAG, "Storage number:\t\t%d", data_block->storage_number);
        ESP_LOGI(TAG, "Unit:\t\t\t%d", data_block->unit);
        ESP_LOGI(TAG, "Ten power:\t\t\t%d", data_block->ten_power);
        ESP_LOGI(TAG, "Data length:\t\t%d", data_block->data_length);
        ESP_LOGI(TAG, "-------------------------------");
      }
      this->have_dumped_data_blocks_ = true;
    }

    void HeatMeterMbus::enable_mbus() {
      ESP_LOGI(TAG, "Enabling Mbus");
      pwm.enable();
      mbusEnabled = true;
    }

    void HeatMeterMbus::disable_mbus() {
      ESP_LOGI(TAG, "Disabling Mbus");
      pwm.disable();
      mbusEnabled = false;
      this->have_dumped_data_blocks_ = false;
    }

    void HeatMeterMbus::read_mbus()
    {
      updateRequested = true;
    }

    float HeatMeterMbus::get_setup_priority() const
    {
      // After UART bus
      return setup_priority::BUS - 1.0f;
    }

    void HeatMeterMbus::dump_config()
    {
      ESP_LOGCONFIG(TAG, "HeatMeterMbus sensor");
    }
  } // namespace warmtemetermbus
} // namespace esphome

#endif // UNIT_TEST
