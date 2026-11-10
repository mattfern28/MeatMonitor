void muxBegin() {
  pinMode(PIN_MUX_A, OUTPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_ADC, ADC_11db);   
}

float muxReadCounts(int ch) {
  digitalWrite(PIN_MUX_A, ch);
  delayMicroseconds(MUX_SETTLE_US);
  analogRead(PIN_ADC);                          
  uint32_t sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) { sum += analogRead(PIN_ADC); delayMicroseconds(200); }
  return (float)sum / ADC_SAMPLES;
}

float countsToVolts(float counts) { return (counts - B_ADC) / M_ADC; }
