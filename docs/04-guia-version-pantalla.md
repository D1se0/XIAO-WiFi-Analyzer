# 4. Guía: versión con pantalla (Round Display for XIAO)

> **Antes de empezar:** confirma que la [versión básica](03-guia-version-basica.md) funciona en tu placa. Esta guía añade varias librerías de terceros con dependencias entre sí — es mucho más fácil depurar un problema si ya sabes que el AP, el escáner y el servidor funcionan bien por separado.

Esta guía instala exactamente las librerías, en las versiones exactas, que se usaron para desarrollar y validar este proyecto sobre una XIAO ESP32-C5 real. Cada paso incluye el motivo, porque varios de ellos no son obvios y cuestan horas de depuración si se saltan.

---

## 4.1 Resumen de librerías necesarias

| Librería | Versión | Por qué esta y no otra | Instalación |
|---|---|---|---|
| **Seeed_GFX** | La última del repo | Fork de TFT_eSPI adaptado al ESP32-C5. La versión "genérica" de TFT_eSPI (Gestor de Librerías) **no compila** en este chip. | ZIP manual |
| **SeeedStudio_lvgl** | **v8.3** (no v9.x) | El ejemplo oficial de la pantalla está escrito para LVGL v8. La v9 que instala el Gestor de Librerías por defecto **rompe la compatibilidad** (structs internos opacos, enums estrictos). | ZIP manual |
| **Seeed_Arduino_RoundDisplay** | La última del repo | Contiene el ejemplo `HardwareTest` y el sistema de "combo" de pines (`BOARD_SCREEN_COMBO`). | ZIP manual |

**Ninguna de estas tres está en el Gestor de Librerías de Arduino** — se instalan todas manualmente desde GitHub, con `Add .ZIP Library...`.

---

## 4.2 Paso 1 — Montaje físico

1. Con la XIAO **desconectada de USB**, encaja la Round Display sobre el conector de 14 pines, presionando con firmeza y alineando bien.
2. **El conector USB-C de la XIAO debe quedar orientado hacia el exterior** de la Round Display (no hacia el centro). La pantalla tiene protección contra polaridad inversa, pero hazlo bien a la primera para ahorrarte una vuelta.
3. Confirma que el **interruptor KE** de la Round Display (en su parte trasera/lateral) está en posición **ON**. Sin esto, la pantalla no recibe alimentación pase lo que pase con el software.
4. Conecta el USB-C.

Si vas a soldar tú mismo los pines antes de este paso, lee primero el aviso de soldadura en [`01-requisitos-hardware.md`](01-requisitos-hardware.md#️-aviso-crítico-soldadura-de-los-pines-solo-versión-con-pantalla).

---

## 4.3 Paso 2 — Instalar las librerías, en este orden exacto

### a) Seeed_GFX

1. Ve a **[github.com/Seeed-Studio/Seeed_GFX](https://github.com/Seeed-Studio/Seeed_GFX)**.
2. Botón verde `Code` → `Download ZIP`.
3. En Arduino IDE: `Sketch → Include Library → Add .ZIP Library...` → selecciona el ZIP descargado.
4. **Importante — comprueba que no tengas ya una `TFT_eSPI` genérica instalada.** Ve a tu carpeta de librerías de Arduino (`Documents/Arduino/libraries/` en Windows/macOS, `~/Arduino/libraries/` en Linux) y, si existe una carpeta `TFT_eSPI` (sin "Seeed" en el nombre), **sácala por completo de esa carpeta** (no la borres, solo muévela a otro sitio fuera del árbol de librerías). Si Arduino detecta dos definiciones de `TFT_eSPI.h`, usará la que no queremos y la compilación fallará con errores como `'VSPI' was not declared`.

### b) SeeedStudio_lvgl (la versión 8.3, NO la del Gestor de Librerías)

1. Ve a **[github.com/Seeed-Projects/SeeedStudio_lvgl](https://github.com/Seeed-Projects/SeeedStudio_lvgl)**.
2. `Code → Download ZIP`.
3. `Sketch → Include Library → Add .ZIP Library...`.
4. Si ya tenías instalada una `lvgl` genérica (v9.x) desde el Gestor de Librerías, desinstálala primero (`Tools → Manage Libraries...` → busca `lvgl` → `Remove`) para evitar el mismo tipo de conflicto de doble definición.

### c) Configurar `lv_conf.h` (paso obligatorio, se salta fácilmente)

LVGL necesita un archivo de configuración que **no** se genera solo:

1. Ve a `Documents/Arduino/libraries/SeeedStudio_lvgl/` y busca el archivo `lv_conf.h` (o `lv_conf_template.h`, según la versión del repo).
2. **Cópialo** (no lo muevas) a la carpeta justo por encima:
   ```
   Documents/Arduino/libraries/lv_conf.h
   ```
   (Como archivo suelto, hermano de la carpeta `SeeedStudio_lvgl/`, no dentro de ella.)
3. Ábrelo con un editor de texto y busca cerca del principio:
   ```c
   #if 0 /*Set it to "1" to enable content*/
   ```
   Cámbialo a:
   ```c
   #if 1 /*Set it to "1" to enable content*/
   ```
   **Este es el paso que más gente olvida** — con `#if 0`, todo el resto del archivo se ignora al compilar, como si no existiera.
4. Un poco más abajo, busca:
   ```c
   #define LV_COLOR_DEPTH 32
   ```
   y cámbialo a:
   ```c
   #define LV_COLOR_DEPTH 16
   ```
   (la Round Display es de 16 bits de color; con 32 compila pero la imagen sale mal).
5. Guarda el archivo.

### d) Seeed_Arduino_RoundDisplay

1. Ve a **[github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay](https://github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay)**.
2. `Code → Download ZIP`.
3. `Sketch → Include Library → Add .ZIP Library...`.

### e) Cierra y reabre Arduino IDE por completo

Esto es importante — el menú `File → Examples` y la detección de librerías no siempre se refrescan en caliente tras instalar por ZIP.

---

## 4.4 Paso 3 — Verificar la configuración de pines del combo (opcional, pero recomendable la primera vez)

El proyecto usa el combo **501** ("Round Display for Seeed Studio XIAO, GC9A01"), definido en `driver.h`:

```cpp
#define BOARD_SCREEN_COMBO 501 // Round Display for Seeed Studio XIAO (GC9A01)
```

Ese número selecciona automáticamente el archivo de configuración correcto dentro de la librería:
```
Documents/Arduino/libraries/Seeed_GFX/User_Setups/Setup501_Seeed_XIAO_Round_Display.h
```

Si quieres confirmar que todo está bien antes de compilar el proyecto completo, abre ese archivo y comprueba que contiene (entre otras cosas):

```cpp
#define GC9A01_DRIVER
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define TFT_SCLK D8
#define TFT_MOSI D10
#define TFT_CS D1
#define TFT_DC D3
#define TFT_BL D6
#define TFT_BACKLIGHT_ON HIGH
#define TFT_RST -1
#define TOUCH_INT D7
```

Estos valores están verificados contra el esquemático oficial de Seeed — ver [`07-pines-y-hardware.md`](07-pines-y-hardware.md) para la tabla completa con el porqué de cada uno.

**Si en ese archivo ves la línea `#define SMOOTH_FONT` sin comentar**, coméntala:
```cpp
//#define SMOOTH_FONT
```
Sin esto, el driver intenta incluir `Seeed_Arduino_FS.h`, que entra en conflicto con la librería `FS`/`SD` que ya trae el core de `esp32` (necesaria porque este proyecto usa `WebServer.h`). El proyecto no usa fuentes suavizadas para nada, así que desactivarlo no quita funcionalidad.

---

## 4.5 Paso 4 — Compilar y cargar el proyecto completo

1. Descarga la carpeta [`firmware/XIAO_WiFi_Analyzer_Pantalla`](../firmware/XIAO_WiFi_Analyzer_Pantalla) de este repositorio.
2. Ábrela con Arduino IDE (doble clic en `XIAO_WiFi_Analyzer.ino`). Deberías ver **10 pestañas**: el `.ino`, `config.h`, `scanner.h/.cpp`, `api_handlers.h/.cpp`, `web_content.h`, `wifi_state.h/.cpp`, `display_ui.h/.cpp`, `driver.h`.
3. `Tools → Board → XIAO_ESP32C5`, `Tools → Port → [tu puerto]`.
4. `Sketch → Verify/Compile`. **Con LVGL de por medio, la primera compilación puede tardar 7-10 minutos** — es normal, no está colgado. Ten paciencia y no interrumpas el proceso.
5. Si compila limpio: `Sketch → Upload` (con **BOOT + RESET** manual si te lo pide, como en la versión básica).

Si te sale cualquier error de compilación, casi seguro que ya lo hemos visto y solucionado nosotros — ve directo a [`05-solucion-problemas.md`](05-solucion-problemas.md), están listados en el orden en que suelen aparecer.

## 4.6 Paso 5 — Comprobar que funciona

1. Abre el Monitor Serie (115200 baudios). Deberías ver, en este orden:
   ```
   === XIAO WiFi Analyzer ===
   [AP] Started OK
   ...
   [HTTP] Server started on port 80
   ```
   seguido, tras un momento, de la pantalla iniciando sin errores.
2. Mira la pantalla física: debería aparecer la pantalla **HOME** con el título, un punto de estado verde "respirando", un anillo circular (inicialmente en 0) y el botón **SCAN**.
3. Toca **SCAN**. Deberías ver un overlay con un spinner girando y "SCANNING...".
4. Cuando termine, el anillo debería animarse hasta el número real de redes detectadas.
5. Desliza el dedo hacia la izquierda para pasar a la pantalla de **NETWORKS** (lista de redes), otra vez para **CHANNELS** (barras de congestión), y otra vez para **STATUS** (uptime, RAM libre, clientes conectados).
6. Confirma en paralelo que el **dashboard web sigue funcionando exactamente igual** que en la versión básica — es la prueba de que añadir la pantalla no ha roto nada del resto.

## 4.7 Estructura de archivos de esta versión

```
XIAO_WiFi_Analyzer_Pantalla/
├── XIAO_WiFi_Analyzer.ino    → arranque de AP, DNS, servidor Y pantalla
├── config.h                   → configuración general (igual que la version basica)
├── driver.h                    → BOARD_SCREEN_COMBO 501 (selecciona el combo de pines)
├── scanner.h / scanner.cpp     → escaneo Wi-Fi (identico a la version basica)
├── wifi_state.h / .cpp         → estado agregado compartido entre dashboard y pantalla
├── api_handlers.h / .cpp       → rutas HTTP + JSON (identico a la version basica)
├── web_content.h               → dashboard web (identico a la version basica)
├── display_ui.h / .cpp         → LVGL: 4 pantallas, tactil manual, animaciones
```

**Importante sobre la arquitectura:** el escáner Wi-Fi (`scanner.cpp`) es **una única fuente de datos**. Tanto si escaneas desde el móvil (dashboard web) como si escaneas desde la pantalla física, ambos llaman a la misma función `startWifiScan()`, y ambos leen el mismo resultado (`g_lastScan` / `g_state`). No hay lógica de escaneo duplicada en ningún sitio.
