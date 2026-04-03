# JW_DWIN_RS485

Librería Arduino para manejar pantallas táctiles DWIN que se comunican por **RS-485 / Modbus RTU**, usando la dependencia **ModbusMaster**.

Esta librería encapsula funciones comunes para:

- cambiar de página,
- habilitar o deshabilitar controles táctiles,
- enviar y leer valores,
- enviar texto fijo,
- leer teclas,
- y manejar teclas tipo `+ / -` como selector circular.

## Características

- Compatible con Arduino IDE
- Compatible con Arduino Library Manager en cuanto a estructura del repositorio
- Basada en `ModbusMaster`
- Orientada a ESP32, pero usable en cualquier plataforma compatible con Arduino y ModbusMaster
- API simple basada en estructuras `DWIN_Key`, `DWIN_String` y `DWIN_Value`

## Dependencias

Esta librería requiere:

- `ModbusMaster`

## Instalación manual

1. Descargar este repositorio como `.zip`
2. En Arduino IDE ir a:
   - **Programa / Sketch > Incluir Librería > Añadir librería .ZIP**
3. Seleccionar el archivo descargado

## Instalación por Library Manager

Cuando la librería esté publicada y registrada, podrá instalarse directamente desde el gestor de librerías del Arduino IDE.

## Estructura principal

- `DWIN_Key`: manejo de teclas o controles táctiles
- `DWIN_String`: manejo de textos de longitud fija
- `DWIN_Value`: manejo de valores enteros y flotantes escalados
- `JW_DWIN_RS485`: clase principal de comunicación

## Ejemplo básico

```cpp
#include <HardwareSerial.h>
#include <ModbusMaster.h>
#include <JW_DWIN_RS485.h>

HardwareSerial Serial485(2);
ModbusMaster node;
JW_DWIN_RS485 dwin(node);

const int RXD2 = 16;
const int TXD2 = 17;
const int DE_RE_PIN = 4;

void preTransmission() {
  digitalWrite(DE_RE_PIN, HIGH);
}

void postTransmission() {
  digitalWrite(DE_RE_PIN, LOW);
}

DWIN_Value V_Speed(0x10, 0x00, 0x01, 0x00, 0);
DWIN_Key   K_Minus(0x10, 0x10, 0x01, 0x05);
DWIN_Key   K_Plus (0x10, 0x12, 0x02, 0x05);

uint32_t speedCount = 0;

void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, LOW);

  Serial.begin(115200);
  Serial485.begin(115200, SERIAL_8N1, RXD2, TXD2);

  node.begin(1, Serial485);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  dwin.setPage(0x10);
  dwin.enableTouch(K_Minus, 1, true);
  dwin.enableTouch(K_Plus, 1, true);
  dwin.sendValue(V_Speed, speedCount, true);
}

void loop() {
  dwin.handleKeyEncoder(K_Minus, K_Plus, speedCount, 0, 999);
  dwin.sendValue(V_Speed, speedCount);
}
```

## Notas

- Si usas un transceiver RS-485, configura correctamente los pines `DE` y `RE`.
- La librería no incluye la lógica específica de una máquina o proyecto concreto.
- Se recomienda dejar en el sketch principal la lógica de aplicación y usar la librería solo para lo reutilizable.

## Licencia

MIT License
