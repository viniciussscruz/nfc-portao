#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>

const int MAX_USERS = 10;
const int EEPROM_SIZE = sizeof(AuthorizedUser) * MAX_USERS; // Tamanho para guardar os dados

struct AuthorizedUser {
  char uid[30];
  char name[30];
};

void initDataManager();
void loadUsersFromEEPROM();
void saveUsersToEEPROM();
bool addUser(String uid, String name);
bool deleteUser(int index);
extern AuthorizedUser authorizedUsers[MAX_USERS];

#endif