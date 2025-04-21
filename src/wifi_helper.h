#pragma once
#include <WiFi.h>
#include "secrets.h" // pour WIFI_KNOWN[]

bool connectToStrongestKnownWiFi(int timeout = 10000) {
  int n = WiFi.scanNetworks();
  if (n <= 0) return false;

  int bestIdx = -1;
  int bestRSSI = -1000;

  for (int i = 0; i < n; i++) {
    String scannedSSID = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);

    for (int j = 0; j < WIFI_KNOWN_COUNT; j++) {
      if (scannedSSID == WIFI_KNOWN[j].ssid && rssi > bestRSSI) {
        bestRSSI = rssi;
        bestIdx = j;
      }
    }
  }

  if (bestIdx >= 0) {
    WiFi.begin(WIFI_KNOWN[bestIdx].ssid, WIFI_KNOWN[bestIdx].password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) delay(500);
    return WiFi.status() == WL_CONNECTED;
  }

  return false;
}
