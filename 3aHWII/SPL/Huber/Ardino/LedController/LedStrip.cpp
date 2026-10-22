#include "LedStrip.h"

// Confirmation flash: first FLASH_LED_COUNT LEDs, FLASH_TIMES times on/off.
const uint8_t FLASH_LED_COUNT = 5;
const uint8_t FLASH_TIMES = 5;
const uint16_t FLASH_HALF_PERIOD_MS = 150;

LedStrip::LedStrip(uint8_t pin, uint16_t ledCount)
  : strip_(ledCount, pin, NEO_GRB + NEO_KHZ800),
    ledCount_(ledCount),
    activeCount_(ledCount),
    pattern_(PATTERN_RED),
    red_(255),
    green_(80),
    blue_(0),
    frameIntervalMs_(40),
    lastUpdateMs_(0),
    step_(0),
    staticDrawn_(false),
    overlay_(OVERLAY_NONE),
    flashStep_(0),
    lastFlashMs_(0) {
}

void LedStrip::begin() {
  strip_.begin();
  strip_.setBrightness(64);
  strip_.clear();
  strip_.show();
  lastUpdateMs_ = millis();
}

void LedStrip::update() {
  uint32_t now = millis();

  // Overlays have priority over the pattern.
  if (overlay_ == OVERLAY_FLASH) {
    updateFlash(now);
    return;
  }
  if (overlay_ == OVERLAY_COUNT) {
    return;  // drawn by showCount(), nothing to animate
  }

  // Frame interval not elapsed: leave immediately.
  // Subtraction keeps this correct on millis() overflow.
  if (now - lastUpdateMs_ < frameIntervalMs_) {
    return;
  }
  lastUpdateMs_ = now;

  // Static patterns (red, blue) are drawn once; show() is not repeated
  // so the HM-10 timer is not disturbed by pointless refreshes.
  bool isStatic = (pattern_ == PATTERN_RED || pattern_ == PATTERN_BLUE);
  if (isStatic && staticDrawn_) {
    return;
  }

  // Exactly one frame / step of the active pattern.
  switch (pattern_) {
    case PATTERN_RED:       renderRed();       break;
    case PATTERN_BLUE:      renderBlue();      break;
    case PATTERN_CHASE:     renderChase();     break;
    case PATTERN_RAINBOW:   renderRainbow();   break;
    case PATTERN_BREATHING: renderBreathing(); break;
    default:                renderRed();       break;
  }
  strip_.show();
  staticDrawn_ = isStatic;
  step_++;
}

void LedStrip::nextPattern() {
  setPattern((pattern_ + 1) % PATTERN_COUNT);
}

void LedStrip::setPattern(uint8_t pattern) {
  pattern_ = pattern % PATTERN_COUNT;
  step_ = 0;
  staticDrawn_ = false;
  // Make the next update() render immediately instead of waiting for the interval.
  lastUpdateMs_ = millis() - frameIntervalMs_;
}

uint8_t LedStrip::getPattern() const {
  return pattern_;
}

void LedStrip::setColor(uint8_t r, uint8_t g, uint8_t b) {
  red_ = r;
  green_ = g;
  blue_ = b;
}

void LedStrip::setBrightness(uint8_t brightness) {
  strip_.setBrightness(brightness);
  staticDrawn_ = false;  // redraw static patterns with the new brightness
}

void LedStrip::setSpeed(uint16_t frameIntervalMs) {
  frameIntervalMs_ = frameIntervalMs;
}

void LedStrip::setActiveCount(uint16_t count) {
  activeCount_ = constrain(count, 1, ledCount_);  // never 0 LEDs
  if (overlay_ == OVERLAY_COUNT) {
    overlay_ = OVERLAY_NONE;
  }
  // LEDs behind activeCount_ are never touched by the patterns, so switch them off once.
  strip_.clear();
  staticDrawn_ = false;
  lastUpdateMs_ = millis() - frameIntervalMs_;
}

uint16_t LedStrip::getActiveCount() const {
  return activeCount_;
}

void LedStrip::flashConfirm() {
  overlay_ = OVERLAY_FLASH;
  flashStep_ = 0;
  lastFlashMs_ = millis();
  strip_.clear();
  drawFlash(true);
}

void LedStrip::showCount(uint16_t count) {
  overlay_ = OVERLAY_COUNT;
  strip_.clear();
  strip_.fill(strip_.Color(255, 255, 255), 0, constrain(count, 1, ledCount_));
  strip_.show();
}

void LedStrip::updateFlash(uint32_t now) {
  if (now - lastFlashMs_ < FLASH_HALF_PERIOD_MS) {
    return;
  }
  lastFlashMs_ = now;
  flashStep_++;

  if (flashStep_ >= FLASH_TIMES * 2) {
    // Done: back to the pattern, rendered on the next update().
    overlay_ = OVERLAY_NONE;
    strip_.clear();
    staticDrawn_ = false;
    lastUpdateMs_ = now - frameIntervalMs_;
    return;
  }
  drawFlash(flashStep_ % 2 == 0);
}

void LedStrip::drawFlash(bool on) {
  uint16_t count = min((uint16_t)FLASH_LED_COUNT, ledCount_);
  strip_.fill(on ? strip_.Color(255, 255, 255) : 0, 0, count);
  strip_.show();
}

void LedStrip::renderRed() {
  strip_.fill(strip_.Color(255, 0, 0), 0, activeCount_);
}

void LedStrip::renderBlue() {
  strip_.fill(strip_.Color(0, 0, 255), 0, activeCount_);
}

void LedStrip::renderChase() {
  strip_.clear();
  strip_.setPixelColor(step_ % activeCount_, strip_.Color(red_, green_, blue_));
}

void LedStrip::renderRainbow() {
  // step_ shifts the rainbow; uint16_t arithmetic wraps around on purpose.
  uint16_t offset = (uint16_t)(step_ * 256u);
  for (uint16_t i = 0; i < activeCount_; i++) {
    uint16_t hue = offset + (uint16_t)((uint32_t)i * 65536UL / activeCount_);
    strip_.setPixelColor(i, strip_.ColorHSV(hue));
  }
}

void LedStrip::renderBreathing() {
  // Triangle wave 0..252..0 over 128 steps.
  uint8_t phase = step_ % 128;
  uint8_t level = (phase < 64 ? phase : 127 - phase) * 4;

  uint8_t r = (uint16_t)red_ * level / 255;
  uint8_t g = (uint16_t)green_ * level / 255;
  uint8_t b = (uint16_t)blue_ * level / 255;
  strip_.fill(strip_.Color(r, g, b), 0, activeCount_);
}
