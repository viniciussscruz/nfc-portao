#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>

// Estados da aplicação
enum AppState {
  STATE_IDLE,
  STATE_WAITING_FOR_CARD_TO_ADD
};

void initWebServer();
void handleClient();
void addLog(String uid);

#endif