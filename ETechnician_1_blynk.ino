
#define BLYNK_TEMPLATE_ID "TMPL6Nap1P2MW"
#define BLYNK_TEMPLATE_NAME "hung cloud"
#define BLYNK_AUTH_TOKEN "WyvirgReSWsTWpHRXeyDuR8HOlppvDjO"
//https://blynk.cloud/dashboard/1164610/global/devices/448309/organization/1164610/devices/4253556/dashboard
//109horng@mc327

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ===== WiFi =====
char ssid[] = "husthung";
char pass[] = "ma2553968";

// ===== GPIO =====
#define PB1   16
#define LED1  13
#define LED2  12
#define LED3  14

// ===== 時間設定 =====
#define DEBOUNCE_TIME   50
#define LONG_PRESS_TIME 1000

// V4 設定值（秒）
unsigned long ledTime = 5;

// ===== 系統狀態 =====
bool systemRunning = false;
bool waitingV4 = false;

int ledStep = 0;
unsigned long ledStartTime = 0;

// ===== 按鈕狀態 =====
bool buttonState = HIGH;
bool lastButtonReading = HIGH;
bool longPressHandled = false;

unsigned long debounceTime = 0;
unsigned long pressStartTime = 0;

// ===== LED 全部關閉 =====
void allLEDOff() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}

// ===== 開始 LED 循環 =====
void startSystem() {

  systemRunning = true;

  ledStep = 0;
  ledStartTime = millis();

  allLEDOff();
  digitalWrite(LED1, HIGH);

  Serial.println("System START");
  Serial.print("LED ON Time: ");
  Serial.print(ledTime);
  Serial.println(" seconds");

  Blynk.virtualWrite(V0, 1);
}

// ===== 關閉系統 =====
void stopSystem() {

  systemRunning = false;
  waitingV4 = false;

  allLEDOff();

  Serial.println("System STOP");

  Blynk.virtualWrite(V0, 0);
}

// ===== 要求讀取 V4 =====
void requestStart() {

  if (systemRunning || waitingV4) {
    return;
  }

  if (Blynk.connected()) {

    waitingV4 = true;

    Serial.println("Reading Blynk V4...");

    Blynk.syncVirtual(V4);
  }
  else {

    // Blynk 未連線時使用最近一次取得的 V4 值
    startSystem();
  }
}

// ===== Blynk V0：啟動 / 關閉 =====
BLYNK_WRITE(V0) {

  int value = param.asInt();

  if (value == 1) {
    requestStart();
  }
  else {
    stopSystem();
  }
}

// ===== Blynk V4：讀取 LED 亮燈時間 =====
BLYNK_WRITE(V4) {

  int value = param.asInt();

  // 限制設定範圍 1～3600 秒
  if (value < 1) value = 1;
  if (value > 3600) value = 3600;

  ledTime = value;

  Serial.print("V4 LED Time = ");
  Serial.print(ledTime);
  Serial.println(" seconds");

  // 等待 V4 讀取完成後才啟動
  if (waitingV4) {

    waitingV4 = false;

    startSystem();
  }
}

// ===== Blynk 連線 =====
BLYNK_CONNECTED() {

  Serial.println("Blynk Connected");

  Blynk.syncVirtual(V0);
}

// ===== LED 循環 =====
void ledSequence() {

  if (!systemRunning) {
    return;
  }

  unsigned long currentTime = millis();

  unsigned long interval = ledTime * 1000UL;

  if (currentTime - ledStartTime >= interval) {

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

// ===== PB1 實體按鈕 =====

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

      // PB1 按下
      if (buttonState == LOW) {

        pressStartTime = currentTime;
        longPressHandled = false;
      }

      // PB1 放開：短擊啟動
      else {

        if (!longPressHandled) {

          if (!systemRunning && !waitingV4) {
            requestStart();
          }
        }
      }
    }
  }

  // PB1 長按 1 秒：系統關閉
  if (buttonState == LOW &&
      !longPressHandled &&
      currentTime - pressStartTime >= LONG_PRESS_TIME) {

    longPressHandled = true;

    if (systemRunning || waitingV4) {
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

  Serial.println("ESP32 Starting...");

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );
}

// ===== LOOP =====
void loop() {

  Blynk.run();

  readButton();

  ledSequence();
}

