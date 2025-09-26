#include "data_manager.h"
#include <Preferences.h>

AuthorizedUser authorizedUsers[MAX_USERS];
LogEntry accessLogs[LOG_MAX_SIZE];
int logCurrentIndex = 0;

Preferences prefs;

void initDataManager() {
  prefs.begin("nfc_system", false);
  // Para testes, descomente para limpar tudo:
  // prefs.clear();
  loadUsersFromPrefs();
  loadLogsFromPrefs();
}

void loadUsersFromPrefs() {
  size_t len = prefs.getBytes("users", authorizedUsers, sizeof(authorizedUsers));
  if (len != sizeof(authorizedUsers)) {
    memset(authorizedUsers, 0, sizeof(authorizedUsers));
  }
}

void saveUsersToPrefs() {
  prefs.putBytes("users", authorizedUsers, sizeof(authorizedUsers));
}

void loadLogsFromPrefs() {
  size_t len = prefs.getBytes("logs", accessLogs, sizeof(accessLogs));
  if (len != sizeof(accessLogs)) {
    memset(accessLogs, 0, sizeof(accessLogs));
  }
  logCurrentIndex = prefs.getInt("logIndex", 0);
}

void saveLogsToPrefs() {
  prefs.putBytes("logs", accessLogs, sizeof(accessLogs));
  prefs.putInt("logIndex", logCurrentIndex);
}

bool addUser(String uid, String name) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (strlen(authorizedUsers[i].uid) == 0) {
      uid.toCharArray(authorizedUsers[i].uid, 30);
      name.toCharArray(authorizedUsers[i].name, 30);
      Serial.println("Adicionando UID: " + uid + ", Nome: " + name);
      saveUsersToPrefs();
      Serial.println("Usuario salvo nas Preferences.");
      return true;
    }
  }
  return false;
}

bool deleteUser(int index) {
  if (index >= 0 && index < MAX_USERS) {
    memset(&authorizedUsers[index], 0, sizeof(AuthorizedUser));
    saveUsersToPrefs();
    return true;
  }
  return false;
}