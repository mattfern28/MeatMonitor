# MEAT Monitor

ESP32 smoker/cooking monitor for the ELEE4230 Sensors Project. A K-type probe tracks core
temperature and adds up Salmonella lethality (log reduction) as the meat cooks, so it can tell
you when the meat is **SAFE**. During cooling, an IR sensor fits a Newton's-law cooling curve to
the surface temperature and predicts how many minutes are left until the meat is ready to eat
(60 °C / 140 °F). Readings are shown on an OLED and posted to ThingSpeak once a minute.

## Hardware

Full list with links is in [`Parts_List.xlsx`](Parts_List.xlsx). Main parts:

| Part | Role |
|---|---|
| ESP32-WROOM-32 | MCU, Wi-Fi |
| K-type probe + INA121 amp | core temperature (thermocouple) |
| NTC thermistor | cold-junction compensation |
| CD4051 analog mux | switches the thermocouple/thermistor onto one ADC pin |
| MLX90614 IR thermometer | surface temperature while cooling |
| BMP388 | ambient temperature + pressure |
| SH1107 128×64 OLED | display |
| LM339 + copper pad | capacitive touch button |

### Pin map (`config.h`)

| Signal | GPIO |
|---|---|
| ADC input (mux output) | 34 |
| Mux select A | 25 |
| Touch drive | 18 |
| Touch comparator in | 19 |
| I²C SDA / SCL | 21 / 22 |

Mux channel 0 = thermocouple, channel 1 = thermistor. I²C addresses: OLED `0x3C`,
BMP388 `0x77`, MLX90614 `0x5A`.

## Building

1. **Board:** install the *esp32 by Espressif* core (3.x) and select **ESP32 Dev Module**.
2. **Libraries** (Library Manager):
   - Adafruit SH110X
   - Adafruit GFX Library
   - Adafruit BMP3XX Library
   - Adafruit MLX90614 Library
   - ThingSpeak
3. **Secrets:** copy `secrets.example.h` to `secrets.h` and fill in your Wi-Fi and ThingSpeak
   details. Set `USE_HOME_WIFI` to `1` for home Wi-Fi or `0` for campus WPA2-Enterprise.
   `secrets.h` is git-ignored, so it never gets committed.
4. **Sketch folder:** the Arduino IDE requires the folder to have the same name as the main
   sketch. Put the files in a folder called `SmokerMonitor01/` (the IDE offers to do this for
   you when you open `SmokerMonitor01.ino`).

With arduino-cli:

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 SmokerMonitor01
arduino-cli upload  --fqbn esp32:esp32:esp32 -p COM3 SmokerMonitor01
```

## Using it

Keep your hand off the touch pad for about 2 s at boot while it captures its baseline.

| Mode | Tap | Hold 2 s |
|---|---|---|
| **Select** | next meat | start cooking |
| **Cooking** | — | switch to cooling |
| **Cooling** | — | back to select |

- **Cooking:** shows core temperature and lethality progress. Shows **SAFE!** once the
  target log reduction for that meat is reached.
- **Cooling:** point the IR sensor at the meat. After a 3-minute settle period it fits the
  cooling curve and shows the minutes left until 140 °F, then **EAT NOW!**
- If the screen dims (1 min) or turns off (5 min), the first tap only wakes it up.

Serial monitor runs at 115200 baud and prints a status line every second.

## ThingSpeak fields

| Field | Value |
|---|---|
| 1 | mode (0 select, 1 cooking, 2 cooling) |
| 2 | core temperature (°F) |
| 3 | log reduction |
| 4 | lethality progress (%) |
| 5 | surface temperature (°F) |
| 6 | ambient temperature (°F) |
| 7 | minutes until ready to eat |
| 8 | pressure (hPa) |

## Code layout

The `.ino` files are joined in alphabetical order after `SmokerMonitor01.ino`, so the
prefixes group them:

- `a_*`: algorithms (Kalman filter, lethality, cooling-curve fit)
- `d_*`: display
- `i_*`: IoT / ThingSpeak
- `s_*`: sensors (mux/ADC, thermistor, thermocouple, MLX90614, BMP388, touch)
- `config.h`: pins, calibration constants and tuning
