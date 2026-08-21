# 6. Referencia de la API REST

Todas las rutas están servidas por la propia XIAO en `http://192.168.4.1` (o la IP que configures en `config.h`), definidas en `api_handlers.cpp`. No requieren autenticación — el propio Access Point ya actúa como control de acceso (solo quien tiene la contraseña Wi-Fi puede llegar a estos endpoints).

## Rutas estáticas (dashboard)

| Ruta | Método | Descripción |
|---|---|---|
| `/` | GET | Dashboard HTML principal |
| `/style.css` | GET | Hoja de estilos del dashboard |
| `/app.js` | GET | JavaScript del dashboard (vanilla, sin frameworks) |

## API de datos

### `GET /api/status`

Estado general del dispositivo.

```json
{
  "ssid": "XIAO-WIFI-ANALYZER",
  "ip": "192.168.4.1",
  "clients": 1,
  "freeHeapBytes": 187234,
  "uptimeMillis": 45231,
  "lastScanEverRun": true,
  "scanning": false
}
```

### `GET /api/config`

Configuración activa del Access Point.

```json
{
  "apSsid": "XIAO-WIFI-ANALYZER",
  "apIp": "192.168.4.1",
  "maxNetworks": 30,
  "httpPort": 80
}
```

### `GET /api/networks`

Resultado del **último** escaneo (no dispara uno nuevo).

```json
{
  "count": 12,
  "lastScanMillis": 45230,
  "lastScanDurationMillis": 2840,
  "summary": {
    "total": 12, "open": 1, "band24": 8, "band5": 4,
    "wpa3": 2, "wpa2": 8, "wpa": 0, "wep": 0, "otherSecure": 0,
    "rssi": { "excellent": 3, "good": 5, "fair": 2, "weak": 1, "veryWeak": 1 }
  },
  "channels24": [0,3,0,1,0,2,0,0,0,0,1,0,1],
  "networks": [
    {
      "ssid": "MiRedWiFi",
      "bssid": "AA:BB:CC:DD:EE:FF",
      "rssi": -45,
      "quality": "Excellent",
      "channel": 6,
      "band": "2.4GHz",
      "security": "WPA2",
      "open": false,
      "estDistanceM": "3.2"
    }
  ]
}
```

> `estDistanceM` es una **estimación aproximada** basada únicamente en RSSI (fórmula log-distance path loss genérica). No es una medición real de distancia — paredes, materiales y orientación de antena la desvían fácilmente en un factor de 2-3x.

### `GET /api/scan/status`

Estado del escaneo asíncrono en curso (o del último completado). Pensado para hacer *polling* mientras se muestra la barra de progreso.

```json
{
  "scanning": true,
  "lastScanMillis": 40000,
  "lastScanDurationMillis": 0,
  "estimateMs": 6000
}
```

### `POST /api/scan`

Inicia un nuevo escaneo **asíncrono** (no bloqueante — el resto del firmware sigue funcionando mientras escanea).

- Si no hay ningún escaneo en curso, lo inicia y responde:
  ```json
  { "status": "started" }
  ```
- Si ya hay uno en curso, no inicia otro y responde:
  ```json
  { "status": "already_running" }
  ```

El resultado no se devuelve en esta misma respuesta — hay que consultar `/api/scan/status` hasta que `scanning` pase a `false`, y entonces leer `/api/networks`.

### `GET /api/history`

Historial en RAM de los últimos escaneos (tamaño configurable con `SCAN_HISTORY_SIZE` en `config.h`, por defecto 8). Se pierde al reiniciar la placa — no hay almacenamiento persistente.

```json
{
  "count": 3,
  "entries": [
    { "total": 10, "open": 0, "ts": 12000 },
    { "total": 12, "open": 1, "ts": 34000 },
    { "total": 11, "open": 1, "ts": 58000 }
  ]
}
```

## Endpoints de captive portal

Estas rutas responden con el propio dashboard, y existen para favorecer que el sistema operativo del dispositivo conectado detecte automáticamente el portal cautivo:

- `/generate_204`, `/gen_204` (Android)
- `/hotspot-detect.html`, `/library/test/success.html` (Apple / iOS / macOS)
- `/ncsi.txt`, `/connecttest.txt` (Windows)
- Cualquier otra ruta no reconocida (`onNotFound`)

No es necesario llamarlas manualmente — las gestiona el propio sistema operativo del dispositivo cliente.

## Ejemplo de uso desde `curl`

```bash
# Ver estado
curl http://192.168.4.1/api/status

# Lanzar un escaneo
curl -X POST http://192.168.4.1/api/scan

# Esperar y consultar resultados
curl http://192.168.4.1/api/scan/status
curl http://192.168.4.1/api/networks
```
