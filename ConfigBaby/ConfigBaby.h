/*
 * @file ConfigBaby.h
 * Provide simple key-value configuration storage and methods to access it, including an interactive menu.
 * 
 * @author David Horton - https://github.com/DavesCodeMusings
 */

#ifndef __CONFIG_BABY_H
#define __CONFIG_BABY_H

#include <Arduino.h>

#define MAX_KEYS 9  // Menu selections are single digits 1-9, so think carefully before changing.
#define MAX_KEY_LEN 16  // 15 chars + 1 null terminator.
#define MAX_VALUE_LEN 16  // Just big enough for an IP address (e.g. 192.168.100.200)

/**
 * @class ConfigBaby
 */
class ConfigBaby {    
  public:
    char readlnBuffer[MAX_VALUE_LEN + 1];  // temporary storage for menu input
    char keys[MAX_KEYS][MAX_KEY_LEN];  // keys portion of the associative array
    char values[MAX_KEYS][MAX_KEY_LEN];  // values portion of the associate array
    int numPairs;  // the number of key-value pairs actually in use

    /** Constructor */
    ConfigBaby();

    /** Initialize the associative array with key names.
        @param keysCSV Pointer to a comma-separated value string of key names in preferred order. */
    int begin(char *keysCSV);

    /** Directly insert a value using its numeric index rather than its key. Used internally.
        @param index Array index for determining where value should be stored in values[][].
        @param value The value to be stored. */
    void writeValue(int index, const char *value);

    /** Search for key and return its array index. Used internally.
        @param key Pointer to search key. */
    int indexOf(char *key);

    /** Return the string value associated with key.
        @param key Pointer to key used in associative lookup. */
    char *read(char *key);

    /** Replace the current value associated with key with value.
        @param key Pointer to key used in associative indexing
        @param value The new value to insert. */
    bool write(char *key, char *value);

    /** Read a string of characters from serial input. Used internally for input() to fetch values.
        @param timeout The maximum time to wait for input before giving up and returning. */
    void readln(int timeout);

    /** Gather configuration values using an interactive menu. */
    bool input();
};

#endif
