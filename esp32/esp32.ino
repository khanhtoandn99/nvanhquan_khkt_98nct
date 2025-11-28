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
#include <SoftwareSerial.h>

// HWSerial mapping. Avoid G3, G1 of Serial, and avoid G9, G10 of flash pins
#define PIN_SWSERIAL1_TX 4 //G4
#define PIN_SWSERIAL1_RX 0 //G0

#define PIN_SWSERIAL2_TX 17 // G17
#define PIN_SWSERIAL2_RX 16 // G16

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Dien Nuoc Xuan An";
char pass[] = "Anhquan@2011";

// ==================== SERIAL COMM SM ====================
// From ESP to Arduino:
#define CMD_LIGHT_STATE        "CMD:LIGHT=" // + 1-byte (char). "1"->ON, "0"->OFF
#define CMD_DOOR_STATE         "CMD:DOOR="  // + 1-byte (char). "1"->OPEN, "0"->CLOSE

// From Arduino to ESP
#define ACK_OK                 "ACK:OK"

#define EV_LIGHT_STATE         "EV:LIGHT=" // + 1-byte (char). "1"->ON, "0"->OFF
#define EV_DOOR_STATE          "EV:DOOR=" // + 1-byte (char). "1"->ON, "0"->OFF
#define EV_RAINCOVER_STATE     "EV:RAINCOVER=" // + 1-byte (char). "1"->ON, "0"->OFF
#define EV_FIREPUMP_STATE      "EV:FIREPUMP=" // + 1-byte (char). "1"->ON, "0"->OFF
#define EV_GARDENPUMP_STATE    "EV:GARDENPUMP=" // + 1-byte (char). "1"->ON, "0"->OFF
#define EV_TEMP_VALUE          "EV:TEMP=" // + 2-byte (char). Ex: "24" degree
#define EV_HUMI_VALUE          "EV:HUMI=" // + 2-byte (char). Ex: "72" %

// SoftwareSerial arduino1Serial(PIN_SWSERIAL1_RX, PIN_SWSERIAL1_TX);
// SoftwareSerial arduino2Serial(PIN_SWSERIAL2_RX, PIN_SWSERIAL2_TX);
#define arduino1Serial Serial1
#define arduino2Serial Serial2

// BLYNK_WRITE from Web/App
BLYNK_WRITE(V0)
{
  Serial.println("Living light request: " + String(param.asInt()));
  arduino2Serial.println(CMD_LIGHT_STATE+String(param.asInt()));
  Serial.println(CMD_LIGHT_STATE+String(param.asInt()));
}

BLYNK_WRITE(V1)
{
  Serial.println("Door request: " + String(param.asInt()));
  arduino1Serial.println(CMD_DOOR_STATE+String(param.asInt()));
  Serial.println(CMD_DOOR_STATE+String(param.asInt()));
}

void handleArduino1Event(String ev) {
  ev.trim();
  Serial.println("handleArduino1Event: " + ev);
  if (ev.startsWith(EV_DOOR_STATE)) {
      if (ev.substring(String(EV_DOOR_STATE).length()).toInt() == 1) {
        Blynk.virtualWrite(V1, 1);
      } else {
        Blynk.virtualWrite(V1, 0);
      }
  }

  else if (ev.startsWith(EV_TEMP_VALUE)) {
      int temp = ev.substring(String(EV_TEMP_VALUE).length()).toInt();
      Blynk.virtualWrite(V2, temp);
  }

  else if (ev.startsWith(EV_HUMI_VALUE)) {
      int humi = ev.substring(String(EV_HUMI_VALUE).length()).toInt();
      Blynk.virtualWrite(V3, humi);
  }
}

void handleArduino2Event(String ev) {
  ev.trim();
  Serial.println("handleArduino2Event: " + ev);

  if (ev.startsWith(CMD_LIGHT_STATE)) {
      if (ev.substring(String(CMD_LIGHT_STATE).length()).toInt() == 1) {
        Blynk.virtualWrite(V0, 1);
      } else {
        Blynk.virtualWrite(V0, 0);
      }
  }

  else if (ev.startsWith(EV_RAINCOVER_STATE)) {
    if (ev.substring(String(EV_RAINCOVER_STATE).length()).toInt() == 1) {
      Serial.println("Update EV_RAINCOVER_STATE=1 to server");
      Blynk.virtualWrite(V4, 1);
    } else {
      Serial.println("Update EV_RAINCOVER_STATE=0 to server");
      Blynk.virtualWrite(V4, 0);
    }
  }

  else if (ev.startsWith(EV_FIREPUMP_STATE)) {
    if (ev.substring(String(EV_FIREPUMP_STATE).length()).toInt() == 1) {
      Blynk.virtualWrite(V5, 1);
    } else {
      Blynk.virtualWrite(V5, 0);
    }
  }

  else if (ev.startsWith(EV_GARDENPUMP_STATE)) {
    if (ev.substring(String(EV_GARDENPUMP_STATE).length()).toInt() == 1) {
      Blynk.virtualWrite(V6, 1);
    } else {
      Blynk.virtualWrite(V6, 0);
    }
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  arduino1Serial.begin(9600, SERIAL_8N1, PIN_SWSERIAL1_RX, PIN_SWSERIAL1_TX);
  arduino2Serial.begin(9600, SERIAL_8N1, PIN_SWSERIAL2_RX, PIN_SWSERIAL2_TX);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
  Blynk.run();
  if (arduino1Serial.available()) {
    String ev = arduino1Serial.readStringUntil('\n');
    handleArduino1Event(ev);
  }
  if (arduino2Serial.available()) {
    String ev = arduino2Serial.readStringUntil('\n');
    handleArduino2Event(ev);
  }
}

