#include "ConfigBaby.h"

ConfigBaby::ConfigBaby() {}

int ConfigBaby::begin(char *keysCSV) {
  char keysBuffer[strlen(keysCSV)];  // strtok() is destructive to the string, so a temporary buffer is used.
  strcpy(keysBuffer, keysCSV);
  char *tokenPointer;
  int index = 0;

  tokenPointer = strtok(keysBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(this->keys[index], tokenPointer, MAX_KEY_LEN - 1);
    this->values[index][0] = NULL;  // Value string is empty.
    index++;
    if (index > MAX_KEYS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }
  this->numPairs = index;

  return this->numPairs;
}

int ConfigBaby::begin(char *keysCSV, char *valuesCSV) {
  char keysBuffer[strlen(keysCSV)];  // strtok() is destructive to the string, so a temporary buffer is used.
  strcpy(keysBuffer, keysCSV);
  char valuesBuffer[strlen(valuesCSV)];
  strcpy(valuesBuffer, valuesCSV);
  char *tokenPointer;
  int index = 0;

  // Keys.
  tokenPointer = strtok(keysBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(this->keys[index], tokenPointer, MAX_KEY_LEN - 1);
    this->values[index][0] = NULL;  // Set as empty in case not all values are filled.
    index++;
    if (index > MAX_KEYS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }
  this->numPairs = index;

  // Default values.
  index = 0;
  tokenPointer = strtok(valuesBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(this->values[index], tokenPointer, MAX_VALUE_LEN - 1);
    index++;
    if (index > MAX_KEYS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }

  return this->numPairs;  // Return the number of indexes stored, regardless if the number of values matched. 
}

void ConfigBaby::writeValue(int index, const char *value) {
  strncpy(this->values[index], value, MAX_VALUE_LEN - 1);  
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

bool ConfigBaby::write(char *key, char *value) {
  bool success = false;
  int index = this->indexOf(key);
  
  if (index != -1) {
    strncpy(this->values[index], value, MAX_VALUE_LEN - 1);
    success = true;
  }
  
  return success;
}

void ConfigBaby::readln(int timeout) {
  int index = 0;
  bool done = false;

  while (!done) {
    if (Serial.available()) {
      char c = Serial.read();
      switch(c) {
        case 0x0d:  // Carriage Return (Enter key)
          this->readlnBuffer[index] = NULL;  // Terminate string.
          done = true;
          break;
        case 0x7f:  // Backspace on a PC
        case 0x08:  // ^H (Backspace on a terminal???)
          if (index > 0) {
            Serial.print(c);
            index--;
          }
          else {
            index = 0;
          }
          break;
        default:  // Any other key
          if (index < MAX_VALUE_LEN - 1) {  // Save one character for null terminator.
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

bool ConfigBaby::input() {
  bool done = false;

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
      
    if (choice == 0) {
      Serial.println(choice);
      done = true;
    }
    else {
      if (choice < 0 || choice > this->numPairs) {
        Serial.println("\007");  // ASCII BEL (Should beep or flash the screen depending on terminal emulator.) 
        Serial.print("Select a number between 0 and ");
        Serial.print(this->numPairs);
        Serial.println(".\n");
      }
      else {
        Serial.println(choice);
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

int ConfigBaby::serialize(char *buffer) {
  int offset = 0;  // Tracks current position from start of buffer.

  // Why does snprintf use a maximum size specifier of MAX_KEY_LEN + MAX_VALUE_LEN + 4 ?
  // The maximum characters in each key will be MAX_KEY_LEN - 1. -1, because of the null terminator which is not saved to the file.
  // The same calculation applies to the maximum characters in each value. The 'key = value' format adds 1 character for the equal
  // sign and 2 for each space. The carriage return and newline add 2 more. There also needs to be room for 1 null terminator. 
  for(int index = 0; index < numPairs; index++) {
     offset += snprintf(buffer + offset, MAX_KEY_LEN + MAX_VALUE_LEN + 4, "%s = %s\r\n", keys[index], values[index]);
  }

  return offset;  // Equates to the number of characters in the buffer (not including null terminator.)
}
