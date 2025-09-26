#include <Arduino.h>
#include "../hw/hardware_control.h"
#include "../web/web_server.h"
#include "../main/data_manager.h"

// Referência para o estado definido no web_server
extern AppState currentState;
extern String lastReadUIDForAdd;

void setup() {
  Serial.begin(115200);
  
  initDataManager();
  initNFC();
  initWebServer();

  Serial.println("Setup concluido. Sistema pronto.");
}
void loop() {
  // 1. Lida com requisições web
  handleClient();

    // 2. Se estiver no modo de adicionar cartão, tenta ler
  if (currentState == STATE_WAITING_FOR_CARD_TO_ADD) {
    String uid = readNFC();
    if (uid != "") {
      Serial.println("Cartao lido para cadastro: " + uid);
      lastReadUIDForAdd = uid;
      currentState = STATE_IDLE; // Volta ao estado normal para mostrar o formulário
    }
  } else {
    // 3. Em modo normal, apenas lê e adiciona ao log
    String uid = readNFC();
    if (uid != "") {
       Serial.println("Cartao lido no log: " + uid);
       addLog(uid);
    }
  }
}