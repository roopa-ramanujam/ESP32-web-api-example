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

*If you have not completed the prerequisites, please look at this tutorial

# Get the MAC Address of the ESP32

Each ESP32 device has a unique MAC address identifier, which we need to connect to the Berkeley-IoT WiFi network. 

To get your device MAC address, open and upload this sketch File->Examples->Basics->GetESP32MACAddress

In the Serial monitor, you should see a line printed that looks like this:

``` [DEFAULT] ESP32 Board MAC Address: xx:xx:xx:xx:xx:xx```

The 'xx:xx:xx:xx:xx:xx' is your ESP32 MAC address. Copy/save it somewhere, you will need it for the next step.

# Connect to Berkeley-IoT network

# Test WiFi connection

## Set up the secrets.h file

# Run the Weather API example
