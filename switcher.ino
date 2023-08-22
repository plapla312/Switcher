#include <RCSwitch.h>
#include <LiquidCrystal.h>
#include "EEPROM.h"

RCSwitch mySwitch = RCSwitch();
#define RELAY_R1 3
#define RELAY_R2 1
#define RELAY_R3 0
#define RELAY_R4 19

#define recPin 7
#define delPin 4

struct myRemotes {
  unsigned long int myKey;
  byte myRelay;
};

const int REMOTE_CNT = 80;

myRemotes remotes[REMOTE_CNT + 1];

unsigned long int remoteKey = 0;
byte relayNum = 0;
unsigned int recBtnCounter = 0, delBtnCounter = 0, delRemoteCNT = 0;
bool isDataValid = false;

const int rs = 9, en = 10, d4 = 12, d5 = 11, d6 = 8, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {

  mySwitch.enableReceive(0); //interrupt 0, PD2 
  
  pinMode(recPin, INPUT);
  pinMode(delPin, INPUT);
  
  pinMode(RELAY_R1, OUTPUT);
  pinMode(RELAY_R2, OUTPUT);
  pinMode(RELAY_R3, OUTPUT);
  pinMode(RELAY_R4, OUTPUT);
  //----------------------------------------------
  myRemotes emptyStruct = { 0, 0 };
  EEPROM.get(REMOTE_CNT * sizeof(emptyStruct), remotes[REMOTE_CNT]);
  if (remotes[REMOTE_CNT].myKey != 1234) initialEEwrite();
  //----------------------------------------------
  readEEprom();
  //----------------------------------------------
  
  lcd.begin(8, 2);
  lcd.setCursor(0, 0);
  lcd.print("  4-CH  ");
  lcd.setCursor(0, 1);
  lcd.print("Switcher");
}

void loop() {
 
  if (digitalRead(recPin) == 0) {
    delay(50);
    recBtnCounter++;
    if (recBtnCounter > 65000) recBtnCounter = 0;
  }  
  
  if (digitalRead(delPin) == 0) {
    delay(50);
    delBtnCounter++;
    if (delBtnCounter > 65000) delBtnCounter = 0;
    }
  
  //----------------------------------------------
  
  if (delBtnCounter > 25 && digitalRead(delPin) == 1) {
    delBtnCounter = 0;
    if (delRemoteCNT > 0) {
      isDataValid = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(remotes[delRemoteCNT - 1].myKey);
      lcd.setCursor(0, 1);
      lcd.print("Deleted!");
      deleteEEprom(delRemoteCNT - 1);
      delRemoteCNT = 0;
    }
  }
  
  if (delBtnCounter < 15 && delBtnCounter > 4 && digitalRead(delPin) == 1) {
    delBtnCounter = 0;
    isDataValid = false;
    lcd.clear();
    readEEprom();
    lcd.setCursor(0, 0);
    lcd.print(remotes[delRemoteCNT].myKey);
    lcd.setCursor(0, 1);
    lcd.print(delRemoteCNT + 1);
    lcd.setCursor(3, 1);
    lcd.print("Rel:");
    lcd.setCursor(7, 1);
    lcd.print(remotes[delRemoteCNT].myRelay);
    delRemoteCNT++;
    if (delRemoteCNT > REMOTE_CNT) delRemoteCNT = 0;
  }
  
  //----------------------------------------------
  
  if ((recBtnCounter > 25) && (digitalRead(recPin) == 1)) {
    recBtnCounter = 0;
    if (relayNum > 0) {
      isDataValid = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Relay");
      lcd.setCursor(7, 0);
      lcd.print(relayNum);
      lcd.setCursor(0, 1);
      lcd.print("Saved!");
      saveEEPROM(remoteKey, relayNum, findWriteIndex());
      relayNum = 0;
    }
  }
  
  if ((recBtnCounter < 15) && (recBtnCounter > 4) && (digitalRead(recPin) == 1)) {
    recBtnCounter = 0;
    if (isDataValid) {
      relayNum++;
      if (relayNum == 5) relayNum = 1;
      lcd.clear();
      switch (relayNum) {
        case 1:
          {
            lcd.setCursor(0, 0);
            lcd.print("R1");
            lcd.setCursor(0, 1);
            lcd.print(remoteKey);
            break;
          }
        case 2:
          {
            lcd.setCursor(0, 0);
            lcd.print("R2");
            lcd.setCursor(0, 1);
            lcd.print(remoteKey);
            break;
          }
        case 3:
          {
            lcd.setCursor(0, 0);
            lcd.print("R3");
            lcd.setCursor(0, 1);
            lcd.print(remoteKey);
            break;
          }
        case 4:
          {
            lcd.setCursor(0, 0);
            lcd.print("R4");
            lcd.setCursor(0, 1);
            lcd.print(remoteKey);
            break;
          }
      }
    }
  }
  
  if (mySwitch.available()) {
    delay(100);
    remoteKey = mySwitch.getReceivedValue();
    lcd.clear();
    delRemoteCNT = 0;
    switch (findRemoteKey(remoteKey)) {
      case 1:
        {
          lcd.setCursor(0, 0);
          lcd.print("Valid ");
          lcd.setCursor(0, 1);
          lcd.print("Relay 1");
          digitalWrite(RELAY_R1, !digitalRead(RELAY_R1));
          break;
        }
      case 2:
        {
          lcd.setCursor(0, 0);
          lcd.print("Valid ");
          lcd.setCursor(0, 1);
          lcd.print("Relay 2");
          digitalWrite(RELAY_R2, !digitalRead(RELAY_R2));
          break;
        }
      case 3:
        {
          lcd.setCursor(0, 0);
          lcd.print("Valid ");
          lcd.setCursor(0, 1);
          lcd.print("Relay 3");
          digitalWrite(RELAY_R3, !digitalRead(RELAY_R3));
          break;
        }
      case 4:
        {
          lcd.setCursor(0, 0);
          lcd.print("Valid ");
          lcd.setCursor(0, 1);
          lcd.print("Relay 4");
          digitalWrite(RELAY_R4, !digitalRead(RELAY_R4));
          break;
        }
      case 0:
        {
          lcd.setCursor(0, 0);
          lcd.print("Invalid");
          lcd.setCursor(0, 1);
          lcd.print(remoteKey);
          isDataValid = true;
          relayNum = 0;
          break;
        }
    }
    mySwitch.resetAvailable();
  }  
}

void saveEEPROM(unsigned long int key, byte relay, int index) {
  myRemotes emptyStruct = { 0, 0 };
  remotes[index].myKey = key;
  remotes[index].myRelay = relay;
  EEPROM.put(index * sizeof(emptyStruct), remotes[index]);
}

void initialEEwrite() {
  myRemotes emptyStruct = { 1234, 9 };
  for (int i = 0; i < (REMOTE_CNT + 1); i++) {
    EEPROM.put(i * sizeof(emptyStruct), emptyStruct);
  }
}

int findWriteIndex() {
  myRemotes emptyStruct = { 0 , 0 };
  for (int i = 0; i < REMOTE_CNT; i++) {
    EEPROM.get(i * sizeof(emptyStruct), remotes[i]);
    if (remotes[i].myKey == 1234) { return i; }
    }
}

int findRemoteKey(unsigned long int key) {
  myRemotes emptyStruct = { 0, 0 };
  for (int i = 0; i < REMOTE_CNT; i++) {
    EEPROM.get(i * sizeof(emptyStruct), remotes[i]);
    if (key == remotes[i].myKey) {
      return remotes[i].myRelay;
    }
  }
  return 0;
}

void readEEPROM()) {
  myRemotes emptyStruct = { 0, 0 };
  for (int i = 0; i < REMOTE_CNT; i++)
    EEPROM.get(i * sizeof(emptyStruct), remotes[i]);
}

void deleteEEPROM(int index) {
  myRemotes emptyStruct = { 1234, 9 };
  EEPROM.put(index * sizeof(emptyStruct), emptyStruct);
}
