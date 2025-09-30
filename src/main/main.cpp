#include <Arduino.h>
#include "hw/hardware_control.h"
#include "web/web_server.h"
#include "main/data_manager.h"
#include <HTTPClient.h>

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
    if (authorized) {
      HTTPClient http;
      http.begin("http://192.168.15.75/cm?user=admin&password=hazeng4N&cmnd=Power4%20ON");
      int code = http.GET();
      if (code == 200) {
        Serial.println("Pulso de 500 ms enviado com sucesso via Tasmota.");
      } else {
        Serial.println("Erro ao enviar pulso: " + String(code));
      }
      http.end();
    }
  }
}