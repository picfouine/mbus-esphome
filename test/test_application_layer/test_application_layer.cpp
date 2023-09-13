#include <Arduino.h>
#include <test_includes.h>
#include <TestableKamstrup303WA02.h>
#include <test_data_link_layer/TestableDataLinkLayer.h>
#include <DataBlockReader.h>
#include <unity.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector>

using std::vector;
using namespace esphome::warmtemetermbus;

void setUp(void) {}
void tearDown(void) {}

void test_datablockreader_read_data_blocks_from_long_frame_single_not_extended_dif_and_vif(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x04, 0x06, 0x78, 0x56, 0x34, 0x12 // data block: instantaneous, 32 bit integer, Energy in 10^(6-3) Wh (=kWh), value 0x12345678
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 21,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x78, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[3]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_extended_dif_and_vif_one_manufacturer_specific(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x84, 0xB1, 0x20, 0x06, 0x25, 0x0F, 0x00, 0x00, // data block: instantaneous, 32 bit integer, Energy in 10^(6-3) Wh (=kWh), value 0x00000F25; storage number 0b10 = 2, tariff 0b1011 = 0xB
    0x12, 0xFF, 0x07, 0xC4, 0x81 // data block: maximum, 16 bit integer, manufacturer specific, value 0x81C4
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 28,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(2, actual_data_blocks->size());

  // Block 0
  // DIF: 0b1000 0100 DIFE: 0b1011 0001 DIFE: 0b0010 0000
  //  Data length / function: 4, instantaneous
  //  Storage nr: 0b0 0000 0010 = 2
  //  Tariff: 0b1011 = 11
  // VIF: 0b0000 0110
  //  Primary VIF, Energy in Wh, 10^(6 - 3)
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(2, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(11, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x25, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x0F, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x00, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x00, actual_data_block->binary_data[3]);

  // Block 1
  // DIF: 0b0001 0010
  //  Data length / function: 2, maximum
  //  Storage nr: 0
  //  Tariff: 0
  // VIF: 0b1111 1111
  //  Manufacturer specific, extended
  actual_data_block = actual_data_blocks->at(1);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::maximum, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::manufacturer_specific, actual_data_block->unit);
  TEST_ASSERT_EQUAL(1, actual_data_block->index);
  TEST_ASSERT_TRUE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0xC4, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x81, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_minimum_8_bit(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x21, 0x05, 0x12 // data block: minimum, 8 bit integer, Energy in 10^(5-3) Wh (=kWh), value 0x12
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 18,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::minimum, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(2, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(1, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[0]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_during_error_state_24_bit(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x33, 0x05, 0x12, 0x34, 0x56 // data block: during error state, 24 bit integer, Energy in 10^(5-3) Wh (=kWh), value 0x563412
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 20,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::during_error_state, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(2, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(3, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[2]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_energy(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x04, 0x06, 0x78, 0x56, 0x34, 0x12, // data block: instantaneous, 32 bit integer, Energy in 10^(6-3) Wh (=kWh), value 0x12345678
    0x04, 0x0B, 0x78, 0x56, 0x34, 0x12, // data block: instantaneous, 32 bit integer, Energy in 10^(3) J (=kJ), value 0x12345678
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 27,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(2, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::Wh, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x78, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[3]);

  // Block 1
  actual_data_block = actual_data_blocks->at(1);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::J, actual_data_block->unit);
  TEST_ASSERT_EQUAL(1, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x78, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[3]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_volume(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x04, 0x13, 0x78, 0x56, 0x34, 0x12 // data block: instantaneous, 32 bit integer, Volume in 10^(3-6) m3 (=l), value 0x12345678
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 21,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(-3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::cubic_meter, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(4, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x78, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x56, actual_data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[3]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_on_time(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x02, 0x20, 0x34, 0x12, // data block: instantaneous, 16 bit integer, On Time in seconds, value 0x1234
    0x02, 0x21, 0x1E, 0x00, // data block: instantaneous, 16 bit integer, On Time in minutes, value 0x001E
    0x02, 0x22, 0x34, 0x12, // data block: instantaneous, 16 bit integer, On Time in hours, value 0x1234
    0x02, 0x23, 0x23, 0x01  // data block: instantaneous, 16 bit integer, On Time in days, value 0x0123
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 31,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(4, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::seconds, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);

  // Block 1
  actual_data_block = actual_data_blocks->at(1);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::minutes, actual_data_block->unit);
  TEST_ASSERT_EQUAL(1, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x1E, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x00, actual_data_block->binary_data[1]);

  // Block 2
  actual_data_block = actual_data_blocks->at(2);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::hours, actual_data_block->unit);
  TEST_ASSERT_EQUAL(2, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);

    // Block 3
  actual_data_block = actual_data_blocks->at(3);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::days, actual_data_block->unit);
  TEST_ASSERT_EQUAL(3, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x23, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x01, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_power(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x02, 0x2A, 0x34, 0x12, // data block: instantaneous, 16 bit integer, Power in 10^(2-3) W, value 0x1234
    0x02, 0x33, 0x12, 0x00 // data block: instantaneous, 16 bit integer, Power in 10^3 J/h, value 0x0012
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 23,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(2, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(-1, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::W, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);

  // Block 1
  actual_data_block = actual_data_blocks->at(1);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(3, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::J_per_hour, actual_data_block->unit);
  TEST_ASSERT_EQUAL(1, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x00, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_volume_flow(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x02, 0x39, 0x34, 0x12, // data block: instantaneous, 16 bit integer, Volume Flow in 10^(1-6) m3/h, value 0x1234
    0x02, 0x45, 0x10, 0x02, // data block: instantaneous, 16 bit integer, Volume Flow in 10^(5-7) m3/min, value 0x1234
    0x02, 0x4A, 0x34, 0x12 // data block: instantaneous, 16 bit integer, Volume Flow in 10^(2-9) m3/s, value 0x1234
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 27,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(3, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(-5, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::cubic_meter_per_hour, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);

  // Block 1
  actual_data_block = actual_data_blocks->at(1);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(-2, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::cubic_meter_per_minute, actual_data_block->unit);
  TEST_ASSERT_EQUAL(1, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x10, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x02, actual_data_block->binary_data[1]);

  // Block 2
  actual_data_block = actual_data_blocks->at(2);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(-7, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::cubic_meter_per_second, actual_data_block->unit);
  TEST_ASSERT_EQUAL(2, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_flow_temperature(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x02, 0x5B, 0x34, 0x12 // data block: instantaneous, 16 bit integer, Flow Temperature in 10^(3-3) deg C, value 0x1234
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 19,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::degrees_celsius, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_return_temperature(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x02, 0x5D, 0x34, 0x12 // data block: instantaneous, 16 bit integer, Return Temperature in 10^(1-3) deg C, value 0x1234
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 19,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(-2, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::degrees_celsius, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_temperature_difference(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x02, 0x61, 0x34, 0x12 // data block: instantaneous, 16 bit integer, Temperature Difference in 10^(1-3) K, value 0x1234
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 19,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(-2, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::K, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0x34, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x12, actual_data_block->binary_data[1]);
}

void test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_time_point_date(void) {
  // Arrange
  DataBlockReader data_block_reader;
  uint8_t user_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Fixed data header
    0x02, 0x6C, 0xE1, 0x21 // data block: instantaneous, 16 bit integer, Time Point date only,
    // value 01 Jan 2023: 0b YYYY MMMM YYYD DDDD = 0b 0010 0001 1110 0001 = 0x21E1
  };
  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame = {
    .l = 19,
    .c = 0x08,
    .a = 0x0A,
    .ci = 0x72,
    .user_data = user_data
  };

  // Act
  vector<Kamstrup303WA02::DataBlock*>* actual_data_blocks = data_block_reader.read_data_blocks_from_long_frame(&long_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_data_blocks != nullptr);
  TEST_ASSERT_EQUAL(1, actual_data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *actual_data_block = actual_data_blocks->at(0);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Function::instantaneous, actual_data_block->function);
  TEST_ASSERT_EQUAL(0, actual_data_block->storage_number);
  TEST_ASSERT_EQUAL(0, actual_data_block->tariff);
  TEST_ASSERT_EQUAL(0, actual_data_block->ten_power);
  TEST_ASSERT_EQUAL(Kamstrup303WA02::Unit::date, actual_data_block->unit);
  TEST_ASSERT_EQUAL(0, actual_data_block->index);
  TEST_ASSERT_FALSE(actual_data_block->is_manufacturer_specific);
  TEST_ASSERT_EQUAL(2, actual_data_block->data_length);
  TEST_ASSERT_EQUAL(0xE1, actual_data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x21, actual_data_block->binary_data[1]);
}

void test_kamstrup303wa02_read_data(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0xE5 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 1
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task1", // name
                    30000,                       // stack size (in words)
                    &args,                       // input params
                    1,                           // priority
                    nullptr,                     // Handle, not needed
                    0                            // core
  );

  const uint8_t fake_data2[] = { 
    0x68, 0x1B, 0x1B, 0x68, 0x08, 0xB2, 0x72, // start of package
    0x01, 0x23, 0x81, 0x82, 0x2D, 0x2C, 0x40, 0x04, 0xDE, 0x10, 0x00, 0x00, // Fixed Data Header
    0x04, 0x06, 0x25, 0x0F, 0x00, 0x00, // data block 0
    0x04, 0x14, 0x7A, 0xCF, 0x01, 0x00, // data block 1
    0x7E, 0x16 // end of package
  };
  FakeUartInterfaceTaskArgs args2 = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 2,
    .delay_in_ms  = 200,
    .data_to_return = fake_data2,
    .len_of_data_to_return = 33
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task2", // name
                    30000,                       // stack size (in words)
                    &args2,                      // input params
                    1,                           // priority
                    nullptr,                     // Handle, not needed
                    0                            // core
  );
  TestableKamstrup303WA02 kamstrup303wa02(&uart_interface);

  // Act
  Kamstrup303WA02::MbusMeterData meter_data;
  kamstrup303wa02.read_meter_data(&meter_data, 0xB2);

  // Assert
  TEST_ASSERT_NOT_NULL(meter_data.data_blocks);
  TEST_ASSERT_EQUAL(2, meter_data.data_blocks->size());

  // Block 0
  Kamstrup303WA02::DataBlock *data_block = meter_data.data_blocks->at(0);
  TEST_ASSERT_EQUAL(0, data_block->index);
  TEST_ASSERT_EQUAL(4, data_block->data_length);
  TEST_ASSERT_EQUAL(0x25, data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0x0F, data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x00, data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x00, data_block->binary_data[3]);

  // Block 1
  data_block = meter_data.data_blocks->at(1);
  TEST_ASSERT_EQUAL(1, data_block->index);
  TEST_ASSERT_EQUAL(4, data_block->data_length);
  TEST_ASSERT_EQUAL(0x7A, data_block->binary_data[0]);
  TEST_ASSERT_EQUAL(0xCF, data_block->binary_data[1]);
  TEST_ASSERT_EQUAL(0x01, data_block->binary_data[2]);
  TEST_ASSERT_EQUAL(0x00, data_block->binary_data[3]);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_not_extended_dif_and_vif);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_extended_dif_and_vif_one_manufacturer_specific);

  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_minimum_8_bit);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_dif_during_error_state_24_bit);

  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_energy);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_volume);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_on_time);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_power);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_volume_flow);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_flow_temperature);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_return_temperature);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_temperature_difference);
  RUN_TEST(test_datablockreader_read_data_blocks_from_long_frame_single_block_primary_vif_time_point_date);

  RUN_TEST(test_kamstrup303wa02_read_data);

  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
