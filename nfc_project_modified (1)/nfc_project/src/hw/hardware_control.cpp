#include "hardware_control.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

const int SDA_PIN = 8;
const int SCL_PIN = 9;
Adafruit_PN532 nfc(SCL_PIN, SDA_PIN);

bool sensorOnline = false;

bool initNFC() {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    sensorOnline = false;
  } else {
    nfc.SAMConfig();
    sensorOnline = true;
  }
  return sensorOnline;
}

String readNFC() {
  if (!sensorOnline) return "";

  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);

  if (success) {
    String uidString = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) uidString += "0";
      uidString += String(uid[i], HEX);
      if (i < uidLength - 1) uidString += ":";
    }
    uidString.toUpperCase();
    
    while (nfc.inListPassiveTarget()) { delay(50); }
    return uidString;
  }
  return "";
}