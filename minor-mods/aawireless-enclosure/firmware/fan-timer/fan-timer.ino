#include <Adafruit_FreeTouch.h>

#define TIME_LIMIT_SEC    (30 * 60)
#define PIN_FAN           1
#define PIN_TOUCH         A10
#define TOUCH_CONTINUOUS  100

bool fan_enable = false;
Adafruit_FreeTouch qt = Adafruit_FreeTouch(PIN_TOUCH, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);

uint32_t calib_sum = 0;
uint32_t calib_max = 0;
uint32_t calib_cnt = 0;
uint32_t calib_val = 0;
uint32_t touch_thresh = 100;
uint32_t touch_cnt = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_FAN, OUTPUT);
  digitalWrite(PIN_FAN, LOW);
  Serial.begin(9600);
  if (!qt.begin()) {
    Serial.println("Failed to begin qt");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  int x = qt.measure();
  if ((millis() < 5000 || calib_cnt < 100) && calib_val == 0) {
    calib_sum += x;
    calib_cnt += 1;
    calib_max = x > calib_max ? x : calib_max;
  }
  else if (calib_val == 0) {
    calib_val = calib_sum / calib_cnt;
    int y = ((calib_max - calib_val) * 2);
    touch_thresh = y > touch_thresh ? y : touch_thresh;
    Serial.printf("[%u ms]: calib = %u   ,   thresh = %u\r\n", millis(), calib_val, touch_thresh);
  }

  if (calib_val != 0) {
    if (x > (calib_val + touch_thresh)) {
      if (touch_cnt < TOUCH_CONTINUOUS) {
        touch_cnt += 1;
      }
      else {
        fan_enable |= true;
      }
    }
    else {
      touch_cnt = 0;
    }
    Serial.printf("[%u ms]: %u  %u\r\n", millis(), x, touch_cnt);
  }
  else {
    Serial.printf("[%u ms]: %u\r\n", millis(), x);
  }

  fan_enable |= (millis() >= (TIME_LIMIT_SEC * 1000));
  digitalWrite(PIN_FAN, fan_enable);
}
