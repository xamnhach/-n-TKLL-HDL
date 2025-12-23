# Hướng Dẫn Kết Nối Blynk

## 📌 Tổng Quan

Hệ thống đã được tích hợp Blynk để điều khiển và giám sát từ xa qua smartphone.

## 🔧 Cấu Hình

### 1. Cập nhật thông tin WiFi và Blynk Token

Mở file `include/blynk_config.h` và điền thông tin:

```cpp
#define WIFI_SSID "Tên_WiFi_Của_Bạn"
#define WIFI_PASSWORD "Mật_Khẩu_WiFi"
#define BLYNK_AUTH_TOKEN "Token_Từ_Blynk_App"
```

### 2. Lấy Blynk Auth Token

1. Mở Blynk app trên điện thoại
2. Vào Settings → Device Info
3. Copy **Auth Token**
4. Dán vào `BLYNK_AUTH_TOKEN` trong file `blynk_config.h`

## 📱 Cấu Hình Template Blynk

### Virtual Pins được sử dụng:

- **V0** - `VPIN_DOOR_STATUS`: Trạng thái cửa (0=đóng, 1=mở)
  - Widget: **LED** hoặc **Value Display**
  - Label: "Door Status"
  
- **V1** - `VPIN_DOOR_CONTROL`: Điều khiển cửa từ app
  - Widget: **Button** hoặc **Switch**
  - Label: "Open/Close Door"
  - Mode: Switch (0=Close, 1=Open)
  
- **V2** - `VPIN_PASSWORD_STATUS`: Trạng thái mật khẩu (0=sai, 1=đúng)
  - Widget: **LED** hoặc **Value Display**
  - Label: "Password Status"
  
- **V3** - `VPIN_LOCKOUT_STATUS`: Trạng thái lockout (0=không, 1=có)
  - Widget: **LED** hoặc **Value Display**
  - Label: "Lockout Status"
  
- **V4** - `VPIN_FAIL_COUNT`: Số lần nhập sai mật khẩu
  - Widget: **Value Display**
  - Label: "Failed Attempts"
  
- **V5** - `VPIN_CHANGE_PASSWORD`: Switch để vào chế độ đổi mật khẩu
  - Widget: **Switch**
  - Label: "Change Password"
  - Mode: Switch (0=Off, 1=On)

## 🎯 Cách Thêm Widget Vào Template

### Bước 1: Thêm Widget
1. Mở Blynk app
2. Vào template của bạn
3. Nhấn "+" để thêm widget

### Bước 2: Cấu hình từng Widget

#### V0 - Door Status (LED)
- Chọn **LED** widget
- Pin: **V0**
- Label: "Door Status"
- Color: Green (mở), Red (đóng)

#### V1 - Door Control (Button)
- Chọn **Button** hoặc **Switch** widget
- Pin: **V1**
- Label: "Door Control"
- Mode: **Switch**
- Values: 0 = Close, 1 = Open

#### V2 - Password Status (LED)
- Chọn **LED** widget
- Pin: **V2**
- Label: "Password Status"
- Color: Green (đúng), Red (sai)

#### V3 - Lockout Status (LED)
- Chọn **LED** widget
- Pin: **V3**
- Label: "Lockout Status"
- Color: Red (lockout), Gray (normal)

#### V4 - Fail Count (Value Display)
- Chọn **Value Display** widget
- Pin: **V4**
- Label: "Failed Attempts"
- Format: Integer

#### V5 - Change Password (Switch)
- Chọn **Switch** widget
- Pin: **V5**
- Label: "Change Password"
- Mode: **Switch**
- Values: 0 = Off, 1 = On
- **Lưu ý**: Chỉ hoạt động khi ở màn hình "Enter Password" và chưa nhập số nào

## ⚙️ Tính Năng

### Điều Khiển Từ Blynk:
- ✅ Mở/Đóng cửa từ app (V1)
- ✅ Vào chế độ đổi mật khẩu (V5)
- ✅ Xem trạng thái cửa (V0)
- ✅ Xem trạng thái mật khẩu (V2)
- ✅ Xem trạng thái lockout (V3)
- ✅ Xem số lần nhập sai (V4)

### Tự Động Gửi Dữ Liệu:
- ✅ Trạng thái cửa khi mở/đóng
- ✅ Trạng thái mật khẩu (đúng/sai)
- ✅ Số lần nhập sai
- ✅ Trạng thái lockout

## 🔍 Kiểm Tra Kết Nối

1. Upload code lên board
2. Mở Serial Monitor (115200 baud)
3. Kiểm tra:
   - "WiFi connected!" → WiFi OK
   - "Blynk connecting..." → Blynk đang kết nối
   - Nếu không kết nối được, kiểm tra lại WiFi SSID/Password và Blynk Token

## ⚠️ Lưu Ý

1. **WiFi**: Board phải trong phạm vi WiFi
2. **Blynk Token**: Phải đúng token từ template của bạn
3. **Virtual Pins**: Phải khớp với template (V0-V4)
4. **Nếu không có WiFi**: Hệ thống vẫn hoạt động bình thường, chỉ không có Blynk

## 🐛 Troubleshooting

### WiFi không kết nối:
- Kiểm tra SSID và password
- Đảm bảo WiFi 2.4GHz (ESP32 không hỗ trợ 5GHz)
- Kiểm tra khoảng cách đến router

### Blynk không kết nối:
- Kiểm tra Auth Token
- Kiểm tra kết nối internet
- Kiểm tra Blynk server (có thể dùng Blynk Cloud hoặc Private Server)

### Widget không hoạt động:
- Kiểm tra Virtual Pin number
- Đảm bảo widget đã được cấu hình đúng pin
- Kiểm tra template đã được share với device chưa

