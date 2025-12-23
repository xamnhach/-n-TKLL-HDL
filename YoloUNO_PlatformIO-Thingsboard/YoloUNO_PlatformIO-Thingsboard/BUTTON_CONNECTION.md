# Hướng Dẫn Kết Nối Button Vào A3 và A2

## 📌 Thông Tin Pin

Theo pinout của Yolo Uno:
- **A3** = **GPIO4** → Button 1 (Mở servo)
- **A2** = **GPIO3** → Button 2 (Đóng servo)

## 🔌 Cách Kết Nối

### Sơ Đồ Kết Nối

```
Button 1 (Mở Servo):
  ┌─────────┐
  │ Button1 │───[A3/GPIO4]─── Yolo Uno
  └────┬────┘
       │
      GND

Button 2 (Đóng Servo):
  ┌─────────┐
  │ Button2 │───[A2/GPIO3]─── Yolo Uno
  └────┬────┘
       │
      GND
```

### Chi Tiết Kết Nối

1. **Button 1 (Mở Servo)**:
   - Một chân button → **A3** (GPIO4) của Yolo Uno
   - Chân còn lại → **GND** (bất kỳ chân GND nào)

2. **Button 2 (Đóng Servo)**:
   - Một chân button → **A2** (GPIO3) của Yolo Uno
   - Chân còn lại → **GND** (bất kỳ chân GND nào)

### Vị Trí Trên Board

- **A3** và **A2** nằm ở **header bên trái** (Analog Input Block)
- **GND** có nhiều chân, bạn có thể dùng bất kỳ chân GND nào

## ⚙️ Code Đã Cấu Hình

```cpp
#define BUTTON1_PIN 4   // A3 = GPIO4
#define BUTTON2_PIN 3   // A2 = GPIO3
```

## 🎯 Cách Hoạt Động

- **Button 1 (A3)**: Nhấn → Servo mở (90°) → LCD hiển thị "Manual Open"
- **Button 2 (A2)**: Nhấn → Servo đóng (0°) → LCD hiển thị "Manual Close"

## ⚠️ Lưu Ý

1. Code sử dụng **INPUT_PULLUP** nên không cần điện trở pull-up ngoài
2. Khi button không nhấn: GPIO đọc HIGH (pull-up internal)
3. Khi button nhấn: GPIO đọc LOW (kết nối GND)
4. Button hoạt động **độc lập** với hệ thống mật khẩu
5. Button có thể mở/đóng servo **bất cứ lúc nào**

## 🔧 Kiểm Tra Kết Nối

1. Kết nối button theo sơ đồ
2. Upload code lên board
3. Nhấn button → Servo sẽ quay và LCD hiển thị thông báo
4. Nếu không hoạt động, kiểm tra:
   - Dây kết nối có chắc chắn không
   - Button có hoạt động không (dùng multimeter)
   - Đã upload code mới chưa

