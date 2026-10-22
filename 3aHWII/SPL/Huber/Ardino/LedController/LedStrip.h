#ifndef LEDSTRIP_H
#define LEDSTRIP_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

enum Pattern : uint8_t {
  PATTERN_RED,        // Task 1
  PATTERN_BLUE,       // Task 1
  PATTERN_RAINBOW,    // Task 2: more patterns
  PATTERN_CHASE,      // Task 2
  PATTERN_BREATHING,  // Task 2
  PATTERN_COUNT  // number of patterns, keep last
};

// Controls one WS2812 LED strip and its patterns. Knows nothing about buttons or Bluetooth.
// Every pattern is a state machine: update() renders at most ONE frame per call and
// returns immediately when the frame interval has not elapsed yet.
class LedStrip {
public:
  LedStrip(uint8_t pin, uint16_t ledCount);

  // Call once in setup().
  void begin();

  // Call in every loop(). Never blocks.
  void update();

  // Pattern selection
  void nextPattern();
  void setPattern(uint8_t pattern);
  uint8_t getPattern() const;

  // Parameters (also used by Phase 2 / RemoteXY)
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t brightness);
  void setSpeed(uint16_t frameIntervalMs);  // time between two animation steps

  // Number of LEDs the patterns use (1..ledCount). The rest stays off.
  // Also ends the count display.
  void setActiveCount(uint16_t count);
  uint16_t getActiveCount() const;

  // Confirmation: the first 5 LEDs flash white 5 times, then the pattern continues.
  void flashConfirm();

  // Count display while choosing the LED count: LEDs 0..count-1 white, rest off.
  // Patterns pause until setActiveCount() is called.
  void showCount(uint16_t count);

private:
  enum Overlay : uint8_t { OVERLAY_NONE, OVERLAY_FLASH, OVERLAY_COUNT };

  void updateFlash(uint32_t now);
  void drawFlash(bool on);

  void renderRed();
  void renderBlue();
  void renderChase();
  void renderRainbow();
  void renderBreathing();

  Adafruit_NeoPixel strip_;
  uint16_t ledCount_;     // physical LEDs on the strip
  uint16_t activeCount_;  // LEDs used by the patterns

  uint8_t pattern_;
  uint8_t red_;
  uint8_t green_;
  uint8_t blue_;

  uint16_t frameIntervalMs_;
  uint32_t lastUpdateMs_;
  uint16_t step_;
  bool staticDrawn_;  // true once a static pattern was shown; avoids repeated show()

  Overlay overlay_;
  uint8_t flashStep_;  // half periods done: even = on, odd = off
  uint32_t lastFlashMs_;
};

#endif
