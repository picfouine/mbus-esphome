#ifndef DATABLOCKREADER_H_
#define DATABLOCKREADER_H_

#ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

#include <vector>
#include "Kamstrup303WA02.h"

namespace esphome {
namespace warmtemetermbus {

class DataBlockReader {
  public:
    std::vector<Kamstrup303WA02::DataBlock*>* read_data_blocks_from_long_frame(Kamstrup303WA02::DataLinkLayer::LongFrame* long_frame);

  protected:
     Kamstrup303WA02::DataLinkLayer::LongFrame* long_frame_;
     uint8_t current_position_in_user_data_;

  private:
    static const uint8_t DIF_BIT_EXTENDED = 7;
    static const uint8_t DIF_BIT_LSB_STORAGE_NUMBER = 6;
    static const uint8_t DIF_BIT_FUNCTION_FIELD_LOW_BIT = 4;
    static const uint8_t DIF_BITS_DATA_FIELD = 0x0F;
    static const uint8_t DIFE_BITS_STORAGE_NUMBER = 0x0F;
    static const uint8_t DIFE_BITS_TARIFF = 0x30;
    static const uint8_t DIFE_BIT_TARIFF_LOW_BIT = 4;
    static const uint8_t VIF_BITS_UNIT_AND_MULTIPLIER = 0x7F;
    static const uint8_t VIF_BIT_EXTENDED = 7;

    Kamstrup303WA02::DataBlock* read_next_data_block();
    void read_dif_into_block(Kamstrup303WA02::DataBlock* data_block);
    void read_vif_into_block(Kamstrup303WA02::DataBlock* data_block);
    void read_data_into_block(Kamstrup303WA02::DataBlock* data_block);
    uint8_t read_next_byte();
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // DATABLOCKREADER_H_