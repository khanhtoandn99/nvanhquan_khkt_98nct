#include <Servo.h>
#include <SoftwareSerial.h>

// ==================== PIN DEFINE ====================
#define PIN_RAIN_SENSOR            2
#define PIN_GAS_SENSOR             4
#define PIN_IR_SENDOR              5
#define PIN_SOIL_MOISTURE_SENSOR   6

#define PIN_RAIN_COVER_RELAY       7
#define PIN_FIRE_PUMP_RELAY        8
#define PIN_GARDEN_PUMP_RELAY      9
#define PIN_RELAY                 10

#define PIN_SERVO_RAIN_COVER       3

// Avoid A4,A5 of I2C pins
#define PIN_SWSERIAL_TX            A2
#define PIN_SWSERIAL_RX            A3

// ==================== SENSOR STATE ====================
#define STATE_RAINING              LOW
#define STATE_BURNING              LOW
#define STATE_GOOD                 LOW


// ==================== SERVO CONTROL ====================
Servo servoRainCover;
int servoRainCoverPos = 180; // 180 = colapsed, 90 = expanded


void moveServoTo(int target);

// ==================== RAIN COVER SM ====================
enum RainCoverState {
  RAIN_IDLE,
  RAIN_EXPANDING,
  RAIN_COLAPSING
};

RainCoverState rainState = RAIN_IDLE;
unsigned long rainLastCheck = 0;

void expandRainCover();
void colapseRainCover();
void RainCoverSM();

// ==================== FIRE PROTECTION SM ====================
enum FireState {
  FIRE_IDLE,
  FIRE_RUNNING
};

FireState fireState = FIRE_IDLE;
unsigned long fireStartTime = 0;

void FireProtectionSM();

// ==================== GARDEN CARE SM ====================
enum GardenState {
  GARDEN_IDLE,
  GARDEN_RUNNING
};

GardenState gardenState = GARDEN_IDLE;
unsigned long gardenStartTime = 0;

void GardenCareSM();


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

SoftwareSerial espSerial(PIN_SWSERIAL_RX, PIN_SWSERIAL_TX);

void sendToESP(const String &msg);
void handleEspCommand(String cmd);

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(PIN_RAIN_SENSOR, INPUT_PULLUP);
  pinMode(PIN_GAS_SENSOR, INPUT_PULLUP);
  pinMode(PIN_IR_SENDOR, INPUT_PULLUP);
  pinMode(PIN_SOIL_MOISTURE_SENSOR, INPUT_PULLUP);

  pinMode(PIN_RAIN_COVER_RELAY, OUTPUT);
  pinMode(PIN_FIRE_PUMP_RELAY, OUTPUT);
  pinMode(PIN_GARDEN_PUMP_RELAY, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);

  digitalWrite(PIN_RAIN_COVER_RELAY, LOW);
  digitalWrite(PIN_FIRE_PUMP_RELAY, LOW);
  digitalWrite(PIN_GARDEN_PUMP_RELAY, LOW);
  digitalWrite(PIN_RELAY, LOW);

  servoRainCover.attach(PIN_SERVO_RAIN_COVER);
  servoRainCover.write(servoRainCoverPos);

  Serial.println("System Ready!");
}

// ==================== LOOP ====================
void loop() {
  RainCoverSM();
  FireProtectionSM();
  GardenCareSM();
  if (espSerial.available()) {
    String cmd = espSerial.readStringUntil('\n');
    handleEspCommand(cmd);
  }
}

// ==================== FUNCTION DEFINITION ====================
void expandRainCover() {
  for (; servoRainCoverPos > 90; servoRainCoverPos -= 1) { // goes from 0 degrees to 180 degrees
    servoRainCover.write(servoRainCoverPos);
    delay(15);
  }
}
void colapseRainCover() {
  for (; servoRainCoverPos < 180; servoRainCoverPos += 1) { // goes from 0 degrees to 180 degrees
    servoRainCover.write(servoRainCoverPos);
    delay(15);
  }
}

void RainCoverSM() {
  if (millis() - rainLastCheck < 100) return;
  rainLastCheck = millis();

  bool raining = (digitalRead(PIN_RAIN_SENSOR) == STATE_RAINING);

  switch (rainState) {
    case RAIN_IDLE:
      if (raining && servoRainCoverPos > 90) {
        rainState = RAIN_EXPANDING;
        sendToESP(String(EV_RAINCOVER_STATE)+"1");
        Serial.println("Rain detected → expanding cover");
      }
      else if (!raining && servoRainCoverPos < 180) {
        rainState = RAIN_COLAPSING;
        sendToESP(String(EV_RAINCOVER_STATE)+"0");
        Serial.println("No rain → colapsing cover");
      }
      break;

    case RAIN_EXPANDING:
      expandRainCover();
      if (servoRainCoverPos <= 90) {
        Serial.println("Rain cover expanded!");
        rainState = RAIN_IDLE;
      }
      break;

    case RAIN_COLAPSING:
      colapseRainCover();
      if (servoRainCoverPos >= 180) {
        Serial.println("Rain cover colapsed!");
        rainState = RAIN_IDLE;
      }
      break;
  }
}

void FireProtectionSM() {
  bool fire = (digitalRead(PIN_GAS_SENSOR) == STATE_BURNING ||
               digitalRead(PIN_IR_SENDOR) == STATE_BURNING);

  switch (fireState) {
    case FIRE_IDLE:
      if (fire) {
        fireState = FIRE_RUNNING;
        fireStartTime = millis();
        digitalWrite(PIN_FIRE_PUMP_RELAY, HIGH);
        sendToESP(String(EV_FIREPUMP_STATE)+"1");
        Serial.println("Fire pump running!");
      }
      break;

    case FIRE_RUNNING:
      if (millis() - fireStartTime >= 3000) {
        digitalWrite(PIN_FIRE_PUMP_RELAY, LOW);
        sendToESP(String(EV_FIREPUMP_STATE)+"0");
        fireState = FIRE_IDLE;
      }
      break;
  }
}

void GardenCareSM() {
  bool dry = (digitalRead(PIN_SOIL_MOISTURE_SENSOR) != STATE_GOOD);

  switch (gardenState) {
    case GARDEN_IDLE:
      if (dry) {
        gardenState = GARDEN_RUNNING;
        gardenStartTime = millis();
        digitalWrite(PIN_GARDEN_PUMP_RELAY, HIGH);
        sendToESP(String(EV_GARDENPUMP_STATE)+"1");
        Serial.println("Garden pump running!");
      }
      break;

    case GARDEN_RUNNING:
      if (millis() - gardenStartTime >= 3000) {
        digitalWrite(PIN_GARDEN_PUMP_RELAY, LOW);
        sendToESP(String(EV_GARDENPUMP_STATE)+"0");
        gardenState = GARDEN_IDLE;
      }
      break;
  }
}

void sendToESP(const String &msg) {
  espSerial.println(msg);
}

void handleEspCommand(String cmd) {
  cmd.trim();
  Serial.println("handleEspCommand: " + cmd);

  if (cmd.startsWith(CMD_LIGHT_STATE)) {
    digitalWrite(PIN_RELAY,
        cmd.substring(String(CMD_LIGHT_STATE).length()).toInt());
  }
}