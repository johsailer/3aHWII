#include "Settings.h"
#include <EEPROM.h>

// EEPROM layout
const int ADDR_MAGIC = 0;
const int ADDR_VERSION = 1;
const int ADDR_PATTERN = 2;
const int ADDR_LED_COUNT = 3;

const uint8_t MAGIC = 0xA5;  // marks the EEPROM as written by this sketch
const uint8_t VERSION = 1;   // increase when the layout changes

bool loadSettings(Settings &settings, uint8_t patternCount, uint8_t maxLedCount) {
  // Defaults: first pattern (red), all LEDs.
  settings.pattern = 0;
  settings.ledCount = maxLedCount;

  if (EEPROM.read(ADDR_MAGIC) != MAGIC || EEPROM.read(ADDR_VERSION) != VERSION) {
    return false;
  }

  uint8_t pattern = EEPROM.read(ADDR_PATTERN);
  uint8_t ledCount = EEPROM.read(ADDR_LED_COUNT);
  if (pattern >= patternCount || ledCount < 1 || ledCount > maxLedCount) {
    return false;
  }

  settings.pattern = pattern;
  settings.ledCount = ledCount;
  return true;
}

void saveSettings(const Settings &settings) {
  // update() instead of write(): the EEPROM cell is only written when the value differs.
  // Magic last, so a power loss in between leaves the old data marked as invalid, not half-new.
  EEPROM.update(ADDR_PATTERN, settings.pattern);
  EEPROM.update(ADDR_LED_COUNT, settings.ledCount);
  EEPROM.update(ADDR_VERSION, VERSION);
  EEPROM.update(ADDR_MAGIC, MAGIC);
}
