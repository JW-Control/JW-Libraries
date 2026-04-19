RTC library for **DS3232M / DS3232** with a clean JW-style API.

## Version 1.0.1 scope

This version is designed for the **JWPLC package** and uses `jwplc_i2c_bridge` internally.

It adds cleaner public aliases so sketches can look more natural in Arduino style.

Examples:
- `JWRTCDateTime`
- `JWRTCAlarm1Config`
- `JWRTCAlarm2Config`
- `JWRTCError`

## Main features

- Clean `DateTime` API
- Friendly aliases like `JWRTCDateTime`
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

  JWRTCDateTime dt;
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
```

## DateTime struct

```cpp
JWRTCDateTime dt;
dt.year = 2026;
dt.month = 4;
dt.day = 19;
dt.hour = 23;
dt.minute = 45;
dt.second = 0;
dt.dayOfWeek = 0; // optional, auto-calculated on write
```

## NVRAM access

```cpp
uint8_t value = 42;
rtc.nvramWriteByte(0, value);

uint8_t out = 0;
rtc.nvramReadByte(0, out);
```

## Error handling

```cpp
if (!rtc.read(dt)) {
  Serial.println(JW_RTC::errorToString(rtc.lastError()));
}
```

## Notes

- Year range is `2000..2099`
- `dayOfWeek` uses `1=Sunday ... 7=Saturday`
- `dayOfWeek = 0` on write means auto-calculate
- v1.0.1 uses `jwplc_i2c_bridge`
- a future release will add `Wire` support
