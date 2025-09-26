#include "data_manager.h"
#include <EEPROM.h>

AuthorizedUser authorizedUsers[MAX_USERS];

void initDataManager() {
  EEPROM.begin(EEPROM_SIZE);
  // Opcional: Limpar a EEPROM na primeira inicialização ou se corrompida
  // Para testes, pode-se descomentar a linha abaixo para limpar a EEPROM a cada inicialização
  // for (int i = 0; i < EEPROM_SIZE; i++) { EEPROM.write(i, 0); }
  // EEPROM.commit();
  loadUsersFromEEPROM();
}

void loadUsersFromEEPROM() {
  EEPROM.get(0, authorizedUsers);
}

void saveUsersToEEPROM() {
  EEPROM.put(0, authorizedUsers);
  EEPROM.commit();
}

bool addUser(String uid, String name) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (strlen(authorizedUsers[i].uid) == 0) { // Encontra um espaço vazio
      uid.toCharArray(authorizedUsers[i].uid, 30);
      name.toCharArray(authorizedUsers[i].name, 30);
      Serial.println("Adicionando UID: " + uid + ", Nome: " + name);
      saveUsersToEEPROM();
      Serial.println("Usuario salvo na EEPROM.");
      return true;
    }
  }
  return false; // Lista cheia
}

bool deleteUser(int index) {
  if (index >= 0 && index < MAX_USERS) {
    strcpy(authorizedUsers[index].uid, "");
    strcpy(authorizedUsers[index].name, "");
    saveUsersToEEPROM();
    return true;
  }
  return false;
}