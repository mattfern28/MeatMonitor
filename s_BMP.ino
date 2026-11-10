#include "Adafruit_BMP3XX.h"
Adafruit_BMP3XX bmp;
bool bmpOK = false;

void bmpBegin() {
  bmpOK = bmp.begin_I2C(0x77, &Wire);
  if (!bmpOK) { Serial.println("BMP388 not found - check wiring/address"); return; }
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
}

static float bmpTempC = NAN, bmpPresHPa = NAN;
static void bmpRead() {
  if (bmpOK && bmp.performReading()) { bmpTempC = bmp.temperature; bmpPresHPa = bmp.pressure / 100.0; }
}

float readAmbientC()    { bmpRead(); return bmpTempC; }
float readPressureHPa() { return bmpPresHPa; }   

float boilingPointC() { return isnan(bmpPresHPa) ? 100.0 : 100.0 + 0.028 * (bmpPresHPa - 1013.25); }
