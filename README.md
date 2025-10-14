# ESP32 Web API Example

# Introduction
In this tutorial, we’re going to connect our ESP32s to WiFi, receive data about Jacobs Hall from a weather API endpoint, and display the temperature and wind speed through a simple ambient display. We will be connecting to WiFi through the Berkeley IoT network, which is recommended for IoT (Internet of Things) devices like microcontrollers.

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

To get your device MAC address, copy the code below into a new Arduino sketch or download and open this example sketch: [GetESP32MACAddress.ino](https://github.com/roopa-ramanujam/ESP32-web-api-example/blob/main/GetESP32MACAddress.ino)

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

Copy the code below into a new Arduino sketch or download and open this example sketch: [WiFiTest.ino](https://github.com/roopa-ramanujam/ESP32-web-api-example/blob/main/WiFiTest.ino)

```cpp
#include <WiFi.h>
#include <secrets.h>

// WiFi credentials
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

We will be setting up a ```secrets.h``` file, which is a separate file used to store sensitive information such as your WiFi credentials and API keys. Keeping these details in a ```secrets.h``` file helps protect your private information, especially if you share your code publicly or work in a team. It also keeps your main code cleaner and makes it easier to update your credentials without changing the core program. In software development more broadly, this idea is called secrets management: the practice of securely storing, accessing, and rotating sensitive credentials (API keys, tokens, certificates) so they’re never hard-coded or exposed. Good secrets management helps prevent unauthorized access, protects user data, and supports safer collaboration across systems and teams.

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

## Run the WiFi test code

Compile + upload the sketch to your ESP32. You should see something like this if the WiFi successfully connects:

<img width="342" height="55" alt="image" src="https://github.com/user-attachments/assets/ad6dd697-186d-49d3-b129-499b81a956d4" />


# Weather API example

## Wire up your ESP32 with the RGB LED

We will change the color of the LED and its blink rate based on the temperature and wind speed at Jacobs Hall.

<img width="784" height="716" alt="image" src="https://github.com/user-attachments/assets/ef2aba43-2ff9-435e-b12e-f43985cc0a9e" />

## Set up the code

Copy the code below into a new Arduino sketch or download and open this example sketch: [WeatherAPIExample.ino](https://github.com/roopa-ramanujam/ESP32-web-api-example/blob/main/WeatherAPIExample.ino)

```cpp
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



```

Create another ```secrets.h``` file for this sketch with your Berkeley-IoT credentials, just like in the WiFi test sketch.

Install Arduino_JSON from the libraries manager.

<img width="257" height="300" alt="image" src="https://github.com/user-attachments/assets/d3c9b893-6b18-4547-a4d5-ff78e39e74d3" />

Compile + upload the sketch. You should see something like this in the Serial monitor:

<img width="1258" height="210" alt="image" src="https://github.com/user-attachments/assets/8e871777-cf16-46f0-9153-f933f553f9b0" />

Your LED should be red, green, or blue, depending on the temperature, and blinking depending on the wind speed.

That's it! You have successfully connected your ESP32 to WiFi and fetched data from a weather API endpoint.

# Troubleshooting

## WiFi not connecting (stuck on "Connecting to WiFi...")
- Double-check that your ESP32’s MAC address is correctly registered on the Berkeley-IoT portal
- Make sure you are connecting to "Berkeley-IoT" and not "eduroam" or "CalVisitor".
- Verify that your secrets.h file has the correct SSID ("Berkeley-IoT") and the exact password you copied from the IoT registration page (no extra spaces or quotation mark errors).
- If connection still fails, open Tools → Serial Monitor and press the RESET button on your ESP32 to see fresh connection logs.

## HTTP Error Code -1 or 400/401/403
This usually means the ESP32 couldn’t reach the weather API. Try pasting this link into your web browser: http://api.open-meteo.com/v1/forecast?latitude=37.876270&longitude=-122.258499&current_weather=true&temperature_unit=fahrenheit

If you see a response that looks like this, then the API endpoint is not the issue and it has something to do with your code trying to connect to the endpoint.

<img width="1564" height="128" alt="image" src="https://github.com/user-attachments/assets/f098db12-0fa5-4306-99f7-0182cfcb7d9f" />

Try resetting the ESP32 or adding code to print out the WiFi connection status before attempting to send the HTTP GET request (you can ask ChatGPT for help with this).

## Serial Monitor shows “Failed to parse JSON”
-This means the weather API response didn’t match the expected format.
-Try printing the full API response using Serial.println(payload); right before parsing — this helps confirm whether the API returned valid JSON or an error message.
-Verify that ArduinoJson is installed and up to date (Library Manager → search “ArduinoJson” → install version 6 or newer).

## LED not lighting up
-Ensure your RGB LED is wired correctly: Common cathode → GND. Red, Green, and Blue pins each go through a 220 Ω resistor to your specified pins (14, 27, 33).
-Try testing with a simpler sketch that just lights each color individually to confirm wiring.
-Make sure your ESP32 board is selected under Tools → Board → Adafruit ESP32 Feather V2.

## Compilation errors about missing secrets.h
-Make sure the file name is exactly secrets.h (not .txt or saved in another folder).
-It must be in the same sketch folder as your .ino file.

# Advanced: Using APIs with API keys

# Additional Resources/Reading

[HTTP response code reference](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status)







