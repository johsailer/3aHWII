#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

// Settings that survive a power cycle (stored in the EEPROM).
struct Settings {
  uint8_t pattern;
  uint8_t ledCount;
};

// Reads the settings. Returns false (and fills in the defaults) when the EEPROM
// holds nothing valid yet, e.g. on the very first start.
bool loadSettings(Settings &settings, uint8_t patternCount, uint8_t maxLedCount);

// Writes the settings. Only bytes that changed are written (protects the EEPROM).
void saveSettings(const Settings &settings);

#endif
