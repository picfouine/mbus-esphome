#ifndef IMBUSSENSOR_H_
#define IMBUSSENSOR_H_

#include "kamstrup_303wa02.h"

namespace esphome {
namespace mbus_controller {

using DataBlock = Kamstrup303WA02::DataBlock;

class IMbusSensor {
 public:
  virtual void transform_and_publish(const DataBlock * const data_block) = 0;
  virtual bool is_right_sensor_for_data_block(const DataBlock * const data_block) = 0;
};

} //namespace mbus_controller
} //namespace esphome

#endif // IMBUSSENSOR_H_