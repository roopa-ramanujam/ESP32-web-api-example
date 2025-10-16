#include <secrets.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Adafruit_NeoPixel.h>

// ---------- WiFi credentials ----------
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

// ---------- Location (Jacobs Hall, Berkeley) ----------
const double latitude  = 37.876270;
const double longitude = -122.258499;

// ---------- Open-Meteo API ----------
String weather_api = "http://api.open-meteo.com/v1/forecast?latitude="
                     + String(latitude, 6)
                     + "&longitude="
                     + String(longitude, 6)
                     + "&current_weather=true"
                     + "&temperature_unit=fahrenheit";

// ---------- Variables ----------
double currentTemp = -1;  // ⭐ start as invalid
double currentWind = 0;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60 * 1000; // refresh every 1 min

// ---------- Onboard NeoPixel ----------
#define PIN_NEOPIXEL        0
#define NEOPIXEL_I2C_POWER  2
#define NUMPIXELS           1

Adafruit_NeoPixel pixel(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER, HIGH); // ⭐ ensure power enabled

  pixel.begin();
  pixel.clear();
  pixel.show();

  connectToWiFi();
  delay(1000);
  getWeatherData();  // initial fetch
}

// ---------- Main Loop ----------
void loop() {
  if (millis() - lastUpdate > updateInterval) {
    getWeatherData();
    lastUpdate = millis();
  }

  // ⭐ Only show light if temperature is valid
  if (currentTemp != -1) {
    visualizeWeather(currentTemp, currentWind);
  } else {
    pixel.clear();
    pixel.show(); // keep LED off when no valid data
  }
}

// ---------- WiFi Connection ----------
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
}

// ---------- Fetch Weather ----------
void getWeatherData() {
  Serial.print("Requesting: ");
  Serial.println(weather_api);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(weather_api);
    int httpResponseCode = http.GET();

    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar jsonObject = JSON.parse(payload);

      if (JSON.typeof(jsonObject) == "undefined") {
        Serial.println("Parsing input failed!");
        currentTemp = -1;  // ⭐ invalidate reading
        http.end();
        return;
      }

      currentTemp = double(jsonObject["current_weather"]["temperature"]);
      currentWind = double(jsonObject["current_weather"]["windspeed"]);

      Serial.print("🌡 Temp: ");
      Serial.print(currentTemp);
      Serial.println(" °F");
      Serial.print("💨 Wind: ");
      Serial.print(currentWind);
      Serial.println(" mph");
    } else {
      Serial.print("HTTP error: ");
      Serial.println(httpResponseCode);
      currentTemp = -1;  // ⭐ invalidate reading
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.reconnect();
    currentTemp = -1;  // ⭐ invalidate reading
  }
}

// ---------- LED Visualization ----------
void visualizeWeather(double tempF, double windspeed) {
  int red = 0, green = 0, blue = 0;

  if (tempF <= 65) {
    blue = 255;
  } else if (tempF <= 80) {
    green = 255;
  } else {
    red = 255;
  }

  int blinkDelay = map((int)windspeed, 0, 30, 800, 100);

  pixel.setPixelColor(0, pixel.Color(red, green, blue));
  pixel.show();
  delay(blinkDelay);

  pixel.clear();
  pixel.show();
  delay(blinkDelay);
}
