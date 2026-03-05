# GitHub Copilot Instructions - LoRABLE

## Project Overview
This is a LoRA BLE application for the **Heltec WiFi LoRA 32 V3** board that:
- Receives LoRA syncWord configuration via BLE (Bluetooth Low Energy)
- Transmits/receives LoRA messages using the configured syncWord
- Displays incoming messages on the built-in OLED screen
- Based on: https://randomnerdtutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/

## Architecture Principles

### Modularity Requirements
**CRITICAL**: All code MUST follow a highly modular architecture. Each functional component should be:
- Self-contained in separate header/source files
- Loosely coupled with clear interfaces
- Independently testable
- Single responsibility per module

### Required Module Structure

#### 1. **BLE Module** (`lib/BLE/`)
- Handle BLE server initialization
- Manage BLE characteristics for syncWord configuration
- Provide callbacks for connection/disconnection events
- Interface: `BLEManager.h` / `BLEManager.cpp`
- Methods:
  - `void init()`
  - `void setSyncWordCallback(void (*callback)(uint8_t))`
  - `bool isConnected()`

#### 2. **LoRA Module** (`lib/LoRA/`)
- Initialize LoRA transceiver (SX1262 on Heltec V3)
- Configure syncWord, frequency, spreading factor, bandwidth
- Send and receive LoRA packets
- Interface: `LoRAManager.h` / `LoRAManager.cpp`
- Methods:
  - `void init()`
  - `void setSyncWord(uint8_t syncWord)`
  - `bool sendMessage(const String& message)`
  - `String receiveMessage()`
  - `void setFrequency(long frequency)`
  - `void setSpreadingFactor(int sf)`

#### 3. **Display Module** (`lib/Display/`)
- Initialize and manage the built-in OLED display (128x64 SSD1306)
- Display status messages, received LoRA data, BLE connection status
- Interface: `DisplayManager.h` / `DisplayManager.cpp`
- Methods:
  - `void init()`
  - `void showMessage(const String& message)`
  - `void showStatus(const String& status)`
  - `void clear()`
  - `void showBLEStatus(bool connected)`
  - `void showSyncWord(uint8_t syncWord)`

#### 4. **Config Module** (`lib/Config/`)
- Store and retrieve persistent configuration (syncWord, frequency, etc.)
- Use ESP32 Preferences library or EEPROM
- Interface: `ConfigManager.h` / `ConfigManager.cpp`
- Methods:
  - `void init()`
  - `void saveSyncWord(uint8_t syncWord)`
  - `uint8_t loadSyncWord()`
  - `void saveFrequency(long frequency)`
  - `long loadFrequency()`

## Hardware-Specific Guidelines

### Heltec WiFi LoRA 32 V3 Specifications
- **MCU**: ESP32-S3FN8
- **LoRA Chip**: SX1262
- **Display**: 0.96" OLED (128x64) SSD1306
- **Frequency**: 863-928 MHz (check your region)

### Pin Definitions (Heltec V3)
```cpp
// LoRA SX1262 Pins
#define LORA_SCK     9
#define LORA_MISO    11
#define LORA_MOSI    10
#define LORA_NSS     8
#define LORA_RST     12
#define LORA_DIO1    14
#define LORA_BUSY    13

// OLED Pins
#define OLED_SDA     17
#define OLED_SCL     18
#define OLED_RST     21

// Power Control
#define VEXT_CTRL    36  // OLED power control
```

### Recommended Libraries
```ini
; In platformio.ini
lib_deps = 
    sandeepmistry/LoRa@^0.8.0
    thingpulse/ESP8266 and ESP32 OLED driver for SSD1306 displays@^4.4.0
    bblanchon/ArduinoJson@^6.21.0
```

## Coding Standards

### File Organization
```
src/
  main.cpp              # Main application loop, module orchestration only
lib/
  BLE/
    BLEManager.h
    BLEManager.cpp
  LoRA/
    LoRAManager.h
    LoRAManager.cpp
  Display/
    DisplayManager.h
    DisplayManager.cpp
  Config/
    ConfigManager.h
    ConfigManager.cpp
```

### Code Style
- Use **PascalCase** for class names: `BLEManager`, `LoRAManager`
- Use **camelCase** for methods and variables: `setSyncWord()`, `isConnected`
- Use **UPPER_SNAKE_CASE** for constants: `LORA_FREQUENCY`, `DEFAULT_SYNC_WORD`
- Include header guards in all `.h` files
- Add comprehensive comments for public interfaces
- Keep `main.cpp` minimal - only initialize modules and coordinate between them

### Main Application Pattern
```cpp
// main.cpp should follow this pattern:
void setup() {
  Serial.begin(115200);
  
  // Initialize all modules
  displayManager.init();
  configManager.init();
  loraManager.init();
  bleManager.init();
  
  // Setup callbacks
  bleManager.setSyncWordCallback(onSyncWordChanged);
  
  // Display startup screen
  displayManager.showStatus("LoRABLE Ready");
}

void loop() {
  // Check for LoRA messages
  String message = loraManager.receiveMessage();
  if (message.length() > 0) {
    displayManager.showMessage(message);
  }
}
```

### Error Handling
- Always check initialization return values
- Log errors to Serial for debugging
- Display critical errors on OLED
- Implement graceful fallbacks

### BLE Characteristics
- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **SyncWord Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
  - Type: Read/Write
  - Size: 1 byte (0x00 - 0xFF)

## LoRA Configuration Defaults
```cpp
#define LORA_FREQUENCY      915E6   // 915 MHz (adjust for your region)
#define LORA_BANDWIDTH      125E3   // 125 kHz
#define LORA_SPREADING_FACTOR  7    // SF7
#define LORA_CODING_RATE    5       // 4/5
#define LORA_TX_POWER       20      // 20 dBm
#define DEFAULT_SYNC_WORD   0x12    // Default sync word
```

## Testing Approach
- Each module should be testable in isolation
- Create example sketches in `test/` for individual module validation
- Test BLE connection with nRF Connect app or similar
- Test LoRA with another Heltec device if available

## Documentation Requirements
- Add README.md in each `lib/` subdirectory explaining the module
- Include usage examples in code comments
- Document all public methods with purpose, parameters, and return values

## Additional Notes
- Keep blocking operations to a minimum in `loop()`
- Use non-blocking patterns where possible
- Implement proper power management for battery operation
- Consider adding a timeout for BLE connections
- Add LED indicators for status feedback if using onboard LED

## Reference Tutorial
Base implementation on: https://randomnerdtutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
Adapt pin definitions and initialization for Heltec V3 hardware.
