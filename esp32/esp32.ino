/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP32 chip.

  NOTE: This requires ESP32 support package:
    https://github.com/espressif/arduino-esp32

  Please be sure to select the right ESP32 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6EYdTlCvW"
#define BLYNK_TEMPLATE_NAME         "Smarthome NCT2025"
#define BLYNK_AUTH_TOKEN            "8TkXn02WS5zlRidcQDuhoRwuKwQELPXs"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Dien Nuoc Xuan An";
char pass[] = "Anhquan@2011";

// Request from Web/App
BLYNK_WRITE(V0)
{
  Serial.println("Living light request: " + String(param.asInt()));
}

BLYNK_WRITE(V1)
{
  Serial.println("Door request: " + String(param.asInt()));
}

unsigned long lastTimer = 0;

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
  Blynk.run();
  if (millis() - lastTimer > 3000) {
    Blynk.virtualWrite(V1, 0); // Send update from device to app
    lastTimer = millis();
    // Serial.println("Door closed!");
  }
}

