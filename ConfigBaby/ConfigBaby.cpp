#include "ConfigBaby.h"

ConfigBaby::ConfigBaby() {}

int ConfigBaby::begin(const char *keysCSV) {
  char keysBuffer[strlen(keysCSV)];  // strtok() is destructive to the string, so a temporary buffer is used.
  strcpy(keysBuffer, keysCSV);
  char *tokenPointer;
  int kvIndex = 0;

  tokenPointer = strtok(keysBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(keys[kvIndex], tokenPointer, MAX_KEY_LEN - 1);
    values[kvIndex][0] = NULL;  // Value string is empty.
    kvIndex++;
    if (kvIndex > MAX_PAIRS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }
  totalPairs = kvIndex;

  return kvIndex;
}

int ConfigBaby::begin(const char *keysCSV, const char *valuesCSV) {
  char keysBuffer[strlen(keysCSV)];  // strtok() is destructive to the string, so a temporary buffer is used.
  strcpy(keysBuffer, keysCSV);
  char valuesBuffer[strlen(valuesCSV)];
  strcpy(valuesBuffer, valuesCSV);
  char *tokenPointer;
  int kvIndex = 0;

  // Keys.
  tokenPointer = strtok(keysBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(keys[kvIndex], tokenPointer, MAX_KEY_LEN - 1);
    values[kvIndex][0] = NULL;  // Set as empty in case not all values are filled.
    kvIndex++;
    if (kvIndex > MAX_PAIRS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }
  totalPairs = kvIndex;

  // Default values.
  kvIndex = 0;
  tokenPointer = strtok(valuesBuffer, ",");
  while (tokenPointer != NULL) {
    strncpy(values[kvIndex], tokenPointer, MAX_VALUE_LEN - 1);
    kvIndex++;
    if (kvIndex > MAX_PAIRS) {
      break;
    }
    tokenPointer = strtok(NULL, ",");
  }

  return totalPairs;  // Return the number of key-value pairs stored, regardless if the number of values matched. 
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

  Serial.setTimeout(60000); // 60 seconds expressed in milliseconds.
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

    Serial.print("Parameter? ");
    while (!Serial.available());
    int choice = Serial.read() - '0';  // Convert ASCII character to a number by subracting ASCII value of zero.
      
    if (choice == 0) {
      Serial.println(choice);
      done = true;
    }
    else {
      if (choice < 0 || choice > totalPairs) {
        Serial.println("\007");  // ASCII BEL (Should beep or flash the screen depending on terminal emulator.) 
        Serial.print("Select a number between 0 and ");
        Serial.print(totalPairs);
        Serial.println(".\n");
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
