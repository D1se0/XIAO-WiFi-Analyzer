#include "wifi_state.h"
#include "scanner.h"

WifiScanState g_state = {0, 0, 0, 0, false, 0};

void recomputeWifiState() {
  g_state.totalNetworks = g_lastScan.count;
  g_state.openNetworks = 0;
  g_state.wpa2Networks = 0;
  g_state.wpa3Networks = 0;

  for (int i = 0; i < g_lastScan.count; i++) {
    NetworkEntry &e = g_lastScan.networks[i];
    if (isOpenNetwork(e.encType)) g_state.openNetworks++;
    const char* sec = encTypeToString(e.encType);
    if (strstr(sec, "WPA3")) g_state.wpa3Networks++;
    else if (strstr(sec, "WPA2")) g_state.wpa2Networks++;
  }

  g_state.everScanned = g_lastScan.scanEverRun;
  g_state.lastScanMillis = g_lastScan.lastScanMillis;
}