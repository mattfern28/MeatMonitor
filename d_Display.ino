#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
static Adafruit_SH1107 oled(64, 128, &Wire);
static bool oledOK = false;

const uint32_t DIM_MS = 60000;          
const uint32_t OFF_MS = 300000;         
const uint8_t  BRIGHT = 0x2F;           
const uint8_t  DIM    = 0x01;           
static uint32_t lastWakeMs = 0;
static uint8_t  screenState = 0;   // 0 = bright, 1 = dim, 2 = off

bool displayWake() {
  lastWakeMs = millis();
  if (!oledOK || screenState == 0) return true;
  if (screenState == 2) oled.oled_command(SH110X_DISPLAYON);
  oled.setContrast(BRIGHT);
  screenState = 0;
  return false;
}

static void screenSaver() {
  uint32_t idle = millis() - lastWakeMs;
  if (screenState == 0 && idle > DIM_MS) { oled.setContrast(DIM); screenState = 1; }
  if (screenState == 1 && idle > OFF_MS) { oled.oled_command(SH110X_DISPLAYOFF); screenState = 2; }
}

void displayBegin() {
  delay(250);                                   
  oledOK = oled.begin(0x3C, true);
  if (!oledOK) { Serial.println("OLED not found - check wiring/address"); return; }
  oled.setRotation(1);                          
  oled.clearDisplay();
  oled.setTextColor(SH110X_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("MEAT Monitor");
  oled.println();
  oled.println("Calibrating touch...");
  oled.println("don't touch the pad");
  oled.display();
  oled.setContrast(BRIGHT);
  lastWakeMs = millis();
}

static void tempLine(const char* label, float c) {    
  oled.setTextSize(1); oled.print(label);
  oled.setTextSize(2);
  if (isnan(c)) oled.println(" --.-"); else oled.printf(" %.1fF\n", cToF(c));
  oled.setTextSize(1);
}

static void progressBar(int y, float frac) {
  frac = constrain(frac, 0.0f, 1.0f);
  oled.drawRect(0, y, 128, 8, SH110X_WHITE);
  oled.fillRect(2, y + 2, (int)(124 * frac), 4, SH110X_WHITE);
}


void displayUpdate() {
  if (!oledOK) return;
  screenSaver();
  if (screenState == 2) return;                 
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);

  switch (mode) {
    case MODE_SELECT:
      oled.println("SELECT MEAT");
      oled.setTextSize(1); oled.println();
      oled.println(MEATS[meatIndex].name);
      oled.println();
      tempLine("Probe", coreC);
      oled.println("tap:next  hold:start");
      break;

    case MODE_COOKING: {
      float frac = logReduction / MEATS[meatIndex].logTarget;
      oled.println(MEATS[meatIndex].name);
      tempLine("Core", coreC);
      oled.printf("Lethality %3.0f%%\n", 100.0 * frac);
      progressBar(36, frac);
      oled.setCursor(0, 48);
      if (safeReached) { oled.setTextSize(2); oled.println("SAFE!"); }
      break;
    }

    case MODE_COOLING:
      oled.println("COOLING");
      tempLine("Surf", surfaceC);
      oled.printf("Ambient %.1fF\n", cToF(ambientC));
      oled.setCursor(0, 44);
      if (readyToEat)            { oled.setTextSize(2); oled.println("EAT NOW!"); }
      else if (isnan(minutesToEat)) oled.println("Fitting cooling...");
      else                       { oled.setTextSize(2); oled.printf("%.0f min\n", minutesToEat); }
      break;
  }
  oled.display();
}