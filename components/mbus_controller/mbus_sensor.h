#ifndef MBUSSENSOR_H_
#define MBUSSENSOR_H_

#include "esphome/components/sensor/sensor.h"

#include "i_mbus_sensor.h"
#include "mbus_reader.h"

namespace esphome {
namespace mbus_controller {

using DataBlock = MbusReader::DataBlock;

class MbusSensor : public sensor::Sensor, public IMbusSensor {
 public:
  MbusSensor(uint8_t index) : index_(index) {}

  virtual void transform_and_publish(const DataBlock * const data_block);
  virtual bool is_right_sensor_for_data_block(const DataBlock * const data_block);
  uint8_t get_index() const;
  
 protected:
  uint8_t index_;
};

} //namespace mbus_controller
} //namespace esphome

#endif // MBUSSENSOR_H_