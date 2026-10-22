#include "Taster.h"

Taster::Taster(uint8_t pin, bool activeLow, uint32_t longPressMs, uint32_t debounceMs)
  : pin_(pin),
    activeLow_(activeLow),
    currentlyPressed_(false),
    previouslyPressed_(false),
    pressEvent_(false),
    shortPressEvent_(false),
    longPressEvent_(false),
    releaseEvent_(false),
    releaseDurationMs_(0),
    lastLevelChangeMs_(0),
    lastRawLevel_(LOW),
    debounceMs_(debounceMs),
    longPressThresholdMs_(longPressMs),
    pressStartMs_(0),
    longPressAlreadyReported_(false) {
  // pinMode only in begin(): before setup() the hardware is not yet reliably initialized.
}

void Taster::begin() {
  pinMode(pin_, activeLow_ ? INPUT_PULLUP : INPUT);

  // Adopt the initial state so that a button already pressed at power-up
  // does not trigger a press event.
  lastRawLevel_ = digitalRead(pin_);
  currentlyPressed_ = levelIsPressed(lastRawLevel_);
  previouslyPressed_ = currentlyPressed_;
  lastLevelChangeMs_ = millis();
}

void Taster::update() {
  int rawLevel = digitalRead(pin_);
  uint32_t now = millis();

  if (rawLevel != lastRawLevel_) {
    lastLevelChangeMs_ = now;
    lastRawLevel_ = rawLevel;
  }

  previouslyPressed_ = currentlyPressed_;
  // Debounce: only adopt the raw level once it has been stable for debounceMs_.
  if (now - lastLevelChangeMs_ >= debounceMs_) {
    currentlyPressed_ = levelIsPressed(lastRawLevel_);
  }

  if (currentlyPressed_ && !previouslyPressed_) {
    pressEvent_ = true;
    pressStartMs_ = now;
    longPressAlreadyReported_ = false;
  }

  if (currentlyPressed_) {
    // Held: report the long press once when the threshold is reached.
    // Subtraction keeps this correct on millis() overflow.
    if (!longPressAlreadyReported_ && (now - pressStartMs_ >= longPressThresholdMs_)) {
      longPressEvent_ = true;
      longPressAlreadyReported_ = true;
    }
  } else if (previouslyPressed_) {
    releaseEvent_ = true;
    releaseDurationMs_ = now - pressStartMs_;
    if (!longPressAlreadyReported_) {
      // Released before the threshold: short press.
      shortPressEvent_ = true;
    }
  }
}

bool Taster::isPressed() const {
  return currentlyPressed_;
}

bool Taster::wasPressed() {
  // Read and clear: a held button reports only once.
  bool event = pressEvent_;
  pressEvent_ = false;
  return event;
}

bool Taster::wasShortPressed() {
  bool event = shortPressEvent_;
  shortPressEvent_ = false;
  return event;
}

bool Taster::wasLongPressed() {
  bool event = longPressEvent_;
  longPressEvent_ = false;
  return event;
}

uint32_t Taster::heldMs() const {
  return currentlyPressed_ ? millis() - pressStartMs_ : 0;
}

bool Taster::wasReleased(uint32_t &durationMs) {
  bool event = releaseEvent_;
  releaseEvent_ = false;
  if (event) {
    durationMs = releaseDurationMs_;
  }
  return event;
}

bool Taster::levelIsPressed(int level) const {
  return activeLow_ ? (level == LOW) : (level == HIGH);
}
