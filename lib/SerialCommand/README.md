# SerialCommand Module

## Overview
Provides serial command interface that mimics BLE characteristic functionality, allowing control via USB serial connection.

## Features

### Serial Commands
All commands are case-insensitive and end with newline/carriage return.

#### `HELP` or `?`
Show list of available commands with examples.

#### `STATUS`
Display current device status (syncword, BLE connection, etc.).

#### `SYNCWORD:0xXX`
Set LoRA syncword. Replace XX with hex value (00-FF).
```
Example: SYNCWORD:0x12
```

#### `MESSAGE:text`
Send a message via LoRA.
```
Example: MESSAGE:Hello from serial!
```

#### `WIFI_SSID:ssid`
Set WiFi SSID for OTA updates.
```
Example: WIFI_SSID:MyNetwork
```

#### `WIFI_PASSWORD:password`
Set WiFi password for OTA updates.
```
Example: WIFI_PASSWORD:MyPassword123
```

## Usage

```cpp
#include "SerialCommand.h"

SerialCommand serialCommand;

void setup() {
    Serial.begin(115200);
    serialCommand.init();
    
    // Set callbacks
    serialCommand.setSyncWordCallback(onSyncWordChanged);
    serialCommand.setMessageCallback(onMessageReceived);
    serialCommand.setWiFiCallback(onWiFiCredentialsChanged);
    serialCommand.setStatusCallback(onStatusRequest);
    
    // Enable/disable as needed
    serialCommand.setEnabled(false);  // Disabled by default
}

void loop() {
    serialCommand.update();  // Process incoming commands
}
```

## API

### Methods

#### `void init()`
Initialize serial command handler. Call once in setup().

#### `void update()`
Process incoming serial data. Call in loop() when enabled.

#### `void setEnabled(bool enabled)`
Enable or disable serial command processing.

#### `bool isEnabled()`
Check if serial commands are currently enabled.

#### `void setSyncWordCallback(void (*callback)(uint8_t))`
Set callback for SYNCWORD command.

#### `void setMessageCallback(void (*callback)(const String&))`
Set callback for MESSAGE command.

#### `void setWiFiCallback(void (*callback)(const String&, const String&))`
Set callback for WiFi credentials commands.

#### `void setStatusCallback(void (*callback)())`
Set callback for STATUS command.

## Notes

- Serial mode is disabled by default
- Double-press PRG button to toggle serial mode on/off
- Commands are buffered until newline is received
- Maximum command length is 256 characters
- Hex values can be specified with or without '0x' prefix
- WiFi credentials are only used for OTA updates on boot
