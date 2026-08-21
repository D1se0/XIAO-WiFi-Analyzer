#ifndef SCANNER_H
#define SCANNER_H

#include <Arduino.h>
#include "config.h"

enum WifiBand {
  BAND_2_4GHZ,
  BAND_5GHZ,
  BAND_UNKNOWN
};

enum ScanRunState {
  SCAN_STATE_IDLE,
  SCAN_STATE_RUNNING
};

struct NetworkEntry {
  char    ssid[33];
  uint8_t bssid[6];
  int32_t rssi;
  int32_t channel;
  uint8_t encType;
  WifiBand band;
  float   estDistanceM; // Estimacion APROXIMADA, no una medicion real (ver estimateDistanceMeters)
};

struct ScanResult {
  NetworkEntry networks[MAX_NETWORKS];
  int count;
  unsigned long lastScanMillis;
  unsigned long lastScanDurationMillis;
  bool scanEverRun;
};

struct ScanHistoryEntry {
  int total;
  int open;
  unsigned long ts;
};

extern ScanResult g_lastScan;
extern ScanHistoryEntry g_scanHistory[SCAN_HISTORY_SIZE];
extern int g_scanHistoryHead;
extern int g_scanHistoryFilled;
extern volatile ScanRunState g_scanState;
extern unsigned long g_scanStartMillis;

// Inicia un escaneo NO bloqueante (async). Llamar desde el handler HTTP.
void startWifiScan();

// Debe llamarse en cada loop() para comprobar si el escaneo async ha terminado.
void pollWifiScan();

WifiBand classifyBand(int32_t channel);
const char* bandToString(WifiBand band);
const char* rssiQuality(int32_t rssi);
const char* encTypeToString(uint8_t encType);
bool isOpenNetwork(uint8_t encType);
float estimateDistanceMeters(int32_t rssi);

#endif