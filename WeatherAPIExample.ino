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
double currentTemp = 0;
double currentWind = 0;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60 * 1000; // refresh every 1 min

// ---------- Onboard NeoPixel ----------
#define PIN_NEOPIXEL        0   // data pin
#define NEOPIXEL_I2C_POWER  2   // power enable pin
#define NUMPIXELS           1

Adafruit_NeoPixel pixel(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Turn on NeoPixel power
  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);

  connectToWiFi();
  delay(1000);
  getWeatherData();  // initial fetch
}

// ---------- Main Loop ----------
void loop() {
  // Update weather every 1 min
  if (millis() - lastUpdate > updateInterval) {
    getWeatherData();
    lastUpdate = millis();
  }

  // Visualize current readings
  visualizeWeather(currentTemp, currentWind);
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

    // Successful response
    if (httpResponseCode == 200) {

      // Get the JSON payload
      String payload = http.getString();

      // Parse it into a JSONVar object
      JSONVar jsonObject = JSON.parse(payload);

      // Check if parsing succeeded
      if (JSON.typeof(jsonObject) == "undefined") {
        Serial.println("Parsing input failed!");
        http.end();
        return;
      }

      // Log the JSON response
      Serial.print("raw JSON response: ");
      Serial.println(jsonObject);

      // Extract the nested values
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
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.reconnect();
  }
}

// ---------- LED Visualization ----------
void visualizeWeather(double tempF, double windspeed) {
  int red = 0, green = 0, blue = 0;

  // Color by temperature
  if (tempF <= 65) {          // cold
    blue = 255;
  } else if (tempF <= 80) {   // mild
    green = 255;
  } else {                    // hot
    red = 255;
  }

  // Map windspeed (0–30 mph) → blink rate (slower = calm, faster = windy)
  int blinkDelay = map((int)windspeed, 0, 30, 800, 100);

  // Blink once per loop iteration
  pixel.setPixelColor(0, pixel.Color(red, green, blue));
  pixel.show();
  delay(blinkDelay);

  pixel.clear();
  pixel.show();
  delay(blinkDelay);
}
