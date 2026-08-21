#include "scanner.h"
#include "config.h"
#include <WiFi.h>
#include <math.h>

ScanResult g_lastScan = { {}, 0, 0, 0, false };
ScanHistoryEntry g_scanHistory[SCAN_HISTORY_SIZE];
int g_scanHistoryHead = 0;
int g_scanHistoryFilled = 0;

volatile ScanRunState g_scanState = SCAN_STATE_IDLE;
unsigned long g_scanStartMillis = 0;

WifiBand classifyBand(int32_t channel) {
  if (channel >= 1 && channel <= 14) return BAND_2_4GHZ;
  if (channel >= 36 && channel <= 173) return BAND_5GHZ;
  return BAND_UNKNOWN;
}

const char* bandToString(WifiBand band) {
  switch (band) {
    case BAND_2_4GHZ: return "2.4GHz";
    case BAND_5GHZ:   return "5GHz";
    default:          return "Unknown";
  }
}

const char* rssiQuality(int32_t rssi) {
  if (rssi >= RSSI_EXCELLENT) return "Excellent";
  if (rssi >= RSSI_GOOD)      return "Good";
  if (rssi >= RSSI_FAIR)      return "Fair";
  if (rssi >= RSSI_WEAK)      return "Weak";
  return "Very Weak";
}

bool isOpenNetwork(uint8_t encType) {
  return encType == WIFI_AUTH_OPEN;
}

const char* encTypeToString(uint8_t encType) {
  switch (encType) {
    case WIFI_AUTH_OPEN:            return "Open";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
    case WIFI_AUTH_WPA3_PSK:        return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/WPA3";
    case WIFI_AUTH_WAPI_PSK:        return "WAPI";
    default:                        return "Unknown";
  }
}

float estimateDistanceMeters(int32_t rssi) {
  // ESTIMACION APROXIMADA, NO una medicion real de distancia.
  // Se basa en una formula generica de perdida de trayecto (log-distance path loss),
  // asumiendo una potencia de referencia tipica a 1 metro (-40 dBm) y un exponente de
  // perdida de entorno interior "medio" (2.5). Paredes, materiales y orientacion de
  // antena pueden desviar este valor facilmente en un factor de 2-3x. Es orientativo.
  const float refRssiAt1m = -40.0f;
  const float pathLossExponent = 2.5f;
  float ratio = (refRssiAt1m - (float)rssi) / (10.0f * pathLossExponent);
  return pow(10.0f, ratio);
}

static void pushScanHistory(int total, int open) {
  int idx = g_scanHistoryHead;
  g_scanHistory[idx].total = total;
  g_scanHistory[idx].open = open;
  g_scanHistory[idx].ts = millis();
  g_scanHistoryHead = (g_scanHistoryHead + 1) % SCAN_HISTORY_SIZE;
  if (g_scanHistoryFilled < SCAN_HISTORY_SIZE) g_scanHistoryFilled++;
}

void startWifiScan() {
  if (g_scanState == SCAN_STATE_RUNNING) return;

#if defined(WIFI_BAND_MODE_AUTO)
  WiFi.setBandMode(WIFI_BAND_MODE_AUTO);
#endif

  WiFi.scanNetworks(
    /*async=*/true,
    /*show_hidden=*/SCAN_SHOW_HIDDEN,
    /*passive=*/SCAN_PASSIVE,
    /*max_ms_per_chan=*/SCAN_MAX_MS_PER_CHANNEL
  );

  g_scanState = SCAN_STATE_RUNNING;
  g_scanStartMillis = millis();
  Serial.println("[SCAN] Escaneo asincrono iniciado");
}

void pollWifiScan() {
  if (g_scanState != SCAN_STATE_RUNNING) return;

  int n = WiFi.scanComplete();

  if (n == WIFI_SCAN_RUNNING) return; // sigue en curso

  if (n == WIFI_SCAN_FAILED) {
    Serial.println("[SCAN] Fallo el escaneo");
    WiFi.scanDelete();
    g_lastScan.count = 0;
    g_lastScan.lastScanMillis = millis();
    g_lastScan.lastScanDurationMillis = millis() - g_scanStartMillis;
    g_lastScan.scanEverRun = true;
    pushScanHistory(0, 0);
    g_scanState = SCAN_STATE_IDLE;
    return;
  }

  Serial.print("[SCAN] Redes encontradas (crudo): ");
  Serial.println(n);

  int limit = (n > MAX_NETWORKS) ? MAX_NETWORKS : n;
  int openCount = 0;

  for (int i = 0; i < limit; i++) {
    NetworkEntry &e = g_lastScan.networks[i];

    String ssid;
    uint8_t encType;
    int32_t rssi;
    uint8_t *bssidPtr;
    int32_t channel;

    WiFi.getNetworkInfo(i, ssid, encType, rssi, bssidPtr, channel);

    strncpy(e.ssid, ssid.c_str(), sizeof(e.ssid) - 1);
    e.ssid[sizeof(e.ssid) - 1] = '\0';
    if (ssid.length() == 0) {
      strncpy(e.ssid, "(hidden)", sizeof(e.ssid) - 1);
    }

    if (bssidPtr != nullptr) {
      memcpy(e.bssid, bssidPtr, 6);
    } else {
      memset(e.bssid, 0, 6);
    }

    e.rssi = rssi;
    e.channel = channel;
    e.encType = encType;
    e.band = classifyBand(channel);
    e.estDistanceM = estimateDistanceMeters(rssi);

    if (isOpenNetwork(e.encType)) openCount++;

    Serial.printf("[SCAN] %2d) SSID=\"%s\" CH=%ld BAND=%s RSSI=%ld\n",
      i, e.ssid, (long)e.channel, bandToString(e.band), (long)e.rssi);
  }

  g_lastScan.count = limit;
  g_lastScan.lastScanMillis = millis();
  g_lastScan.lastScanDurationMillis = millis() - g_scanStartMillis;
  g_lastScan.scanEverRun = true;

  pushScanHistory(limit, openCount);

  WiFi.scanDelete();
  g_scanState = SCAN_STATE_IDLE;
}