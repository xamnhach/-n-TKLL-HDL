#include "button_arduino.h"

// ================= BUTTON 1 =================
static uint8_t button1Pin = 0;
static int KeyReg0_1 = BUTTON_NORMAL_STATE;
static int KeyReg1_1 = BUTTON_NORMAL_STATE;
static int KeyReg2_1 = BUTTON_NORMAL_STATE;
static int KeyReg3_1 = BUTTON_NORMAL_STATE;
static int TimeOutForKeyPress_1 = 500;
static bool button1_flag = false;
static bool button1_long_pressed = false;

// ================= BUTTON 2 =================
static uint8_t button2Pin = 0;
static int KeyReg0_2 = BUTTON_NORMAL_STATE;
static int KeyReg1_2 = BUTTON_NORMAL_STATE;
static int KeyReg2_2 = BUTTON_NORMAL_STATE;
static int KeyReg3_2 = BUTTON_NORMAL_STATE;
static int TimeOutForKeyPress_2 = 500;
static bool button2_flag = false;
static bool button2_long_pressed = false;

// ================= INIT =================
void buttonInit(uint8_t btn1Pin, uint8_t btn2Pin) {
  button1Pin = btn1Pin;
  button2Pin = btn2Pin;
  
  // Cấu hình pin với pull-up internal
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  
  // Đọc giá trị ban đầu
  KeyReg0_1 = digitalRead(button1Pin);
  KeyReg1_1 = KeyReg0_1;
  KeyReg2_1 = KeyReg0_1;
  KeyReg3_1 = KeyReg0_1;
  
  KeyReg0_2 = digitalRead(button2Pin);
  KeyReg1_2 = KeyReg0_2;
  KeyReg2_2 = KeyReg0_2;
  KeyReg3_2 = KeyReg0_2;
}

// ================= SCAN BUTTON 1 =================
static void scanButton1() {
  KeyReg2_1 = KeyReg1_1;
  KeyReg1_1 = KeyReg0_1;
  KeyReg0_1 = digitalRead(button1Pin);

  if ((KeyReg1_1 == KeyReg0_1) && (KeyReg1_1 == KeyReg2_1)) {
    if (KeyReg2_1 != KeyReg3_1) {
      KeyReg3_1 = KeyReg2_1;

      if (KeyReg3_1 == BUTTON_PRESSED_STATE) {
        TimeOutForKeyPress_1 = 500;
        button1_flag = true;
      }
    } else {
      TimeOutForKeyPress_1--;
      if (TimeOutForKeyPress_1 == 0) {
        TimeOutForKeyPress_1 = 500;
        if (KeyReg3_1 == BUTTON_PRESSED_STATE) {
          button1_flag = true;
          button1_long_pressed = true;
        }
      }
    }
  }
}

// ================= SCAN BUTTON 2 =================
static void scanButton2() {
  KeyReg2_2 = KeyReg1_2;
  KeyReg1_2 = KeyReg0_2;
  KeyReg0_2 = digitalRead(button2Pin);

  // Debug: Kiểm tra giá trị đọc được
  // Serial.print("Btn2 Pin:"); Serial.print(button2Pin);
  // Serial.print(" State:"); Serial.println(KeyReg0_2);

  if ((KeyReg1_2 == KeyReg0_2) && (KeyReg1_2 == KeyReg2_2)) {
    if (KeyReg2_2 != KeyReg3_2) {
      KeyReg3_2 = KeyReg2_2;

      if (KeyReg3_2 == BUTTON_PRESSED_STATE) {
        TimeOutForKeyPress_2 = 500;
        button2_flag = true;
        // Serial.println("Button 2 Pressed detected!");
      } else {
        // Button released - reset long press flag
        button2_long_pressed = false;
      }
    } else {
      TimeOutForKeyPress_2--;
      if (TimeOutForKeyPress_2 == 0) {
        TimeOutForKeyPress_2 = 500;
        if (KeyReg3_2 == BUTTON_PRESSED_STATE) {
          button2_flag = true;
          button2_long_pressed = true;
        }
      }
    }
  }
}

// ================= SCAN ALL BUTTONS =================
void buttonScan() {
  scanButton1();
  scanButton2();
}

// ================= CHECK BUTTON 1 =================
bool isButton1Pressed() {
  if (button1_flag == true) {
    button1_flag = false;
    return true;
  }
  return false;
}

// ================= CHECK BUTTON 2 =================
bool isButton2Pressed() {
  if (button2_flag == true) {
    button2_flag = false;
    return true;
  }
  return false;
}

// ================= CHECK LONG PRESS =================
bool isButton1LongPressed() {
  if (button1_long_pressed == true) {
    button1_long_pressed = false;
    return true;
  }
  return false;
}

bool isButton2LongPressed() {
  if (button2_long_pressed == true) {
    button2_long_pressed = false;
    return true;
  }
  return false;
}

