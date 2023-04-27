#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace warmtemetermbus {

#define HEATMETERMBUS_METER_SENSOR(name) \
 protected: \
  sensor::Sensor *name##_sensor_{nullptr}; \
\
 public: \
  void set_##name##_sensor(sensor::Sensor *(name)) { this->name##_sensor_ = name; }

const uint8_t CFieldBitDirection = 6;
const uint8_t CFieldBitFCB = 5;
const uint8_t CFieldBitFCV = 4;
const uint8_t CFieldFunctionSndNke = 0x0;
const uint8_t CFieldFunctionSndUd = 0x3;
const uint8_t CFieldFunctionReqUd2 = 0xB;
const uint8_t CFieldFunctionReqUd1 = 0xA;
const uint8_t CFieldFunctionRspUd = 0x8;
const uint8_t StartByteSingleCharacter = 0xE5;
const uint8_t StartByteShortFrame = 0x10;
const uint8_t StartByteControlAndLongFrame = 0x68;
const uint8_t StopByte = 0x16;

class HeatMeterMbus : public PollingComponent, public uart::UARTDevice {
  public:  
    HeatMeterMbus() : PollingComponent(10000) {}
    
    HEATMETERMBUS_METER_SENSOR(test_temperature)

//    HEATMETERMBUS_METER_BINARYSENSOR(info_v1_air)

    void setup() override;
    void update() override;
    void dump_config() override;
    float get_setup_priority() const override;

  private:
    uint8_t calculateChecksum(const uint8_t data[], uint8_t length);
};

} //namespace warmtemetermbus
} //namespace esphome