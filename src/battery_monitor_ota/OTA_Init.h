// OTA_Init.h
#pragma once

#include <WiFi.h>
#include <ArduinoOTA.h>

void setupOTA(const char* hostname = "DeepAstroTrack_Battery_Monitor") {
  ArduinoOTA.setHostname(hostname);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
    else type = "filesystem";
    Serial.println("[OTA] Début mise à jour: " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Fin de la mise à jour.");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progression : %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Erreur [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.setPassword("DeepAstroTrack");
  ArduinoOTA.begin();
  Serial.println("[OTA] Prêt pour les mises à jour OTA");
}

void handleOTA() {
  ArduinoOTA.handle();
}