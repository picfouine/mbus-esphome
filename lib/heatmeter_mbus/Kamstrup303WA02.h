#ifndef KAMSTRUP303WA02_H_
#define KAMSTRUP303WA02_H_

#ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#include "esphome/components/uart/uart.h"
#endif // UNIT_TEST

#include <vector>
#include "UartInterface.h"

namespace esphome {
namespace warmtemetermbus {

class Kamstrup303WA02 {
  public:
    typedef enum Function : uint8_t {
      instantaneous = 0,
      maximum = 1,
      minimum = 2,
      during_error_state = 3
    } Function;

    enum class Unit {
      Wh,
      J,
      cubic_meter,
      kg,
      seconds,
      minutes,
      hours,
      days,
      W,
      J_per_hour,
      cubic_meter_per_hour,
      cubic_meter_per_minute,
      cubic_meter_per_second,
      kg_per_hour,
      degrees_celsius,
      K,
      bar,
      date,
      time_and_date,
      manufacturer_specific,
      dimensionless
    };

    typedef struct DataBlock {
      Function function;
      uint64_t storage_number;
      uint32_t tariff;
      size_t data_length;
      uint8_t* binary_data;
      uint8_t index;
      int8_t ten_power;
      Unit unit;
      bool is_manufacturer_specific;
    } DataBlock;

    typedef struct MbusMeterData {
      public:
        MbusMeterData() {}
        ~MbusMeterData();
        std::vector<DataBlock*>* data_blocks { nullptr };

      private:
        void deallocate_data_blocks();
    } MbusMeterData;

    static const uint8_t FIXED_DATA_HEADER_SIZE = 12;

    class DataLinkLayer {
      public:
        typedef struct LongFrame {
          uint8_t l;
          uint8_t c;
          uint8_t a;
          uint8_t ci;
          uint8_t check_sum;
          uint8_t* user_data;
        } LongFrame;
        
        DataLinkLayer(UartInterface* uart_interface) : uart_interface_(uart_interface) {}
        bool req_ud2(const uint8_t address, LongFrame* response_frame);
        bool snd_nke(const uint8_t address);

      protected:
        bool try_send_short_frame(const uint8_t c, const uint8_t a);
        void flush_rx_buffer();
        void send_short_frame(const uint8_t c, const uint8_t a);
        bool wait_for_incoming_data();
        uint8_t calculate_checksum(const LongFrame* long_frame) const;
        uint8_t calculate_checksum(const uint8_t* data, size_t length) const;
        
        UartInterface* uart_interface_;
        bool next_req_ud2_fcb_ { true };
        bool meter_is_initialized_ { false };

      private:
        const uint8_t START_BYTE_SINGLE_CHARACTER = 0xE5;
        const uint8_t START_BYTE_SHORT_FRAME = 0x10;
        const uint8_t START_BYTE_CONTROL_AND_LONG_FRAME = 0x68;
        const uint8_t STOP_BYTE = 0x16;
        const uint8_t C_FIELD_BIT_DIRECTION = 6;
        const uint8_t C_FIELD_BIT_FCB = 5;
        const uint8_t C_FIELD_BIT_FCV = 4;
        const uint8_t C_FIELD_FUNCTION_SND_NKE = 0x0;
        const uint8_t C_FIELD_FUNCTION_REQ_UD2 = 0xB;

        bool initialize_meter(const uint8_t address);
        bool parse_long_frame_response(LongFrame* longFrame);
        bool read_next_byte(uint8_t* received_byte);
    };

    Kamstrup303WA02(UartInterface* uart_interface);

    bool read_meter_data(MbusMeterData* meter_data, uint8_t address);

  protected:
    DataLinkLayer* data_link_layer_;
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // KAMSTRUP303WA02_H_