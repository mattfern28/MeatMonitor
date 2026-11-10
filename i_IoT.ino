#include <WiFi.h>
#include "esp_eap_client.h"
#include <ThingSpeak.h>

static WiFiClient tsClient;
static bool     connecting = false;
static uint32_t connectStartMs = 0;
const  uint32_t CONNECT_TIMEOUT_MS = 15000;   // give up on this upload after 15 s

static void wifiStart() {
#if USE_HOME_WIFI
  WiFi.begin(HOME_SSID, HOME_PASSWORD);                     // home network (WPA2-Personal)
#else
  WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);  // campus
#endif
}

static void radioOff() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  connecting = false;
}

void iotBegin() {
  ThingSpeak.begin(tsClient);
  radioOff();                      
}

static void setIfValid(int field, float v) { if (!isnan(v)) ThingSpeak.setField(field, v); }

static void sendNow() {
  ThingSpeak.setField(1, (int)mode);
  setIfValid(2, cToF(coreC));
  ThingSpeak.setField(3, logReduction);
  ThingSpeak.setField(4, 100.0f * logReduction / MEATS[meatIndex].logTarget);
  setIfValid(5, cToF(surfaceC));
  setIfValid(6, cToF(ambientC));
  setIfValid(7, minutesToEat);
  setIfValid(8, pressureHPa);
  int status = ThingSpeak.writeFields(TS_CHANNEL_ID, TS_WRITE_KEY);
  Serial.printf("ThingSpeak status: %d (connected in %lu ms)\n", status,
                (unsigned long)(millis() - connectStartMs));   // 200 = OK
}

void iotPost() {
  if (connecting) return;       
  WiFi.mode(WIFI_STA);
  wifiStart();
  connecting = true;
  connectStartMs = millis();
}

void iotMaintain() {
  if (!connecting) return;
  if (WiFi.status() == WL_CONNECTED) {
    sendNow();
    radioOff();
  } else if (millis() - connectStartMs > CONNECT_TIMEOUT_MS) {
    Serial.println("WiFi: no connection, skipping this upload");
    radioOff();
  }
}
