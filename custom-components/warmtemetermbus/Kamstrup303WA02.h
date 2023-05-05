#pragma once

#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace warmtemetermbus {
 
class Kamstrup303WA02 {
public:
  typedef struct VariableDataHeader {
      uint32_t identNr;
      uint16_t manufacturer;
      uint8_t version;
      uint8_t medium;
      uint8_t accessNr;
      uint8_t status;
      uint16_t signature;
  } VariableDataHeader;

  typedef struct VariableDataRecord {
    unsigned function : 2;
    unsigned dataType : 4;
    unsigned long long storageNumber : 41;
    unsigned long tariff : 20;
    unsigned subUnit : 10;
    unsigned unitAndMultiplier : 7;
    uint8_t data[8];
    uint8_t dataLength;
  } VariableDataRecord;

  typedef enum EnergyUnit {
      Wh,
      J
  } EnergyUnit;

  typedef enum PowerUnit {
      W,
      JperH
  } PowerUnit;

  typedef enum DurationUnit {
    seconds = 0x0,
    minutes = 0x1,
    hours = 0x2,
    days = 0x3
  } DurationUnit;

  typedef enum VolumeFlowUnit {
      m3PerHour,
      m3PerMinute,
      m3PerSecond
  } VolumeFlowUnit;
      
  typedef struct EnergyValue {
      uint32_t value;
      EnergyUnit unit;
      int8_t tenPower;
  } EnergyValue;

  typedef struct VolumeValue {
      uint32_t value;
      // No unit required; always m3
      int8_t tenPower;
  } VolumeValue;

  typedef struct EnergyE8E9Value {
      uint32_t value;
      // No unit required; always cubic m * deg C
      int8_t tenPower;
  } EnergyE8E9Value;

  typedef struct DurationValue {
      uint32_t value;
      DurationUnit unit;
      // No multiplier!
  } DurationValue;

  typedef struct TemperatureValue {
      int16_t value;
      // No unit required; always deg C
      int8_t tenPower;
  } TemperatureValue;

  typedef struct PowerValue {
      uint16_t value;
      PowerUnit unit;
      int8_t tenPower;
  } PowerValue;

  typedef struct VolumeFlowValue {
      uint16_t value;
      VolumeFlowUnit unit;
      int8_t tenPower;
  } VolumeFlowValue;

  typedef struct InfoBitsValue {
      unsigned noVoltageSupply : 1;
      unsigned lowBatteryLevel : 1;
      unsigned notUsed1 : 1;
      unsigned t1AboveRangeOrDisconnected : 1;
      unsigned t2AboveRangeOrDisconnected : 1;
      unsigned t1BelowRangeOrShirtCircuited : 1;
      unsigned t2BelowRangeOrShirtCircuited : 1;
      unsigned invalidTempDifference : 1;
      unsigned v1Air : 1;
      unsigned v1WrongFlowDirection : 1;
      unsigned notUsed2 : 1;
      unsigned v1GreaterThanQsMoreThanHour : 1;
  } InfoBitsValue;

  typedef struct DateValue {
      uint8_t day;
      uint8_t month;
      uint8_t year;
  } DateValue;

  typedef struct MeterData {
    EnergyValue heatEnergyE1;
    VolumeValue volumeV1;
    EnergyE8E9Value energyE8;
    EnergyE8E9Value energyE9;
    DurationValue operatingHours;
    DurationValue errorHourCounter;
    TemperatureValue t1Actual;
    TemperatureValue t2Actual;
    TemperatureValue diffT1T2;
    PowerValue powerE1OverE3Actual;
    PowerValue powerMaxMonth;
    VolumeFlowValue flowV1Actual;
    VolumeFlowValue flowV1MaxMonth;
    InfoBitsValue infoBits;
    EnergyValue heatEnergyE1Old;
    VolumeValue volumeV1Old;
    EnergyE8E9Value energyE8Old;
    EnergyE8E9Value energyE9Old;
    PowerValue powerMaxYear;
    VolumeFlowValue flowV1MaxYear;
    DateValue dateTimeLogged;
  } MeterData;

private:
  class DataLinkLayer {
  public:
    typedef struct TelegramData {
      uint8_t c;
      uint8_t a;
      uint8_t ci;
      uint8_t data[246];
      uint8_t dataLength;
    } TelegramData;

  public:
    DataLinkLayer(uart::UARTDevice* _uartDevice) : uartDevice(_uartDevice) {}

  private:
    // Constants
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

    // Fields
    bool slaveInitialized {false};
    bool nextReqUd2Fcb { false };
    bool nextSndUdFcb { false };
    uart::UARTDevice* uartDevice;

    // Methods
    bool readNextByte(uint8_t * const receivedByte);
    bool sndNke(const uint8_t address);
    bool trySendShortFrame(const uint8_t c, const uint8_t a);
    void sendShortFrame(const uint8_t c, const uint8_t a);
    bool waitForIncomingData();
    void flushRxBuffer();
    uint8_t calculateChecksum(const uint8_t data[], const uint8_t length);

  public:
    bool reqUd2(const uint8_t address, TelegramData * const dataBuffer);
  };

public:
  Kamstrup303WA02(uart::UARTDevice* _uartDevice) : dataLinkLayer(_uartDevice) {}

  bool readData(MeterData * const data);

private:
  DataLinkLayer dataLinkLayer;

  void readDataRecord(VariableDataRecord * const dataRecord, const DataLinkLayer::TelegramData * const userData, uint16_t * const startOfDataRecordIdx);
  void copyDataToTargetBuffer(VariableDataRecord* dataRecord, void* targetBuffer);
};

} //namespace warmtemetermbus
} //namespace esphome