# Display Module

## Overview
The Display module manages the built-in OLED display (128x64 SSD1306) on the Heltec WiFi LoRA 32 V3 board. It provides a clean interface for showing status information, received messages, and system state.

## Features
- OLED display initialization and power control
- Message display with RSSI/SNR information
- Status screen with BLE and syncWord info
- Startup/splash screen
- Text wrapping for long messages
- Configurable update intervals

## Hardware Configuration

### Pin Definitions (Heltec V3)
```cpp
OLED_SDA     17
OLED_SCL     18
OLED_RST     21
VEXT_CTRL    36  // Power control (active LOW)
```

### Display Specifications
- **Resolution**: 128x64 pixels
- **Controller**: SSD1306
- **Interface**: I2C
- **Size**: 0.96 inch

## Usage

```cpp
#include "DisplayManager.h"

DisplayManager displayManager;

void setup() {
    displayManager.init();
    displayManager.showStartupScreen();
    delay(2000);
}

void loop() {
    // Show status with BLE connection and syncWord
    displayManager.updateStatus(true, 0x12, "Last message");
    
    // Show received message
    displayManager.showMessage("Hello World!", -65, 8.5);
    
    // Show simple status
    displayManager.showStatus("Ready");
}
```

## API Reference

### `bool init()`
Initialize the OLED display and enable power.

**Returns:** `true` if successful

### `void clear()`
Clear the display.

### `void showMessage(const String& message, int rssi = 0, float snr = 0.0)`
Display a received LoRA message with optional signal quality information.

**Parameters:**
- `message`: Message text to display
- `rssi`: RSSI value in dBm (optional)
- `snr`: SNR value in dB (optional)

### `void showStatus(const String& status)`
Display a centered status message.

**Parameters:**
- `status`: Status text to display

### `void showBLEStatus(bool connected)`
Show BLE connection indicator in top-right corner.

**Parameters:**
- `connected`: `true` if BLE client is connected

### `void showSyncWord(uint8_t syncWord)`
Show current sync word in top-left corner.

**Parameters:**
- `syncWord`: Current sync word value

### `void showStartupScreen()`
Display the startup/splash screen with app name.

### `void updateStatus(bool bleConnected, uint8_t syncWord, const String& lastMessage = "")`
Update the status display with current system state.

**Parameters:**
- `bleConnected`: BLE connection status
- `syncWord`: Current sync word
- `lastMessage`: Last received message (optional)

## Display Layout

### Status Screen
```
SW:0x12                    BLE
--------------------------------
Status: Listening
BLE: Connected
Last:
Hello World!
```

### Message Screen
```
Received Message:
--------------------------------
Hello from
another device!

RSSI:-65 SNR:8.5
```

## Power Management

The display power is controlled via the VEXT_CTRL pin (active LOW):
- `LOW`: Display powered ON
- `HIGH`: Display powered OFF

The module automatically handles power control during initialization.
