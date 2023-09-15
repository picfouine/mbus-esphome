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

void MbusSensor::transform_and_publish(const DataBlock * const data_block) {
  switch (data_block->data_length) {
    case 2: {
      int16_t *raw_value = reinterpret_cast<int16_t*>(data_block->binary_data);
      float value = static_cast<float>(*raw_value * pow(10, data_block->ten_power));
      this->publish_state(value);
      break;
    }
    case 4: {
      int32_t *raw_value = reinterpret_cast<int32_t*>(data_block->binary_data);
      float value = static_cast<float>(*raw_value * pow(10, data_block->ten_power));
      this->publish_state(value);
      break;
    }
    default:
      ESP_LOGW(TAG, "Unsupported data length");
      break;
  }
}

bool MbusSensor::is_right_sensor_for_data_block(const DataBlock * const data_block) {
  return (this->index_ == data_block->index);
}

} // namespace mbus_controller
} // namespace esphome

#endif // UNIT_TEST
