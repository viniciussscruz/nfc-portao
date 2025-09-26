#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>

const int MAX_USERS = 10;
const int LOG_MAX_SIZE = 30;

struct AuthorizedUser {
  char uid[30];
  char name[30];
};

struct LogEntry {
  char uid[30];
  char timestamp[20];
  char status[20];
};

void initDataManager();
void loadUsersFromPrefs();
void saveUsersToPrefs();
void loadLogsFromPrefs();
void saveLogsToPrefs();
bool addUser(String uid, String name);
bool deleteUser(int index);
extern AuthorizedUser authorizedUsers[MAX_USERS];
extern LogEntry accessLogs[LOG_MAX_SIZE];
extern int logCurrentIndex;

#endif