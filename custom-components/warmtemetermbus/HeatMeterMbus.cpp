#include "esphome/core/log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
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

    bool vlaggetje = false;

    static const char *TAG = "heatmetermbus.sensor";

    void HeatMeterMbus::setup()
    {
      xTaskCreatePinnedToCore(
        HeatMeterMbus::task_loop,
        "mbus_task", // name
        10000,       // stack size (in words)
        this,        // input params
        1,           // priority
        nullptr,     // Handle, not needed
        0            // core
      );

      xTaskCreatePinnedToCore(
        HeatMeterMbus::adc_task_loop,
        "adc_task", // name
        4096,       // stack size (in words)
        this,       // input params
        2,          // priority
        nullptr,    // handle, not needed
        0           // core
      );
    }

    void HeatMeterMbus::adc_task_loop(void* params)
    {
      HeatMeterMbus *heatMeterMbus = reinterpret_cast<HeatMeterMbus*>(params);

      adc1_config_width(ADC_WIDTH_BIT_12);
      adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
      esp_adc_cal_characteristics_t adc1Characteristics;
      esp_adc_cal_value_t calValType = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1Characteristics);

      while (true) {
        const uint32_t rawAdcValue = adc1_get_raw(ADC1_CHANNEL_0);
        const uint32_t voltageInMv = esp_adc_cal_raw_to_voltage(rawAdcValue, &adc1Characteristics);
        const float voltageInV = static_cast<float>(voltageInMv) / 1000.0f;
        const float busVoltage = voltageInMv * 19.0f / 1000.0f;
        const uint32_t busVoltageInMv = static_cast<float>(voltageInMv) * 19.0f;
        heatMeterMbus->bus_voltage_sensor_->publish_state(busVoltage);
        ESP_LOGI(TAG, "-----------------------------------------");
        switch (calValType) {
          case ESP_ADC_CAL_VAL_EFUSE_VREF:
            ESP_LOGI(TAG, "Calibration type: eFuse");
            break;
          case ESP_ADC_CAL_VAL_EFUSE_TP:
            ESP_LOGI(TAG, "Calibration type: Two Point");
            break;
          case ESP_ADC_CAL_VAL_DEFAULT_VREF:
            ESP_LOGI(TAG, "Calibration type: default reference voltage");
            break;
          default:
            break;
        }
        ESP_LOGI(TAG, "Used Vref: %d", adc1Characteristics.vref);
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "RAW ADC value: %d", rawAdcValue);
        ESP_LOGI(TAG, "ADC Voltage: %.1fV (%dmV)", voltageInV, voltageInMv);
        ESP_LOGI(TAG, "Bus Voltage: %.1fV (%dmV)", busVoltage, busVoltageInMv);
        ESP_LOGI(TAG, "-----------------------------------------");

        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    }

    void HeatMeterMbus::task_loop(void* params)
    {
      HeatMeterMbus *heatMeterMbus = reinterpret_cast<HeatMeterMbus*>(params);
      Kamstrup303WA02 &kamstrup(heatMeterMbus->kamstrup);

      while (true)
      {
        const bool shouldReadNow = vlaggetje;
        if (shouldReadNow)
        {
          // Let's request data, and wait for its results :-)
          Kamstrup303WA02::MeterData meterData;
          ESP_LOGI(TAG, "About to readData");
          bool readSuccessful {kamstrup.readData(&meterData)};

          if (readSuccessful)
          {
            ESP_LOGI(TAG, "Successfully read meter data");

            // Heat energy E1
            // Source unit can be Wh or J
            // TODO: Check source unit. For now, assume Wh
            // Use tenPower - 3 to convert from Wh to kWh
            const float heatEnergyE1Value {static_cast<float>(pow(10, meterData.heatEnergyE1.tenPower - 3) * meterData.heatEnergyE1.value)};
            heatMeterMbus->heat_energy_e1_sensor_->publish_state(heatEnergyE1Value);

            // Volume V1
            // Source unit is always m3
            // Use tenPower + 3 to convert to liters
            const float volumeV1Value {static_cast<float>(pow(10, meterData.volumeV1.tenPower + 3) * meterData.volumeV1.value)};
            heatMeterMbus->volume_v1_sensor_->publish_state(volumeV1Value);

            // Energy E8 Inlet
            // Source unit is always m3 * deg Celcius
            const float energyE8InletValue {static_cast<float>(pow(10, meterData.energyE8.tenPower) * meterData.energyE8.value)};
            heatMeterMbus->energy_e8_inlet_sensor_->publish_state(energyE8InletValue);

            // Energy E9 Outlet
            // Source unit is always m3 * deg Celcius
            const float energyE9OutletValue {static_cast<float>(pow(10, meterData.energyE9.tenPower) * meterData.energyE9.value)};
            heatMeterMbus->energy_e9_outlet_sensor_->publish_state(energyE9OutletValue);
            
            // Operating Hours
            // Source unit can be seconds, minutes, hours or days
            // TODO: Check source unit. For now, assume hours, convert to days
            // No ten power
            const float operatingHoursValueInDays {meterData.operatingHours.value / 24.0f};
            heatMeterMbus->operating_hours_sensor_->publish_state(operatingHoursValueInDays);

            // Error Hour Counter
            // Source unit can be seconds, minutes, hours or days
            // TODO: Check source unit. For now, assume hours, convert to days
            // No ten power
            const float errorHourCounterValueInDays {meterData.errorHourCounter.value / 24.0f};
            heatMeterMbus->error_hour_counter_sensor_->publish_state(errorHourCounterValueInDays);

            // T1 actual
            // Source unit is always degrees Celsius
            const float t1ActualValue {static_cast<float>(pow(10, meterData.t1Actual.tenPower) * meterData.t1Actual.value)};
            heatMeterMbus->t1_actual_sensor_->publish_state(t1ActualValue);
            
            // T2 actual
            // Source unit is always degrees Celsius
            const float t2ActualValue {static_cast<float>(pow(10, meterData.t2Actual.tenPower) * meterData.t2Actual.value)};
            heatMeterMbus->t2_actual_sensor_->publish_state(t2ActualValue);

            // T1 - T2
            // Source unit is always degrees Celcius
            const float t1MinusT2Value {static_cast<float>(pow(10, meterData.diffT1T2.tenPower) * meterData.diffT1T2.value)};
            heatMeterMbus->t1_minus_t2_sensor_->publish_state(t1MinusT2Value);

            // Power E1 / E3
            // Source unit can be W or J/h
            // TODO: Check source unit. For now, assume W
            const float powerE1OverE3Value {static_cast<float>(pow(10, meterData.powerE1OverE3Actual.tenPower) * meterData.powerE1OverE3Actual.value)};
            heatMeterMbus->power_e1_over_e3_sensor_->publish_state(powerE1OverE3Value);

            // Power Max Month
            // Source unit can be W or J/h
            // TODO: Check source unit. For now, assume W
            const float powerMaxMonthValue {static_cast<float>(pow(10, meterData.powerMaxMonth.tenPower) * meterData.powerMaxMonth.value)};
            heatMeterMbus->power_max_month_sensor_->publish_state(powerMaxMonthValue);

            // Flow V1 Actual
            // Source unit can be m3/s, m3/m or m3/h
            // TODO: Check source unit. For now, assume m3/h.
            // Use tenPower + 3 to convert to l/h
            const float flowV1ActualValue {static_cast<float>(pow(10, meterData.flowV1Actual.tenPower + 3) * meterData.flowV1Actual.value)};
            heatMeterMbus->flow_v1_actual_sensor_->publish_state(flowV1ActualValue);

            // Flow V1 Max Month
            // Source unit can be m3/s, m3/m or m3/h
            // TODO: Check source unit. For now, assume m3/h.
            // Use tenPower + 3 to convert to l/h
            const float flowV1MaxMonthValue {static_cast<float>(pow(10, meterData.flowV1MaxMonth.tenPower + 3) * meterData.flowV1MaxMonth.value)};
            heatMeterMbus->flow_v1_max_month_sensor_->publish_state(flowV1MaxMonthValue);

            // Info bits
            // No Voltage Supply
            const bool infoNoVoltageSupplyValue {1 == meterData.infoBits.noVoltageSupply};
            heatMeterMbus->info_no_voltage_supply_binary_sensor_->publish_state(infoNoVoltageSupplyValue);
            // T1 Above Measuring Range or Disconnected
            const bool infoT1AboveMeasRangeOrDisconValue {1 == meterData.infoBits.t1AboveRangeOrDisconnected};
            heatMeterMbus->info_t1_above_range_or_disconnected_binary_sensor_->publish_state(infoT1AboveMeasRangeOrDisconValue);
            // T2 Above Measuring Range or Disconnected
            const bool infoT2AboveMeasRangeOrDisconValue {1 == meterData.infoBits.t2AboveRangeOrDisconnected};
            heatMeterMbus->info_t2_above_range_or_disconnected_binary_sensor_->publish_state(infoT2AboveMeasRangeOrDisconValue);
            // T1 Below Measuring Range or Short-circuited
            const bool infoT1BelowMeasRangeOrShortedValue {1 == meterData.infoBits.t1BelowRangeOrShirtCircuited};
            heatMeterMbus->info_t1_below_range_or_shorted_binary_sensor_->publish_state(infoT1BelowMeasRangeOrShortedValue);
            // T2 Below Measuring Range or Short-circuited
            const bool infoT2BelowMeasRangeOrShortedValue {1 == meterData.infoBits.t2BelowRangeOrShirtCircuited};
            heatMeterMbus->info_t2_below_range_or_shorted_binary_sensor_->publish_state(infoT2BelowMeasRangeOrShortedValue);
            // Invalid Temperature Difference (T1 - T2)
            const bool infoInvalidTempDifferenceValue {1 == meterData.infoBits.invalidTempDifference};
            heatMeterMbus->info_invalid_temp_difference_binary_sensor_->publish_state(infoInvalidTempDifferenceValue);
            // V1 Air
            const bool infoV1AirValue {1 == meterData.infoBits.v1Air};
            heatMeterMbus->info_v1_air_binary_sensor_->publish_state(infoV1AirValue);
            // V1 Wrong Flow Direction
            const bool infoV1WrongFlowDirectionValue {1 == meterData.infoBits.v1WrongFlowDirection};
            heatMeterMbus->info_v1_wrong_flow_direction_binary_sensor_->publish_state(infoV1WrongFlowDirectionValue);
            // V1 > Qs For More Than An Hour
            const bool infoV1GreaterThanQsForMoreThanHourValue {1 == meterData.infoBits.v1WrongFlowDirection};
            heatMeterMbus->info_v1_greater_than_qs_more_than_hour_binary_sensor_->publish_state(infoV1GreaterThanQsForMoreThanHourValue);

            // Heat Energy E1 Old (storage nr != 0 in DIF)
            // Source unit can be Wh or J
            // TODO: Check source unit. For now, assume Wh
            // Use tenPower - 3 to convert from Wh to kWh
            const float heatEnergyE1OldValue {static_cast<float>(pow(10, meterData.heatEnergyE1Old.tenPower - 3) * meterData.heatEnergyE1Old.value)};
            heatMeterMbus->heat_energy_e1_old_sensor_->publish_state(heatEnergyE1OldValue);

            // Volume V1 Old (storage nr != 0 in DIF)
            // Source unit is always m3
            // Use tenPower + 3 to convert to liters
            const float volumeV1OldValue {static_cast<float>(pow(10, meterData.volumeV1Old.tenPower + 3) * meterData.volumeV1Old.value)};
            heatMeterMbus->volume_v1_old_sensor_->publish_state(volumeV1OldValue);

            // Energy E8 Inlet Old (storage nr != 0 in DIF)
            // Source unit is always m3 * deg Celcius
            const float energyE8InletOldValue {static_cast<float>(pow(10, meterData.energyE8Old.tenPower) * meterData.energyE8Old.value)};
            heatMeterMbus->energy_e8_inlet_old_sensor_->publish_state(energyE8InletOldValue);

            // Energy E9 Outlet Old (storage nr != 0 in DIF)
            // Source unit is always m3 * deg Celcius
            const float energyE9OutletOldValue {static_cast<float>(pow(10, meterData.energyE9Old.tenPower) * meterData.energyE9Old.value)};
            heatMeterMbus->energy_e9_outlet_old_sensor_->publish_state(energyE9OutletOldValue);
            
            // Power Max Year Old (storage nr != 0 in DIF)
            // Source unit can be W or J/h
            // TODO: Check source unit. For now, assume W
            const float powerMaxYearOldValue {static_cast<float>(pow(10, meterData.powerMaxYear.tenPower) * meterData.powerMaxYear.value)};
            heatMeterMbus->power_max_year_old_sensor_->publish_state(powerMaxYearOldValue);

            // Flow V1 Max Year Old (storage nr != 0 in DIF)
            // Source unit can be m3/s, m3/m or m3/h
            // TODO: Check source unit. For now, assume m3/h.
            // Use tenPower + 3 to convert to l/h
            const float flowV1MaxYearOldValue {static_cast<float>(pow(10, meterData.flowV1MaxYear.tenPower + 3) * meterData.flowV1MaxYear.value)};
            heatMeterMbus->flow_v1_max_year_old_sensor_->publish_state(flowV1MaxYearOldValue);

            // Log Year
            heatMeterMbus->log_year_sensor_->publish_state(meterData.dateTimeLogged.year);

            // Log Month
            heatMeterMbus->log_month_sensor_->publish_state(meterData.dateTimeLogged.month);

            // Log Day
            heatMeterMbus->log_day_sensor_->publish_state(meterData.dateTimeLogged.day);
          }
          else
          {
            ESP_LOGE(TAG, "Did not successfully read meter data");
          }
          vlaggetje = false;
        }
        else
        {
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
      }
    }

    void HeatMeterMbus::update()
    {
      vlaggetje = true;
    }

    float HeatMeterMbus::get_setup_priority() const
    {
      // After UART bus
      return setup_priority::BUS - 1.0f;
    }

    void HeatMeterMbus::dump_config()
    {
      ESP_LOGCONFIG(TAG, "HeatMeterMbus sensor");
      LOG_SENSOR("  ", "T1 Actual", this->t1_actual_sensor_);
      LOG_SENSOR("  ", "Heat Energy E1", this->heat_energy_e1_sensor_);
      LOG_SENSOR("  ", "Volume V1", this->volume_v1_sensor_);
      LOG_SENSOR("  ", "Energy E8 Inlet", this->energy_e8_inlet_sensor_);
      LOG_SENSOR("  ", "Energy E9 Outlet", this->energy_e9_outlet_sensor_);
      LOG_SENSOR("  ", "Operating Hours", this->operating_hours_sensor_);
      LOG_SENSOR("  ", "Error Hour Counter", this->error_hour_counter_sensor_);
      LOG_SENSOR("  ", "T1 Actual", this->t1_actual_sensor_);
      LOG_SENSOR("  ", "T2 Actual", this->t2_actual_sensor_);
      LOG_SENSOR("  ", "T1 - T2", this->t1_minus_t2_sensor_);
      LOG_SENSOR("  ", "Power E1 / E3", this->power_e1_over_e3_sensor_);
      LOG_SENSOR("  ", "Power Max Month", this->power_max_month_sensor_);
      LOG_SENSOR("  ", "Flow V1 Actual", this->flow_v1_actual_sensor_);
      LOG_SENSOR("  ", "Flow V1 Max Month", this->flow_v1_max_month_sensor_);
      LOG_SENSOR("  ", "Heat Energy E1 Old", this->heat_energy_e1_old_sensor_);
      LOG_SENSOR("  ", "Volume V1 Old", this->volume_v1_old_sensor_);
      LOG_SENSOR("  ", "Energy E8 Inlet Old", this->energy_e8_inlet_old_sensor_);
      LOG_SENSOR("  ", "Energy E9 Outlet Old", this->energy_e9_outlet_old_sensor_);
      LOG_SENSOR("  ", "Power Max Year Old", this->power_max_year_old_sensor_);
      LOG_SENSOR("  ", "Flow V1 Max Year Old", this->flow_v1_max_year_old_sensor_);
      LOG_SENSOR("  ", "Log Year", this->log_year_sensor_);
      LOG_SENSOR("  ", "Log Month", this->log_month_sensor_);
      LOG_SENSOR("  ", "Log Day", this->log_day_sensor_);
      LOG_SENSOR("  ", "Bus Voltage", this->bus_voltage_sensor_);

      LOG_BINARY_SENSOR("  ", "No Voltage Supply", this->info_no_voltage_supply_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T1 Above Measuring Range or Disconnected", this->info_t1_above_range_or_disconnected_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T2 Above Measuring Range or Disconnected", this->info_t2_above_range_or_disconnected_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T1 Below Measuring Range or Short-circuited", this->info_t1_below_range_or_shorted_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T2 Below Measuring Range or Short-circuited", this->info_t2_below_range_or_shorted_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "Invalid Temperature Difference (T1 - T2)", this->info_invalid_temp_difference_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "V1 Air", this->info_v1_air_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "V1 Wrong Flow Direction", this->info_v1_wrong_flow_direction_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "V1 > Qs For More Than An Hour", this->info_v1_greater_than_qs_more_than_hour_binary_sensor_);
    }

  } // namespace warmtemetermbus
} // namespace esphome