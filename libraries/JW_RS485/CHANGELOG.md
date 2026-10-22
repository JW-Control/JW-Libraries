# Changelog

## [0.1.0] - 2026-10-22

### Añadido
- Primera versión, extraída de `JWPLC_RS485` 1.0.1 (platform-jwplc 2.1.0-alpha.12).
- Clase instanciable `JW_RS485(HardwareSerial&, rxPin, txPin)`: una instancia por UART.
- Control de dirección configurable: AutoDirection (`setAutoDirection`), pin DE/RE (`setDirectionPin`) o callbacks (`setCallbackDirection`).
- Callbacks pre/post TX y de actividad por instancia, con `userData`.
- `setForceApbClock()` para ESP32 clásico con core 3.x.
- Misma API de transporte que `JWPLC_RS485`: `readAvailable`, `writeQueued`, telemetría y `printStatus`.
- Prueba N1 en host (`extras/test`).
