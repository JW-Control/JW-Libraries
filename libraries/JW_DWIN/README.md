# JW_DWIN

Librería para pantallas **DWIN DGUS** con objetos de teclas, valores y textos, independiente del medio físico.

Conserva la API de la clase `JW_DWIN` de `JW_DWIN_RS485` 1.x: los mismos structs `DWIN_Key`, `DWIN_Value` y `DWIN_String`, y los mismos métodos (`setPage`, `enableTouch`, `sendValue`, `readValue`, `sendText`, `checkKey`, `keyEncoder`, `withDebug`). Un sketch hecho con esa versión solo cambia la declaración del objeto `dwin`.

## Transportes

| Transporte | Medio | Estado |
|---|---|---|
| `JW_DWIN_ModbusRTU` | RS-485, firmware DGUS Modbus RTU | v0.1 |
| UART con protocolo DGUS nativo (`5A A5`) | UART TTL / RS-232 | Previsto |

## Compatibilidad

- ESP32 con Arduino core 3.x.

## Dependencias

- `JW_ModbusRTU` y `JW_RS485` (solo si se usa `JW_DWIN_ModbusRTU`).

## Conexión básica (RS-485)

Pantalla DWIN con proyecto DGUS compilado para **Modbus RTU** (esclavo), conectada al bus RS-485 del ESP32. Configurar el ID y la velocidad iguales en la pantalla y en `JW_DWIN_ModbusRTU`.

## Ejemplo mínimo

```cpp
#include <JW_DWIN.h>
#include <JW_DWIN_ModbusRTU.h>

JW_RS485 rs485(Serial2, 16, 17);
JW_ModbusRTU modbus(rs485);
JW_DWIN_ModbusRTU transport(modbus, 1, 115200, SERIAL_8N1);  // esclavo 1
JW_DWIN dwin(transport);

DWIN_Value prog = {0x10, 0x00, 0, 0, 0};  // VP 0x1000

void setup()
{
    rs485.setAutoDirection();
    dwin.begin();
    dwin.setPage(0x10);
}

void loop()
{
    dwin.readValue(prog);
    dwin.sendValue(prog, prog.val);
}
```

## API principal

| Método | Descripción |
|---|---|
| `begin()` | Inicia el transporte. |
| `setPage(page)` | Cambia de página (`0x5A01` en `0x0084`). |
| `enableTouch(obj, state, force)` | Habilita o bloquea un control táctil (`0x5AA5` en `0x00B0`). |
| `sendValue(obj, value, force, debug)` | Escribe un VP numérico si cambió o no estaba confirmado. |
| `readValue(obj, debug)` | Lee un VP; devuelve `true` si cambió. La lectura que restablece el enlace nunca cuenta como cambio. |
| `isConfirmed(obj)` | `true` si la pantalla confirmó el valor o texto del objeto desde la última reconexión. |
| `sendText(obj, text, force)` | Escribe un texto de largo fijo (relleno con espacios). |
| `checkKey(key)` | Si la tecla fue pulsada, `key.Enable = 1` y limpia el VP. |
| `keyEncoder(minus, plus, count, min, max)` | Dos teclas como encoder, con vuelta circular. |
| `isOnline()`, `reconnected()`, `failureCount()` | Estado del enlace. |
| `setOfflinePolicy(fallos, ms)` | Fallos seguidos para pasar a sin enlace e intervalo de reintento. |
| `invalidateCache()` | Fuerza el reenvío de todo. |
| `setDebug(Stream&, mask)`, `withDebug(...)` | Debug opcional, apagado por defecto. |

## Errores posibles

Las operaciones devuelven `false` si la pantalla no confirmó. `lastErrorString()` informa la causa del transporte (por ejemplo, timeout o CRC).

## Notas de uso

- **Cache confirmada:** un valor, texto o estado táctil solo se da por enviado cuando la pantalla responde. Si falla, la siguiente llamada lo reintenta.
- **Sin enlace:** tras `JW_DWIN_OFFLINE_FAILURES` fallos seguidos (3), solo se intenta una operación cada `JW_DWIN_OFFLINE_RETRY_MS` (500 ms). El loop de la aplicación sigue corriendo sin esperar timeouts repetidos.
- **Reconexión:** al volver a comunicar se invalida la cache y `reconnected()` devuelve `true` una vez. Usarlo para volver a la página actual.
- **Valores editables por el operador:** leerlos como edición solo si `isConfirmed(obj)`; si no, reenviar primero el valor vigente. Así un envío fallido o el valor inicial de una pantalla reiniciada nunca se toman como una edición.
- **Estados enviados una sola vez** (íconos, habilitación táctil): tras una reconexión no se reenvían solos. Conviene enviarlos en cada ciclo del `loop` según el estado de la aplicación; la cache evita el tráfico repetido.
- Las operaciones son bloqueantes con timeout (`JW_DWIN_ModbusRTU::setTimeout`, 150 ms por defecto).

## Limitaciones

- v0.1: solo transporte Modbus RTU.
- Una operación por vez (sin cola asíncrona).

## Pruebas

```bash
bash extras/test/run_tests.sh
```
