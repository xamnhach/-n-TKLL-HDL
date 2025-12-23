#include "globals.h"

byte ttp229_read_key() {
    byte key = 0;

    for (byte i = 1; i <= 16; i++) {

        digitalWrite(TTP229_SCL, LOW);
        delayMicroseconds(150);

        bool pressed = !digitalRead(TTP229_SDO);

        digitalWrite(TTP229_SCL, HIGH);
        delayMicroseconds(150);

        if (pressed) key = i;
    }

    // Debounce
    static byte lastStable = 0;
    static uint32_t lastTime = 0;

    if (key != lastStable) {
        lastStable = key;
        lastTime = millis();
    }

    if (key == 0) return 0;

    if (millis() - lastTime > 30) {
        return key; // stable
    }

    return 0;
}

void password_task(void *param) {
    const int PASSWORD_LENGTH = 4;
    byte lastKey = 0;

    while (true) {

        byte key = ttp229_read_key();

        if (key > 0 && key != lastKey) {
            lastKey = key;

            Serial.print("Key pressed: ");
            Serial.println(key);

            // convert keypad values to digits
            if (key >= 1 && key <= 9) {
                inputPassword += String(key);
            } else if (key == 11) {
                inputPassword += "0"; // key 11 = 0
            }

            Serial.print("Current Input = ");
            Serial.println(inputPassword);

            if (inputPassword.length() == PASSWORD_LENGTH) {
                if (inputPassword == currentPassword)
                    Serial.println("MẬT KHẨU ĐÚNG!");
                else
                    Serial.println("MẬT KHẨU SAI!");

                inputPassword = "";
            }
        }

        if (key == 0) lastKey = 0;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
