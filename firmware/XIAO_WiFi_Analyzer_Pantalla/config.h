#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// CONFIGURACIÓN DEL ACCESS POINT
// ============================================================
#define AP_SSID        "XIAO-WIFI-ANALYZER"
#define AP_PASSWORD    "analyzer2026"   // Mínimo 8 caracteres (WPA2). Cambia esto.
#define AP_CHANNEL     6                // Canal 2.4 GHz del propio AP (1-13)
#define AP_HIDDEN      0                // 0 = SSID visible, 1 = oculto
#define AP_MAX_CLIENTS 4                // Máximo de dispositivos conectados a la vez

// IP fija del AP
#define AP_IP_ADDR   192, 168, 4, 1
#define AP_GATEWAY   192, 168, 4, 1
#define AP_SUBNET    255, 255, 255, 0

// ============================================================
// SERVIDOR WEB Y DNS
// ============================================================
#define HTTP_PORT     80
#define DNS_PORT      53

// ============================================================
// ESCANEO WI-FI
// ============================================================
#define MAX_NETWORKS      30
#define SCAN_SHOW_HIDDEN  false
#define SCAN_PASSIVE      false
#define SCAN_MAX_MS_PER_CHANNEL 200

// ============================================================
// UMBRALES DE CALIDAD DE SEÑAL (RSSI en dBm)
// ============================================================
#define RSSI_EXCELLENT   -50
#define RSSI_GOOD        -60
#define RSSI_FAIR        -70
#define RSSI_WEAK        -80

// ============================================================
// HISTORIAL Y PROGRESO (NUEVO)
// ============================================================
#define SCAN_HISTORY_SIZE        8     // Nº de escaneos anteriores guardados en RAM
#define SCAN_PROGRESS_ESTIMATE_MS 6000 // Duracion estimada para animar la barra de progreso (no es un dato exacto del chip)

#endif