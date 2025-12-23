#ifndef BLYNK_CONFIG_H
#define BLYNK_CONFIG_H

// ================= BLYNK CONFIGURATION =================
// Thay đổi các thông tin sau theo template của bạn

// WiFi credentials
#define WIFI_SSID "ACLAB"
#define WIFI_PASSWORD "ACLAB2023"

// Blynk Auth Token (lấy từ email Blynk hoặc trong app)
#define BLYNK_AUTH_TOKEN "TfCBekQ8fO6KROLTGqcjG4JZ_hiVV5kR_"
#define BLYNK_TEMPLATE_ID "TMPL60EoNCy7n"
#define BLYNK_TEMPLATE_NAME "khóa cửa thông minh"
// Blynk Virtual Pins (thay đổi theo template của bạn)
#define VPIN_DOOR_STATUS  V0  // Trạng thái cửa (0=đóng, 1=mở)
#define VPIN_DOOR_CONTROL V1  // Điều khiển cửa từ app (0=đóng, 1=mở)
#define VPIN_PASSWORD_STATUS V2  // Trạng thái mật khẩu (0=sai, 1=đúng)
#define VPIN_LOCKOUT_STATUS V3  // Trạng thái lockout (0=không, 1=có)
#define VPIN_FAIL_COUNT V4      // Số lần nhập sai
#define VPIN_CHANGE_PASSWORD V5  // Switch để vào chế độ đổi mật khẩu (0=off, 1=on)

#endif

