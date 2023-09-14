#ifndef UNIT_TEST

#include "heat_meter_mbus.h"

#include "esphome/core/log.h"
#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp32_arduino_uart_interface.h"
#include "i_mbus_sensor.h"
#include "kamstrup_303wa02.h"
#include "pwm.h"

using namespace std; 

namespace esphome
{
  namespace warmtemetermbus
  {
    static const char *TAG = "heatmetermbus.sensor";

    bool pwm_initialized { false };
    bool pwm_enabled { false };

    HeatMeterMbus::HeatMeterMbus() {
      Esp32ArduinoUartInterface *uart_interface = new Esp32ArduinoUartInterface(this);
      this->kamstrup = new Kamstrup303WA02(uart_interface);
    }

    void HeatMeterMbus::setup()
    {
      ESP_LOGI(TAG, "setup()");
      if (ESP_OK != this->initialize_and_enable_pwm(&pwm))
      {
        ESP_LOGE(TAG, "Error initializing and enabling PWM");
        mark_failed();
        return;
      }

      if (xTaskCreatePinnedToCore(HeatMeterMbus::read_mbus_task_loop,
                                  "mbus_task", // name
                                  10000,       // stack size (in words)
                                  this,        // input params
                                  1,           // priority
                                  nullptr,     // Handle, not needed
                                  0            // core
        ) != pdPASS) {
          ESP_LOGE(TAG, "Could not start mbus_task");
          mark_failed();
          return;
      }
    }

    esp_err_t HeatMeterMbus::initialize_and_enable_pwm(Pwm* pwm)
    {
      esp_err_t config_result = pwm->initialize(32, 18000, 0.85f);
      if (ESP_OK != config_result)
      {
        ESP_LOGE(TAG, "Error initializing PWM: %d", config_result);
        return config_result;
      }
      else
      {
        ESP_LOGI(TAG, "Initialized PWM");
        pwm_initialized = true;
      }

      esp_err_t pwm_enable_result = pwm->enable();
      if (ESP_OK != pwm_enable_result)
      {
        ESP_LOGE(TAG, "Error enabling PWM channel");
      }
      else
      {
        ESP_LOGI(TAG, "Enabled PWM channel");
        pwm_enabled = true;
      }
      return pwm_enable_result;
    }

    void HeatMeterMbus::read_mbus_task_loop(void* params)
    {
      HeatMeterMbus *heat_meter_mbus = reinterpret_cast<HeatMeterMbus*>(params);

      while (true)
      {
        bool should_read_now = heat_meter_mbus->update_requested && heat_meter_mbus->mbus_enabled;
        if (heat_meter_mbus->update_requested && !heat_meter_mbus->mbus_enabled)
        {
          ESP_LOGD(TAG, "Read Mbus requested but Mbus disabled");
        }
        if (should_read_now)
        {
          // Let's request data, and wait for its results :-)
          Kamstrup303WA02::MbusMeterData mbus_meter_data;
          bool read_is_successful { heat_meter_mbus->kamstrup->read_meter_data(&mbus_meter_data, heat_meter_mbus->address) };

          if (read_is_successful) {
            ESP_LOGI(TAG, "Successfully read meter data");

            if (!heat_meter_mbus->have_dumped_data_blocks_) {
              heat_meter_mbus->dump_data_blocks(&mbus_meter_data);
            }

            for (auto sensor : heat_meter_mbus->sensors_) {
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
          heat_meter_mbus->update_requested = false;
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
      this->pwm.enable();
      this->mbus_enabled = true;
    }

    void HeatMeterMbus::disable_mbus() {
      ESP_LOGI(TAG, "Disabling Mbus");
      this->pwm.disable();
      this->mbus_enabled = false;
      this->have_dumped_data_blocks_ = false;
    }

    void HeatMeterMbus::read_mbus()
    {
      this->update_requested = true;
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
