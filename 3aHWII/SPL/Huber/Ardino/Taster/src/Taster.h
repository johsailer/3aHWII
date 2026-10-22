#ifndef TASTER_H
#define TASTER_H

#include <Arduino.h>

// Manages a digital button.
// Reads the level, detects the rising edge (newly pressed) and distinguishes
// short from long presses. Presses are debounced in update().
class Taster {
public:
  // activeLow = true: pressed = LOW (button to GND, internal pull-up)
  // activeLow = false: pressed = HIGH (button to VCC, external pull-down)
  Taster(uint8_t pin, bool activeLow = true, uint32_t longPressMs = 1000, uint32_t debounceMs = 20);

  // Call once in setup().
  void begin();

  // Call at the beginning of every loop() (at least every 10 ms).
  // The only place where the pin is read.
  void update();

  // Is the button pressed according to the last update()?
  bool isPressed() const;

  // Returns true exactly once per transition not pressed -> pressed.
  bool wasPressed();

  // Returns true exactly once when the button is released before the
  // long-press threshold was reached.
  bool wasShortPressed();

  // Returns true exactly once as soon as the button has been held for the
  // long-press threshold (while it is still held).
  bool wasLongPressed();

  // How long the button has been held so far (0 when not pressed).
  uint32_t heldMs() const;

  // Returns true exactly once per transition pressed -> not pressed.
  // durationMs receives how long the button was held.
  bool wasReleased(uint32_t &durationMs);

private:
  bool levelIsPressed(int level) const;

  uint8_t pin_;
  bool activeLow_;

  bool currentlyPressed_;
  bool previouslyPressed_;
  bool pressEvent_;
  bool shortPressEvent_;
  bool longPressEvent_;
  bool releaseEvent_;
  uint32_t releaseDurationMs_;

  // Debouncing: a level change is accepted once it stayed stable for debounceMs_
  uint32_t lastLevelChangeMs_;
  int lastRawLevel_;
  uint32_t debounceMs_;

  // Short/long press detection
  uint32_t longPressThresholdMs_;
  uint32_t pressStartMs_;
  bool longPressAlreadyReported_;
};

#endif
