# LED Controller (WS2812) – Phase 3

Local control and non-blocking engine. Phase 2 (RemoteXY / HM-10 Bluetooth) is not started.

## Tasks

- **Task 1:** Switch with the button between two patterns: strip completely red and strip completely blue (first two patterns).
- **Task 2:** More patterns: rainbow, chase and breathing (chase and breathing use the color set via `setColor()`, default orange).
- **Task 3:** Long press saves the current pattern permanently (EEPROM); success = first 5 LEDs flash white 5×. Holding longer than 3 s chooses the LED count.

## Structure

| File | Hardware / responsibility |
|---|---|
| `Taster` library (`../Taster`) | Push button: debouncing, press / short press / long press, `heldMs()`, `wasReleased()` |
| `LedStrip.h/.cpp` | WS2812 strip: patterns as state machines, active LED count, confirmation flash, count display |
| `Settings.h/.cpp` | EEPROM: saved pattern and LED count (magic byte + version, `EEPROM.update()`) |
| `LedController.ino` | Only connects them; button timing and counting mode |

## Behavior

- **Short press (< 1 s):** next pattern (Red → Blue → Rainbow → Chase → Breathing → Red …)
- **Press 1–3 s, then release:** save the current pattern; first 5 LEDs flash white 5×
- **Hold > 3 s:** LEDs light up one by one (every 300 ms, stops at `LED_COUNT`). On release that number becomes the LED count, is saved and confirmed with the white flash.
- **After power-up:** the saved pattern and LED count are loaded (defaults: Red, all LEDs).
- The action is decided on **release**, so holding for the LED count never triggers "save" first.

## Non-blocking rules

- No `delay()`, no waiting `while()` loops.
- `LedStrip::update()` compares `millis()` with `lastUpdateMs_`. If the frame interval has not elapsed it returns immediately; otherwise it renders **one** frame and returns.
- `Taster::update()` is called at the start of every `loop()`; debouncing uses a `millis()` timestamp (20 ms).
- With `SHOW_LOOP_RATE 1` the sketch prints `loops/s` over serial (115200 baud). The value must be in the thousands.

## Wiring

- Button: pin 5 to GND (internal pull-up).
- WS2812 data: pin 10 (through about 330 Ω), 5 V and GND shared with the Arduino. Use an external 5 V supply for many LEDs.
- `LED_COUNT` is 30 (physical LEDs, max 255); change it in `LedController.ino`. The active count is chosen with the button.

## Setup

1. Install the `Taster` library (copy `../Taster` to `Documents/Arduino/libraries/`).
2. Install `Adafruit NeoPixel` via the Library Manager.
3. Open `LedController.ino`, select the board, upload.

## Prepared for Phase 2

`LedStrip` already has `setPattern()`, `setColor(r, g, b)`, `setBrightness()` and `setSpeed()`, which the RemoteXY variables will call.

## Test cases (on the hardware)

1. Short press: the pattern changes.
2. Press 1.5 s and release: first 5 LEDs flash white 5×, then the pattern continues. Power off and on: the same pattern starts.
3. Hold > 3 s: LEDs count up. Release at e.g. 12: white flash, then the pattern runs on 12 LEDs only. After a restart it is still 12.
4. Keep holding up to 30: it stays at 30.
5. Serial monitor: `loops/s` stays high while flashing and counting (nothing blocks).
6. Empty EEPROM (first start): defaults Red / 30 LEDs.

## Status

Compiled with avr-g++ (ATmega328P) without warnings. Not tested on hardware yet.
