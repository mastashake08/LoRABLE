# Config Module

## Overview
The Config module provides persistent storage for application settings using the ESP32 Preferences library. Settings are preserved across reboots and power cycles.

## Features
- Non-volatile storage (survives reboots)
- Key-value storage for configuration
- Default value support
- Settings validation
- Debug printing of all settings

## Stored Settings

The following settings are persisted:
- **SyncWord**: LoRA sync word (1 byte)
- **Frequency**: LoRA frequency (Hz)
- **Spreading Factor**: LoRA SF (6-12)
- **TX Power**: Transmission power (dBm)

## Usage

```cpp
#include "ConfigManager.h"

ConfigManager configManager;

void setup() {
    configManager.init();
    
    // Save settings
    configManager.saveSyncWord(0xAB);
    configManager.saveFrequency(915E6);
    configManager.saveSpreadingFactor(7);
    configManager.saveTxPower(20);
    
    // Load settings
    uint8_t syncWord = configManager.loadSyncWord(0x12);  // Default: 0x12
    long frequency = configManager.loadFrequency(915E6);
    int sf = configManager.loadSpreadingFactor(7);
    int power = configManager.loadTxPower(20);
    
    // Print all settings
    configManager.printSettings();
    
    // Clear all settings
    configManager.clearAll();
}
```

## API Reference

### `bool init()`
Initialize the configuration manager.

**Returns:** `true` if successful

### `void saveSyncWord(uint8_t syncWord)`
Save sync word to persistent storage.

**Parameters:**
- `syncWord`: Sync word byte value (0x00-0xFF)

### `uint8_t loadSyncWord(uint8_t defaultValue = 0x12)`
Load sync word from persistent storage.

**Parameters:**
- `defaultValue`: Default value if not found (default: 0x12)

**Returns:** Stored sync word or default value

### `void saveFrequency(long frequency)`
Save LoRA frequency.

**Parameters:**
- `frequency`: Frequency in Hz (e.g., 915E6)

### `long loadFrequency(long defaultValue = 915E6)`
Load LoRA frequency.

**Parameters:**
- `defaultValue`: Default frequency if not found

**Returns:** Stored frequency or default value

### `void saveSpreadingFactor(int sf)`
Save spreading factor.

**Parameters:**
- `sf`: Spreading factor (6-12)

### `int loadSpreadingFactor(int defaultValue = 7)`
Load spreading factor.

**Parameters:**
- `defaultValue`: Default value if not found

**Returns:** Stored spreading factor or default value

### `void saveTxPower(int power)`
Save transmission power.

**Parameters:**
- `power`: TX power in dBm (2-20)

### `int loadTxPower(int defaultValue = 20)`
Load transmission power.

**Parameters:**
- `defaultValue`: Default value if not found

**Returns:** Stored TX power or default value

### `void clearAll()`
Clear all stored preferences. Use with caution!

### `void printSettings()`
Print all current settings to Serial output for debugging.

## Storage Details

- **Namespace**: `lorable`
- **Storage**: ESP32 NVS (Non-Volatile Storage)
- **Persistence**: Survives reboots, firmware updates, and power loss
- **Wear Leveling**: Handled automatically by ESP32 NVS

## Default Values

If no configuration is stored, the following defaults are used:
- **SyncWord**: 0x12
- **Frequency**: 915 MHz
- **Spreading Factor**: 7
- **TX Power**: 20 dBm

## Notes

- Settings are stored in flash memory with wear leveling
- Each save operation writes to flash (limit frequent saves)
- Preferences are opened/closed for each operation to ensure data integrity
- The ESP32 NVS has a finite write cycle limit (~100,000 writes per sector)
