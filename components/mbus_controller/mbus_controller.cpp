#ifndef UNIT_TEST

#include "mbus_controller.h"

#include <map>
#include <string>

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"
#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp32_arduino_uart_interface.h"
#include "i_mbus_sensor.h"
#include "mbus_reader.h"

using namespace std;

namespace esphome
{
  namespace mbus_controller
  {
    static const char *TAG = "MbusController";

    // correction de map< en std::map<
    
    std::map<MbusReader::Unit, std::string> MbusController::unit_names_ = {
      { MbusReader::Unit::WH, "Wh" },
      { MbusReader::Unit::J, "J" },
      { MbusReader::Unit::CUBIC_METER, "m3" },
      { MbusReader::Unit::KG, "kg" },
      { MbusReader::Unit::SECONDS, "s" },
      { MbusReader::Unit::MINUTES, "min" },
      { MbusReader::Unit::HOURS, "h" },
      { MbusReader::Unit::DAYS, "days" },
      { MbusReader::Unit::W, "W" },
      { MbusReader::Unit::J_PER_HOUR, "J/h" },
      { MbusReader::Unit::CUBIC_METER_PER_HOUR, "m3/h" },
      { MbusReader::Unit::CUBIC_METER_PER_MINUTE, "m3/min" },
      { MbusReader::Unit::CUBIC_METER_PER_SECOND, "m3/s" },
      { MbusReader::Unit::KG_PER_HOUR, "kg/h" },
      { MbusReader::Unit::DEGREES_CELSIUS, "deg c" },
      { MbusReader::Unit::K, "K" },
      { MbusReader::Unit::BAR, "bar" },
      { MbusReader::Unit::DATE, "date" },
      { MbusReader::Unit::TIME_AND_DATE, "time and date" },
      { MbusReader::Unit::MANUFACTURER_SPECIFIC, "manuf. specific" },
      { MbusReader::Unit::DIMENSIONLESS, "-" }
    };

    // correction de map< en std::map<

    std::map<MbusReader::Function, std::string> MbusController::function_names_ = {
      { MbusReader::Function::INSTANTANEOUS, "instantaneous" },
      { MbusReader::Function::MAXIMUM, "maximum" },
      { MbusReader::Function::MINIMUM, "minimum" },
      { MbusReader::Function::DURING_ERROR_STATE, "during error state" }
    };

    MbusController::MbusController(uint8_t address) : address_(address) {
      Esp32ArduinoUartInterface *uart_interface = new Esp32ArduinoUartInterface(this);
      this->kamstrup_ = new MbusReader(uart_interface);
    }

    void MbusController::setup()
    {
      if (xTaskCreatePinnedToCore(MbusController::read_mbus_task_loop,
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

    void MbusController::read_mbus_task_loop(void* params)
    {
      MbusController *mbus_controller = reinterpret_cast<MbusController*>(params);

      while (true)
      {
        bool should_read_now = mbus_controller->update_requested_ && mbus_controller->mbus_enabled_;
        if (mbus_controller->update_requested_ && !mbus_controller->mbus_enabled_)
        {
          ESP_LOGV(TAG, "Read Mbus requested but Mbus disabled");
        }
        if (should_read_now)
        {
          // Let's request data, and wait for its results :-)
          MbusReader::MbusMeterData mbus_meter_data;
          bool read_is_successful { mbus_controller->kamstrup_->read_meter_data(&mbus_meter_data, mbus_controller->address_) };

          if (read_is_successful) {
            ESP_LOGI(TAG, "Successfully read meter data");

            if (!mbus_controller->have_dumped_data_blocks_) {
              mbus_controller->dump_data_blocks(&mbus_meter_data);
            }

            for (auto sensor : mbus_controller->sensors_) {
              for (auto data_block : *(mbus_meter_data.data_blocks)) {
                if (sensor->is_right_sensor_for_data_block(data_block)) {
                  ESP_LOGD(TAG, "Found matching data block");
                  sensor->transform_and_publish(data_block);
                  break;
                }
              }
            }
          }
          else {
            ESP_LOGW(TAG, "Did not successfully read meter data");
          }
          mbus_controller->update_requested_ = false;
        }
        else
        {
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
      }
    }

    void MbusController::dump_data_blocks(MbusReader::MbusMeterData* meter_data) {
      for (auto data_block : *(meter_data->data_blocks)) {
        ESP_LOGI(TAG, "-- Index:\t\t\t%d --", data_block->index);
        ESP_LOGI(TAG, "Function:\t\t\t%s", MbusController::function_names_[data_block->function].c_str());
        ESP_LOGI(TAG, "Storage number:\t\t%d", data_block->storage_number);
        ESP_LOGI(TAG, "Unit:\t\t\t%s", MbusController::unit_names_[data_block->unit].c_str());
        ESP_LOGI(TAG, "Ten power:\t\t\t%d", data_block->ten_power);
        ESP_LOGI(TAG, "Data length:\t\t%d", data_block->data_length);
        ESP_LOGI(TAG, "-------------------------------");
      }
      this->have_dumped_data_blocks_ = true;
    }

    void MbusController::enable_mbus() {
      ESP_LOGI(TAG, "Enabling Mbus");
      this->mbus_enabled_ = true;
    }

    void MbusController::disable_mbus() {
      ESP_LOGI(TAG, "Disabling Mbus");
      this->mbus_enabled_ = false;
      this->have_dumped_data_blocks_ = false;
    }

    void MbusController::read_mbus()
    {
      this->update_requested_ = true;
    }

    float MbusController::get_setup_priority() const
    {
      // After UART bus
      return setup_priority::AFTER_CONNECTION;
    }

    void MbusController::dump_config()
    {
      ESP_LOGCONFIG(TAG, "MbusController config:");
      if (this->is_failed()) {
        ESP_LOGE(TAG, "MbusController not able to function properly");
      } else {
        for (auto sensor : this->sensors_) {
          LOG_SENSOR("  ", "MbusSensor", sensor);
          ESP_LOGCONFIG(TAG, "    index: %d", sensor->get_index());
        }
      }
    }
  } // namespace mbus_controller
} // namespace esphome

#endif // UNIT_TEST


