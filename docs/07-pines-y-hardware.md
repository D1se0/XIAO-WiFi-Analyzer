# 7. Pines y hardware interno

Toda la información de esta página está verificada contra el esquemático y la documentación oficial de Seeed Studio (no son suposiciones).

## 7.1 Mapeo de pines XIAO ESP32-C5

El core Arduino resuelve los nombres simbólicos `D0`...`D10` al GPIO físico real de cada chip. En la XIAO ESP32-C5, ese mapeo es:

| Nombre Arduino | GPIO real | Función por defecto |
|---|---|---|
| D0 | GPIO1 | Analógico / GPIO general |
| D1 | GPIO0 | GPIO general — **también pin de strapping de arranque** |
| D2 | GPIO25 | Analógico / GPIO general |
| D3 | GPIO7 | GPIO general / `SS` (SPI Chip Select nativo) |
| D4 | GPIO23 | `SDA` (I2C) |
| D5 | GPIO24 | `SCL` (I2C) |
| D6 | GPIO11 | `TX` (UART) |
| D7 | GPIO12 | `RX` (UART) |
| D8 | GPIO8 | `SCK` (SPI) |
| D9 | GPIO9 | `MISO` (SPI) |
| D10 | GPIO10 | `MOSI` (SPI) |

Adicionales (sin equivalente `Dx`, se usan por nombre de GPIO directamente):
- **GPIO6** — lectura de voltaje de batería (`BAT_VOLT_PIN`)
- **GPIO26** — habilitación de la lectura de batería (`BAT_VOLT_PIN_EN`)
- **GPIO27** — LED integrado (`LED_BUILTIN`)
- **GPIO28** — botón BOOT

> ⚠️ **D1 = GPIO0** es un pin de *strapping* — el ESP32 lo muestrea en el instante del arranque para decidir el modo de boot. Cualquier periférico que use D1 como salida activa (como `TFT_CS` en la Round Display) puede, en teoría, interferir con el arranque si queda en un estado incorrecto en ese instante concreto. En la práctica, el problema real encontrado en este proyecto fue un **puente de soldadura** en ese pin, no el uso normal como CS — ver [`05-solucion-problemas.md`](05-solucion-problemas.md#fallo-1-puente-de-soldadura).

## 7.2 Mapeo de pines de la Round Display for XIAO

La Round Display es una placa de expansión que se enchufa directamente sobre el conector de 14 pines. Al ser un estándar común a toda la familia XIAO, la función de cada **posición** física es la misma en cualquier XIAO, aunque el GPIO real que hay detrás cambie según el chip.

| Posición | Función en la Round Display | GPIO real en XIAO ESP32-C5 |
|---|---|---|
| D0 | Lectura de voltaje de batería (circuito propio de la pantalla) | GPIO1 |
| D1 | `TFT_CS` (chip select del LCD) | GPIO0 |
| D2 | `SD_CS` (ranura microSD, no usada en este proyecto) | GPIO25 |
| D3 | `TFT_DC` (data/command del LCD) | GPIO7 |
| D4 | `SDA` — I2C compartido (táctil + RTC) | GPIO23 |
| D5 | `SCL` — I2C compartido | GPIO24 |
| D6 | Backlight (retroiluminación) | GPIO11 |
| D7 | `TOUCH_INT` (interrupción táctil) | GPIO12 |
| D8 | `SCK` — SPI compartido (LCD + SD) | GPIO8 |
| D9 | `MISO` — SPI compartido | GPIO9 |
| D10 | `MOSI` — SPI compartido | GPIO10 |

Este proyecto **no usa** la ranura microSD (D2) ni el RTC (aunque comparta el bus I2C con el táctil, no se inicializa ningún RTC en el firmware).

## 7.3 Chip: Seeed Studio XIAO ESP32-C5

- **SoC:** Espressif ESP32-C5, arquitectura **RISC-V** (no Xtensa, a diferencia de los ESP32 clásicos).
- **Núcleos:** 1 núcleo principal a 240MHz + 1 núcleo de bajo consumo (LP core).
- **Radios:** Wi-Fi 6 **dual-band real** (2.4GHz + 5GHz simultáneo), Bluetooth 5 LE, 802.15.4 (Zigbee/Thread).
- **USB:** USB-Serial-JTAG nativo — no necesita chip adaptador USB-serie adicional.
- **Tamaño:** 21 × 17.8 mm.
- **Documentación oficial:** [wiki.seeedstudio.com/xiao_esp32c5_getting_started](https://wiki.seeedstudio.com/xiao_esp32c5_getting_started/)

## 7.4 Pantalla: Round Display for XIAO

- **Panel:** LCD circular 1.28", 240×240px, controlador **GC9A01** (SPI).
- **Táctil:** capacitivo, controlador **CHSC6x** (I2C).
- **Documentación oficial:** [wiki.seeedstudio.com/get_start_round_display](https://wiki.seeedstudio.com/get_start_round_display/)
- **Interruptor KE:** debe estar en ON para que la pantalla reciba alimentación.
- **Orientación de montaje:** el USB-C de la XIAO debe quedar hacia el exterior de la pantalla circular.

## 7.5 Por qué la distinción de banda 2.4GHz/5GHz no usa una API especial

`WiFi.getNetworkInfo()` en el core `esp32` no devuelve un campo "banda" explícito por red. En su lugar, este proyecto deriva la banda directamente del número de canal devuelto (`scanner.cpp`, función `classifyBand()`):

- Canales 1–14 → 2.4GHz
- Canales 36 en adelante → 5GHz

Esta regla es determinista según el propio estándar Wi-Fi y no depende de la versión exacta de ESP-IDF ni de funciones adicionales como `WiFi.setBandMode()`.
