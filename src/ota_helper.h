#pragma once
#include <ArduinoOTA.h>

void setupOTA(const char* hostname = "DeepAstroESP32") {
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() { Serial.println("OTA : démarrage..."); });
  ArduinoOTA.onEnd([]() { Serial.println("\nOTA : terminé."); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA : %u%%\r", (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Erreur [%u] : ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
