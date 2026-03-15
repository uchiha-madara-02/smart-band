# ⌚ Smart Band Project

Dự án thiết bị đeo tay thông minh (Smart Band) hỗ trợ theo dõi sức khỏe và cảnh báo qua Telegram.

## 🛠 Linh kiện thiết bị (Components)

Dưới đây là danh sách các linh kiện chính được sử dụng trong dự án:

| Hình ảnh | Tên linh kiện | Mô tả chức năng |
| :---: | :--- | :--- |
| <img src="./Link kiện/esp32s3foot1-68fc162c83984487b89add5b09fd74a0.webp" width="150"> | **ESP32-S3 Super Mini** | Vi điều khiển chính, hỗ trợ WiFi/Bluetooth để gửi dữ liệu. |
| <img src="./Link kiện/SN-M30102-OXI-e.jpg" width="150"> | **MAX30102** | Cảm biến đo nhịp tim và nồng độ Oxy trong máu (SpO2). |
| <img src="./Link kiện/0256654mpu6050.webp" width="150"> | **MPU6050** | Cảm biến gia tốc và con quay hồi chuyển (dùng để phát hiện té ngã). |
| <img src="./Link kiện/module-gy906-2.webp" width="150"> | **GY-906** | Cảm biến nhiệt độ hồng ngoại không tiếp xúc. |

---

## ⚠️ Lưu ý quan trọng

Để dự án hoạt động chính xác, anh em cần tùy chỉnh các thông số cấu hình trong mã nguồn:

* **WiFi:** Cập nhật `SSID` và `Password`.
* **Telegram:** Thay đổi `Chat ID` và `Bot Token` của cá nhân bạn.
* **Thư viện:** Đảm bảo đã cài đặt đầy đủ thư viện cho các cảm biến I2C (MPU6050, MAX30102, MLX90614).
