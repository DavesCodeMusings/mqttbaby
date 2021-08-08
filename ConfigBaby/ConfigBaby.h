/*
 * @file ConfigBaby.h
 * Provide simple key-value configuration storage and methods to access it, including an interactive menu.
 * 
 * @author David Horton - https://github.com/DavesCodeMusings
 */

#ifndef __CONFIG_BABY_H
#define __CONFIG_BABY_H

#include <Arduino.h>

#define MAX_PAIRS 9  // Menu selections are single digits 1-9, so think carefully before changing.
#define MAX_KEY_LEN 16  // 15 chars + 1 null terminator.
#define MAX_VALUE_LEN 16  // Just big enough for an IP address (e.g. 192.168.100.200) and a null terminator.

/**
 * @class ConfigBaby
 */
class ConfigBaby {    
  private:
    int totalPairs;  // the number of key-value pairs actually in use
    char keys[MAX_PAIRS][MAX_KEY_LEN];  // keys portion of the associative array
    char values[MAX_PAIRS][MAX_KEY_LEN];  // values portion of the associate array
    char readlnBuffer[MAX_VALUE_LEN + 1];  // temporary storage for menu input

    /** Search for key and return its array index.
        @param key Pointer to search key. */
    int indexOf(const char *key);        

    /** Directly insert a value using its numeric index rather than its key. Used for input().
        @param index Array index for determining where value should be stored in values[][].
        @param value The value to be stored. */
    void writeValue(const int index, const char *value);

    /** Read a string of characters from serial input. Used internally for input() to fetch values. */
    void readln();

  public:
    ConfigBaby();

    /** Initialize the associative array with key names and optional default values.
        @param keysCSV Pointer to a comma-separated value string of key names in preferred order. */
    int begin(const char *keysCSV);
    int begin(const char *keysCSV, const char *valuesCSV);
    
    /** Return the string value associated with key.
        @param key Pointer to key used in associative lookup. */
    char *read(const char *key);

    /** Replace the current value associated with key with value.
        @param key Pointer to key used in associative indexing
        @param value The new value to insert. */
    bool write(const char *key, const char *value);

    /** Gather configuration values using an interactive menu. */
    bool input();

    int serialize(char *buffer);

    int deserialize(const char *buffer);
};

#endif
