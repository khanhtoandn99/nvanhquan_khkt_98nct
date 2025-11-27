#include <Servo.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// Read pins
#define PIN_RAIN_SENSOR 2
#define PIN_GAS_SENSOR 4
#define PIN_IR_SENDOR 5
#define PIN_SOIL_MOISTURE_SENSOR 6
// Write pins
#define PIN_RAIN_COVER_RELAY 7
#define PIN_FIRE_PUMP_RELAY 8
#define PIN_GARDEN_PUMP_RELAY 9
#define PIN_RELAY 10 // reserve
// PWM pins
#define PIN_SERVO_RAIN_COVER 3

#define STATE_RAINING LOW
#define STATE_BURNING LOW
#define STATE_GOOD    LOW


Servo servoRainCover;
int servoRainCoverPos = 180;

void expandRainCover() {
  for (; servoRainCoverPos >= 90; servoRainCoverPos -= 1) { // goes from 0 degrees to 180 degrees
    servoRainCover.write(servoRainCoverPos);
    vTaskDelay(15 / portTICK_PERIOD_MS);
  }
}
void colapseRainCover() {
  for (; servoRainCoverPos < 180; servoRainCoverPos += 1) { // goes from 0 degrees to 180 degrees
    servoRainCover.write(servoRainCoverPos);
    vTaskDelay(15 / portTICK_PERIOD_MS);
  }
}

void TaskRainCoverController(void *pvParameters) {
  Serial.println("TaskRainCoverController Started!");
  TickType_t delayTime = *((TickType_t*)pvParameters); // Use task parameters to define delay
  bool bRainCoverOpened = true;
  unsigned long lastTime = millis();

  for (;;)
  {
    unsigned long currentTime = millis();

    if (currentTime - lastTime < 3000) continue;

    if (digitalRead(PIN_RAIN_SENSOR) == STATE_RAINING && bRainCoverOpened == true) {
      expandRainCover();
      bRainCoverOpened = false;
      Serial.println("Rain cover closed!");
    }

    if (digitalRead(PIN_RAIN_SENSOR) != STATE_RAINING && bRainCoverOpened == false) {
      colapseRainCover();
      bRainCoverOpened = true;
      Serial.println("Rain cover opened!");
    }

    lastTime = millis();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskFireProtectionController(void *pvParameters) {
  Serial.println("TaskFireProtectionController Started!");
  for (;;)
  {
    if (digitalRead(PIN_GAS_SENSOR) == STATE_BURNING || digitalRead(PIN_IR_SENDOR) == STATE_BURNING) {
      digitalWrite(PIN_FIRE_PUMP_RELAY, HIGH);
      Serial.println("Fire pump running!");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
    else {
      digitalWrite(PIN_FIRE_PUMP_RELAY, LOW);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskGardenCareController(void *pvParameters) {
  Serial.println("TaskGardenCareController Started!");
  for (;;)
  {
    if (digitalRead(PIN_SOIL_MOISTURE_SENSOR) != STATE_GOOD) {
      digitalWrite(PIN_GARDEN_PUMP_RELAY, HIGH);
      Serial.println("Garden pump running!");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
    else {
      digitalWrite(PIN_GARDEN_PUMP_RELAY, LOW);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(PIN_RAIN_SENSOR, INPUT);
  pinMode(PIN_GAS_SENSOR, INPUT);
  pinMode(PIN_IR_SENDOR, INPUT);
  pinMode(PIN_SOIL_MOISTURE_SENSOR, INPUT);

  pinMode(PIN_RAIN_COVER_RELAY, OUTPUT);
  pinMode(PIN_FIRE_PUMP_RELAY, OUTPUT);
  pinMode(PIN_GARDEN_PUMP_RELAY, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);

  digitalWrite(PIN_RAIN_COVER_RELAY, LOW);
  digitalWrite(PIN_FIRE_PUMP_RELAY, LOW);
  digitalWrite(PIN_GARDEN_PUMP_RELAY, LOW);
  digitalWrite(PIN_RELAY, LOW);

  servoRainCover.attach(PIN_SERVO_RAIN_COVER);
  servoRainCover.write(180);

  Serial.println("Started!");

  xTaskCreate(TaskRainCoverController, // Task function
              "TaskRainCoverController", // Task name for humans
              128, 
              1000, // Task parameter
              1, // Task priority
              NULL);

  xTaskCreate(TaskFireProtectionController, // Task function
              "TaskFireProtectionController", // Task name for humans
              128, 
              1000, // Task parameter
              1, // Task priority
              NULL);

  xTaskCreate(TaskGardenCareController, // Task function
              "TaskGardenCareController", // Task name for humans
              128, 
              1000, // Task parameter
              1, // Task priority
              NULL);
}

void loop() {
}
