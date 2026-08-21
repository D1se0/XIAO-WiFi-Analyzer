#ifndef WIFI_STATE_H
#define WIFI_STATE_H

#include <Arduino.h>

// Estado agregado y compartido: lo consume tanto la pantalla como (en el futuro)
// cualquier otro consumidor. El dashboard web sigue usando /api/networks directamente,
// asi que esto no le afecta ni lo duplica.
struct WifiScanState {
  int totalNetworks;
  int openNetworks;
  int wpa2Networks;
  int wpa3Networks;
  bool everScanned;
  unsigned long lastScanMillis;
};

extern WifiScanState g_state;

void recomputeWifiState();

#endif