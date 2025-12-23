#ifndef BUTTON_ARDUINO_H
#define BUTTON_ARDUINO_H

#include <Arduino.h>

// Định nghĩa trạng thái button
#define BUTTON_NORMAL_STATE HIGH  // Button không nhấn (pull-up)
#define BUTTON_PRESSED_STATE LOW  // Button nhấn (kết nối GND)

// Khởi tạo button
void buttonInit(uint8_t button1Pin, uint8_t button2Pin);

// Đọc input button (gọi trong loop)
void buttonScan();

// Kiểm tra button 1 nhấn (trả về 1 lần, tự reset)
bool isButton1Pressed();

// Kiểm tra button 2 nhấn (trả về 1 lần, tự reset)
bool isButton2Pressed();

// Kiểm tra button 1 giữ lâu (chưa implement, có thể thêm sau)
bool isButton1LongPressed();

// Kiểm tra button 2 giữ lâu (chưa implement, có thể thêm sau)
bool isButton2LongPressed();

#endif

