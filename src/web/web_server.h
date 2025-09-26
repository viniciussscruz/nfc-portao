#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>

void initWebServer();
void handleClient();
void addAccessLog(String uid, bool authorized);
bool isAuthorized(String uid);

#endif