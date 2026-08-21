# Changelog

Formato basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/).

## [1.0.0] — Primer release público

### Añadido
- **Versión básica**: Access Point propio, escaneo Wi-Fi asíncrono (2.4GHz + 5GHz), servidor web con captive portal, dashboard completo (resumen por banda, distribución de RSSI, análisis de canales con recomendación, análisis de seguridad WPA3/WPA2/WPA/WEP/abiertas, historial de escaneos en RAM, estado del dispositivo, buscador y ordenación de redes, estimación de distancia por RSSI).
- **Versión con pantalla**: todo lo anterior + Round Display for XIAO (1.28" táctil) con 4 pantallas navegables por gesto (Home con medidor circular animado, lista de redes, análisis de canales, estado del dispositivo).
- API REST documentada (`/api/status`, `/api/networks`, `/api/scan`, `/api/scan/status`, `/api/history`, `/api/config`).
- Documentación completa de instalación, guía paso a paso para ambas versiones, y guía de solución de problemas con todos los errores reales encontrados durante el desarrollo.

### Corregido durante el desarrollo (documentado en detalle en `docs/05-solucion-problemas.md`)
- Conflicto entre la librería `TFT_eSPI` genérica y el fork `Seeed_GFX` adaptado al ESP32-C5.
- Incompatibilidad entre LVGL v9.x y el ejemplo oficial de Seeed (escrito para v8.3).
- Conflicto de definiciones entre `Seeed_Arduino_FS` y la librería `FS`/`SD` del core `esp32` (resuelto desactivando `SMOOTH_FONT`, no necesario en este proyecto).
- Registro manual del dispositivo táctil en LVGL, más fiable que `lv_xiao_touch_init()` en este combo concreto.
- Avance manual del reloj interno de LVGL (`lv_tick_inc()`) en LVGL v8, necesario porque `lv_tick_set_cb()` es una API exclusiva de v9 y el ejemplo oficial no cubre el porting a v8 para esta placa.
- Inestabilidad del Access Point por el modo de ahorro de energía del Wi-Fi (`WiFi.setSleep(false)`).
