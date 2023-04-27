#pragma once

#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace warmtemetermbus {

class Kamstrup303WA02 {
private:
  class DataLinkLayer {
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
    bool nextReqUd2Fcb { false };
    bool nextSndUdFcb { false };
    uart::UARTDevice* uartDevice;

    // Methods
    bool trySendShortFrame(uint8_t c, uint8_t a);
    void sendShortFrame(uint8_t c, uint8_t a);
    bool waitForIncomingData();
    uint8_t calculateChecksum(const uint8_t data[], uint8_t length);

  public:
    bool sndNke(uint8_t address);
  };

public:
  Kamstrup303WA02(uart::UARTDevice* _uartDevice) : dataLinkLayer(_uartDevice) {}

  bool sndNke(uint8_t address);
private:
  DataLinkLayer dataLinkLayer;
};

} //namespace warmtemetermbus
} //namespace esphome