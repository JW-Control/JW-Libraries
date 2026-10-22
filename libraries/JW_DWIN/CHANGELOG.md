# Changelog

## [0.1.0] - 2026-10-22

### Añadido
- Primera versión. API de la clase `JW_DWIN` de `JW_DWIN_RS485` 1.x (local, sep-2026) con los mismos structs `DWIN_Key`, `DWIN_Value` y `DWIN_String`.
- Interfaz `JW_DWIN_Transport`; transporte `JW_DWIN_ModbusRTU` (RS-485, DGUS Modbus) sobre `JW_ModbusRTU`.
- Cache confirmada por la pantalla: un envío fallido se reintenta en la siguiente llamada.
- Modo sin enlace (`setOfflinePolicy`), aviso de reconexión (`reconnected()`) e invalidación de cache.
- Prueba N1 en host (`extras/test`).

### Cambiado respecto a JW_DWIN_RS485 1.x
- El constructor recibe un transporte en lugar de `HardwareSerial` y pines.
- `keyEncoder()` ya no imprime por `Serial`; usa la salida de debug (`DWIN_DBG_FLOW`).
- Sin `setPrePostTransmission()` ni `modbus()`: la dirección del bus la maneja `JW_RS485`.
