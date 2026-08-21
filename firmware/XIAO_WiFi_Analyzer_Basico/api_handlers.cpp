#include "api_handlers.h"
#include "scanner.h"
#include "web_content.h"
#include "config.h"
#include <WiFi.h>

static char jsonBuffer[8192];
static char historyBuffer[512];

static size_t buildNetworksJson(char *buf, size_t bufSize) {
  int band24 = 0, band5 = 0, openCount = 0;
  int wpa3 = 0, wpa2 = 0, wpa = 0, wep = 0, otherSec = 0;
  int rssiExcellent = 0, rssiGood = 0, rssiFair = 0, rssiWeak = 0, rssiVeryWeak = 0;
  int channels24[14] = {0}; // indices 1..13 usados

  for (int i = 0; i < g_lastScan.count; i++) {
    NetworkEntry &e = g_lastScan.networks[i];

    if (e.band == BAND_2_4GHZ) {
      band24++;
      if (e.channel >= 1 && e.channel <= 13) channels24[e.channel]++;
    } else if (e.band == BAND_5GHZ) {
      band5++;
    }

    if (isOpenNetwork(e.encType)) openCount++;

    const char* sec = encTypeToString(e.encType);
    if (strstr(sec, "WPA3")) wpa3++;
    else if (strstr(sec, "WPA2")) wpa2++;
    else if (strcmp(sec, "WEP") == 0) wep++;
    else if (strstr(sec, "WPA")) wpa++;
    else if (!isOpenNetwork(e.encType)) otherSec++;

    const char* q = rssiQuality(e.rssi);
    if (strcmp(q, "Excellent") == 0) rssiExcellent++;
    else if (strcmp(q, "Good") == 0) rssiGood++;
    else if (strcmp(q, "Fair") == 0) rssiFair++;
    else if (strcmp(q, "Weak") == 0) rssiWeak++;
    else rssiVeryWeak++;
  }

  size_t offset = 0;
  offset += snprintf(buf + offset, bufSize - offset,
    "{\"count\":%d,\"lastScanMillis\":%lu,\"lastScanDurationMillis\":%lu,"
    "\"summary\":{\"total\":%d,\"open\":%d,\"band24\":%d,\"band5\":%d,"
    "\"wpa3\":%d,\"wpa2\":%d,\"wpa\":%d,\"wep\":%d,\"otherSecure\":%d,"
    "\"rssi\":{\"excellent\":%d,\"good\":%d,\"fair\":%d,\"weak\":%d,\"veryWeak\":%d}},"
    "\"channels24\":[",
    g_lastScan.count, g_lastScan.lastScanMillis, g_lastScan.lastScanDurationMillis,
    g_lastScan.count, openCount, band24, band5,
    wpa3, wpa2, wpa, wep, otherSec,
    rssiExcellent, rssiGood, rssiFair, rssiWeak, rssiVeryWeak);

  for (int ch = 1; ch <= 13; ch++) {
    offset += snprintf(buf + offset, bufSize - offset, "%s%d", (ch > 1 ? "," : ""), channels24[ch]);
  }
  offset += snprintf(buf + offset, bufSize - offset, "],\"networks\":[");

  for (int i = 0; i < g_lastScan.count; i++) {
    NetworkEntry &e = g_lastScan.networks[i];

    char safeSsid[65];
    int p = 0;
    for (int k = 0; e.ssid[k] != '\0' && p < (int)sizeof(safeSsid) - 2; k++) {
      char c = e.ssid[k];
      if (c == '"' || c == '\\') safeSsid[p++] = '\\';
      safeSsid[p++] = c;
    }
    safeSsid[p] = '\0';

    char bssidStr[18];
    snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X",
      e.bssid[0], e.bssid[1], e.bssid[2], e.bssid[3], e.bssid[4], e.bssid[5]);

    char distBuf[12];
    dtostrf(e.estDistanceM, 4, 1, distBuf);
    // recorta espacios iniciales que deja dtostrf al alinear a la derecha
    char *distTrim = distBuf;
    while (*distTrim == ' ') distTrim++;

    offset += snprintf(buf + offset, bufSize - offset,
      "%s{\"ssid\":\"%s\",\"bssid\":\"%s\",\"rssi\":%ld,\"quality\":\"%s\","
      "\"channel\":%ld,\"band\":\"%s\",\"security\":\"%s\",\"open\":%s,\"estDistanceM\":%s}",
      (i > 0 ? "," : ""),
      safeSsid, bssidStr, (long)e.rssi, rssiQuality(e.rssi),
      (long)e.channel, bandToString(e.band), encTypeToString(e.encType),
      isOpenNetwork(e.encType) ? "true" : "false",
      distTrim
    );

    if (offset >= bufSize - 250) break;
  }

  offset += snprintf(buf + offset, bufSize - offset, "]}");
  return offset;
}

static size_t buildHistoryJson(char *buf, size_t bufSize) {
  size_t offset = 0;
  offset += snprintf(buf + offset, bufSize - offset, "{\"count\":%d,\"entries\":[", g_scanHistoryFilled);

  int start = (g_scanHistoryFilled < SCAN_HISTORY_SIZE) ? 0 : g_scanHistoryHead;
  for (int k = 0; k < g_scanHistoryFilled; k++) {
    int idx = (start + k) % SCAN_HISTORY_SIZE;
    offset += snprintf(buf + offset, bufSize - offset, "%s{\"total\":%d,\"open\":%d,\"ts\":%lu}",
      (k > 0 ? "," : ""), g_scanHistory[idx].total, g_scanHistory[idx].open, g_scanHistory[idx].ts);
  }

  offset += snprintf(buf + offset, bufSize - offset, "]}");
  return offset;
}

void registerStaticRoutes(WebServer &server) {
  server.on("/", HTTP_GET, [&server]() {
    server.send_P(200, "text/html", INDEX_HTML);
  });
  server.on("/style.css", HTTP_GET, [&server]() {
    server.send_P(200, "text/css", STYLE_CSS);
  });
  server.on("/app.js", HTTP_GET, [&server]() {
    server.send_P(200, "application/javascript", APP_JS);
  });
}

void registerApiRoutes(WebServer &server) {
  server.on("/api/status", HTTP_GET, [&server]() {
    char buf[380];
    snprintf(buf, sizeof(buf),
      "{\"ssid\":\"%s\",\"ip\":\"%s\",\"clients\":%d,\"freeHeapBytes\":%u,"
      "\"uptimeMillis\":%lu,\"lastScanEverRun\":%s,\"scanning\":%s}",
      AP_SSID,
      WiFi.softAPIP().toString().c_str(),
      WiFi.softAPgetStationNum(),
      (unsigned)ESP.getFreeHeap(),
      millis(),
      g_lastScan.scanEverRun ? "true" : "false",
      (g_scanState == SCAN_STATE_RUNNING) ? "true" : "false"
    );
    server.send(200, "application/json", buf);
  });

  server.on("/api/config", HTTP_GET, [&server]() {
    char buf[200];
    snprintf(buf, sizeof(buf),
      "{\"apSsid\":\"%s\",\"apIp\":\"%s\",\"maxNetworks\":%d,\"httpPort\":%d}",
      AP_SSID, WiFi.softAPIP().toString().c_str(), MAX_NETWORKS, HTTP_PORT);
    server.send(200, "application/json", buf);
  });

  server.on("/api/networks", HTTP_GET, [&server]() {
    buildNetworksJson(jsonBuffer, sizeof(jsonBuffer));
    server.send(200, "application/json", jsonBuffer);
  });

  server.on("/api/scan/status", HTTP_GET, [&server]() {
    char buf[160];
    snprintf(buf, sizeof(buf),
      "{\"scanning\":%s,\"lastScanMillis\":%lu,\"lastScanDurationMillis\":%lu,\"estimateMs\":%d}",
      (g_scanState == SCAN_STATE_RUNNING) ? "true" : "false",
      g_lastScan.lastScanMillis, g_lastScan.lastScanDurationMillis, SCAN_PROGRESS_ESTIMATE_MS);
    server.send(200, "application/json", buf);
  });

  server.on("/api/history", HTTP_GET, [&server]() {
    buildHistoryJson(historyBuffer, sizeof(historyBuffer));
    server.send(200, "application/json", historyBuffer);
  });

  server.on("/api/scan", HTTP_POST, [&server]() {
    if (g_scanState == SCAN_STATE_RUNNING) {
      server.send(200, "application/json", "{\"status\":\"already_running\"}");
    } else {
      startWifiScan();
      server.send(200, "application/json", "{\"status\":\"started\"}");
    }
  });
}

void registerCaptivePortalRoutes(WebServer &server) {
  const char *captiveProbes[] = {
    "/generate_204",
    "/gen_204",
    "/hotspot-detect.html",
    "/library/test/success.html",
    "/ncsi.txt",
    "/connecttest.txt"
  };
  for (size_t i = 0; i < sizeof(captiveProbes) / sizeof(captiveProbes[0]); i++) {
    server.on(captiveProbes[i], HTTP_GET, [&server]() {
      server.send_P(200, "text/html", INDEX_HTML);
    });
  }

  server.onNotFound([&server]() {
    server.send_P(200, "text/html", INDEX_HTML);
  });
}