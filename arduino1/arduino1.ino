#include <Servo.h>
#include <Keypad.h>
#include <DHT11.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// Read pins
#define PIN_LDR_SENSOR1 A0
#define PIN_LDR_SENSOR2 A1
// Write pins
// PWM pins
#define PIN_SERVO_DOOR 10
#define PIN_SERVO_SOLAR_TRACKER 11
// Digital Communication pins
#define PIN_DHT11 12

#define STATE_SUNNY LOW
#define STATE_NO_SUNNY HIGH

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

DHT11 dht11(PIN_DHT11);

Servo servoDoor;

Servo servoSolarTracker;
int servoSolarTrackerPos = 90;

void openDoor();
void closeDoor();

void openDoor() {
  for (int pos = 0; pos <= 90; pos += 1) {
    servoDoor.write(pos);
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void closeDoor() {
  for (int pos = 90; pos > 0; pos -= 1) {
    servoDoor.write(pos);
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}


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

void TaskTempHumiReaderController(void *pvParameters) {
  Serial.println("TaskTempHumiReaderController Started!");
  for (;;)
  {
    int temperature = 0;
    int humidity = 0;
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    if (result == 0) {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C\tHumidity: ");
        Serial.print(humidity);
        Serial.println(" %");
    } else {
        Serial.println(DHT11::getErrorString(result));
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(PIN_LDR_SENSOR1, INPUT);
  pinMode(PIN_LDR_SENSOR2, INPUT);

  servoDoor.attach(PIN_SERVO_DOOR);
  servoDoor.write(0);

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
  xTaskCreate(TaskTempHumiReaderController, // Task function
              "TaskTempHumiReaderController", // Task name for humans
              128, 
              1000, // Task parameter
              1, // Task priority
              NULL);
}

void loop() {
}
