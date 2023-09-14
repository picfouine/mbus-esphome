#ifndef MBUSSENSOR_H_
#define MBUSSENSOR_H_

#include "esphome/components/sensor/sensor.h"

#include "i_mbus_sensor.h"
#include "kamstrup_303wa02.h"

namespace esphome {
namespace warmtemetermbus {

class MbusSensor : public sensor::Sensor, public IMbusSensor {
  public:
    MbusSensor(uint8_t index) : index_(index) {}

    virtual void transform_and_publish(const Kamstrup303WA02::DataBlock * const data_block);
    virtual bool is_right_sensor_for_data_block(const Kamstrup303WA02::DataBlock * const data_block);

  protected:
    uint8_t index_;
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // MBUSSENSOR_H_