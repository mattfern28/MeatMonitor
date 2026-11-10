float lethalityRate(const Meat &m, float coreC) {
  if (isnan(coreC)) return 0;
  float tF = coreC * 9.0 / 5.0 + 32.0;
  if (tF < 130.0) return 0;            
  return (m.logTarget / m.minutesAt145F) * pow(10.0, (tF - 145.0) / m.zF);
}
