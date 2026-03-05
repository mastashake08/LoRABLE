# ButtonManager Module

## Overview
Manages PRG button input on the Heltec WiFi LoRA 32 V3 board with support for long press and double press detection.

## Hardware
- **Button**: PRG button (GPIO 0)
- **Configuration**: Internal pull-up enabled (button connects to GND when pressed)

## Features

### Long Press (3 seconds)
- Toggles sleep mode on/off
- Must hold button for full 3 seconds
- Callback triggered immediately when threshold reached

### Double Press
- Toggles between BLE-only mode and BLE+Serial mode
- Must press twice within 400ms
- Allows serial commands to control the device

## Usage

```cpp
#include "ButtonManager.h"

ButtonManager buttonManager;

void setup() {
    buttonManager.init();
    buttonManager.setLongPressCallback(onLongPress);
    buttonManager.setDoublePressCallback(onDoublePress);
}

void loop() {
    buttonManager.update();  // Call frequently for accurate timing
}
```

## API

### Methods

#### `void init()`
Initialize button with pull-up resistor. Call once in setup().

#### `void update()`
Process button state and detect press patterns. Call in loop() for accurate timing.

#### `void setLongPressCallback(void (*callback)())`
Set function to call when long press (3s) is detected.

#### `void setDoublePressCallback(void (*callback)())`
Set function to call when double press is detected.

#### `bool isPressed()`
Check if button is currently pressed.

## Timing Constants

- **DEBOUNCE_DELAY**: 50ms - Filters mechanical bounce
- **LONG_PRESS_TIME**: 3000ms - Duration for long press
- **DOUBLE_PRESS_GAP**: 400ms - Maximum time between presses for double press

## Notes

- Button uses internal pull-up, so pressed state is LOW
- Debouncing prevents false triggers from mechanical bounce
- Long press cancels press count (won't trigger single/double press)
- Double press window expires after DOUBLE_PRESS_GAP with no second press
