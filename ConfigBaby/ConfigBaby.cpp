#include "ConfigBaby.h"

ConfigBaby::ConfigBaby() {}

int ConfigBaby::begin(const char *keysCSV) {
  return setKeys(keysCSV);
}

int ConfigBaby::begin(const char *keysCSV, const char *valuesCSV) {
  int totalKeys = setKeys(keysCSV);
  setValues(valuesCSV);
  return totalKeys;
}
    
int ConfigBaby::setKeys(const char *keysCSV) {
  char keysBuffer[strlen(keysCSV) + 1];  // strtok() is destructive to the string, so a temporary buffer is used.
  strcpy(keysBuffer, keysCSV);
  char *tokenPointer;
  int keyIndex = 0;

  tokenPointer = strtok(keysBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(keys[keyIndex], tokenPointer, MAX_KEY_LEN - 1);
    values[keyIndex][0] = NULL;  // Value string is intialized as empty.
    keyIndex++;
    if (keyIndex > MAX_PAIRS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }
  totalPairs = keyIndex;

  return keyIndex;
}

int ConfigBaby::setValues(const char *valuesCSV) {
  char valuesBuffer[strlen(valuesCSV) + 1]; // strtok() is destructive to the string, so a temporary buffer is used.
  strcpy(valuesBuffer, valuesCSV);
  char *tokenPointer;
  int valueIndex = 0;

  tokenPointer = strtok(valuesBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(values[valueIndex], tokenPointer, MAX_VALUE_LEN - 1);
    valueIndex++;
    if (valueIndex > MAX_PAIRS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }

  return valueIndex;
}

void ConfigBaby::writeValue(const int kvIndex, const char *value) {
  strncpy(values[kvIndex], value, MAX_VALUE_LEN - 1);  
}
  
int ConfigBaby::indexOf(const char *key) {
  int match = -1;  // Signifies key was not found.
  
  for (int kvIndex=0; kvIndex<MAX_PAIRS; kvIndex++) {
    if (strcmp(key, keys[kvIndex]) == 0) {
      match = kvIndex;
      break;
    }
  }
  
  return match;
}

char *ConfigBaby::read(const char *key) {
  char *value = NULL;  // Signifies key was not found.

  int kvIndex = indexOf(key);
  if (kvIndex != -1) {
    value = values[kvIndex];
  }
  
  return value;
}

bool ConfigBaby::write(const char *key, const char *value) {
  bool success = false;
  int kvIndex = indexOf(key);
  
  if (kvIndex != -1) {
    strncpy(values[kvIndex], value, MAX_VALUE_LEN - 1);
    success = true;
  }
  
  return success;
}

void ConfigBaby::readln() {
  int index = 0;
  bool done = false;

  while (!done) {
    if (Serial.available()) {
      char c = Serial.read();
      switch(c) {
        case 0x0d:  // Carriage Return (Enter key)
          readlnBuffer[index] = NULL;  // Terminate string.
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
            readlnBuffer[index] = c;
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

  Serial.println();
  Serial.println("Device Configuration Menu");

  while (!done) {
    for(int kvIndex = 0; kvIndex < totalPairs; kvIndex++) {
      Serial.print("(");
      Serial.print(kvIndex + 1);  // C starts at 0, humans start at 1.
      Serial.print(") ");
      Serial.print(keys[kvIndex]);
      for(int i = 0; i < MAX_KEY_LEN - strlen(keys[kvIndex]); i++) {
        Serial.print(" ");
      }
      Serial.print("  ");
      Serial.println(values[kvIndex]);
    }
    Serial.println("(0) Save and exit.");

    Serial.print("?");
    while (!Serial.available());
    int choice = Serial.read() - '0';  // Convert ASCII character to a number by subracting ASCII value of zero.

    if (choice == 0) {
      Serial.println(choice);
      done = true;
    }
    else {
      if (choice < 0 || choice > totalPairs) {
        Serial.println("\007");  // ASCII BEL (Should beep or flash the screen depending on terminal emulator.) 
      }
      else {
        Serial.println(choice);
        Serial.print("Enter new value for ");
        Serial.print(keys[choice - 1]);
        Serial.print(": ");
        readln();
        Serial.println("\n");
        writeValue(choice - 1, readlnBuffer);
      }
    }
  }

  return done;
}

int ConfigBaby::select(char *value) {
  for(int kvIndex = 0; kvIndex < totalPairs; kvIndex++) {
    Serial.print("(");
    Serial.print(kvIndex + 1);  // C starts at 0, humans start at 1.
    Serial.print(") ");
    Serial.println(keys[kvIndex]);
  }
  Serial.print("?");

  int choice = 0;
  while (choice < 1 || choice > totalPairs) {
    while (!Serial.available());
    choice = Serial.read() - '0';  // Convert ASCII character to a number by subracting ASCII value of zero.
  }
  strncpy(value, values[choice - 1], MAX_VALUE_LEN);  // values[] is zero indexed, choice starts with 1.
  
  return choice;
}

int ConfigBaby::serialize(char *buffer) {
  int offset = 0;  // Tracks current position from start of buffer.

  // Why does snprintf use a maximum size specifier of MAX_KEY_LEN + MAX_VALUE_LEN + 4 ?
  // The maximum characters in each key will be MAX_KEY_LEN - 1. -1, because of the null terminator which is not saved to the file.
  // The same calculation applies to the maximum characters in each value. The 'key = value' format adds 1 character for the equal
  // sign and 2 for each space. The carriage return and newline add 2 more. There also needs to be room for 1 null terminator. 
  for(int kvIndex = 0; kvIndex < totalPairs; kvIndex++) {
     offset += snprintf(buffer + offset, MAX_KEY_LEN + MAX_VALUE_LEN + 4, "%s=%s\r\n", keys[kvIndex], values[kvIndex]);
  }

  return offset;  // Equates to the number of characters in the buffer (not including null terminator.)
}

int ConfigBaby::deserialize(const char *input) {
  char key[MAX_KEY_LEN], value[MAX_KEY_LEN];
  int kvIndex = 0;

  char buffer[strlen(input) + 1];
  strcpy(buffer, input);
  
  char *linePointer = strtok(buffer, "\n");
  while (linePointer != NULL) {
    sscanf(linePointer, "%[^=]=%[^\r]", key, value);  // %[^=] means everything up, but not including the equal sign. %[^\r] is the same for carriage returns.
    strncpy(keys[kvIndex], key, MAX_KEY_LEN - 1);
    strncpy(values[kvIndex], value, MAX_VALUE_LEN - 1);
    linePointer = strtok(NULL, "\n");
    kvIndex++;
    if (kvIndex > MAX_PAIRS) {
      break;
    }

  }
  totalPairs = kvIndex;  // This is how many key-value pairs were read.

  return kvIndex;
}
