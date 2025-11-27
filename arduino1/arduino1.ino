#include <Servo.h>
#include <Keypad.h>
#include <DHT11.h>
#include <LiquidCrystal_I2C.h>

// ===== PIN DEFINE =====
#define PIN_LDR_SENSOR1 A0
#define PIN_LDR_SENSOR2 A1

#define PIN_SERVO_DOOR 10
#define PIN_SERVO_SOLAR_TRACKER 11

#define PIN_DHT11 12

#define STATE_SUNNY LOW
#define STATE_NO_SUNNY HIGH

enum E_LCD_STATE {
  E_LCD_STATE_TEMPHUMI,
  E_LCD_STATE_INPUT_PWD,
  E_LCD_STATE_CORRECT_PWD,
  E_LCD_STATE_WRONG_PWD
};

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

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// ===== GLOBAL STATE =====
int solarPos = 90;
String passwordInput = "";
const String CorrectPassword = "2222"; // Mat khau cua
E_LCD_STATE lcdState = E_LCD_STATE_TEMPHUMI;
int temp = 26, humi = 71;
int doorPos = 60;

// Timers cho non-blocking
unsigned long lastDHTRead = 0;
unsigned long lastSolarCheck = 0;
unsigned long lastKeypadCheck = 0;
unsigned long lastLcdCheck = 0;
unsigned long lastKeyTime = 0;
unsigned long lastLcdStateTime = 0;


// Functions
void openDoor();
void closeDoor();
void solarTrackingUpdate();
void showLcdTempHumi();
void updateLCDState();

void setup() {
  Serial.begin(9600);
  Serial.println("System start!");

  pinMode(PIN_LDR_SENSOR1, INPUT);
  pinMode(PIN_LDR_SENSOR2, INPUT);

  servoDoor.attach(PIN_SERVO_DOOR);
  void closeDoor();

  servoSolarTracker.attach(PIN_SERVO_SOLAR_TRACKER);
  servoSolarTracker.write(solarPos);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hello, welcome!");

  Serial.println("Initialized!");
}

void loop() {
  // ====== DHT READER EVERY 5s ======
  if (millis() - lastDHTRead >= 5000) {
    lastDHTRead = millis();

    int result = dht11.readTemperatureHumidity(temp, humi); /** WARNING: this block 1sec */

    if (result == 0) {
      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print(" C\tHumi: ");
      Serial.print(humi);
      Serial.println(" %");
    } else {
      Serial.println(DHT11::getErrorString(result));
    }
  }

  // ===== SOLAR TRACKING every 100ms =====
  if (millis() - lastSolarCheck >= 20) {
    // Serial.println(" lastSolarCheck");
    lastSolarCheck = millis();
    solarTrackingUpdate();
  }

  // ===== LCD update every 100ms =====
  if (millis() - lastLcdCheck >= 200) {
    lastLcdCheck = millis();
    updateLCDState();
  }
}


// ===============================================================
// ====================== FUNCTION ZONE ==========================
// ===============================================================

// Smooth open
void openDoor() {
  if (doorPos >= 60) return;
  for (; doorPos <= 60; doorPos+=1) {
    servoDoor.write(doorPos);
    delay(15);
  }
}

// Smooth close
void closeDoor() {
  if (doorPos <= 0) return;
  for (; doorPos > 0; doorPos-=1) {
    servoDoor.write(doorPos);
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

void showLcdTempHumi() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("-Smart Home NCT-");
  lcd.setCursor(0,1);
  lcd.print("Temp:"+String(temp)+"  Humi:"+String(humi));
  lcd.print(temp, 1);
}

void updateLCDState() {
  char key = kpad.getKey();
  switch (lcdState) {
    case E_LCD_STATE_TEMPHUMI:
      if (millis() - lastLcdStateTime > 500) {
        lastLcdStateTime = millis();
        showLcdTempHumi();
        closeDoor();
      }
      if (key) {
        lcdState = E_LCD_STATE_INPUT_PWD;
        passwordInput = "";
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Input password:");
        lastKeyTime = millis();
      }
      break;

    case E_LCD_STATE_INPUT_PWD:
      if (key) {
        lastKeyTime = millis();
        passwordInput += key;
        lcd.setCursor(0,1);
        lcd.print(passwordInput);
      }

      if (millis() - lastKeyTime >= 3000) {
        lcdState = E_LCD_STATE_TEMPHUMI;
        lastLcdStateTime = millis();
        showLcdTempHumi();
        closeDoor();
      }

      if (passwordInput.length() == CorrectPassword.length()) {
        if (passwordInput.equals(CorrectPassword)) {
          lcdState = E_LCD_STATE_CORRECT_PWD;
          lastLcdStateTime = millis();
          lcd.clear();
          lcd.print("Success");
          openDoor();
        } else {
          lcdState = E_LCD_STATE_WRONG_PWD;
          lastLcdStateTime = millis();
          lcd.clear();
          lcd.print("Wrong!");
        }
      }
      break;

    case E_LCD_STATE_CORRECT_PWD:
      if (millis() - lastLcdStateTime >= 2000) {
        lcdState = E_LCD_STATE_TEMPHUMI;
        lastLcdStateTime = millis();
        showLcdTempHumi();
        closeDoor();
      }
      break;

    case E_LCD_STATE_WRONG_PWD:
      if (millis() - lastLcdStateTime >= 2000) {
        lcdState = E_LCD_STATE_TEMPHUMI;
        lastLcdStateTime = millis();
        showLcdTempHumi();
        closeDoor();
      }
      break;
  }
}
