#ifndef UNIT_TEST

#include "mbus_sensor.h"

#include <math.h>

#include "esphome/core/datatypes.h"
#include "esphome/core/log.h"

#include "mbus_reader.h"

using namespace std; 

namespace esphome
{
namespace mbus_controller
{

using DataBlock = MbusReader::DataBlock;

static const char *TAG = "MbusSensor";

uint8_t MbusSensor::get_index() const {
  return this->index_;
}

void MbusSensor::transform_and_publish(const DataBlock * const data_block) {
  if (data_block == nullptr) {
    ESP_LOGW(TAG, "transform_and_publish: data_block is null");
    return;
  }

  // Priorité : utiliser la valeur décodée et stockée dans data_block->value
  float final_value = data_block->value;

  // Vérifier que la valeur est finie (non NaN, non inf)
  bool value_ok = std::isfinite(final_value);

  // Fallback : reconstruire depuis binary_data si nécessaire
  if (!value_ok) {
    if (data_block->data_length == 0 || data_block->binary_data == nullptr) {
      ESP_LOGW(TAG, "MbusSensor: no valid data to publish for index %d", data_block->index);
      return;
    }

    // Reconstruire raw LSB-first (sûr jusqu'à 4 octets)
    uint32_t raw = 0;
    uint8_t max_bytes = data_block->data_length > 4 ? 4 : data_block->data_length;
    for (uint8_t i = 0; i < max_bytes; ++i) {
      raw |= (uint32_t)data_block->binary_data[i] << (8 * i);
    }

    final_value = static_cast<float>(raw * powf(10.0f, data_block->ten_power));
    ESP_LOGD(TAG, "Sensor '%s' fallback raw value: %u, publish value: %f", this->get_name().c_str(), raw, final_value);
  }

  // Publier la valeur finale
  this->publish_state(final_value);
  ESP_LOGI(TAG, "MbusSensor publish index=%d name='%s' unit=%d value=%.6f (ten=%d)",
           data_block->index, this->get_name().c_str(), data_block->unit, final_value, data_block->ten_power);
}


bool MbusSensor::is_right_sensor_for_data_block(const DataBlock * const data_block) {
  return (this->index_ == data_block->index);
}

} // namespace mbus_controller
} // namespace esphome

#endif // UNIT_TEST
