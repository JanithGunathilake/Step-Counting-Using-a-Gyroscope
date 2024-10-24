# ESP32 Step Counter and Fall Detection System

This project is a wearable IoT device based on the ESP32 that counts steps and detects falls using an MPU6050 accelerometer/gyroscope sensor. It also integrates with Notify.lk for sending emergency SMS alerts in case of a detected fall. Additionally, it publishes step count and fall detection status to an MQTT server and displays real-time step count on an OLED screen.

## Features
- Step counting using the gyroscope data.
- Fall detection using the accelerometer data.
- Real-time display of step count on a 128x64 OLED screen.
- Sends emergency SMS alerts via Notify.lk when a fall is detected.
- Publishes step count and fall status to an MQTT broker.
- Simple web dashboard for monitoring step count and resetting step counter.

## Hardware Requirements
- ESP32
- MPU6050 Accelerometer/Gyroscope sensor
- 128x64 OLED display (I2C)
- WiFi connection for MQTT and Notify.lk API integration

## Software Requirements
- Arduino IDE with ESP32 board support
- Libraries:
  - `Adafruit MPU6050`
  - `Adafruit GFX`
  - `Adafruit SSD1306`
  - `ESPAsyncWebServer`
  - `PubSubClient`
  - `HTTPClient`

## Circuit Diagram
- **OLED (I2C)**:
  - `VCC` to 3.3V
  - `GND` to GND
  - `SCL` to GPIO 22 (SCL)
  - `SDA` to GPIO 21 (SDA)
  
- **MPU6050 (I2C)**:
  - `VCC` to 3.3V
  - `GND` to GND
  - `SCL` to GPIO 22
  - `SDA` to GPIO 21

## How It Works
1. **Step Detection**: Gyroscope data is used to count steps. A threshold is set to detect when the gyroscope’s Z-axis exceeds a defined value, indicating a step.
2. **Fall Detection**: Accelerometer data is monitored to detect a fall based on a threshold for sudden changes in acceleration. When a fall is detected, an SMS alert is sent.
3. **Display**: Step count is displayed on a 128x64 OLED screen using the Adafruit SSD1306 library.
4. **Web Dashboard**: A simple web interface is provided to view the current step count and reset it.
5. **MQTT Integration**: The system publishes step count and fall detection data to an MQTT server.
6. **SMS Alerts**: When a fall is detected, the system sends an emergency SMS alert using the Notify.lk API.

## Installation
1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/esp32-step-counter-fall-detection.git



