float kalmanUpdate(Kalman1D &kf, float meas) {
  if (isnan(meas)) return kf.init ? kf.x : NAN;
  if (!kf.init) { kf.x = meas; kf.p = 1.0; kf.init = true; return kf.x; }
  kf.p += kf.q;                         
  float k = kf.p / (kf.p + kf.r);       
  kf.x += k * (meas - kf.x);            
  kf.p *= (1.0 - k);
  return kf.x;
}
