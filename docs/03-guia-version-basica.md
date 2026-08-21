# 3. Guía: versión básica (sin pantalla)

Requisitos previos: haber completado [`02-instalacion-entorno.md`](02-instalacion-entorno.md) y confirmado que el sketch `HOLA` funciona en tu placa.

## 3.1 Librerías necesarias

**Ninguna.** Esta versión usa exclusivamente librerías incluidas en el core `esp32`:
- `WiFi.h`
- `WebServer.h`
- `DNSServer.h`

No hay que instalar nada de terceros para esta versión. Esto es intencional: menos dependencias, más estabilidad, compilación mucho más rápida.

## 3.2 Descargar y abrir el proyecto

1. Descarga la carpeta [`firmware/XIAO_WiFi_Analyzer_Basico`](../firmware/XIAO_WiFi_Analyzer_Basico) de este repositorio (o clona el repo entero).
2. Ábrela con Arduino IDE haciendo doble clic en `XIAO_WiFi_Analyzer.ino`. Deberías ver **7 pestañas** abiertas a la vez:
   - `XIAO_WiFi_Analyzer.ino`
   - `config.h`
   - `scanner.h` / `scanner.cpp`
   - `api_handlers.h` / `api_handlers.cpp`
   - `web_content.h`

   Si solo ves el `.ino` y no las demás pestañas, es que la carpeta contenedora no se llama igual que el `.ino` (Arduino exige que coincidan) — asegúrate de que la carpeta se llama `XIAO_WiFi_Analyzer` y contiene `XIAO_WiFi_Analyzer.ino` dentro.

## 3.3 Configuración antes de compilar (opcional pero recomendado)

Abre `config.h` y revisa/cambia si quieres:

```cpp
#define AP_SSID        "XIAO-WIFI-ANALYZER"
#define AP_PASSWORD    "analyzer2026"   // Cambia esto por tu propia contraseña
#define AP_CHANNEL     6
```

El resto de valores (umbrales de RSSI, tamaño de historial, número máximo de redes) están documentados con comentarios en el propio archivo y no hace falta tocarlos para el uso normal.

## 3.4 Compilar y cargar

1. `Tools → Board → XIAO_ESP32C5` (si no lo tenías ya seleccionado).
2. `Tools → Port → [tu puerto]`.
3. `Sketch → Verify/Compile`. Sin librerías de terceros, esto debería tardar poco (unos segundos a un par de minutos, según tu equipo).
4. `Sketch → Upload`. Si falla con `Failed to start stub flasher`, usa el truco de **BOOT + RESET** descrito en la sección anterior.

## 3.5 Comprobar que funciona

1. Abre el Monitor Serie (115200 baudios) y pulsa **RESET** una vez. Deberías ver:
   ```
   === XIAO WiFi Analyzer ===
   [AP] Started OK
   [AP] SSID: XIAO-WIFI-ANALYZER
   [AP] IP: 192.168.4.1
   [HTTP] Server started on port 80
   Connect to the AP and open http://192.168.4.1
   ```
2. Desde tu móvil o PC, busca la red Wi-Fi **`XIAO-WIFI-ANALYZER`** y conéctate con la contraseña que hayas puesto en `config.h`.
3. Es normal que el dispositivo avise de "red sin Internet" — acepta/mantén la conexión igualmente.
4. Espera unos segundos por si aparece la notificación de "Iniciar sesión en la red" (captive portal). Si no aparece, abre manualmente el navegador en:
   ```
   http://192.168.4.1
   ```
5. Pulsa **SCAN NETWORKS** y observa la barra de progreso, seguida de los resultados: resumen por banda, distribución de señal, análisis de canales, análisis de seguridad, e historial de escaneos.

## 3.6 Qué esperar si no ves redes o el RSSI es pésimo

Revisa **primero** que la antena esté conectada — ver [`01-requisitos-hardware.md`](01-requisitos-hardware.md#️-aviso-crítico-la-antena). Es, con diferencia, la causa más común.

## 3.7 Estructura de archivos de esta versión

```
XIAO_WiFi_Analyzer_Basico/
├── XIAO_WiFi_Analyzer.ino   → setup()/loop(): arranque de AP, DNS y servidor web
├── config.h                  → toda la configuración (SSID, password, umbrales...)
├── scanner.h / scanner.cpp   → escaneo asíncrono, clasificación de banda/RSSI/seguridad
├── api_handlers.h / .cpp     → rutas HTTP + construcción manual de JSON (sin librerías)
└── web_content.h             → dashboard completo: HTML + CSS + JS embebidos en PROGMEM
```

Consulta [`06-referencia-api.md`](06-referencia-api.md) para el detalle de cada endpoint que expone `api_handlers.cpp`.

## 3.8 Siguiente paso

Si todo esto funciona con normalidad, y quieres añadir la pantalla táctil, continúa con [`04-guia-version-pantalla.md`](04-guia-version-pantalla.md).
