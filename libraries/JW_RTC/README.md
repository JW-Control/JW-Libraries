# JW_RTC

RTC library for **DS3232M / DS3232** with a clean JW-style API.

## Version 1.0.0 scope

This first version is designed for the **JWPLC package** and uses `jwplc_i2c_bridge` internally.

That means:

- it works naturally inside the JWPLC environment
- it does **not** depend on `TimeLib`
- it provides a direct `DateTime` API
- a future version will add a `Wire` backend for generic Arduino/ESP32 boards

## Main features

- Clean `DateTime` struct
- Read/write date and time
- Unix timestamp conversion
- RTC validity check (`OSF`)
- Temperature reading
- Aging offset read/write
- Square-wave and 32kHz output control
- Alarm 1 / Alarm 2 configuration
- Battery-backed SRAM access (`236 bytes`)

## Basic usage

```cpp
#include <JW_RTC.h>

JW_RTC rtc;

void setup() {
  Serial.begin(115200);

  if (!rtc.begin()) {
    Serial.println(JW_RTC::errorToString(rtc.lastError()));
    return;
  }

  JW_RTC::DateTime dt;
  if (rtc.read(dt)) {
    Serial.print(dt.year);
    Serial.print("/");
    Serial.print(dt.month);
    Serial.print("/");
    Serial.println(dt.day);
  }
}

void loop() {
}