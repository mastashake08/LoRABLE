# BLE Module

## Overview
The BLE module manages Bluetooth Low Energy operations for the LoRABLE application. It provides a BLE server that allows external devices (smartphones, tablets) to connect and configure the LoRA syncWord.

## Features
- BLE server initialization with custom service UUID
- SyncWord characteristic (read/write/notify)
- Message characteristic (read/write) for sending LoRA messages
- Battery Service (standard Bluetooth SIG service)
- Connection/disconnection event handling
- Callback mechanisms for syncWord changes and message reception
- Auto-restart advertising after disconnection
- Automatic battery level monitoring and reporting

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

### LoRABLE Custom Service
- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Device Name**: `LoRABLE`

### SyncWord Characteristic
- **UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **Properties**: Read, Write, Notify
- **Data Type**: 1 byte (0x00 - 0xFF)
- **Description**: LoRA syncWord for network separation
- **User Description**: "Sync Word"

### Message Characteristic
- **UUID**: `a3c87500-8ed3-4bdf-8a39-a01bebede295`
- **Properties**: Read, Write
- **Data Type**: String (up to 255 bytes)
- **Description**: Message to send via LoRA
- **User Description**: "LoRA Message"

### Battery Service (Standard Bluetooth SIG)
- **Service UUID**: `0x180F`
- **Characteristic UUID**: `0x2A19`
- **Properties**: Read, Notify
- **Data Type**: 1 byte (0-100 representing %)
- **Description**: Battery level percentage
- **User Description**: "Battery Level"
- **Update Interval**: Every 30 seconds

## Testing with nRF Connect

1. Install nRF Connect app on your smartphone
2. Scan for "LoRABLE" device
3. Connect to the device
4. You should see two services:
   - **LoRABLE Custom Service** (4fafc201...)
   - **Battery Service** (0x180F)

**To change SyncWord:**
5. Navigate to LoRABLE service
6. Find "Sync Word" characteristic
7. Write a single byte value (e.g., 0x34)

**To send LoRA message:**
5. Navigate to LoRABLE service
6. Find "LoRA Message" characteristic
7. Write text message (e.g., "Hello LoRA!")
8. The device will transmit this message via LoRA

**To monitor battery:**
5. Navigate to Battery Service (0x180F)
6. Read or subscribe to "Battery Level" characteristic
7. Value shows battery percentage (0-100)

## API Reference

### `bool init()`
Initialize the BLE server and start advertising.

**Returns:** `true` if successful, `false` otherwise

### `void setSyncWordCallback(void (*callback)(uint8_t))`
Set callback function to be called when syncWord is changed via BLE.

**Parameters:**
- `callback`: Function pointer that takes `uint8_t syncWord` parameter

### `void setMessageCallback(void (*callback)(const String&))`
Set callback function to be called when message is received via BLE.

**Parameters:**
- `callback`: Function pointer that takes `const String& message` parameter

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

### `void updateBatteryLevel(uint8_t level)`
Update battery level and notify connected clients.

**Parameters:**
- `level`: Battery level percentage (0-100)

### `uint8_t getBatteryLevel()`
Get current battery level from ADC reading.

**Returns:** Battery level percentage (0-100)

**Note:** Returns 100% if ADC_VBAT is not defined. On Heltec V3, battery voltage is read from GPIO1 through a voltage divider.
