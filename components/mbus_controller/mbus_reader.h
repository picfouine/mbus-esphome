#ifndef KAMSTRUP303WA02_H_
#define KAMSTRUP303WA02_H_

#include <vector>

#ifndef UNIT_TEST
#include "esphome/components/uart/uart.h"
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

#include "uart_interface.h"

namespace esphome {
namespace mbus_controller {

class MbusReader {
 public:
  typedef enum Function : uint8_t {
    INSTANTANEOUS = 0,
    MAXIMUM = 1,
    MINIMUM = 2,
    DURING_ERROR_STATE = 3
  } Function;

  enum class Unit {
    WH,
    J,
    CUBIC_METER,
    KG,
    SECONDS,
    MINUTES,
    HOURS,
    DAYS,
    W,
    J_PER_HOUR,
    CUBIC_METER_PER_HOUR,
    CUBIC_METER_PER_MINUTE,
    CUBIC_METER_PER_SECOND,
    KG_PER_HOUR,
    DEGREES_CELSIUS,
    K,
    BAR,
    DATE,
    TIME_AND_DATE,
    MANUFACTURER_SPECIFIC,
    DIMENSIONLESS
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

  typedef enum CiMeterToMasterCode : uint8_t {
    GENERAL_APPLICATION_ERRORS = 0,
    ALARM_STATUS = 1,
    VARIABLE_DATA_RESPOND = 2,
    FIXED_DATA_RESPOND = 3
  } CiMeterToMasterCode;

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
    
    DataLinkLayer(IUartInterface* uart_interface) : uart_interface_(uart_interface) {}

    bool req_ud2(const uint8_t address, LongFrame* response_frame);
    bool snd_nke(const uint8_t address);

   protected:
    bool try_send_short_frame(const uint8_t c, const uint8_t a);
    uint8_t calculate_checksum(const LongFrame* long_frame) const;
    uint8_t calculate_checksum(const uint8_t* data, size_t length) const;
    
    IUartInterface* uart_interface_;
    bool next_req_ud2_fcb_ { true };
    bool meter_is_initialized_ { false };

   private:
    static const uint8_t START_BYTE_SINGLE_CHARACTER = 0xE5;
    static const uint8_t START_BYTE_SHORT_FRAME = 0x10;
    static const uint8_t START_BYTE_CONTROL_AND_LONG_FRAME = 0x68;
    static const uint8_t STOP_BYTE = 0x16;
    static const uint8_t C_FIELD_BIT_DIRECTION = 6;
    static const uint8_t C_FIELD_BIT_FCB = 5;
    static const uint8_t C_FIELD_BIT_FCV = 4;
    static const uint8_t C_FIELD_FUNCTION_SND_NKE = 0x0;
    static const uint8_t C_FIELD_FUNCTION_REQ_UD2 = 0xB;

    bool initialize_meter(const uint8_t address);
    bool parse_long_frame_response(LongFrame* longFrame);
    void send_short_frame(const uint8_t c, const uint8_t a);
    bool wait_for_incoming_data();
    void flush_rx_buffer();
    bool read_next_byte(uint8_t* received_byte);
  };

  static const uint8_t FIXED_DATA_HEADER_SIZE = 12;

  MbusReader(IUartInterface* uart_interface);

  bool read_meter_data(MbusMeterData* meter_data, const uint8_t address);

 protected:
  DataLinkLayer* data_link_layer_;
};

} //namespace mbus_controller
} //namespace esphome

#endif // KAMSTRUP303WA02_H_