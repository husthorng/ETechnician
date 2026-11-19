//https://chatgpt.com/c/6afb838d-8a28-83ee-9de9-b2e4babb94d5
#include <Arduino.h>

// ===== GPIO 設定 =====
#define PB1   16

#define LED1  13
#define LED2  12
#define LED3  14

// ===== 時間設定 =====
#define DEBOUNCE_TIME  50
#define LONG_PRESS_TIME 1000
#define LED_TIME       5000

// ===== 系統狀態 =====
bool systemRunning = false;

bool buttonState = HIGH;
bool lastButtonReading = HIGH;

unsigned long debounceTime = 0;
unsigned long pressStartTime = 0;

bool longPressHandled = false;

// ===== LED 循環控制 =====
int ledStep = 0;

unsigned long ledStartTime = 0;

// ===== 關閉所有 LED =====
void allLEDOff() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}

// ===== 啟動 LED 循環 =====
void startSystem() {

  systemRunning = true;

  ledStep = 0;
  ledStartTime = millis();

  allLEDOff();

  digitalWrite(LED1, HIGH);

  Serial.println("System START");
  Serial.println("LED1 ON");
}

// ===== 關閉系統 =====
void stopSystem() {

  systemRunning = false;

  allLEDOff();

  Serial.println("System STOP");
  Serial.println("All LED OFF");
}

// ===== LED 循環控制 =====
void ledSequence() {

  if (!systemRunning) {
    return;
  }

  unsigned long currentTime = millis();

  if (currentTime - ledStartTime >= LED_TIME) {

    ledStartTime = currentTime;

    allLEDOff();

    ledStep++;

    if (ledStep >= 3) {
      ledStep = 0;
    }

    switch (ledStep) {

      case 0:
        digitalWrite(LED1, HIGH);
        Serial.println("LED1 ON");
        break;

      case 1:
        digitalWrite(LED2, HIGH);
        Serial.println("LED2 ON");
        break;

      case 2:
        digitalWrite(LED3, HIGH);
        Serial.println("LED3 ON");
        break;
    }
  }
}

// ===== PB1 按鈕偵測 =====
void readButton() {

  unsigned long currentTime = millis();

  bool reading = digitalRead(PB1);

  // 防彈跳
  if (reading != lastButtonReading) {
    debounceTime = currentTime;
    lastButtonReading = reading;
  }

  if (currentTime - debounceTime >= DEBOUNCE_TIME) {

    if (reading != buttonState) {

      buttonState = reading;

      // 按下按鈕
      if (buttonState == LOW) {

        pressStartTime = currentTime;
        longPressHandled = false;
      }

      // 放開按鈕
      else {

        // 短按：啟動
        if (!longPressHandled) {

          if (!systemRunning) {
            startSystem();
          }
        }
      }
    }
  }

  // 長按：關閉
  if (buttonState == LOW &&
      !longPressHandled &&
      currentTime - pressStartTime >= LONG_PRESS_TIME) {

    longPressHandled = true;

    if (systemRunning) {
      stopSystem();
    }
  }
}

// ===== SETUP =====
void setup() {

  Serial.begin(115200);

  pinMode(PB1, INPUT_PULLUP);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  allLEDOff();

  Serial.println("ESP32 Ready");
  Serial.println("Single Click: START");
  Serial.println("Long Press 1s: STOP");
}

// ===== LOOP =====
void loop() {

  readButton();

  ledSequence();

}
