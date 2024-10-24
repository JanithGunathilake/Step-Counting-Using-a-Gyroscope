#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <HTTPClient.h>  // Include library to make HTTP requests
#include <ESPAsyncWebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C OLED display address
#define OLED_ADDR   0x3C

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // -1 is used because there is no reset pin for I2C

const char* ssid = "your-ssid";
const char* password = "your-password";
const char* mqttServer = "your-mqtt-server";
const int mqttPort = 1883;
const char* mqttUser = "your-mqtt-username";
const char* mqttPassword = "your-mqtt-password";
const char* fallStatusTopic = "esp32/fallStatus";


WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);

// Notify.lk settings
const String apiKey = "your-notify-lk-api-key";  // Replace with your Notify.lk API key
const String phoneNumber = "947XXXXXXXX";    // Replace with the recipient's phone number in format 947XXXXXXXX
const String message = "Fall detected! Emergency!";
bool previousFallDetected = false;

int stepCount = 0;
bool stepDetected = false;
float gyroThreshold = 0.4;
unsigned long lastStepTime = 0;
const unsigned long debounceDelay = 300;
bool fallDetected = false;
float fallThreshold = 7.5;

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) { while (1); }
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) { for (;;); }
  connectWiFi();
  connectMQTT();
  server.on("/", HTTP_GET, handleRoot);
  server.on("/reset", HTTP_GET, handleReset);
  server.begin();
}


// Function to handle root web page
void handleRoot(AsyncWebServerRequest *request) {
  String html = "<!DOCTYPE html><html><head><title>Step Counter</title>"
                "<style>body{font-family: Arial, sans-serif;background-color: #f9f9f9;margin: 0;padding: 0;}"
                "#container{width: 300px;margin: 50px auto;padding: 20px;background-color: #ffffff;"
                "border-radius: 1px;box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);}"
                "h1{text-align: center;color: #333333;}.info{text-align: center;font-size: 24px;"
                "color: #555555;margin-bottom: 20px;}.btn{display: block;width: 100%;padding: 10px;"
                "text-align: center;color: #ffffff;background-color: #ff0000;border: none;"
                "border-radius: 15px;font-size: 18px;cursor: pointer;transition: background-color 0.2s ease;}"
                ".btn:hover{background-color: #6e0101;}#footer{text-align: center;color: #888888;"
                "position: fixed;bottom: 20px;left: 0;right: 0;}</style></head><body>"
                "<div id=\"container\"><h1>Step Count Monitoring Dashboard</h1>"
                "<p class=\"info\">Step count: <span id=\"step-count\">" + String(stepCount) + "</span></p>"
                "<button class=\"btn\" onclick=\"resetCount()\">Reset Count</button></div>"
                "<script>function updateStepCount(count){document.getElementById(\"step-count\").innerText=count;}"
                "function resetCount(){var xhr=new XMLHttpRequest();xhr.open(\"GET\",\"/reset\",true);"
                "xhr.onreadystatechange=function(){if(xhr.readyState==4&&xhr.status==200){"
                "var response=JSON.parse(xhr.responseText);updateStepCount(response.stepCount);}};"
                "xhr.send();}window.onload=function(){setInterval(function(){location.reload();}, 5000);};</script></body></html>";
  request->send(200, "text/html", html);
}


// Function to handle step counter reset
void handleReset(AsyncWebServerRequest *request) {
  stepCount = 0;
  String response = "{\"stepCount\":" + String(stepCount) + "}";
  request->send(200, "application/json", response);
}

// Function to send an SMS using Notify.lk
void sendEmergencySMS() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Create URL with API key, recipient number, and message
    String url = "https://app.notify.lk/api/v1/send?user_id=&api_key=" + String(apiKey) + 
                 "&sender_id=NotifyDEMO&to=" + String(phoneNumber) + 
                 "&message=" + String(message);

    http.begin(url);  // Initialize HTTP connection
    int httpResponseCode = http.GET();  // Send HTTP GET request

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("SMS sent successfully.");
      Serial.println(response);  // Print the server response
    } else {
      Serial.println("Error sending SMS.");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();  // Close the HTTP connection
  } else {
    Serial.println("WiFi not connected");
  }
}

void updateOledDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Step Count: ");
  display.print(stepCount);
  display.display();
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void connectMQTT() {
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    if (client.connect("ESP32Client99", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT");
    } else {
      delay(5000);
    }
  }
}

// Function to publish step count to MQTT
void publishStepCount() {
  String payload = String(stepCount);
  client.publish("esp32/stepCount", payload.c_str());
}

void publishStatus(bool fallDetected, float accelerationZ, float gyroZ) {
  String payload = "{\"fallDetected\":" + String(fallDetected ? "true" : "false") + 
                   ", \"accelerationZ\":" + String(accelerationZ, 2) + 
                   ", \"gyroZ\":" + String(gyroZ, 2) + "}";
  client.publish("esp32/Status", payload.c_str());
  Serial.print("Fall status, acceleration, and gyro data published: ");
  Serial.println(payload);
}

void publishFallStatus(bool fallDetected) {
  String payload = String(fallDetected ? "Fall Detected!" : "No");  // Correct string construction
  client.publish("esp32/fallStatus", payload.c_str());
  Serial.print("Fall status published: ");
  Serial.println(payload);
}







void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long currentTime = millis();
  
  // Step detection
  if (!stepDetected && abs(g.gyro.z) > gyroThreshold) {
    if (currentTime - lastStepTime > debounceDelay) {
      stepCount++;
      lastStepTime = currentTime;
      stepDetected = true;
    }
  } else if (stepDetected && abs(g.gyro.z) < gyroThreshold - 0.1) {
    stepDetected = false;
  }

  // Fall detection logic based on acceleration
  if (a.acceleration.z < fallThreshold) {
    if (fallDetected) {
      fallDetected = false;  // Set fallDetected to true when a fall is detected
      Serial.println("Fall detected! Sending alert...");
      sendEmergencySMS();  // Send SMS via Notify.lk
    }
  } else {
    fallDetected = true;
  }


  publishFallStatus(fallDetected);

  publishStatus(fallDetected, a.acceleration.z, g.gyro.z);
 


  updateOledDisplay();
  publishStepCount();
  delay(200);
}
