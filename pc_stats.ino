#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ST7789.h>

#define TFT_CS        7
#define TFT_DC        39
#define TFT_RST       40
#define TFT_BACKLITE  45
#define TFT_I2C_POWER 21

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

const char* ssid = "your_ssid";
const char* password = "your_password";
String serverURL = "your_url";

float bytesToGB(uint64_t bytes) {
  return bytes / 1024.0 / 1024.0 / 1024.0;
}

uint16_t getColorForPercent(float p) {
  if (p < 60) return ST77XX_GREEN;
  if (p < 85) return ST77XX_YELLOW;
  return ST77XX_RED;
}

void progressBar(int x, int y, int w, int h, float percent) {
  uint16_t color = getColorForPercent(percent);
  int filled = (w * percent) / 100;
  tft.fillRect(x, y, w, h, 0x7BEF);
  tft.fillRect(x, y, filled, h, color);
}

void setup() {
  Serial.begin(115200);

  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
  delay(10);

  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("connecting to wifi");

  WiFi.begin(ssid, password);
  Serial.print("connecting to wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nconnected");
  tft.fillRect(10, 10, 220, 30, ST77XX_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.println("connected");
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      Serial.println("raw JSON: ");
      Serial.println(payload);

      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float cpu = doc["cpu_percent"];
        float memo = doc["memory_percent"];
        float disk = doc["disk_percent"];
        float memoUsedGB = bytesToGB(doc["memory_used"]);
        float memoTotalGB = bytesToGB(doc["memory_total"]);
        float diskUsedGB = bytesToGB(doc["disk_used"]);
        float diskTotalGB = bytesToGB(doc["disk_total"]);

        int x = 10;
        int barW = 220;
        int barH = 10;

        tft.fillRect(x, 5, 200, 20, ST77XX_BLACK);
        tft.setCursor(x, 5);
        tft.setTextSize(2);
        tft.printf("CPU: %.1f%%", cpu);
        tft.fillRect(x, 25, barW, barH, ST77XX_BLACK);
        progressBar(x, 25, barW, barH, cpu);

        tft.fillRect(x, 45, 200, 20, ST77XX_BLACK);
        tft.setCursor(x, 45);
        tft.setTextSize(2);
        tft.printf("RAM: %.1f%%", memo);
        tft.fillRect(x, 60, 200, 15, ST77XX_BLACK);
        tft.setCursor(x, 60);
        tft.setTextSize(1);
        tft.printf("(%.1f / %.1f GB)", memoUsedGB, memoTotalGB);
        tft.fillRect(x, 75, barW, barH, ST77XX_BLACK);
        progressBar(x, 75, barW, barH, memo);

        tft.fillRect(x, 95, 200, 20, ST77XX_BLACK);
        tft.setCursor(x, 95);
        tft.setTextSize(2);
        tft.printf("DSK: %.1f%%", disk);
        tft.fillRect(x, 110, 200, 15, ST77XX_BLACK);
        tft.setCursor(x, 110);
        tft.setTextSize(1);
        tft.printf("(%.1f / %.1f GB)", diskUsedGB, diskTotalGB);
        tft.fillRect(x, 125, barW, barH, ST77XX_BLACK);
        progressBar(x, 125, barW, barH, disk);
      }
    }
    http.end();
  }
  delay(1000);
}
