#include <WiFi.h>///
#include <UniversalTelegramBot.h>//
#include <WiFiClientSecure.h>//
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>//
#include <MAX30105.h>//
#include <spo2_algorithm.h>//
#include <Adafruit_MLX90614.h>//

const char* ssid = "";           // Thay bằng SSID WiFi của bạn
const char* password = "";   // Thay bằng mật khẩu WiFi của bạn

const char* telegramToken = ""; // Token Telegram bot của bạn
const char* chatID = ""; // Chat ID của bạn

WiFiClientSecure client;
UniversalTelegramBot bot(telegramToken, client);

Adafruit_MPU6050 mpu;
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

TwoWire I2C_GY906 = TwoWire(0); 
TwoWire I2C_SENSORS = TwoWire(1); 

#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int8_t spo2Valid;
int32_t heartRate;
int8_t heartRateValid;

int shakeCount = 0;
float lastAccelerationX = 0;
float lastAccelerationY = 0;
float lastAccelerationZ = 0;
const float accelerationThreshold = 2.0;  // Ngưỡng thay đổi gia tốc
const int shakeCountThreshold = 3;        // Số lần lắc cần thiết
unsigned long lastShakeTime = 0;         // Thời gian của lần lắc tay trước đó
const int shakeTimeWindow = 500;          // Khoảng thời gian giữa các lần lắc (tính bằng ms)
const int totalShakeTime = 880000;       // Tổng thời gian cho phép để nhận diện lắc tay (tính bằng ms)

void setupWiFi() {
  Serial.print("Đang kết nối WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
  bot.sendMessage(chatID, "ESP32 đã kết nối WiFi thành công!", "");
}

void setupsensors() {
  I2C_GY906.begin(9, 10, 100000);
  if (!mlx.begin(0x5A, &I2C_GY906)) {
    Serial.println("Không tìm thấy cảm biến GY-906!");
    bot.sendMessage(chatID, "Không tìm thấy GY-906!", "");
  }
  Serial.println("GY-906 đã sẵn sàng!");
  bot.sendMessage(chatID, "GY-906 đã sẵn sàng!", "");

  I2C_SENSORS.begin(13, 12, 400000);
  if (!mpu.begin(0x68, &I2C_SENSORS)) {
    Serial.println("Không tìm thấy cảm biến MPU6050!");
    bot.sendMessage(chatID, "Không tìm thấy MPU6050!", "");
    while (1);
  }
  Serial.println("MPU6050 đã sẵn sàng!");
  bot.sendMessage(chatID, "MPU6050 đã sẵn sàng!", "");

  if (!particleSensor.begin(I2C_SENSORS)) {
    Serial.println("Không tìm thấy cảm biến MAX30102!");
    bot.sendMessage(chatID, "Không tìm thấy MAX30102!", "");
    while (1);
  }
  Serial.println("MAX30102 đã sẵn sàng!");
  bot.sendMessage(chatID, "MAX30102 đã sẵn sàng!", "");
  particleSensor.setup();
}

void scanSensors() {
  Serial.println("Bắt đầu đọc cảm biến...");

  float ambientTemp = mlx.readAmbientTempC();
  float objectTemp = mlx.readObjectTempC();

  uint32_t irValue = particleSensor.getIR();
  if (irValue < 5000) {
    String message = "Vui lòng đặt tay vào cảm biến!!!";
    Serial.println(message);
    bot.sendMessage(chatID, message, "");
    return;
  }

  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.check()) {
      delay(1);
    }
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
  }

  maxim_heart_rate_and_oxygen_saturation(
    irBuffer, BUFFER_SIZE, redBuffer,
    &spo2, &spo2Valid, &heartRate, &heartRateValid
  );

  String message = "Dữ liệu đo được:\n";
  message += "Nhiệt độ cơ thể: " + String(objectTemp, 1) + " °C\n";
  message += "SpO2: " + (spo2Valid ? String(spo2) + "%" : "Không hợp lệ") + "\n";
  message += "Nhịp tim: " + (heartRateValid ? String(heartRate) + " bpm" : "Không hợp lệ");

  Serial.println(message);
  bot.sendMessage(chatID, message, "");
}

void lactay() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float deltaX = fabs(a.acceleration.x - lastAccelerationX);
  float deltaY = fabs(a.acceleration.y - lastAccelerationY);
  float deltaZ = fabs(a.acceleration.z - lastAccelerationZ);

  if (deltaX > accelerationThreshold || deltaY > accelerationThreshold || deltaZ > accelerationThreshold) {
    unsigned long currentTime = millis();

    if (currentTime - lastShakeTime > shakeTimeWindow) {
      shakeCount++;
      lastShakeTime = currentTime;
      Serial.println("Lắc tay lần: " + String(shakeCount));

      if (shakeCount >= shakeCountThreshold) {
        String warning = "Cảnh báo: Cần trợ giúp ngay lập tức!";
        Serial.println(warning);
        bot.sendMessage(chatID, warning, "");
        shakeCount = 0; 
      }
    }
  }

  lastAccelerationX = a.acceleration.x;
  lastAccelerationY = a.acceleration.y;
  lastAccelerationZ = a.acceleration.z;

  if (millis() - lastShakeTime > totalShakeTime) {
    shakeCount = 0;
  }
}

void checkHandPresence() {
  uint32_t irValue = particleSensor.getIR();
  
  if (irValue < 5000) {
    static unsigned long lastAlertTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastAlertTime > 5000) { 
      String message = "Cảnh báo: Vui lòng đặt tay vào cảm biến";
      Serial.println(message);
      bot.sendMessage(chatID, message, "");
      lastAlertTime = currentTime;
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  client.setInsecure();
  Serial.println("Khởi động hệ thống...");
  setupWiFi();
  setupsensors();
}
void loop() {
  lactay();            
  checkHandPresence(); 

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    if (text == "/scan") {
      scanSensors();
    }
  }

  delay(100);
}
