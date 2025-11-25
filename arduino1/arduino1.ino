#include <Servo.h>
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// Read pins
#define PIN_LDR_SENSOR1 A0
#define PIN_LDR_SENSOR2 A1
// Write pins
// PWM pins
#define PIN_SERVO_SOLAR_TRACKER 10

#define STATE_SUNNY LOW
#define STATE_NO_SUNNY HIGH

Servo servoSolarTracker;
int servoSolarTrackerPos = 90;

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


void TaskSolarTrackingController(void *pvParameters) {
  Serial.println("TaskSolarTrackingController Started!");
  for (;;)
  {
      int s1 = digitalRead(PIN_LDR_SENSOR1);
      int s2 = digitalRead(PIN_LDR_SENSOR2);

      if (s1 == STATE_NO_SUNNY && s2 == STATE_SUNNY) {
        while (digitalRead(PIN_LDR_SENSOR1) == STATE_NO_SUNNY || digitalRead(PIN_LDR_SENSOR2) == STATE_NO_SUNNY) {
          if (servoSolarTrackerPos < 180) {
            ++servoSolarTrackerPos;
            servoSolarTracker.write(servoSolarTrackerPos);
            vTaskDelay(50 / portTICK_PERIOD_MS);
          }
        }
      }
      else if (s2 == STATE_NO_SUNNY && s1 == STATE_SUNNY) {
        while (digitalRead(PIN_LDR_SENSOR1) == STATE_NO_SUNNY || digitalRead(PIN_LDR_SENSOR2) == STATE_NO_SUNNY) {
          if (servoSolarTrackerPos > 0) {
            --servoSolarTrackerPos;
            servoSolarTracker.write(servoSolarTrackerPos);
            vTaskDelay(50 / portTICK_PERIOD_MS);
          }
        }
      }
      else {
        vTaskDelay(20 / portTICK_PERIOD_MS);
      }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskDoorLockController(void *pvParameters) {
  Serial.println("TaskDoorLockController Started!");
  for (;;)
  {
    char key = kpad.getKey();
    if (key) {
      Serial.println(key);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(PIN_LDR_SENSOR1, INPUT);
  pinMode(PIN_LDR_SENSOR2, INPUT);

  servoSolarTracker.attach(PIN_SERVO_SOLAR_TRACKER);
  servoSolarTracker.write(servoSolarTrackerPos);

  Serial.println("Started!");

  xTaskCreate(TaskSolarTrackingController, // Task function
              "TaskSolarTrackingController", // Task name for humans
              128, 
              1000, // Task parameter
              1, // Task priority
              NULL);
  xTaskCreate(TaskDoorLockController, // Task function
              "TaskDoorLockController", // Task name for humans
              128, 
              1000, // Task parameter
              1, // Task priority
              NULL);
}

void loop() {
}
