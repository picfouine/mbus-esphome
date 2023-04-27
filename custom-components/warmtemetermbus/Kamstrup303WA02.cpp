#include "Kamstrup303WA02.h"

namespace esphome {
namespace warmtemetermbus {

bool Kamstrup303WA02::sndNke(uint8_t address) {
  return dataLinkLayer.sndNke(address);
}

bool Kamstrup303WA02::DataLinkLayer::sndNke(uint8_t address) {
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

bool Kamstrup303WA02::DataLinkLayer::trySendShortFrame(uint8_t c, uint8_t a) {
  bool success = false;
  
  bool dataIsReceived = false;
  for (uint8_t transmitAttempt = 0; transmitAttempt < 3 && !dataIsReceived; ++transmitAttempt) {
    sendShortFrame(c, a);
    dataIsReceived = waitForIncomingData();
  }
  success = dataIsReceived;
  return success;
}

void Kamstrup303WA02::DataLinkLayer::sendShortFrame(uint8_t c, uint8_t a) {
  uint8_t data[] = { c, a };
  uint8_t checksum = calculateChecksum(data, 2);
  uint8_t shortFrame[] = { StartByteShortFrame, c, a, checksum, StopByte };
  uartDevice->write_array(shortFrame, 5);
}

bool Kamstrup303WA02::DataLinkLayer::waitForIncomingData() {
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

uint8_t Kamstrup303WA02::DataLinkLayer::calculateChecksum(const uint8_t data[], uint8_t length) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < length; ++i) {
    checksum += data[i];
  }
  return checksum;
}

} //namespace warmtemetermbus
} //namespace esphome