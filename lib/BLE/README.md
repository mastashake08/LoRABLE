# BLE Module

## Overview
The BLE module manages Bluetooth Low Energy operations for the LoRABLE application. It provides a BLE server that allows external devices (smartphones, tablets) to connect and configure the LoRA syncWord.

## Features
- BLE server initialization with custom service UUID
- SyncWord characteristic (read/write/notify)
- Message characteristic (write) for sending LoRA messages
- Connection/disconnection event handling
- Callback mechanisms for syncWord changes and message reception
- Auto-restart advertising after disconnection

## Usage

```cpp
#include "BLEManager.h"

BLEManager bleManager;

void onSyncWordChanged(uint8_t newSyncWord) {
    Serial.print("New syncWord: 0x");
    Serial.println(newSyncWord, HEX);
}

void onMessageReceived(const String& message) {
    Serial.print("Message to send: ");
    Serial.println(message);
    // Send via LoRA
}

void setup() {
    bleManager.init();
    bleManager.setSyncWordCallback(onSyncWordChanged);
    bleManager.setMessageCallback(onMessageReceived);
}

void loop() {
    if (bleManager.isConnected()) {
        // BLE client is connected
    }
}
```

## BLE Service Details

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c33191

### Message Characteristic
- **UUID**: `a3c87500-8ed3-4bdf-8a39-a01bebede295`
- **Properties**: Write
- **Data Type**: String (up to 255 bytes)
- **Description**: Message to send via LoRA4b`
- **Device Name**: `LoRABLE`

### SyncWord Characteristic
- **UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **Properties**: Read, Write, Notify
- **Data Type**: 1 byte (0x00 - 0xFF)
- **Description**: LoRA syncWord for network separation


**To change SyncWord:**
5. Read/Write the SyncWord characteristic (beb5483e...)
6. Write a single byte value (e.g., 0x34)

**To send LoRA message:**
5. Write to the Message characteristic (a3c87500...)
6. Enter text message (e.g., "Hello LoRA!")
7. The device will transmit this message via LoRA

1. Install nRF Connect app on your smartphone
2. Scan for "LoRABLE" device
3. Connect to the device
4. Navigate to the service UUID

### `void setMessageCallback(void (*callback)(const String&))`
Set callback function to be called when message is received via BLE.

**Parameters:**
- `callback`: Function pointer that takes `const String& message` parameter
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
