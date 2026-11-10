#pragma once
#include <Arduino.h>

#define PIN_ADC        34   
#define PIN_MUX_A      25   
#define PIN_TOUCH_DRV  18   
#define PIN_TOUCH_CMP  19   
#define PIN_SDA        21
#define PIN_SCL        22

#define MUX_CH_TC      0    
#define MUX_CH_TH      1    

const float M_ADC   = 1240.0;
const float B_ADC   = 0.0;
const int   MUX_SETTLE_US = 100;
const int   ADC_SAMPLES   = 50;


const float VREF    = 1.65;     
const float TH_GAIN = -10.8;    
const float VEX     = 3.300;    
const float RFIXED  = 400.0;    
const float RPAR    = 730.0;    
const float R25     = 1000.0;   
const float BETA    = 3500.0;   
const float TH_CAL_OFFSET_C = -7.42;   


const float TC_GAIN = 501.0;    
const float TC_SIGN = -1.0;      
const float TC_CAL_OFFSET_C = 0.0;     
const float TC_ZERO_V = 1.380;


const float MLX_SLOPE  = 0.9896;
const float MLX_OFFSET = 1.378;


const float KF_Q = 0.001;   
const float KF_R = 0.05;   

const int      TOUCH_SAMPLES   = 16;
const uint32_t TOUCH_DISCH_US  = 1000;
const uint32_t TOUCH_TIMEOUT_US= 2000;
const float    TOUCH_PRESS_US  = 1.4;   
const float    TOUCH_RELEASE_US= 1.0;   
const uint32_t TOUCH_MIN_MS    = 50;    
const uint32_t TOUCH_HOLD_MS   = 2000;  

const float    T_EAT_C          = 60.0;  
const uint32_t COOL_SETTLE_S    = 180;   

const uint32_t SENSOR_PERIOD_MS = 1000;  
const uint32_t POST_PERIOD_MS   = 60000; 


struct Kalman1D {
  float q, r, x, p;
  bool  init;
};

enum Mode { MODE_SELECT, MODE_COOKING, MODE_COOLING };
enum TouchEvent { TOUCH_NONE, TOUCH_TAP, TOUCH_HOLD };

struct Meat {
  const char* name;
  float logTarget;      
  float minutesAt145F;  
  float zF;  
};
