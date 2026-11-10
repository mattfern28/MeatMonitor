#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx;
bool mlxOK = false;

void mlxBegin() {
  mlxOK = mlx.begin(MLX90614_I2CADDR, &Wire);
  if (!mlxOK) Serial.println("MLX90614 not found - check wiring/5 V supply");
}

float readSurfaceC() {
  if (!mlxOK) return NAN;
  float raw = mlx.readObjectTempC();
  if (isnan(raw)) return NAN;
  return (raw - MLX_OFFSET) / MLX_SLOPE;    
}
