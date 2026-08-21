# 5. Solución de problemas

Todos los problemas listados aquí son **reales** — se encontraron y resolvieron durante el desarrollo de este proyecto sobre una XIAO ESP32-C5 física. Están ordenados aproximadamente en el orden en que suelen aparecer si sigues la guía de instalación desde cero.

---

## Índice

- [No aparece el puerto](#no-aparece-el-puerto)
- [`Failed to start stub flasher`](#failed-to-start-stub-flasher)
- [Fallo 1: puente de soldadura](#fallo-1-puente-de-soldadura)
- [`Multiple libraries were found for "TFT_eSPI.h"`](#multiple-libraries-were-found-for-tft_espih)
- [Errores `'VSPI' was not declared`, `gpio_out_w1ts_reg_t`...](#errores-vspi-was-not-declared-gpio_out_w1ts_reg_t)
- [`fatal error: ../../lv_conf.h: No such file or directory`](#fatal-error-lv_confh-no-such-file-or-directory)
- [`fatal error: TFT_eSPI.h: No such file or directory`](#fatal-error-tft_espih-no-such-file-or-directory)
- [`fatal error: Seeed_Arduino_FS.h: No such file or directory`](#fatal-error-seeed_arduino_fsh-no-such-file-or-directory)
- [`multiple definition of 'fs::File::write...'` (conflicto SD/FS)](#multiple-definition-of-fsfilewrite-conflicto-sdfs)
- [`invalid use of incomplete type 'lv_timer_t'` / errores de `lv_state_t`](#invalid-use-of-incomplete-type-lv_timer_t--errores-de-lv_state_t)
- [`fatal error: I2C_BM8563.h: No such file or directory`](#fatal-error-i2c_bm8563h-no-such-file-or-directory)
- [`exit status 0xc0000142`](#exit-status-0xc0000142)
- [Pantalla en negro pero compila y sube bien](#pantalla-en-negro-pero-compila-y-sube-bien)
- [Hay que reflashear tras cada reset](#hay-que-reflashear-tras-cada-reset)
- [El táctil no responde / el botón no reacciona](#el-táctil-no-responde--el-botón-no-reacciona)
- [Pocas redes detectadas o señal muy débil](#pocas-redes-detectadas-o-señal-muy-débil)
- [El AP se desconecta o se vuelve inestable](#el-ap-se-desconecta-o-se-vuelve-inestable)
- [El PC no ve la red pero el móvil sí](#el-pc-no-ve-la-red-pero-el-móvil-sí)
- [Channel Analysis vacío](#channel-analysis-vacío)

---

## No aparece el puerto

- Comprueba que el cable USB-C es de datos, no solo de carga.
- Prueba otro puerto USB del ordenador (evita hubs si usas uno).
- En Windows, revisa el Administrador de dispositivos por si aparece con algún icono de error.

## `Failed to start stub flasher`

**Síntoma:** la subida llega a identificar el chip (se ve el modelo, el MAC...) pero falla justo después, con `Failed to start stub flasher. Unexpected response: b''`.

**Causa:** comportamiento conocido en placas ESP32 recientes con USB-Serial-JTAG nativo — el auto-reset hacia modo bootloader no siempre llega a tiempo.

**Solución:**
1. Mantén pulsado **BOOT**.
2. Pulsa y suelta **RESET**.
3. Suelta **BOOT**.
4. Pulsa `Upload` inmediatamente.

Si te pasa siempre, es normal — no indica ningún fallo de hardware, solo hazlo cada vez que subas firmware nuevo.

## Fallo 1: puente de soldadura

**Síntoma combinado:** la pantalla no muestra nada Y, además, hay que reflashear el firmware cada vez que se resetea la placa (un simple RESET no la hace arrancar de nuevo).

**Causa:** un puente de estaño entre pines contiguos del conector de 14 pines. Concretamente, si el pin **D1** queda puenteado con un vecino, el problema es doble: D1 se usa como `TFT_CS` (chip select del LCD) **y** es a la vez **GPIO0**, el pin de arranque del ESP32. Puenteado, el chip no puede decidir correctamente su modo de arranque.

**Solución:**
1. Inspecciona los pines con lupa o la cámara del móvil en macro, comparando cada pin con sus vecinos.
2. Si ves estaño conectando dos pines que no deberían tocarse, usa cinta desoldadora o un desoldador de succión para retirar el exceso.
3. Confirma con un multímetro en modo continuidad si tienes dudas.

Este fallo concreto resolvió, de golpe, tanto el problema de imagen como el de arranque — son el mismo fallo físico con dos síntomas distintos.

## `Multiple libraries were found for "TFT_eSPI.h"`

**Síntoma:** el log de compilación incluye una línea como:
```
Multiple libraries were found for "TFT_eSPI.h"
  Used: .../libraries/TFT_eSPI
  Not used: .../libraries/Seeed_GFX
```
seguida de una cascada larga de errores (ver siguiente sección).

**Causa:** tienes instalada una copia **genérica** de `TFT_eSPI` (probablemente del Gestor de Librerías) además de `Seeed_GFX`. Arduino da prioridad a la genérica, que no soporta el ESP32-C5.

**Solución:**
1. Ve a tu carpeta de librerías (`Documents/Arduino/libraries/`).
2. Localiza la carpeta `TFT_eSPI` (sin "Seeed" en el nombre).
3. **Sácala físicamente de esa carpeta** — no basta con renombrarla, Arduino identifica las librerías por el contenido de `library.properties`, no por el nombre de la carpeta. Muévela a otra ubicación completamente fuera del árbol de librerías (por ejemplo tu Escritorio).
4. Reinicia Arduino IDE y vuelve a compilar.

## Errores `'VSPI' was not declared`, `gpio_out_w1ts_reg_t`...

**Síntoma:** decenas de errores como:
```
error: 'VSPI' was not declared in this scope
error: 'SPI_MOSI_DLEN_REG' was not declared in this scope
error: no match for 'operator=' (operand types are 'volatile gpio_out_w1ts_reg_t' and 'int')
```

**Causa:** exactamente la misma que el punto anterior — es la librería `TFT_eSPI` genérica compilando código escrito para los ESP32 clásicos (arquitectura Xtensa), que usan registros y macros (`VSPI`, escritura directa de registros GPIO) que no existen en el ESP32-C5 (RISC-V).

**Solución:** la misma — sacar la `TFT_eSPI` genérica del árbol de librerías (ver arriba).

## `fatal error: ../../lv_conf.h: No such file or directory`

**Causa:** LVGL necesita un archivo de configuración (`lv_conf.h`) que no se genera automáticamente al instalar la librería.

**Solución:**
1. Copia `lv_conf.h` (o `lv_conf_template.h`, renómbralo a `lv_conf.h`) desde `Documents/Arduino/libraries/SeeedStudio_lvgl/` a `Documents/Arduino/libraries/` (un nivel por encima, como archivo suelto).
2. Ábrelo y cambia `#if 0` por `#if 1` cerca del principio del archivo.
3. Cambia `LV_COLOR_DEPTH 32` por `LV_COLOR_DEPTH 16`.

Ver el detalle paso a paso en [`04-guia-version-pantalla.md`](04-guia-version-pantalla.md#c-configurar-lv_confh-paso-obligatorio-se-salta-fácilmente).

## `fatal error: TFT_eSPI.h: No such file or directory`

**Causa:** falta instalar `Seeed_GFX` (que es quien provee ese archivo, adaptado al C5).

**Solución:** instálala desde [github.com/Seeed-Studio/Seeed_GFX](https://github.com/Seeed-Studio/Seeed_GFX) (`Code → Download ZIP` → `Add .ZIP Library...`).

## `fatal error: Seeed_Arduino_FS.h: No such file or directory`

**Causa:** el archivo `Processors/TFT_eSPI_ESP32_C5.h` dentro de `Seeed_GFX` incluye esta librería únicamente si `SMOOTH_FONT` está activado en el Setup del combo.

**Solución (recomendada):** no instales `Seeed_Arduino_FS`. En su lugar, comenta la línea `#define SMOOTH_FONT` en:
```
Documents/Arduino/libraries/Seeed_GFX/User_Setups/Setup501_Seeed_XIAO_Round_Display.h
```
El proyecto no usa fuentes suavizadas, así que no se pierde nada.

## `multiple definition of 'fs::File::write...'` (conflicto SD/FS)

**Síntoma:** una cascada de errores de tipo "multiple definition" y "redefinition of class" mencionando `fs::File`, `fs::FS`, `fs::SDFS`, `sdcard_type_t`, terminando en `reference to 'SD' is ambiguous`.

**Causa:** si instalaste `Seeed_Arduino_FS` (por ejemplo porque `SMOOTH_FONT` seguía activo), esa librería define sus propias clases `fs::File`/`fs::FS`/`fs::SD`, que chocan con las que ya trae el core `esp32` (necesarias porque este proyecto usa `WebServer.h`).

**Solución:** la raíz del problema es tener `SMOOTH_FONT` activado sin necesitarlo. Comenta esa línea (ver punto anterior) y, si llegaste a instalar `Seeed_Arduino_FS`, puedes desinstalarla — este proyecto no la necesita.

## `invalid use of incomplete type 'lv_timer_t'` / errores de `lv_state_t`

**Síntoma:** errores como:
```
error: invalid use of incomplete type 'lv_timer_t' {aka 'struct _lv_timer_t'}
error: invalid conversion from 'int' to 'lv_state_t' [-fpermissive]
```

**Causa:** tienes instalada **LVGL v9.x** (la que ofrece el Gestor de Librerías por defecto), pero el código del proyecto (y el ejemplo oficial de Seeed) está escrito para **LVGL v8.3**. La v9 cambió deliberadamente el acceso a estas estructuras internas.

**Solución:**
1. Desinstala la `lvgl` genérica (`Tools → Manage Libraries...` → busca `lvgl` → `Remove`).
2. Instala **SeeedStudio_lvgl** desde [github.com/Seeed-Projects/SeeedStudio_lvgl](https://github.com/Seeed-Projects/SeeedStudio_lvgl) (fijada en v8.3).
3. Repite el paso de `lv_conf.h` con el archivo de **este** repo concreto (no reutilices uno viejo de la v9).

## `fatal error: I2C_BM8563.h: No such file or directory`

Esto solo aparece si usas el ejemplo oficial `HardwareTest` de Seeed tal cual (que incluye un RTC de demostración). **El firmware de este repositorio ya no usa el RTC ni la tarjeta SD** — si estás siguiendo únicamente los archivos de `firmware/`, no deberías encontrarte este error. Si lo ves, revisa que no estés mezclando archivos del ejemplo oficial con los de este proyecto.

## `exit status 0xc0000142`

**Causa:** no es un error de tu código — es un fallo del propio toolchain de compilación al arrancar en Windows, casi siempre por interferencia del antivirus con los muchos archivos temporales que genera compilar LVGL.

**Solución:**
1. Añade excepciones en tu antivirus para `AppData\Local\Arduino15\` y tu carpeta de sketches.
2. Reinicia el ordenador.
3. Verifica que tienes varios GB libres en el disco del sistema.
4. Si persiste, reinstala el paquete de placas `esp32` desde el Gestor de Placas.

## Pantalla en negro pero compila y sube bien

Si el firmware sube sin errores, el Monitor Serie muestra toda la secuencia de arranque correctamente, pero la pantalla sigue completamente en negro:

1. **Comprueba el interruptor KE** de la Round Display (posición ON).
2. **Revisa la orientación del montaje** — el USB-C de la XIAO debe quedar hacia el exterior de la pantalla.
3. **Fuerza el backlight manualmente** para descartar que sea solo la retroiluminación: añade `pinMode(D6, OUTPUT); digitalWrite(D6, HIGH);` justo después de `lv_xiao_disp_init()`.
4. **Prueba un test aislado sin LVGL**, con llamadas directas a `TFT_eSPI` (`tft.fillScreen(TFT_RED)`), para descartar toda la capa de LVGL de la ecuación.
5. Si ni con eso se ve nada, **revisa la soldadura** — ver [Fallo 1](#fallo-1-puente-de-soldadura). Un puente de estaño, incluso ya "arreglado" a medias, puede dejar algún pin con muy poco contacto tras limpiar el exceso.

## Hay que reflashear tras cada reset

Ver [Fallo 1: puente de soldadura](#fallo-1-puente-de-soldadura) — es prácticamente siempre esta causa, porque D1 (usado como `TFT_CS`) coincide con GPIO0, el pin de arranque del chip.

## El táctil no responde / el botón no reacciona

Este fue el fallo más largo de depurar del proyecto entero. Si el táctil no reacciona en absoluto:

1. **Confirma que el táctil funciona a nivel físico**, con un test aislado sin LVGL:
   ```cpp
   pinMode(TOUCH_INT, INPUT_PULLUP);
   Wire.begin();
   // en loop():
   if (chsc6x_is_pressed()) { ... }
   ```
   Si esto detecta el toque, el hardware está bien.

2. **No confíes en `lv_xiao_touch_init()`** — en las pruebas de este proyecto, esa función de la librería no registraba el táctil de forma fiable dentro de un proyecto completo (con AP + servidor web corriendo a la vez). La solución fue inicializar el táctil a mano y registrar un `lv_indev` propio:
   ```cpp
   pinMode(TOUCH_INT, INPUT_PULLUP);
   Wire.begin();

   lv_indev_drv_init(&indev_drv);
   indev_drv.type = LV_INDEV_TYPE_POINTER;
   indev_drv.read_cb = touchpad_read_cb; // tu propio callback con chsc6x_is_pressed()/chsc6x_get_xy()
   lv_indev_drv_register(&indev_drv);
   ```
   Esto ya está implementado así en `display_ui.cpp` de este repositorio.

3. **El fallo definitivo, y el más sutil: el reloj de LVGL nunca avanza en v8.** El ejemplo oficial de Seeed incluye:
   ```cpp
   #if LVGL_VERSION_MAJOR == 9
     lv_tick_set_cb(millis);
   #endif
   ```
   `lv_tick_set_cb()` es una función **exclusiva de LVGL v9**. Como este proyecto usa deliberadamente v8.3 (ver más arriba), ese bloque **nunca se ejecuta**, y en v8 nadie avanza el reloj interno de LVGL. Sin eso, **ningún `lv_timer_t` se dispara jamás** — ni el refresco de las etiquetas, ni la detección de "click" sobre un botón — aunque la pantalla se dibuje bien la primera vez (eso no depende del reloj) y el táctil detecte el dedo a nivel físico (tampoco depende del reloj).

   **Solución**, ya implementada en `display_ui.cpp` (`displayLoop()`):
   ```cpp
   #if LVGL_VERSION_MAJOR == 8
     static unsigned long lastTickMillis = 0;
     unsigned long nowMillis = millis();
     lv_tick_inc(nowMillis - lastTickMillis);
     lastTickMillis = nowMillis;
   #endif
   ```
   Esta llamada tiene que hacerse en cada vuelta del `loop()`, no una sola vez en `setup()`.

**Cómo diagnosticar esto tú mismo si vuelve a pasar:** compara `millis()` contra `lv_tick_get()` con un `Serial.printf` periódico. Si `millis()` avanza y `lv_tick_get()` se queda fijo, es exactamente este problema.

## Pocas redes detectadas o señal muy débil

Ver [`01-requisitos-hardware.md`](01-requisitos-hardware.md#️-aviso-crítico-la-antena) — casi siempre es la antena externa sin conectar al U.FL.

## El AP se desconecta o se vuelve inestable

**Causa:** el modo de ahorro de energía del radio Wi-Fi interfiere con el mantenimiento del punto de acceso.

**Solución:** ya está aplicada en `XIAO_WiFi_Analyzer.ino` de este repositorio:
```cpp
WiFi.setSleep(false);
```
Si sigues viendo desconexiones tras esto, prueba a alimentar la placa desde un cargador de pared dedicado en vez del puerto USB del PC — las ráfagas de transmisión Wi-Fi pueden provocar caídas de tensión momentáneas en puertos compartidos o de hubs.

## El PC no ve la red pero el móvil sí

Casi siempre es que, en el momento de comprobarlo, el AP ya se había desconectado por el problema anterior (más notorio en el PC porque hace un escaneo puntual, mientras el móvil puede seguir "recordando" la conexión un poco más). Aplica el fix de `WiFi.setSleep(false)` (ya incluido) y comprueba de nuevo justo después de un reset.

## Channel Analysis vacío

El panel de canales se centra deliberadamente en 2.4GHz (canales 1-13); si tu entorno solo tiene redes 5GHz detectadas, es esperado que aparezca sin datos. Si tienes redes 2.4GHz alrededor y aun así sale vacío, comprueba el Monitor Serie tras un escaneo — `scanner.cpp` imprime cada red detectada con su canal y banda clasificada, útil para confirmar si el escaneo las está viendo pero no clasificando bien, o si simplemente no las está detectando (de nuevo, revisa la antena).
