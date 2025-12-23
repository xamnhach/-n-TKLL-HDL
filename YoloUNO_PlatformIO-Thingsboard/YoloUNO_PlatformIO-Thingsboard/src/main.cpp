#include <Arduino.h>
#include <Wire.h>
#include <Arduino_MQTT_Client.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include "button_arduino.h"

#define BLYNK_AUTH_TOKEN "fCBekQ8fO6KROLTGqcjG4JZ_hiVV5kR_"
#define BLYNK_TEMPLATE_ID "TMPL60EoNCy7n"
#define BLYNK_TEMPLATE_NAME "khóa cửa thông minh"

// Blynk Virtual Pins (thay đổi theo template của bạn)
#define VPIN_DOOR_STATUS  V0  // Trạng thái cửa (0=đóng, 1=mở)
#define VPIN_DOOR_CONTROL V1  // Điều khiển cửa từ app (0=đóng, 1=mở)
#define VPIN_PASSWORD_STATUS V2  // Trạng thái mật khẩu (0=sai, 1=đúng)
#define VPIN_LOCKOUT_STATUS V3  // Trạng thái lockout (0=không, 1=có)
#define VPIN_FAIL_COUNT V4      // Số lần nhập sai
#define VPIN_CHANGE_PASSWORD V5  // Switch để vào chế độ đổi mật khẩu (0=off, 1=on)

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

constexpr char WIFI_SSID[] = "iPhone";
constexpr char WIFI_PASSWORD[] = "1234567890";
// ================= PIN =================
#define SDA_PIN 11
#define SCL_PIN 12

#define TTP229_SCL 43
#define TTP229_SDO 44

#define SERVO_PIN 38
#define LED_PIN   48

// Button pin - A3 (GPIO4) để toggle servo
#define BUTTON1_PIN 4   // A3 = GPIO4 (Button toggle servo)

// ================= LCD =================
LiquidCrystal_I2C lcd(0x21, 16, 2);
Servo doorServo;

// ================= FSM =================
enum State {
  STATE_INPUT,
  STATE_CHECK,
  STATE_WRONG,
  STATE_WAIT_BUTTON,  // Chờ bấm nút sau khi nhập đúng mật khẩu
  STATE_UNLOCK,       // Cửa đã mở, đếm 3s rồi đóng
  STATE_LOCKOUT,
  STATE_CHANGE_PASSWORD_OLD,  // Nhập mật khẩu cũ để đổi
  STATE_CHANGE_PASSWORD_NEW,  // Nhập mật khẩu mới
  STATE_CHANGE_PASSWORD_CONFIRM  // Xác nhận mật khẩu mới
};

State currentState = STATE_INPUT;

// ================= PASSWORD =================
String password = "1234";
String inputBuffer = "";
String newPasswordBuffer = "";
String confirmPasswordBuffer = "";

int failCount = 0;
const int MAX_FAIL = 3;

unsigned long stateTimer = 0;
unsigned long lockoutTime = 10000;  // 10s ban đầu

// ================= SERVO CONTROL =================
bool servoIsOpen = false;  // Trạng thái servo (false = đóng, true = mở)
const unsigned long WAIT_BUTTON_TIMEOUT = 10000;  // 10s chờ bấm nút
const unsigned long DOOR_OPEN_DURATION = 3000;    // 3s cửa mở

// ================= BUTTON DOUBLE CLICK =================
unsigned long lastButtonPressTime = 0;
const unsigned long DOUBLE_CLICK_TIMEOUT = 500;  // 500ms để phát hiện double click
bool waitingForDoubleClick = false;

// Phím thoát khỏi chế độ đổi mật khẩu (phím 15 hoặc 16 trên TTP229)
const uint8_t EXIT_CHANGE_PASSWORD_KEY = 15;

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);

// ================= TTP229 =================
uint8_t lastKey = 0;

uint8_t readTTP229() {
  uint8_t key = 0;
  for (uint8_t i = 1; i <= 16; i++) {
    digitalWrite(TTP229_SCL, LOW);
    delayMicroseconds(250);
    if (digitalRead(TTP229_SDO) == 0) key = i;
    digitalWrite(TTP229_SCL, HIGH);
    delayMicroseconds(250);
  }
  return key;
}

// ================= LCD HELPERS =================
void lcdEnterPassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password");
  lcd.setCursor(0, 1);
}

void lcdShowStars() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  for (int i = 0; i < inputBuffer.length(); i++) lcd.print("*");
}

void lcdChangePasswordOld() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Change Password");
  lcd.setCursor(0, 1);
  lcd.print("Old Pass:");
}

void lcdChangePasswordNew() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Change Password");
  lcd.setCursor(0, 1);
  lcd.print("New Pass:");
}

void lcdChangePasswordConfirm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Change Password");
  lcd.setCursor(0, 1);
  lcd.print("Confirm:");
}

// ================= BLYNK CALLBACKS =================
// Hàm callback khi nhận lệnh điều khiển cửa từ Blynk app
BLYNK_WRITE(VPIN_DOOR_CONTROL) {
  int value = param.asInt();
  
  if (value == 1 && currentState == STATE_INPUT) {
    // Mở cửa từ Blynk app
    doorServo.write(90);
    servoIsOpen = true;
    lcd.clear();
    lcd.print("Door Open");
    lcd.setCursor(0, 1);
    lcd.print("Blynk Control");
    stateTimer = millis();
    currentState = STATE_UNLOCK;
    
    // Gửi trạng thái lên Blynk
    Blynk.virtualWrite(VPIN_DOOR_STATUS, 1);
  } else if (value == 0) {
    // Đóng cửa từ Blynk app
    doorServo.write(0);
    servoIsOpen = false;
    Blynk.virtualWrite(VPIN_DOOR_STATUS, 0);
  }
}

// Hàm callback khi nhận lệnh đổi mật khẩu từ Blynk app
BLYNK_WRITE(VPIN_CHANGE_PASSWORD) {
  int value = param.asInt();
  
  // Chỉ cho phép vào chế độ đổi mật khẩu khi ở STATE_INPUT và chưa nhập số
  if (value == 1 && currentState == STATE_INPUT && inputBuffer.length() == 0) {
    // Vào chế độ đổi mật khẩu
    inputBuffer = "";
    newPasswordBuffer = "";
    confirmPasswordBuffer = "";
    lcdChangePasswordOld();
    currentState = STATE_CHANGE_PASSWORD_OLD;
    
    // Reset switch về 0 sau khi đã vào chế độ
    Blynk.virtualWrite(VPIN_CHANGE_PASSWORD, 0);
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TTP229_SCL, OUTPUT);
  pinMode(TTP229_SDO, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Khởi tạo button (chỉ dùng button 1, button 2 không dùng)
  buttonInit(BUTTON1_PIN, BUTTON1_PIN);  // Pass cùng pin cho button2 vì không dùng

  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();

  // 2. Hiển thị thông báo ngay lập tức để biết màn hình còn sống
  lcd.setCursor(0, 0);
  lcd.print("System Start...");
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi");
  delay(1000); // Dừng 1 chút để kịp nhìn
  
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);  // khóa cửa

  // Kết nối WiFi và Blynk
  Serial.println("\n=== WiFi Connection ===");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("Password: ");
  Serial.println(WIFI_PASSWORD ? "***" : "(empty)");
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 60) {  // Tăng timeout lên 30s
    delay(500);
    Serial.print(".");
    
    // Hiển thị trên LCD
    if (wifiTimeout % 4 == 0) {
      lcd.setCursor(0, 1);
      lcd.print("WiFi: ");
      lcd.print(wifiTimeout / 2);
      lcd.print("s    ");
    }
    
    wifiTimeout++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    lcd.clear();
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print("IP: ");
    String ip = WiFi.localIP().toString();
    lcd.print(ip.substring(0, 13));  // Hiển thị IP (tối đa 13 ký tự)
    delay(2000);
    
    // Kết nối Blynk
    Serial.println("\n=== Blynk Connection ===");
    Serial.print("Auth Token: ");
    Serial.println(BLYNK_AUTH_TOKEN);
    Serial.println("Connecting to Blynk...");
    
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    int blynkTimeout = 0;
    while (!Blynk.connected() && blynkTimeout < 20) {
      Blynk.run();
      delay(500);
      Serial.print(".");
      blynkTimeout++;
    }
    
    if (Blynk.connected()) {
      Serial.println("\nBlynk connected!");
      
      // Gửi trạng thái ban đầu
      Blynk.virtualWrite(VPIN_DOOR_STATUS, 0);
      Blynk.virtualWrite(VPIN_PASSWORD_STATUS, 0);
      Blynk.virtualWrite(VPIN_LOCKOUT_STATUS, 0);
      Blynk.virtualWrite(VPIN_FAIL_COUNT, 0);
      Blynk.virtualWrite(VPIN_CHANGE_PASSWORD, 0);
    } else {
      Serial.println("\nBlynk connection failed!");
      Serial.println("System will continue without Blynk");
    }
  } else {
    Serial.println("\nWiFi connection failed!");
    Serial.print("Status code: ");
    Serial.println(WiFi.status());
    
    lcd.clear();
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check SSID/PWD");
    delay(2000);
    
    Serial.println("System will continue without WiFi/Blynk");
  }

  lcdEnterPassword();
}

// ================= LOOP =================
void loop() {
  // Chạy Blynk (nếu đã kết nối WiFi)
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  
  // Scan button input
  buttonScan();

  // Kiểm tra double click button A3 để vào chế độ đổi mật khẩu (chỉ khi ở STATE_INPUT)
  if (currentState == STATE_INPUT && inputBuffer.length() == 0) {
    if (isButton1Pressed()) {
      unsigned long currentTime = millis();
      
      if (waitingForDoubleClick && (currentTime - lastButtonPressTime) < DOUBLE_CLICK_TIMEOUT) {
        // Double click detected!
        inputBuffer = "";
        newPasswordBuffer = "";
        confirmPasswordBuffer = "";
        lcdChangePasswordOld();
        currentState = STATE_CHANGE_PASSWORD_OLD;
        waitingForDoubleClick = false;
      } else {
        // First click, wait for second click
        waitingForDoubleClick = true;
        lastButtonPressTime = currentTime;
      }
    }
    
    // Reset double click detection if timeout
    if (waitingForDoubleClick && (millis() - lastButtonPressTime) >= DOUBLE_CLICK_TIMEOUT) {
      waitingForDoubleClick = false;
    }
  } else {
    // Reset double click detection if not in INPUT state or already typing
    waitingForDoubleClick = false;
  }

  uint8_t key = readTTP229();

  // ========= INPUT =========
  if (currentState == STATE_INPUT) {
    if (key != 0 && lastKey == 0) {
      lastKey = key;

      if (key >= 1 && key <= 10) {
        int digit = key % 10;
        inputBuffer += String(digit);
        lcdShowStars();
        waitingForDoubleClick = false;  // Reset double click khi đã nhập số
      }

      if (inputBuffer.length() == 4) {
        currentState = STATE_CHECK;
      }
    }
    if (key == 0) lastKey = 0;
  }

  // ========= CHECK =========
  else if (currentState == STATE_CHECK) {
    if (inputBuffer == password) {
      // ---- ĐÚNG ----
      failCount = 0;
      lockoutTime = 10000;

      lcd.clear();
      lcd.print("Access Granted");
      lcd.setCursor(0, 1);
      lcd.print("Press Button");

      // Gửi trạng thái lên Blynk
      if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(VPIN_PASSWORD_STATUS, 1);
        Blynk.virtualWrite(VPIN_FAIL_COUNT, 0);
        Blynk.virtualWrite(VPIN_LOCKOUT_STATUS, 0);
      }

      // Không mở cửa ngay, chờ bấm nút
      stateTimer = millis();
      currentState = STATE_WAIT_BUTTON;
    } else {
      // ---- SAI ----
      failCount++;
      digitalWrite(LED_PIN, HIGH);

      lcd.clear();
      lcd.print("Wrong Password");
      lcd.setCursor(0, 1);
      lcd.print("Left: ");
      lcd.print(MAX_FAIL - failCount);

      // Gửi trạng thái lên Blynk
      if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(VPIN_PASSWORD_STATUS, 0);
        Blynk.virtualWrite(VPIN_FAIL_COUNT, failCount);
      }

      stateTimer = millis();
      currentState = STATE_WRONG;
    }
    inputBuffer = "";
  }

  // ========= WRONG =========
  else if (currentState == STATE_WRONG) {
    if (millis() - stateTimer > 1000) {
      digitalWrite(LED_PIN, LOW);

      if (failCount >= MAX_FAIL) {
        // Gửi trạng thái lockout lên Blynk
        if (WiFi.status() == WL_CONNECTED) {
          Blynk.virtualWrite(VPIN_LOCKOUT_STATUS, 1);
        }
        stateTimer = millis();
        currentState = STATE_LOCKOUT;
      } else {
        lcdEnterPassword();
        currentState = STATE_INPUT;
      }
    }
  }

  // ========= WAIT BUTTON =========
  else if (currentState == STATE_WAIT_BUTTON) {
    unsigned long elapsed = millis() - stateTimer;
    
    // Kiểm tra nếu bấm nút A3
    if (isButton1Pressed()) {
      // Bấm nút → Mở cửa
      doorServo.write(90);
      servoIsOpen = true;
      
      // Gửi trạng thái cửa lên Blynk
      if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(VPIN_DOOR_STATUS, 1);
      }
      
      lcd.clear();
      lcd.print("Door Open");
      lcd.setCursor(0, 1);
      lcd.print("Closing in 3s");
      
      stateTimer = millis();
      currentState = STATE_UNLOCK;
    }
    // Kiểm tra timeout 10s
    else if (elapsed >= WAIT_BUTTON_TIMEOUT) {
      // Hết thời gian chờ → Khóa và yêu cầu nhập lại
      doorServo.write(0);
      servoIsOpen = false;
      
      lcd.clear();
      lcd.print("Time Out!");
      lcd.setCursor(0, 1);
      lcd.print("Enter Again");
      delay(1500);
      
      lcdEnterPassword();
      currentState = STATE_INPUT;
    }
    // Hiển thị thời gian còn lại
    else {
      int remain = (WAIT_BUTTON_TIMEOUT - elapsed) / 1000;
      static int lastRemain = -1;
      if (remain != lastRemain) {
        lcd.setCursor(0, 1);
        lcd.print("Press Button ");
        lcd.print(remain);
        lcd.print("s");
        lastRemain = remain;
      }
    }
  }

  // ========= UNLOCK =========
  else if (currentState == STATE_UNLOCK) {
    // Cửa đã mở, đếm 3s rồi đóng
    if (millis() - stateTimer >= DOOR_OPEN_DURATION) {
      doorServo.write(0);
      servoIsOpen = false;
      
      // Gửi trạng thái cửa lên Blynk
      if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(VPIN_DOOR_STATUS, 0);
      }
      
      lcd.clear();
      lcd.print("Door Closed");
      delay(1000);
      
      lcdEnterPassword();
      currentState = STATE_INPUT;
    } else {
      // Hiển thị thời gian còn lại
      int remain = (DOOR_OPEN_DURATION - (millis() - stateTimer)) / 1000 + 1;
      static int lastRemain = -1;
      if (remain != lastRemain) {
        lcd.clear();
        lcd.print("Door Open");
        lcd.setCursor(0, 1);
        lcd.print("Closing in ");
        lcd.print(remain);
        lcd.print("s");
        lastRemain = remain;
      }
    }
  }

  // ========= LOCKOUT =========
  else if (currentState == STATE_LOCKOUT) {
    unsigned long elapsed = millis() - stateTimer;

    if (elapsed >= lockoutTime) {
      lockoutTime += 10000;  // tăng dần nếu tiếp tục sai
      failCount = 0;
      
      // Gửi trạng thái lockout lên Blynk
      if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(VPIN_LOCKOUT_STATUS, 0);
        Blynk.virtualWrite(VPIN_FAIL_COUNT, 0);
      }
      
      lcdEnterPassword();
      currentState = STATE_INPUT;
    } else {
      int remain = (lockoutTime - elapsed) / 1000;

      static int lastRemain = -1;
      if (remain != lastRemain) {
        lcd.clear();
        lcd.print("LOCKED!");
        lcd.setCursor(0, 1);
        lcd.print("Wait ");
        lcd.print(remain);
        lcd.print("s");
        lastRemain = remain;
      }
    }
  }

  // ========= CHANGE PASSWORD - OLD =========
  else if (currentState == STATE_CHANGE_PASSWORD_OLD) {
    // Kiểm tra double-click button A3 để thoát
    if (isButton1Pressed()) {
      unsigned long currentTime = millis();
      if (waitingForDoubleClick && (currentTime - lastButtonPressTime) < DOUBLE_CLICK_TIMEOUT) {
        // Double click để thoát
        inputBuffer = "";
        newPasswordBuffer = "";
        confirmPasswordBuffer = "";
        waitingForDoubleClick = false;
        lcd.clear();
        lcd.print("Exit change");
        lcd.setCursor(0, 1);
        lcd.print("Password");
        delay(1500);
        lcdEnterPassword();
        currentState = STATE_INPUT;
      } else {
        waitingForDoubleClick = true;
        lastButtonPressTime = currentTime;
      }
    }
    
    if (waitingForDoubleClick && (millis() - lastButtonPressTime) >= DOUBLE_CLICK_TIMEOUT) {
      waitingForDoubleClick = false;
    }
    
    if (key != 0 && lastKey == 0) {
      lastKey = key;

      // Kiểm tra phím thoát (phím 15)
      if (key == EXIT_CHANGE_PASSWORD_KEY) {
        inputBuffer = "";
        newPasswordBuffer = "";
        confirmPasswordBuffer = "";
        waitingForDoubleClick = false;
        lcd.clear();
        lcd.print("Exit change");
        lcd.setCursor(0, 1);
        lcd.print("Password");
        delay(1500);
        lcdEnterPassword();
        currentState = STATE_INPUT;
      }
      else if (key >= 1 && key <= 10) {
        int digit = key % 10;
        inputBuffer += String(digit);
        lcd.setCursor(10, 1);
        for (int i = 0; i < inputBuffer.length(); i++) lcd.print("*");
      }

      if (inputBuffer.length() == 4) {
        if (inputBuffer == password) {
          inputBuffer = "";
          newPasswordBuffer = "";
          lcdChangePasswordNew();
          currentState = STATE_CHANGE_PASSWORD_NEW;
        } else {
          lcd.clear();
          lcd.print("Wrong Password!");
          delay(1500);
          inputBuffer = "";
          lcdChangePasswordOld();
        }
      }
    }
    if (key == 0) lastKey = 0;
  }

  // ========= CHANGE PASSWORD - NEW =========
  else if (currentState == STATE_CHANGE_PASSWORD_NEW) {
    // Kiểm tra double-click button A3 để thoát
    if (isButton1Pressed()) {
      unsigned long currentTime = millis();
      if (waitingForDoubleClick && (currentTime - lastButtonPressTime) < DOUBLE_CLICK_TIMEOUT) {
        // Double click để thoát
        inputBuffer = "";
        newPasswordBuffer = "";
        confirmPasswordBuffer = "";
        waitingForDoubleClick = false;
        lcd.clear();
        lcd.print("Exit change");
        lcd.setCursor(0, 1);
        lcd.print("Password");
        delay(1500);
        lcdEnterPassword();
        currentState = STATE_INPUT;
      } else {
        waitingForDoubleClick = true;
        lastButtonPressTime = currentTime;
      }
    }
    
    if (waitingForDoubleClick && (millis() - lastButtonPressTime) >= DOUBLE_CLICK_TIMEOUT) {
      waitingForDoubleClick = false;
    }
    
    if (key != 0 && lastKey == 0) {
      lastKey = key;

      // Kiểm tra phím thoát (phím 15)
      if (key == EXIT_CHANGE_PASSWORD_KEY) {
        inputBuffer = "";
        newPasswordBuffer = "";
        confirmPasswordBuffer = "";
        waitingForDoubleClick = false;
        lcd.clear();
        lcd.print("Exit change");
        lcd.setCursor(0, 1);
        lcd.print("Password");
        delay(1500);
        lcdEnterPassword();
        currentState = STATE_INPUT;
      }
      else if (key >= 1 && key <= 10) {
        int digit = key % 10;
        newPasswordBuffer += String(digit);
        lcd.setCursor(10, 1);
        for (int i = 0; i < newPasswordBuffer.length(); i++) lcd.print("*");
      }

      if (newPasswordBuffer.length() == 4) {
        confirmPasswordBuffer = "";
        lcdChangePasswordConfirm();
        currentState = STATE_CHANGE_PASSWORD_CONFIRM;
      }
    }
    if (key == 0) lastKey = 0;
  }

  // ========= CHANGE PASSWORD - CONFIRM =========
  else if (currentState == STATE_CHANGE_PASSWORD_CONFIRM) {
    // Kiểm tra double-click button A3 để thoát
    if (isButton1Pressed()) {
      unsigned long currentTime = millis();
      if (waitingForDoubleClick && (currentTime - lastButtonPressTime) < DOUBLE_CLICK_TIMEOUT) {
        // Double click để thoát
        inputBuffer = "";
        newPasswordBuffer = "";
        confirmPasswordBuffer = "";
        waitingForDoubleClick = false;
        lcd.clear();
        lcd.print("Exit change");
        lcd.setCursor(0, 1);
        lcd.print("Password");
        delay(1500);
        lcdEnterPassword();
        currentState = STATE_INPUT;
      } else {
        waitingForDoubleClick = true;
        lastButtonPressTime = currentTime;
      }
    }
    
    if (waitingForDoubleClick && (millis() - lastButtonPressTime) >= DOUBLE_CLICK_TIMEOUT) {
      waitingForDoubleClick = false;
    }
    
    if (key != 0 && lastKey == 0) {
      lastKey = key;

      // Kiểm tra phím thoát (phím 15)
      if (key == EXIT_CHANGE_PASSWORD_KEY) {
        inputBuffer = "";
        newPasswordBuffer = "";
        confirmPasswordBuffer = "";
        waitingForDoubleClick = false;
        lcd.clear();
        lcd.print("Exit change");
        lcd.setCursor(0, 1);
        lcd.print("Password");
        delay(1500);
        lcdEnterPassword();
        currentState = STATE_INPUT;
      }
      else if (key >= 1 && key <= 10) {
        int digit = key % 10;
        confirmPasswordBuffer += String(digit);
        lcd.setCursor(9, 1);
        for (int i = 0; i < confirmPasswordBuffer.length(); i++) lcd.print("*");
      }

      if (confirmPasswordBuffer.length() == 4) {
        if (confirmPasswordBuffer == newPasswordBuffer) {
          password = newPasswordBuffer;
          lcd.clear();
          lcd.print("Password");
          lcd.setCursor(0, 1);
          lcd.print("Changed!");
          delay(2000);
          inputBuffer = "";
          newPasswordBuffer = "";
          confirmPasswordBuffer = "";
          lcdEnterPassword();
          currentState = STATE_INPUT;
        } else {
          lcd.clear();
          lcd.print("Not Match!");
          lcd.setCursor(0, 1);
          lcd.print("Try Again");
          delay(2000);
          newPasswordBuffer = "";
          confirmPasswordBuffer = "";
          lcdChangePasswordNew();
          currentState = STATE_CHANGE_PASSWORD_NEW;
        }
      }
    }
    if (key == 0) lastKey = 0;
  }

  delay(20);
}
