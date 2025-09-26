#include <Arduino.h>
#include "hw/hardware_control.h"
#include "web/web_server.h"
#include "main/data_manager.h"

void setup() {
  Serial.begin(115200);
  
  initDataManager();
  initNFC();
  initWebServer();

  Serial.println("Setup concluido. Sistema pronto.");
}

void loop() {
  handleClient();

  String uid = readNFC();
  if (uid != "") {
    Serial.println("Cartao lido: " + uid);
    bool authorized = isAuthorized(uid);
    addAccessLog(uid, authorized);
    // Adicione ação: ex. if (authorized) { digitalWrite(LED_PIN, HIGH); }
  }
}