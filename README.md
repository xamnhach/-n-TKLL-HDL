# 🔐 Đồ Án: Hệ Thống Ổ Khóa Điện Tử Thông Minh (Smart Door Lock)

Dự án thiết kế hệ thống khóa cửa thông minh sử dụng vi điều khiển **ESP32**, kết hợp bảo mật mật mã qua bàn phím cảm ứng, quản lý từ xa qua App **Blynk** và đồng bộ dữ liệu lên **Cloud**.

---

## 📖 Giới thiệu đề tài
Đây là đồ án môn học **Thiết kế Luận lý (CO3091)** - Trường Đại học Bách Khoa TP.HCM. Hệ thống được xây dựng nhằm mục tiêu tăng cường an ninh nhà ở, cho phép người dùng kiểm soát trạng thái cửa mọi lúc mọi nơi và nhận cảnh báo khi có xâm nhập trái phép.

## ✨ Tính năng chính
* **Mở khóa bằng mật mã:** Sử dụng bàn phím cảm ứng TTP229 (Mã mặc định: `8888`).
* **Đổi mật khẩu:** Cho phép người dùng thay đổi mật khẩu trực tiếp trên thiết bị (quy trình gồm nhập mật khẩu cũ, mật khẩu mới và xác nhận).
* **Điều khiển qua Blynk IoT:** Mở/Khóa cửa từ xa, theo dõi trạng thái đóng/mở và nhận thông báo lỗi trên điện thoại.
* **Chế độ bảo mật (Lockout):** Tự động khóa hệ thống nếu nhập sai mật khẩu quá 3 lần để ngăn chặn kẻ gian.
* **Giao diện trực quan:** Hiển thị thông báo hướng dẫn và trạng thái trên màn hình LCD 16x2.

## 🛠 Linh kiện phần cứng
* **Vi điều khiển:** ESP32 DevKit V1.
* **Bàn phím:** TTP229 (Cảm ứng 16 phím).
* **Màn hình:** LCD 16x2 (Module I2C).
* **Động cơ:** Servo SG90 (Mô phỏng chốt cửa).
* **Phụ kiện:** Đèn LED trạng thái.

## 📺 Demo & Báo cáo
* **Video Demo sản phẩm:** [Xem tại đây]([https://drive.google.com/file/d/19KoV1095K_jjL295RKRCHVGJ1paQmWhU/view?usp=sharing](https://drive.google.com/file/d/19KoV1095K_jjL295RKRCHVGJ1paQmWhU/view?usp=drive_link))
* **Tài liệu chi tiết:** Xem file `Đồ_Án_TKLL.pdf` trong kho lưu trữ này.

## 👥 Thành viên thực hiện
**Nhóm sinh viên Lớp L05 - ĐH Bách Khoa TP.HCM:**
1. **La Anh Tú** - 2112601
2. **Trần Việt Hoàng** - 2211122
3. **Trang Hiểu Nghĩa** - 2312279

---
**Giảng viên hướng dẫn:** Thầy Nguyễn Thành Lộc
