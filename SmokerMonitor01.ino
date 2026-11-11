#include <Wire.h>
#include "config.h"
#include "secrets.h"

Meat MEATS[] = {
  {"Chicken", 7.0, 13.8, 10.08},
  {"Beef (whole cut)", 6.5,  4.0, 10.37},
  {"Pork",  6.5,  4.0, 10.37},
  {"Burgers", 7.0,  4.0, 10.13},
};

const int N_MEATS = sizeof(MEATS) / sizeof(MEATS[0]);

Kalman1D kfTC   = {KF_Q, KF_R, 0, 1, false};
Kalman1D kfTH   = {KF_Q, KF_R, 0, 1, false};
Kalman1D kfSurf = {KF_Q, KF_R, 0, 1, false};

Mode  mode       = MODE_SELECT;
int   meatIndex  = 0;

float coldJunctionC = NAN, coreC = NAN, surfaceC = NAN, ambientC = NAN, pressureHPa = NAN;
float coreRawC = NAN;

//cooking
float logReduction = 0; 
bool  safeReached  = false;

//cooling
uint32_t coolStartMs = 0;
float    minutesToEat = NAN;
bool     readyToEat   = false;

uint32_t lastSensorMs = 0, lastPostMs = 0;

void setup() {
  setCpuFrequencyMhz(80);  //lower power on battery; must run before touchBegin()
  Serial.begin(115200);
  delay(300);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000); 

  displayBegin();
  muxBegin();
  bmpBegin();
  mlxBegin();
  touchBegin();  //captures untouched baseline
  iotBegin();

  Serial.println();
  Serial.println("=== MEAT Monitor ===");
  Serial.printf("SELECT MEAT: %s   (tap = next, hold 2 s = start)\n", MEATS[meatIndex].name);
}

void loop() {
  TouchEvent ev = touchUpdate();
  if (ev != TOUCH_NONE && displayWake()) handleTouch(ev);

  uint32_t now = millis();
  if (now - lastSensorMs >= SENSOR_PERIOD_MS) {
    float dtMin = (now - lastSensorMs) / 60000.0;
    lastSensorMs = now;
    readAllSensors();
    runAlgorithms(dtMin);
    printStatus();
    displayUpdate();
  }

  if (now - lastPostMs >= POST_PERIOD_MS) {
    lastPostMs = now;
    iotPost();
  }
  iotMaintain();
}

void handleTouch(TouchEvent ev) {
  switch (mode) {
    case MODE_SELECT:
      if (ev == TOUCH_TAP) {
        meatIndex = (meatIndex + 1) % N_MEATS;
        Serial.printf(">> Selected: %s\n", MEATS[meatIndex].name);
      } else {
        mode = MODE_COOKING;
        logReduction = 0; safeReached = false;
        Serial.printf(">> COOKING %s - target %.1f-log Salmonella reduction\n",
                      MEATS[meatIndex].name, MEATS[meatIndex].logTarget);
      }
      break;

    case MODE_COOKING:
      if (ev == TOUCH_HOLD) {
        mode = MODE_COOLING;
        coolingReset();
        coolStartMs = millis();
        readyToEat = false; minutesToEat = NAN;
        Serial.println(">> COOLING - place MEAT in front of the IR sensor");
      }
      break;

    case MODE_COOLING:
      if (ev == TOUCH_HOLD) {
        mode = MODE_SELECT;
        Serial.printf(">> SELECT MEAT: %s\n", MEATS[meatIndex].name);
      }
      break;
  }
}

void readAllSensors() {
  coldJunctionC = kalmanUpdate(kfTH, readThermistorC());
  coreRawC      = readThermocoupleC(coldJunctionC);
  coreC         = kalmanUpdate(kfTC, coreRawC);
  ambientC      = readAmbientC();
  pressureHPa   = readPressureHPa();

  float s = readSurfaceC();
  surfaceC = isnan(s) ? NAN : kalmanUpdate(kfSurf, s);
}

void runAlgorithms(float dtMin) {
  if (mode == MODE_COOKING) {
    logReduction += lethalityRate(MEATS[meatIndex], coreC) * dtMin;
    if (!safeReached && logReduction >= MEATS[meatIndex].logTarget) {
      safeReached = true;
      displayWake();
      Serial.println(">> SAFE:");
    }
  }
  if (mode == MODE_COOLING) {
    float tSec = (millis() - coolStartMs) / 1000.0;
    minutesToEat = coolingUpdate(tSec, surfaceC, ambientC);
    bool nowReady = !isnan(surfaceC) && surfaceC <= T_EAT_C;
    if (nowReady && !readyToEat) {
      displayWake();
      Serial.println(">> READY TO FEAST");
    }
    readyToEat = nowReady;
  }
}

float cToF(float c) { return c * 9.0 / 5.0 + 32.0; }

void printStatus() {
  switch (mode) {
    case MODE_SELECT:
      //Redundancy
      Serial.printf("[SELECT %s] TC %.1fF  TH %.1fF  BMP %.1fF  (TC-TH %+.1fF)\n",
                    MEATS[meatIndex].name, cToF(coreC), cToF(coldJunctionC),
                    cToF(ambientC), cToF(coreC) - cToF(coldJunctionC));
      break;
    case MODE_COOKING:
      Serial.printf("[COOK %s] core %.1fF (raw %.1fF) | log red. %.2f / %.1f (%.0f%%)%s\n",
                    MEATS[meatIndex].name, cToF(coreC), cToF(coreRawC),
                    logReduction, MEATS[meatIndex].logTarget,
                    100.0 * logReduction / MEATS[meatIndex].logTarget,
                    safeReached ? "  SAFE" : "");
      break;
    case MODE_COOLING:
      Serial.printf("[COOL] surface %.1fF  ambient %.1fF | %s\n",
                    cToF(surfaceC), cToF(ambientC),
                    readyToEat ? "READY TO FEAST ;)" :
                    isnan(minutesToEat) ? "fitting cooling curve..." : "");
      if (!readyToEat && !isnan(minutesToEat))
        Serial.printf("       predicted %.1f min until %.0fF\n", minutesToEat, cToF(T_EAT_C));
      break;
  }
}
