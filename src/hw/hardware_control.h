#ifndef HARDWARE_CONTROL_H
#define HARDWARE_CONTROL_H

#include <Arduino.h>

bool initNFC();
String readNFC();
extern bool sensorOnline;

#endif