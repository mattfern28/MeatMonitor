float th_voltsToBridge(float vout)  { return (vout - VREF) / TH_GAIN; }
float th_bridgeToRpar(float vb)     { float x = vb / VEX + 0.5; return RFIXED * x / (1.0 - x); }
float th_rparToRth(float rpar)      { return (rpar * RPAR) / (RPAR - rpar); }
float th_rthToTempC(float rth)      { return 1.0 / (log(rth / R25) / BETA + 1.0 / 298.15) - 273.15; }

float readThermistorC() {
  float volts = countsToVolts(muxReadCounts(MUX_CH_TH));
  float rpar  = th_bridgeToRpar(th_voltsToBridge(volts));
  if (rpar <= 0 || rpar >= RPAR) return NAN;    // outside the bridge's valid range
  return th_rthToTempC(th_rparToRth(rpar)) + TH_CAL_OFFSET_C;
}
