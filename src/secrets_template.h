#pragma once

// ⚠️ Copie ce fichier sous le nom secrets.h et remplis les identifiants Wi-Fi
// Format générique pour multi-réseaux connus

struct WifiCredential {
  const char* ssid;
  const char* password;
};

const WifiCredential WIFI_KNOWN[] = {
  { "MonSSID_1", "MonMotDePasse_1" },
  { "MonSSID_2", "MonMotDePasse_2" },
  // Ajoute ou retire des réseaux ici
};

const int WIFI_KNOWN_COUNT = sizeof(WIFI_KNOWN) / sizeof(WifiCredential);
