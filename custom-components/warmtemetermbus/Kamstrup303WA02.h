#pragma once

#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace warmtemetermbus {

class Kamstrup303WA02 {
public:
  Kamstrup303WA02(uart::UARTDevice* _uartDevice) : uartDevice(_uartDevice) {}

  bool sndNke(uint8_t address) {
    bool success = false;
    // Short Frame, expect 0xE5
    uint8_t c = (1 << CFieldBitDirection) | CFieldFunctionSndNke;
    bool dataIsReceived = trySendShortFrame(c, address);
    if (dataIsReceived) {
      uint8_t receivedByte {0};
      uartDevice->read_byte(&receivedByte);
      success = StartByteSingleCharacter == receivedByte;
    }
    nextReqUd2Fcb = true;
    nextSndUdFcb = true;
    return success;
  }

private:
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

  bool nextReqUd2Fcb { false };
  bool nextSndUdFcb { false };

  uart::UARTDevice* uartDevice;

  bool trySendShortFrame(uint8_t c, uint8_t a) {
    bool success = false;
    
    bool dataIsReceived = false;
    for (uint8_t transmitAttempt = 0; transmitAttempt < 3 && !dataIsReceived; ++transmitAttempt) {
      sendShortFrame(c, a);
      dataIsReceived = waitForIncomingData();
    }
    success = dataIsReceived;
    return success;
  }

  void sendShortFrame(uint8_t c, uint8_t a) {
    uint8_t data[] = { c, a };
    uint8_t checksum = calculateChecksum(data, 2);
    uint8_t shortFrame[] = { StartByteShortFrame, c, a, checksum, StopByte };
    uartDevice->write_array(shortFrame, 5);
  }

  bool waitForIncomingData() {
    bool dataReceived = false;
    // 330 bits + 50ms = 330 * 1000 / 2400 + 50 ms = 187,5 ms
    delay(137);
    for (uint8_t i = 0; i < 50; ++i) {
      if (uartDevice->available()) {
        dataReceived = true;
        break;
      }
      delay(1);
    }
    return dataReceived;
  }
  
  uint8_t calculateChecksum(const uint8_t data[], uint8_t length) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; ++i) {
      checksum += data[i];
    }
    return checksum;
  }
};

} //namespace warmtemetermbus
} //namespace esphome