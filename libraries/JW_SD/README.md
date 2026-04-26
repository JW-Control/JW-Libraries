# JW_SD

`JW_SD` es una libreria para microSD basada en la libreria nativa `SD`, pensada para trabajar de forma segura en proyectos con bus SPI compartido.

Fue creada para el ecosistema JWPLC, pero tambien puede usarse en proyectos Arduino/ESP32 normales.

## Objetivo

Permitir un uso limpio:

```cpp
auto file = sd.open("/log.txt", FILE_APPEND);

if (file)
{
    file.println("Hola JWPLC");
    file.close();
}
```

sin que el usuario tenga que recordar manualmente `lock()` / `unlock()` en operaciones comunes de archivo.

## Clases principales

- `JW_SD`: objeto principal para inicializar y manejar la microSD.
- `JWPLCFile`: wrapper de archivo protegido.
- `File`: acceso nativo opcional mediante `openNative()`.

## Ejemplo basico

```cpp
#include <JW_SD.h>

JW_SD sd;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    if (!sd.begin(32))
    {
        Serial.println("No se pudo iniciar la SD");
        return;
    }

    auto file = sd.open("/log.txt", FILE_APPEND);

    if (file)
    {
        file.println("Hola desde JW_SD");
        file.close();
    }
}

void loop()
{
}
```

## Uso con tipo explicito

```cpp
JWPLCFile file = sd.open("/log.txt", FILE_APPEND);
```

## Uso con auto

```cpp
auto file = sd.open("/log.txt", FILE_APPEND);
```

Ambas formas son equivalentes.

## Bus SPI compartido

Para proyectos con varios perifericos SPI, se pueden configurar callbacks:

```cpp
sd.setBusLockCallbacks(lockCallback, unlockCallback, userData, 100);
```

Cada operacion de `JWPLCFile` intenta bloquear el bus antes de ejecutar la operacion sobre el `File` nativo.

## Nota sobre openNative()

`openNative()` devuelve un `File` nativo. Es util para compatibilidad avanzada, pero las operaciones posteriores como `println()`, `read()` o `close()` ya no quedan protegidas automaticamente por `JW_SD`.

Para uso seguro en bus compartido, usar `open()` y `JWPLCFile`.
