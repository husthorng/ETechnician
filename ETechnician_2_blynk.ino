//https://chatgpt.com/c/6afb838d-8a28-83ee-9de9-b2e4babb94d5
#define BLYNK_TEMPLATE_ID "TMPL6Nap1P2MW"
#define BLYNK_TEMPLATE_NAME "hung cloud"
#define BLYNK_AUTH_TOKEN "WyvirgReSWsTWpHRXeyDuR8HOlppvDjO"
//https://blynk.cloud/dashboard/1164610/global/devices/448309/organization/1164610/devices/4253556/dashboard
//109horng@mc327

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ======================================
// WiFi 設定
// ======================================
char ssid[] = "husthung";
char pass[] = "ma2553968";

// ======================================
// GPIO 設定
// ======================================
#define PB1   16

#define LED1  13
#define LED2  12
#define LED3  14

// ======================================
// 時間設定
// ======================================
#define DEBOUNCE_TIME     50
#define LONG_PRESS_TIME 1000

#define BLINK_TIME      1000
#define BLINK_INTERVAL  250

// V4 設定的正式亮燈時間
unsigned long ledTime = 5;

// ======================================
// 系統狀態
// ======================================
bool systemRunning = false;
bool waitingV4 = false;

// LED 編號
// 0 = LED1
// 1 = LED2
// 2 = LED3
int ledStep = 0;

// 閃爍狀態
bool ledBlinking = false;
bool blinkState = false;

// 時間紀錄
unsigned long ledStartTime = 0;
unsigned long blinkStartTime = 0;

// ======================================
// PB1 按鈕狀態
// ======================================
bool buttonState = HIGH;
bool lastButtonReading = HIGH;

bool longPressHandled = false;

unsigned long debounceTime = 0;
unsigned long pressStartTime = 0;

// ======================================
// LED 全部關閉
// ======================================
void allLEDOff() {

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}

// ======================================
// 取得目前 LED GPIO
// ======================================
int getCurrentLED() {

  switch (ledStep) {

    case 0:
      return LED1;

    case 1:
      return LED2;

    case 2:
      return LED3;

    default:
      return LED1;
  }
}

// ======================================
// 啟動系統
// ======================================
void startSystem() {

  if (systemRunning) return;

  waitingV4 = false;
  systemRunning = true;

  // 從 LED1 開始
  ledStep = 0;

  // LED1 先閃爍 1 秒
  ledBlinking = true;
  blinkState = false;

  allLEDOff();

  ledStartTime = millis();
  blinkStartTime = millis();

  Serial.println("====================");
  Serial.println("System START");
  Serial.println("LED1 Blinking");

  Serial.print("LED Time = ");
  Serial.print(ledTime);
  Serial.println(" seconds");

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, 1);
  }
}

// ======================================
// 關閉系統
// ======================================
void stopSystem() {

  systemRunning = false;
  waitingV4 = false;

  ledBlinking = false;
  blinkState = false;

  allLEDOff();

  Serial.println("====================");
  Serial.println("System STOP");
  Serial.println("All LED OFF");

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, 0);
  }
}

// ======================================
// 啟動前讀取 V4
// ======================================
void requestStart() {

  if (systemRunning || waitingV4) {
    return;
  }

  if (Blynk.connected()) {

    waitingV4 = true;

    Serial.println("Reading Blynk V4...");

    Blynk.syncVirtual(V4);

  } else {

    // Blynk 未連線，使用上次取得的設定值
    startSystem();
  }
}

// ======================================
// Blynk V0：系統開關
// ======================================
BLYNK_WRITE(V0) {

  int value = param.asInt();

  if (value == 1) {

    requestStart();

  } else {

    stopSystem();
  }
}

// ======================================
// Blynk V4：LED 正式亮燈時間
// ======================================
BLYNK_WRITE(V4) {

  int value = param.asInt();

  // 限制 1～3600 秒
  if (value < 1) value = 1;
  if (value > 3600) value = 3600;

  ledTime = value;

  Serial.print("V4 LED Time = ");
  Serial.print(ledTime);
  Serial.println(" seconds");

  // 啟動時讀取完成
  if (waitingV4) {

    startSystem();
  }
}

// ======================================
// Blynk 連線
// ======================================
BLYNK_CONNECTED() {

  Serial.println("Blynk Connected");

  // 同步 V0 開關狀態
  Blynk.syncVirtual(V0);
}

// ======================================
// LED 循環控制
// ======================================
void ledSequence() {

  if (!systemRunning) return;

  unsigned long now = millis();

  int currentLED = getCurrentLED();

  // ==================================
  // 第一階段：目前 LED 閃爍 1 秒
  // ==================================
  if (ledBlinking) {

    // 閃爍完成
    if (now - ledStartTime >= BLINK_TIME) {

      // 關閉前一顆 LED
      allLEDOff();

      // 目前 LED 正式亮燈
      digitalWrite(currentLED, HIGH);

      ledBlinking = false;

      ledStartTime = now;

      Serial.print("LED");
      Serial.print(ledStep + 1);
      Serial.println(" ON");

      return;
    }

    // 每 250ms 切換一次
    if (now - blinkStartTime >= BLINK_INTERVAL) {

      blinkStartTime = now;

      blinkState = !blinkState;

      digitalWrite(currentLED, blinkState);
    }

    return;
  }

  // ==================================
  // 第二階段：正式亮燈 V4 秒
  // ==================================
  if (now - ledStartTime >= ledTime * 1000UL) {

    // 下一顆 LED
    int previousStep = ledStep;

    ledStep = (ledStep + 1) % 3;

    // 不關閉前一顆 LED
    // 下一顆 LED 開始閃爍
    ledBlinking = true;

    blinkState = false;

    ledStartTime = now;
    blinkStartTime = now;

    Serial.print("LED");
    Serial.print(ledStep + 1);
    Serial.println(" Blinking");

    Serial.print("LED");
    Serial.print(previousStep + 1);
    Serial.println(" remains ON");
  }

  // ==================================
  // 第三階段：下一顆 LED 閃爍
  // 前一顆 LED 保持亮著
  // ==================================
  if (ledBlinking) {

    int nextLED = getCurrentLED();

    // 閃爍完成
    if (now - ledStartTime >= BLINK_TIME) {

      // 前一顆 LED 熄滅
      allLEDOff();

      // 下一顆 LED 正式亮燈
      digitalWrite(nextLED, HIGH);

      ledBlinking = false;

      ledStartTime = now;

      Serial.print("LED");
      Serial.print(ledStep + 1);
      Serial.println(" ON");

      return;
    }

    // 每 250ms 切換閃爍狀態
    if (now - blinkStartTime >= BLINK_INTERVAL) {

      blinkStartTime = now;

      blinkState = !blinkState;

      digitalWrite(nextLED, blinkState);
    }
  }
}

// ======================================
// PB1：短擊啟動、長按關閉
// ======================================
void readButton() {

  unsigned long now = millis();

  bool reading = digitalRead(PB1);

  // 防彈跳
  if (reading != lastButtonReading) {

    debounceTime = now;

    lastButtonReading = reading;
  }

  if (now - debounceTime >= DEBOUNCE_TIME) {

    if (reading != buttonState) {

      buttonState = reading;

      // PB1 按下
      if (buttonState == LOW) {

        pressStartTime = now;

        longPressHandled = false;
      }

      // PB1 放開：短擊
      else {

        if (!longPressHandled) {

          if (!systemRunning && !waitingV4) {

            requestStart();
          }
        }
      }
    }
  }

  // PB1 長按 1 秒：關閉
  if (buttonState == LOW &&
      !longPressHandled &&
      now - pressStartTime >= LONG_PRESS_TIME) {

    longPressHandled = true;

    if (systemRunning || waitingV4) {

      stopSystem();
    }
  }
}

// ======================================
// SETUP
// ======================================
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

// ======================================
// LOOP
// ======================================
void loop() {

  Blynk.run();

  readButton();

  ledSequence();
}
