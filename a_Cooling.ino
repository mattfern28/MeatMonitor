static bool  coolHaveT0 = false;
static float coolT0 = 0, coolStartSec = 0;
static double sumTY = 0, sumTT = 0;
static int   coolN = 0;

void coolingReset() {
  coolHaveT0 = false; sumTY = 0; sumTT = 0; coolN = 0;
}

float coolingUpdate(float tSec, float surfC, float ambC) {
  if (isnan(surfC) || isnan(ambC)) return NAN;
  if (tSec < COOL_SETTLE_S) return NAN;           
  if (surfC <= T_EAT_C) return 0;
  if (surfC - ambC < 2.0) return NAN;  

  if (!coolHaveT0) { coolT0 = surfC; coolStartSec = tSec; coolHaveT0 = true; return NAN; }

  float t = tSec - coolStartSec;
  float y = log((surfC - ambC) / (coolT0 - ambC));
  sumTY += t * y; sumTT += (double)t * t; coolN++;
  if (coolN < 30 || sumTT <= 0) return NAN;  

  float k = -sumTY / sumTT;                       
  if (k <= 0 || T_EAT_C <= ambC) return NAN;
  float secLeft = log((surfC - ambC) / (T_EAT_C - ambC)) / k;
  return secLeft / 60.0;
}
