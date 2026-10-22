// Phase 3: save pattern and choose the LED count with the button.
// - Short press (< 1 s):         next LED pattern
// - Press 1-3 s, then release:   save the current pattern (EEPROM), first 5 LEDs flash white 5x
// - Hold longer than 3 s:        LEDs light up one after another; on release that number
//                                becomes the LED count (also saved, also flashes white)
// After a power cycle the saved pattern and LED count are used.
// Button logic lives in the Taster library, LED logic in LedStrip, storage in Settings.
// This file only connects them. No delay(), no blocking loops.

#include "Taster.h"
#include "LedStrip.h"
#include "Settings.h"

const uint8_t BUTTON_PIN = 5;
const uint8_t STRIP_PIN = 10;
const uint16_t LED_COUNT = 30;  // physical LEDs, max 255 (stored as one byte)

const uint32_t SAVE_PRESS_MS = 1000;   // from here on a release saves the pattern
const uint32_t COUNT_PRESS_MS = 3000;  // from here on the LED count is chosen
const uint32_t COUNT_STEP_MS = 300;    // one more LED every COUNT_STEP_MS

// Set to 1 to print the loop rate once per second (verifies that loop() never blocks).
#define SHOW_LOOP_RATE 1

Taster button(BUTTON_PIN, true, SAVE_PRESS_MS, 20);  // low-active, debounce 20 ms
LedStrip strip(STRIP_PIN, LED_COUNT);

enum Mode : uint8_t {
  MODE_NORMAL,    // patterns run, button switches / saves
  MODE_COUNTING   // button held > 3 s, LED count is being chosen
};

Mode mode = MODE_NORMAL;
uint16_t countValue = 1;
uint32_t lastCountStepMs = 0;

#if SHOW_LOOP_RATE
uint32_t loopCount = 0;
uint32_t lastRateMs = 0;

void reportLoopRate() {
  loopCount++;
  uint32_t now = millis();
  if (now - lastRateMs >= 1000) {
    Serial.print("loops/s: ");
    Serial.println(loopCount);
    loopCount = 0;
    lastRateMs = now;
  }
}
#endif

void saveAndConfirm() {
  Settings settings;
  settings.pattern = strip.getPattern();
  settings.ledCount = strip.getActiveCount();
  saveSettings(settings);
  strip.flashConfirm();
}

// Counting mode: start after COUNT_PRESS_MS, then add one LED per step (stops at LED_COUNT).
void updateCounting() {
  uint32_t now = millis();

  if (mode == MODE_NORMAL && button.heldMs() >= COUNT_PRESS_MS) {
    mode = MODE_COUNTING;
    countValue = 1;
    lastCountStepMs = now;
    strip.showCount(countValue);
  }

  if (mode == MODE_COUNTING && countValue < LED_COUNT && now - lastCountStepMs >= COUNT_STEP_MS) {
    countValue++;
    lastCountStepMs = now;
    strip.showCount(countValue);
  }
}

void handleRelease() {
  uint32_t heldMs;
  if (!button.wasReleased(heldMs)) {
    return;
  }

  if (mode == MODE_COUNTING) {
    strip.setActiveCount(countValue);
    saveAndConfirm();
    mode = MODE_NORMAL;
  } else if (heldMs >= SAVE_PRESS_MS) {
    saveAndConfirm();
  } else {
    strip.nextPattern();
  }
}

void setup() {
#if SHOW_LOOP_RATE
  Serial.begin(115200);
#endif
  button.begin();
  strip.begin();

  Settings settings;
  loadSettings(settings, PATTERN_COUNT, LED_COUNT);  // defaults if nothing is saved yet
  strip.setActiveCount(settings.ledCount);
  strip.setPattern(settings.pattern);
}

void loop() {
  button.update();  // always first, at least every 10 ms

  handleRelease();
  updateCounting();

  strip.update();  // renders at most one frame

#if SHOW_LOOP_RATE
  reportLoopRate();
#endif
}
