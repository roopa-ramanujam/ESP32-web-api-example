# ESP32 Web API Example

# Introduction
In this tutorial, we’re going to connect our ESP32s to WiFi, receive data from a weather API endpoint, and show it with a simple ambient display.

Ingredients:
- ESP32 with pins soldered
- USB-C cable
- Breadboard
- RGB LED (1)
- 220 ohm resistors (3)
- Jumper wires

Prerequisites*
1. ESP32 pins have been soldered.
2. You can successfully flash code to the ESP32.

*If you have not completed the prerequisites, please look at [Sudhu's tutorial](https://github.com/loopstick/ESP32_V2_Tutorial/tree/master) 

# Get the MAC Address of the ESP32

Each ESP32 device has a unique MAC address identifier, which we need to connect to the Berkeley-IoT WiFi network. 

To get your device MAC address, copy the code below into a new Arduino sketch or download and open this example sketch: GetESP32MACAddress.ino

```cpp
/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <WiFi.h>
#include <esp_wifi.h>

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setup(){
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();

  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
}
 
void loop(){

}
```

In the Serial monitor, you should see a line printed that looks like this:

``` [DEFAULT] ESP32 Board MAC Address: xx:xx:xx:xx:xx:xx```

The 'xx:xx:xx:xx:xx:xx' is your ESP32 MAC address. Copy/save it somewhere, you will need it for the next step.

# Register your ESP32 with the Berkeley-IoT network

1. Go to https://wifi-keys.berkeley.edu/.
2. Click on "Manage devices" > "Create New".
3. Paste in the MAC address from the previous step. Give your device a descriptive name, like "Roopa's ESP32".
4. *VERY IMPORTANT*: Copy/save the "New password" string that's been generated for you. You won't be able to see this password again, and you need it to connect your ESP32 to WiFi.
5. Click "Save device"

<img width="956" height="547" alt="image" src="https://github.com/user-attachments/assets/0265d71d-bfe4-47b9-a7b1-279b1f0f2ea2" />

You should now see your ESP32 in the list of registered devices.

<img width="1089" height="332" alt="image" src="https://github.com/user-attachments/assets/794fb8b0-6a52-4900-a274-1ce17a0a4d54" />


# Test Berkeley-IoT WiFi connection

Copy the code below into a new Arduino sketch or download and open this example sketch: WiFiTest.ino

```cpp
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
```

## Set up the secrets.h file

The secrets.h file is like a password storage; 

Click on the three dots under the Serial toolbar and click on "New tab", name it "secrets.h", and hit "Ok"

   <img width="211" height="271" alt="image" src="https://github.com/user-attachments/assets/933f18e6-a367-49b7-ac2c-25171458e4ee" />

   <img width="465" height="160" alt="image" src="https://github.com/user-attachments/assets/d2c98d99-2421-44af-a242-aae383e3a8f5" />
   

In secrets.h, paste in the following variables to represent your network name and password.

```cpp
#define SECRET_SSID "Berkeley-IoT"
#define SECRET_PASSWORD "your-password"
```

Replace 'your-password' with the password you copied from the Berkeley IoT registration step.

Your ```secrets.h``` file is included in the WiFi test sketch with this line:

```cpp
#include <secrets.h>
```

Now the sketch can put in your password without you have abstracted your password away from the Arduino IDE!

## Run the WiFi test code

Compile + upload the sketch to your ESP32. You should see something like this if the WiFi successfully connects:

<img width="342" height="55" alt="image" src="https://github.com/user-attachments/assets/ad6dd697-186d-49d3-b129-499b81a956d4" />


# Weather API example

## Wire up your ESP32 with the RGB LED

We will change the color of the LED and its blink rate based on the temperature and wind speed at Jacobs Hall.

<img width="784" height="716" alt="image" src="https://github.com/user-attachments/assets/ef2aba43-2ff9-435e-b12e-f43985cc0a9e" />

## Set up the code

Copy the code below into a new Arduino sketch or download and open this example sketch: WeatherAPIExample.ino

```cpp
#include <arduino_secrets.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

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
float currentTemp = 0;
float currentWind = 0;
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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ---------- Fetch weather ----------
void getWeatherData() {
  Serial.print("Requesting: ");
  Serial.println(weather_api);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, weather_api);
    int httpCode = http.GET();

    Serial.print("HTTP code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();

      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        currentTemp = doc["current_weather"]["temperature"];
        currentWind = doc["current_weather"]["windspeed"];

        Serial.print("🌡 Temp: ");
        Serial.print(currentTemp);
        Serial.println(" °F");

        Serial.print("💨 Wind: ");
        Serial.print(currentWind);
        Serial.println(" mph");
      } else {
        Serial.println("Failed to parse JSON");
      }
    } else {
      Serial.print("HTTP error: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.reconnect();
  }
}

// ---------- LED behavior ----------
void visualizeWeather(float tempF, float windspeed) {
  // Base color by temperature
  int red = 0, green = 0, blue = 0;
  if (tempF <= 65)       blue = 255;   // cold
  else if (tempF <= 80)  green = 255;  // mild
  else                   red = 255;    // hot

  // Map windspeed (0–30 mph) → blink rate (slower = calm, faster = windy)
  int blinkDelay = constrain(map((int)windspeed, 0, 30, 800, 100), 100, 800);

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

```

Create another ```secrets.h``` file for this sketch with your Berkeley-IoT credentials, just like in the WiFi test sketch.

Install ArduinoJson from the libraries manager.

<img width="273" height="324" alt="image" src="https://github.com/user-attachments/assets/b107d57d-9fcb-4b1c-b7a4-c9eabaab30f5" />


Compile + upload the sketch. You should see something like this in the Serial monitor:

<img width="1258" height="210" alt="image" src="https://github.com/user-attachments/assets/8e871777-cf16-46f0-9153-f933f553f9b0" />

Your LED should be red, green, or blue, depending on the temperature, and blinking depending on the wind speed.

That's it! You have successfully connected your ESP32 to WiFi and fetched data from a weather API endpoint.

# Troubleshooting

1. 

# Advanced: Using APIs with API keys

# Additional Resources/Reading





