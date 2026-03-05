# BLE Module

## Overview
The BLE module manages Bluetooth Low Energy operations for the LoRABLE application. It provides a BLE server that allows external devices (smartphones, tablets) to connect and configure the LoRA syncWord.

## Features
- BLE server initialization with custom service UUID
- SyncWord characteristic (read/write/notify)
- Connection/disconnection event handling
- Callback mechanism for syncWord changes
- Auto-restart advertising after disconnection

## Usage

```cpp
#include "BLEManager.h"

BLEManager bleManager;

void onSyncWordChanged(uint8_t newSyncWord) {
    Serial.print("New syncWord: 0x");
    Serial.println(newSyncWord, HEX);
}

void setup() {
    bleManager.init();
    bleManager.setSyncWordCallback(onSyncWordChanged);
}

void loop() {
    if (bleManager.isConnected()) {
        // BLE client is connected
    }
}
```

## BLE Service Details

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Device Name**: `LoRABLE`

### SyncWord Characteristic
- **UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **Properties**: Read, Write, Notify
- **Data Type**: 1 byte (0x00 - 0xFF)
- **Description**: LoRA syncWord for network separation

## Testing with nRF Connect

1. Install nRF Connect app on your smartphone
2. Scan for "LoRABLE" device
3. Connect to the device
4. Navigate to the service UUID
5. Read/Write the SyncWord characteristic

## API Reference

### `bool init()`
Initialize the BLE server and start advertising.

**Returns:** `true` if successful, `false` otherwise

### `void setSyncWordCallback(void (*callback)(uint8_t))`
Set callback function to be called when syncWord is changed via BLE.

**Parameters:**
- `callback`: Function pointer that takes `uint8_t syncWord` parameter

### `bool isConnected()`
Check if a BLE client is currently connected.

**Returns:** `true` if connected, `false` otherwise

### `uint8_t getSyncWord()`
Get current syncWord value.

**Returns:** Current syncWord byte value

### `void setSyncWord(uint8_t syncWord)`
Set syncWord value and update BLE characteristic.

**Parameters:**
- `syncWord`: New syncWord byte value (0x00-0xFF)
