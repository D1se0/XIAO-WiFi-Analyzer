#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "config.h"
#include "scanner.h"
#include "api_handlers.h"
#include "wifi_state.h"
#include "display_ui.h"

WebServer server(HTTP_PORT);
DNSServer dnsServer;

void setupAccessPoint() {
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.setSleep(false);

  IPAddress apIP(AP_IP_ADDR);
  IPAddress gateway(AP_GATEWAY);
  IPAddress subnet(AP_SUBNET);
  WiFi.softAPConfig(apIP, gateway, subnet);

  bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, AP_HIDDEN, AP_MAX_CLIENTS);

  Serial.println(ok ? "[AP] Started OK" : "[AP] FAILED to start");
  Serial.print("[AP] SSID: ");
  Serial.println(AP_SSID);
  Serial.print("[AP] IP: ");
  Serial.println(WiFi.softAPIP());
}

void setupCaptivePortalDns() {
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== XIAO WiFi Analyzer ===");

  setupAccessPoint();
  setupCaptivePortalDns();

  registerStaticRoutes(server);
  registerApiRoutes(server);
  registerCaptivePortalRoutes(server);

  server.begin();
  Serial.println("[HTTP] Server started on port 80");
  Serial.println("Connect to the AP and open http://192.168.4.1");

  // AP, escaner y servidor ya estan arriba y funcionando ANTES de tocar la pantalla.
  // Si la pantalla fallara, todo lo anterior sigue operativo igualmente.
  displayInit();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  pollWifiScan();
  displayLoop();
}