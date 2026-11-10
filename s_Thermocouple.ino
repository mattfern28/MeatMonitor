float typeK_mV(float t) {
  const double c[] = {-1.7600413686E-02, 3.8921204975E-02, 1.8558770032E-05,
                      -9.9457592874E-08, 3.1840945719E-10,-5.6072844889E-13,
                       5.6075059059E-16,-3.2020720003E-19, 9.7151147152E-23,
                      -1.2104721275E-26};
  double e = 0, tp = 1;
  for (int i = 0; i < 10; i++) { e += c[i] * tp; tp *= t; }
  e += 1.185976E-01 * exp(-1.183432E-04 * (t - 126.9686) * (t - 126.9686));
  return e;
}


float typeK_C(float mv) {
  const double d[] = {0.0, 2.508355E+01, 7.860106E-02,-2.503131E-01, 8.315270E-02,
                     -1.228034E-02, 9.804036E-04,-4.413030E-05, 1.057734E-06,
                     -1.052755E-08};
  double t = 0, ep = 1;
  for (int i = 0; i < 10; i++) { t += d[i] * ep; ep *= mv; }
  return t;
}


float readThermocoupleC(float coldJunctionC) {
  if (isnan(coldJunctionC)) return NAN;
  float volts = countsToVolts(muxReadCounts(MUX_CH_TC));
  Serial.printf("TC zero volts: %.4f\n", volts);
  float tc_mV = TC_SIGN * (volts - TC_ZERO_V) / TC_GAIN * 1000.0;  // EMF across the probe
  float total = tc_mV + typeK_mV(coldJunctionC);               // add cold-junction EMF
  return typeK_C(total) + TC_CAL_OFFSET_C;
}
