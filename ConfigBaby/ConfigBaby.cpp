#include "ConfigBaby.h"

ConfigBaby::ConfigBaby() {}

int ConfigBaby::begin(char *keysCSV) {
  char *tokenPointer;
  int index = 0;
  
  tokenPointer = strtok(keysCSV, ",");
  while (tokenPointer != NULL) {
    strncpy(this->keys[index], tokenPointer, MAX_KEY_LEN);
    this->values[index][0] = NULL;
    index++;
    if (index > MAX_KEYS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }
  this->numPairs = index;

  return index;
}

void ConfigBaby::writeValue(int index, const char *value) {
  strncpy(this->values[index], value, MAX_VALUE_LEN);  
}
  
int ConfigBaby::indexOf(char *key) {
  int match = -1;  // Signifies key was not found.
  
  for (int index=0; index<MAX_KEYS; index++) {
    if (strcmp(key, this->keys[index]) == 0) {
      match = index;
      break;
    }
  }
  
  return match;
}

char *ConfigBaby::read(char *key) {
  char *value = NULL;  // Signifies key was not found.

  int index = this->indexOf(key);
  if (index != -1) {
    value = this->values[index];
  }
  
  return value;
}

boolean ConfigBaby::write(char *key, char *value) {
  boolean success = false;
  int index = this->indexOf(key);
  
  if (index != -1) {
    strncpy(this->values[index], value, MAX_VALUE_LEN);
    success = true;
  }
  
  return success;
}

void ConfigBaby::readln(int timeout) {
  int index = 0;
  boolean done = false;

  while (!done) {
    if (Serial.available()) {
      char c = Serial.read();
      switch(c) {
        case 0x0d:  // Carriage Return (or Enter key)
          this->readlnBuffer[index] = c;
          done = true;
          break;
        case 0x7f:  // Backspace
          if (index > 0) {
            Serial.print(c);
            index--;
          }
          else {
            index = 0;
          }
          break;
        default:  // Any other key
          if (index < MAX_VALUE_LEN - 1) {  // Save room for null terminator.
            Serial.print(c);
            this->readlnBuffer[index] = c;
            index++;
          }
          else {
            index = MAX_VALUE_LEN;
          }
      }
    }
  }
}

boolean ConfigBaby::input() {
  boolean done = false;

  Serial.setTimeout(60000); // 60 seconds expressed in milliseconds.
  Serial.println();
  Serial.println("Device Configuration Menu");

  while (!done) {
    for(int index = 0; index < this->numPairs; index++) {
      Serial.print("(");
      Serial.print(index + 1);  // C starts at 0, humans start at 1.
      Serial.print(") ");
      Serial.print(this->keys[index]);
      for(int i = 0; i < MAX_KEY_LEN - strlen(this->keys[index]); i++) {
        Serial.print(" ");
      }
      Serial.print("  ");
      Serial.println(this->values[index]);
    }
    Serial.println("(0) Save and exit.");

    Serial.print("Parameter? ");
    while (!Serial.available());
    int choice = Serial.read() - '0';  // Convert ASCII character to a number by subracting ASCII value of zero.
    Serial.println(choice);

    if (choice == 0) {
      done = true;
    }
    else {
      if (choice > this->numPairs) {
        Serial.println("Invalid choice.\n");
      }
      else {
        Serial.print("Enter new value for ");
        Serial.print(this->keys[choice - 1]);
        Serial.print(": ");
        this->readln(30);
        Serial.println("\n");
        this->writeValue(choice - 1, this->readlnBuffer);
      }
    }
  }

  return done;
}
