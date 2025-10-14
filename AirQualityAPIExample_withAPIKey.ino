#include <secrets.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// ---------- WiFi credentials ----------
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

// ---------- API credentials ----------
const char* api_key = SECRET_API_KEY;

// ---------- OpenAQ API v3 ----------
// Ozone sensor in Berkeley
String air_api = "https://api.openaq.org/v3/sensors/3917/measurements?limit=1";

// ---------- Variables ----------
float o3Value = 0;
String o3Units = "";
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60 * 1000; // every 1 min

void setup() {
  Serial.begin(115200);
  delay(1000);

  connectToWiFi();
  getAirData();
}

void loop() {
  if (millis() - lastUpdate > updateInterval) {
    getAirData();
    lastUpdate = millis();
  }
}

// ---------- WiFi ----------
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// ---------- Fetch Air Quality ----------
void getAirData() {
  Serial.print("Requesting: ");
  Serial.println(air_api);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(air_api);

    // Add API key header
    http.addHeader("X-API-Key", api_key);
    http.addHeader("accept", "application/json");

    int httpResponseCode = http.GET();
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar jsonObject = JSON.parse(payload);

      if (JSON.typeof(jsonObject) == "undefined") {
        Serial.println("Parsing failed!");
        http.end();
        return;
      }
      // Log the JSON response
      Serial.print("raw JSON response: ");
      Serial.println(jsonObject);

      // jsonObject["results"] is an array. Since we specified limit=1 in the API URL above, we know that there is only one entry in the array and we can access it with ["results"][0]
      o3Value = double(jsonObject["results"][0]["value"]);
      o3Units = String(jsonObject["results"][0]["parameter"]["units"]);

      Serial.print("o3 Value: ");
      // Print the value with 3 decimal places
      Serial.print(o3Value, 3);
      Serial.print(" ");
      Serial.println(o3Units);
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
