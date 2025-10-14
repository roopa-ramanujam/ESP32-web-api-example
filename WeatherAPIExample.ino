#include <secrets.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// ---------- WiFi credentials ----------
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

// ---------- Pins ----------
const int RED_PIN   = 14;
const int GREEN_PIN = 27;
const int BLUE_PIN  = 33;

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

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  connectToWiFi();
  getWeatherData();
}

void loop() {
  // Update weather every 1 min
  if (millis() - lastUpdate > updateInterval) {
    getWeatherData();
    lastUpdate = millis();
  }

  // Continuously blink LED based on current wind speed
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
  Serial.println("\nWiFi connected!");
}

// ---------- Fetch weather ----------
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
        return;
      }

      // Log the JSON response
      Serial.print("raw JSON response: ");
      Serial.println(jsonObject);

      // Extract the nested values
      currentTemp = double(jsonObject["current_weather"]["temperature"]);
      currentWind = double(jsonObject["current_weather"]["windspeed"]);

      // Print to Serial Monitor
      Serial.print("🌡 Temperature: ");
      Serial.print(currentTemp);
      Serial.println(" °F");

      Serial.print("💨 Wind Speed: ");
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

// ---------- LED behavior ----------
void visualizeWeather(double tempF, double windspeed) {
  // Base color by temperature
  int red = 0, green = 0, blue = 0;

  if (tempF <= 65) {
    blue = 255;   // cold
  }
  else if (tempF <= 80) {
    green = 255;  // mild
  } 
  else {
    red = 255;    // hot
  }

  // Map windspeed (0–30 mph) → blink rate (slower = calm, faster = windy)
  int blinkDelay = map((int)windspeed, 0, 30, 800, 100);

  // On phase
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
  delay(blinkDelay);

  // Off phase
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  delay(blinkDelay);
}
