#include <Servo.h>
#include <Keypad.h>
#include <DHT11.h>

// ===== PIN DEFINE =====
#define PIN_LDR_SENSOR1 A0
#define PIN_LDR_SENSOR2 A1

#define PIN_SERVO_DOOR 10
#define PIN_SERVO_SOLAR_TRACKER 11

#define PIN_DHT11 12

#define STATE_SUNNY LOW
#define STATE_NO_SUNNY HIGH

// ===== KEYPAD =====
const byte KPAD_ROWS_MAX = 4;
const byte KPAD_COLS_MAX = 4;
char keys[KPAD_ROWS_MAX][KPAD_COLS_MAX] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte kpadRowPins[KPAD_ROWS_MAX] = {9,8,7,6};
byte kpadColPins[KPAD_COLS_MAX] = {5,4,3,2};
Keypad kpad = Keypad( makeKeymap(keys), kpadRowPins, kpadColPins, KPAD_ROWS_MAX, KPAD_COLS_MAX );

// ===== DHT =====
DHT11 dht11(PIN_DHT11);

// ===== SERVO =====
Servo servoDoor;
Servo servoSolarTracker;

// ===== GLOBAL STATE =====
int solarPos = 90;

// Timers cho non-blocking
unsigned long lastDHTRead = 0;
unsigned long lastSolarCheck = 0;
unsigned long lastKeypadCheck = 0;

// Functions
void openDoor();
void closeDoor();
void solarTrackingUpdate();

void setup() {
  Serial.begin(9600);
  Serial.println("System start!");

  pinMode(PIN_LDR_SENSOR1, INPUT);
  pinMode(PIN_LDR_SENSOR2, INPUT);

  servoDoor.attach(PIN_SERVO_DOOR);
  servoDoor.write(0);

  servoSolarTracker.attach(PIN_SERVO_SOLAR_TRACKER);
  servoSolarTracker.write(solarPos);

  // Serial.println("Initialized!");
}

void loop() {
  // ====== DHT READER EVERY 1s ======
  if (millis() - lastDHTRead >= 5000) {
    lastDHTRead = millis();

    int temp = 0, hum = 0;
    int result = dht11.readTemperatureHumidity(temp, hum); /** WARNING: this block 1sec */

    if (result == 0) {
      // Serial.print("Temp: ");
      // Serial.print(temp);
      // Serial.print(" C\tHumi: ");
      // Serial.print(hum);
      // Serial.println(" %");
    } else {
      // Serial.println(DHT11::getErrorString(result));
    }
  }

  // ===== KEYPAD SCAN every 50ms =====
  if (millis() - lastKeypadCheck >= 10) {
    // Serial.println(" lastKeypadCheck");
    lastKeypadCheck = millis();
    char key = kpad.getKey();
    if (key) {
      Serial.print("Key: ");
      Serial.println(key);
    }
  }

  // ===== SOLAR TRACKING every 100ms =====
  if (millis() - lastSolarCheck >= 10) {
    // Serial.println(" lastSolarCheck");
    lastSolarCheck = millis();
    solarTrackingUpdate();
  }
}


// ===============================================================
// ====================== FUNCTION ZONE ==========================
// ===============================================================

// Smooth open
void openDoor() {
  for (int pos = 0; pos <= 90; pos+=10) {
    servoDoor.write(pos);
    delay(15);
  }
}

// Smooth close
void closeDoor() {
  for (int pos = 90; pos >= 0; pos-=10) {
    servoDoor.write(pos);
    delay(15);
  }
}


// Solar tracker logic
void solarTrackingUpdate() {
  int s1 = digitalRead(PIN_LDR_SENSOR1);
  int s2 = digitalRead(PIN_LDR_SENSOR2);

  // Light on right → sensor1 = HIGH?
  if (s1 == STATE_NO_SUNNY && s2 == STATE_SUNNY) {
    if (solarPos < 180) {
      solarPos += 1;
      servoSolarTracker.write(solarPos);
    }
  }
  // Light on left
  else if (s2 == STATE_NO_SUNNY && s1 == STATE_SUNNY) {
    if (solarPos > 0) {
      solarPos -= 1;
      servoSolarTracker.write(solarPos);
    }
  }
}
