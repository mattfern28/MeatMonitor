// Copy this file to secrets.h and fill in your own values.
// secrets.h is listed in .gitignore, so your passwords and keys stay off GitHub.
#pragma once

// 1 = home Wi-Fi (WPA2-Personal), 0 = campus Wi-Fi (WPA2-Enterprise / PEAP)
#define USE_HOME_WIFI 1

// Home network
#define HOME_SSID      "your-home-ssid"
#define HOME_PASSWORD  "your-home-password"

// Campus network (WPA2-Enterprise)
#define WIFI_SSID      "your-campus-ssid"
#define EAP_IDENTITY   "you@school.edu"
#define EAP_USERNAME   "you@school.edu"
#define EAP_PASSWORD   "your-campus-password"

// ThingSpeak channel (Channel -> API Keys)
#define TS_CHANNEL_ID  0UL               // your channel number
#define TS_WRITE_KEY   "YOUR_WRITE_KEY"
