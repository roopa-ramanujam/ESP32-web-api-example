#include <WiFi.h>
#include <secrets.h>

// Replace with your network credentials
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Connecting to WiFi...");

  // Start WiFi connection
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Nothing here — just connected to WiFi
}