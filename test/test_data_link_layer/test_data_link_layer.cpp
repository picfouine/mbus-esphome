#include <Arduino.h>
#include <test_includes.h>
#include <TestableDataLinkLayer.h>
#include <unity.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace esphome::warmtemetermbus;

void setUp(void) {}
void tearDown(void) {}

void test_data_link_layer_calculate_checksum(void) {
  FakeUartInterface fakeUartInterface;
  TestableDataLinkLayer dataLinkLayer(&fakeUartInterface);
  
  uint8_t *data = new uint8_t[2] { 0, 0 };
  TEST_ASSERT_EQUAL(0, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;

  data = new uint8_t[2] { 0, 1 };
  TEST_ASSERT_EQUAL(1, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 1, 0 };
  TEST_ASSERT_EQUAL(1, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  
  data = new uint8_t[2] { 1, 1 };
  TEST_ASSERT_EQUAL(2, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  
  data = new uint8_t[2] { 0, 255 };
  TEST_ASSERT_EQUAL(255, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 1, 255 };
  TEST_ASSERT_EQUAL(0, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 255, 1 };
  TEST_ASSERT_EQUAL(0, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;
  data = new uint8_t[2] { 255, 2 };
  TEST_ASSERT_EQUAL(1, dataLinkLayer.call_calculate_checksum(data, 2));
  delete[] data;

  data = new uint8_t[5] { 8, 128, 200, 0, 12 };
  TEST_ASSERT_EQUAL(92, dataLinkLayer.call_calculate_checksum(data, 5));
  delete[] data;
}

void test_data_link_layer_calculate_checksum_with_long_frame() {
  FakeUartInterface fakeUartInterface;
  TestableDataLinkLayer dataLinkLayer(&fakeUartInterface);

  Kamstrup303WA02::DataLinkLayer::LongFrame long_frame {
    .l = 6,
    .c = 0x08,
    .a = 0x54,
    .ci = 0x72,
    .check_sum = 0x00,
    .user_data = new uint8_t[3] { 0x66, 0x77, 0x88 }
  };
  const uint8_t actual_checksum = dataLinkLayer.call_calculate_checksum(&long_frame);
  delete[] long_frame.user_data;
  TEST_ASSERT_EQUAL(0x33, actual_checksum);
}

void test_data_link_layer_try_send_short_frame_reply_to_first_request(void) {
  // Arrange

  // After the request is sent, the fake slave responds after the minimum time.
  // What can be checked: 
  //  - return value (should be true)
  //  - sent data (should contain a full Short Frame: start, C, A, check sum, stop)
  //
  // For this to work, the following scenario is required:
  //  1. In a separate task, the fake uart interface is configured.
  //     It should do a delay of just over 11 bits (11 bits * 1000 ms/s / 2400 bit/s = 4.58ms)
  //     After that delay it should prepare some return data (single byte should be okay)
  //     and set the return value of available() to > 0.
  //  2. In the test function, the testable DataLinkLayer can be created with the fake uart interface.
  //     Then it can use call_try_send_short_frame(), and remember the return value.
  //     Then it waits for the call to finish. After that, it can assert everything is okay.

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
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);
  
  // Act
  const uint8_t c = 0x40;
  const uint8_t a = 0x54;
  const bool actual_return_value = dataLinkLayer.call_try_send_short_frame(c, a);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x40
  // A:         0x54
  // Check sum: 0x94
  // Stop:      0x16
  TEST_ASSERT_EQUAL(1, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[0];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0x54, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0x94, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);
}

void test_data_link_layer_try_send_short_frame_reply_to_second_request(void) {
  // Arrange

  // After the request is sent, the fake slave responds after the minimum time.
  // What can be checked: 
  //  - return value (should be true)
  //  - sent data (should contain a full Short Frame: start, C, A, check sum, stop)
  //
  // For this to work, the following scenario is required:
  //  1. In a separate task, the fake uart interface is configured.
  //     It should do a delay of just over 11 bits (11 bits * 1000 ms/s / 2400 bit/s = 4.58ms)
  //     After that delay it should prepare some return data (single byte should be okay)
  //     and set the return value of available() to > 0.
  //  2. In the test function, the testable DataLinkLayer can be created with the fake uart interface.
  //     Then it can use call_try_send_short_frame(), and remember the return value.
  //     Then it waits for the call to finish. After that, it can assert everything is okay.

  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0xE5 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface, 
    .respond_to_nth_write = 2, 
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 1
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t c = 0x40;
  const uint8_t a = 0x54;
  const bool actual_return_value = dataLinkLayer.call_try_send_short_frame(c, a);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x40
  // A:         0x54
  // Check sum: 0x94
  // Stop:      0x16
  TEST_ASSERT_EQUAL(2, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[1];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0x54, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0x94, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);
}

void test_data_link_layer_snd_nke_correct_response(void) {
  // What can be tested?
  //  - C field in short frame:
  //    FCB / FCV fields in C field: both should be 0
  //    Function field in C field: should be 0
  //    Bit 6 in C field: should be 1
  //    In other words: C should be 0x40
  //  - A field in short frame
  //  - Rest of sent short frame
  //  - Return value: only true if the correct response is received
  //  - Next-to-use value for FCB field for req_ud2 should be 1

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
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  const bool actual_return_value = dataLinkLayer.snd_nke(a);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
  
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x40
  // A:         0xB2
  // Check sum: 0xF2
  // Stop:      0x16
  TEST_ASSERT_EQUAL(1, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[0];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0xB2, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0xF2, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);

  // Test the next-to-use FCB bit for req_ud2
  TEST_ASSERT_TRUE(dataLinkLayer.get_next_req_ud2_fcb());
}

void test_data_link_layer_snd_nke_incorrect_response(void) {
  // What can be tested?
  //  - C field in short frame:
  //    FCB / FCV fields in C field: both should be 0
  //    Function field in C field: should be 0
  //    Bit 6 in C field: should be 1
  //    In other words: C should be 0x40
  //  - A field in short frame
  //  - Rest of sent short frame
  //  - Return value: false

  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0x00 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 1
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    20000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  const bool actual_return_value = dataLinkLayer.snd_nke(a);

  // Assert
  TEST_ASSERT_FALSE(actual_return_value);
  
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x40
  // A:         0xB2
  // Check sum: 0xF2
  // Stop:      0x16
  TEST_ASSERT_EQUAL(1, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[0];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0xB2, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0xF2, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);
}

// Variation points of req_ud2:
//  - Response comes or not
//  - Response contains correct or incorrect A field
//  - Response contains correct or incorrect C function field
//  - Response contains correct or incorrect check sum
//  - Response contains correct or incorrect START / STOP bytes
//  - Response contains identical or different L fields
// Other things to check:
//  - Correct A field is sent
//  - Correct C function field is sent
//  - Correct user data is put in LONG_FRAME
void test_data_link_layer_req_ud2_check_sending_correct_data(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0x68, 0x06, 0x06, 0x68, 0x08, 0xB2, 0x72, 0x03, 0x02, 0x01, 0x32, 0x16 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 12
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x7B
  // A:         0xB2
  // Check sum: 0x2D
  // Stop:      0x16
  TEST_ASSERT_EQUAL(1, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[0];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x7B, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0xB2, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0x2D, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);
}

void test_data_link_layer_req_ud2_check_snd_nke_to_init(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0xE5, 0x68, 0x06, 0x06, 0x68, 0x08, 0xB2, 0x72, 0x03, 0x02, 0x01, 0x32, 0x16 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 13
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task1", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  const uint8_t fake_data2[] = { 0x68, 0x06, 0x06, 0x68, 0x08, 0xB2, 0x72, 0x03, 0x02, 0x01, 0x32, 0x16 };
  FakeUartInterfaceTaskArgs args2 = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 2,
    .delay_in_ms  = 200,
    .data_to_return = fake_data2,
    .len_of_data_to_return = 12
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task2", // name
                    30000,                      // stack size (in words)
                    &args2,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(false);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  // Check the sent data: should be a short frame!
  // Start:     0x10
  // C:         0x7B
  // A:         0xB2
  // Check sum: 0x2D
  // Stop:      0x16
  TEST_ASSERT_EQUAL(2, uart_interface.written_arrays_.size());
  FakeUartInterface::WrittenArray actual_written_array = uart_interface.written_arrays_[0];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x40, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0xB2, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0xF2, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);

  actual_written_array = uart_interface.written_arrays_[1];
  TEST_ASSERT_EQUAL(5, actual_written_array.len);
  TEST_ASSERT_EQUAL(0x10, actual_written_array.data[0]);
  TEST_ASSERT_EQUAL(0x7B, actual_written_array.data[1]);
  TEST_ASSERT_EQUAL(0xB2, actual_written_array.data[2]);
  TEST_ASSERT_EQUAL(0x2D, actual_written_array.data[3]);
  TEST_ASSERT_EQUAL(0x16, actual_written_array.data[4]);
}

void test_data_link_layer_req_ud2_correct_response(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0x68, 0x06, 0x06, 0x68, 0x08, 0xB2, 0x72, 0x03, 0x02, 0x01, 0x32, 0x16 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 12
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  TEST_ASSERT_TRUE(actual_return_value);
  
  // Check fields and user data in telegram
  TEST_ASSERT_EQUAL(6, response_frame.l);
  TEST_ASSERT_EQUAL(0x08, response_frame.c);
  TEST_ASSERT_EQUAL(0xB2, response_frame.a);
  TEST_ASSERT_EQUAL(0x72, response_frame.ci);
  TEST_ASSERT_EQUAL(0x03, response_frame.user_data[0]);
  TEST_ASSERT_EQUAL(0x02, response_frame.user_data[1]);
  TEST_ASSERT_EQUAL(0x01, response_frame.user_data[2]);
}

void test_data_link_layer_req_ud2_no_response(void) {
  // Arrange
  FakeUartInterface uart_interface;
  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  TEST_ASSERT_FALSE(actual_return_value);
}

void test_data_link_layer_req_ud2_incorrect_a_field(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data_with_incorrect_a_field[] = { 0x68, 0x06, 0x06, 0x68, 0x08, 0x10, 0x72, 0x03, 0x02, 0x01, 0x90, 0x16 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data_with_incorrect_a_field,
    .len_of_data_to_return = 12
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  TEST_ASSERT_FALSE(actual_return_value);
}

void test_data_link_layer_req_ud2_incorrect_function(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data_with_incorrect_function[] = { 0x68, 0x06, 0x06, 0x68, 0x00, 0xB2, 0x72, 0x03, 0x02, 0x01, 0x2A, 0x16 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data_with_incorrect_function,
    .len_of_data_to_return = 12
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  TEST_ASSERT_FALSE(actual_return_value);
}

void test_data_link_layer_req_ud2_incorrect_check_sum(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0x68, 0x06, 0x06, 0x68, 0x08, 0xB2, 0x72, 0x03, 0x02, 0x01, 0xAA, 0x16 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 12
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  TEST_ASSERT_FALSE(actual_return_value);
}

void test_data_link_layer_req_ud2_different_l_fields(void) {
  // Arrange
  FakeUartInterface uart_interface;
  const uint8_t fake_data[] = { 0x68, 0x06, 0x05, 0x68, 0x08, 0xB2, 0x72, 0x03, 0x02, 0x01, 0x32, 0x16 };
  FakeUartInterfaceTaskArgs args = { 
    .uart_interface = &uart_interface,
    .respond_to_nth_write = 1,
    .delay_in_ms  = 5,
    .data_to_return = fake_data,
    .len_of_data_to_return = 12
  };
  xTaskCreatePinnedToCore(fake_uart_interface_task,
                    "fake_uart_interface_task", // name
                    30000,                      // stack size (in words)
                    &args,                      // input params
                    1,                          // priority
                    nullptr,                    // Handle, not needed
                    0                           // core
  );

  TestableDataLinkLayer dataLinkLayer(&uart_interface);
  dataLinkLayer.set_meter_is_initialized(true);

  // Act
  const uint8_t a = 0xB2;
  Kamstrup303WA02::DataLinkLayer::LongFrame response_frame;
  const bool actual_return_value = dataLinkLayer.req_ud2(a, &response_frame);

  // Assert
  TEST_ASSERT_FALSE(actual_return_value);
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_data_link_layer_calculate_checksum);
  RUN_TEST(test_data_link_layer_calculate_checksum_with_long_frame);
  RUN_TEST(test_data_link_layer_try_send_short_frame_reply_to_first_request);
  RUN_TEST(test_data_link_layer_try_send_short_frame_reply_to_second_request);
  RUN_TEST(test_data_link_layer_snd_nke_correct_response);
  RUN_TEST(test_data_link_layer_snd_nke_incorrect_response);
  RUN_TEST(test_data_link_layer_req_ud2_check_sending_correct_data);
  RUN_TEST(test_data_link_layer_req_ud2_check_snd_nke_to_init);
  RUN_TEST(test_data_link_layer_req_ud2_correct_response);
  RUN_TEST(test_data_link_layer_req_ud2_no_response);
  RUN_TEST(test_data_link_layer_req_ud2_incorrect_a_field);
  RUN_TEST(test_data_link_layer_req_ud2_incorrect_function);
  RUN_TEST(test_data_link_layer_req_ud2_incorrect_check_sum);
  RUN_TEST(test_data_link_layer_req_ud2_different_l_fields);
  return UNITY_END();
}

void setup() {
  // Wait 2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}
