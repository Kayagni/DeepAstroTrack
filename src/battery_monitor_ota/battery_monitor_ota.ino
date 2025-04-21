// VoltageMonitor_ADC.ino
// Lecture des tensions de cellules Li-ion via ADC + ponts diviseurs
// Ajout d'un serveur web pour affichage distant via Wi-Fi

#include <WiFi.h>
#include <WebServer.h>
#include "wifi_helper.h"
#include "ota_helper.h"

#define PIN_V1 34  // Tension cellule 1 (B1)
#define PIN_V2 35  // Tension cellule 2 (B2)
#define PIN_V3 32  // Tension cellule 3 (B3)
#define PIN_V4 33  // Tension cellule 4 (B4)
#define RT 10000.0
#define R1 3000.0
#define R2 16000.0
#define R3 29000.0
#define R4 42000.0

WebServer server(80);

float cell1 = 0, cell2 = 0, cell3 = 0, cell4 = 0;

float readVoltage(int pin) {
  int adc = analogRead(pin);
  float v_adc = (adc / 4095.0) * 3.3;
  return v_adc;
}

void handleRoot() {
  String html = "<html><head><meta http-equiv='refresh' content='5'/><title>Monitoring Batterie</title></head><body>";
  html += "<h1>ESP32 - Tension des cellules</h1>";
  html += "<p>Cellule 1 : " + String(cell1, 3) + " V</p>";
  html += "<p>Cellule 2 : " + String(cell2, 3) + " V</p>";
  html += "<p>Cellule 3 : " + String(cell3, 3) + " V</p>";
  html += "<p>Cellule 4 : " + String(cell4, 3) + " V</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wifi.mode(WIFI_STA);
  id (connectToStrongestKnownWifi()){
    setupOTA("ESP32_BatteryMonitor");
  }
  Serial.println("Lecture des tensions cellules (ADC)...");

  WiFi.begin(ssid, password);
  Serial.print("Connexion au Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté ! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Serveur HTTP démarré");

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  float V1 = readVoltage(PIN_V1) * ((RT + R1) / RT);
  float V2 = readVoltage(PIN_V2) * ((RT + R2) / RT);
  float V3 = readVoltage(PIN_V3) * ((RT + R3) / RT);
  float V4 = readVoltage(PIN_V4) * ((RT + R4) / RT);

  cell1 = V1;
  cell2 = V2 - V1;
  cell3 = V3 - V2;
  cell4 = V4 - V3;

  server.handleClient();
  delay(500); // rafraîchissement toutes les 0.5 secondes
}
