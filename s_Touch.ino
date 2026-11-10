#include "soc/gpio_reg.h"

static portMUX_TYPE touchMux = portMUX_INITIALIZER_UNLOCKED;
static uint32_t cpuMHz;
static float    baselineUs = 0;
static bool     touched = false, holdFired = false;
static uint32_t pressStartMs = 0;

static uint32_t touchMeasureOnce() {
  REG_WRITE(GPIO_OUT_W1TC_REG, BIT(PIN_TOUCH_DRV));
  delayMicroseconds(TOUCH_DISCH_US);
  if (REG_READ(GPIO_IN_REG) & BIT(PIN_TOUCH_CMP)) return 0;   

  uint32_t timeout = TOUCH_TIMEOUT_US * cpuMHz, t0, t1;
  bool ok = false;
  portENTER_CRITICAL(&touchMux);
  t0 = ESP.getCycleCount();
  REG_WRITE(GPIO_OUT_W1TS_REG, BIT(PIN_TOUCH_DRV));
  do {
    t1 = ESP.getCycleCount();
    if (REG_READ(GPIO_IN_REG) & BIT(PIN_TOUCH_CMP)) { ok = true; break; }
  } while (t1 - t0 < timeout);
  portEXIT_CRITICAL(&touchMux);
  REG_WRITE(GPIO_OUT_W1TC_REG, BIT(PIN_TOUCH_DRV));
  return ok ? (t1 - t0) : 0;
}

float readTouchUs() {
  uint64_t sum = 0; int good = 0, fails = 0;
  while (good < TOUCH_SAMPLES && fails < TOUCH_SAMPLES) {
    uint32_t c = touchMeasureOnce();
    if (c == 0) { fails++; continue; }
    sum += c; good++;
  }
  return good ? (float)sum / good / cpuMHz : 0;
}

void touchBegin() {
  pinMode(PIN_TOUCH_DRV, OUTPUT);
  digitalWrite(PIN_TOUCH_DRV, LOW);
  pinMode(PIN_TOUCH_CMP, INPUT);
  cpuMHz = getCpuFrequencyMhz();
  Serial.println("Capturing touch baseline - don't touch the pad...");
  float sum = 0; int n = 0; uint32_t start = millis();
  while (millis() - start < 2000) { float td = readTouchUs(); if (td > 0) { sum += td; n++; } }
  baselineUs = n ? sum / n : 0;
  Serial.printf("Touch baseline %.2f us\n", baselineUs);
}

TouchEvent touchUpdate() {
  float td = readTouchUs();
  if (td == 0) return TOUCH_NONE;
  float delta = td - baselineUs;
  uint32_t now = millis();
  TouchEvent ev = TOUCH_NONE;

  if (!touched && delta > TOUCH_PRESS_US) {
    touched = true; holdFired = false; pressStartMs = now;
  } else if (touched && delta < TOUCH_RELEASE_US) {
    touched = false;
    uint32_t dur = now - pressStartMs;
    if (!holdFired && dur >= TOUCH_MIN_MS) ev = TOUCH_TAP;
  }
  if (touched && !holdFired && now - pressStartMs >= TOUCH_HOLD_MS) {
    holdFired = true; ev = TOUCH_HOLD;
  }
  if (!touched && delta < 0.5) baselineUs = 0.995f * baselineUs + 0.005f * td;   // track drift
  return ev;
}
