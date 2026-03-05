# LoRA Module

## Overview
The LoRA module manages all LoRA transceiver operations using the **SX1262 chip** on the Heltec WiFi LoRA 32 V3 board via the RadioLib library. It handles initialization, configuration, message transmission, and reception.

## Features
- SX1262 LoRA chip support (Heltec V3) via RadioLib
- Configurable syncWord for network separation
- Message transmission and reception
- RSSI and SNR measurement
- Configurable frequency, spreading factor, and power
- CRC error checking enabled
- Non-blocking receive mode

## Hardware Configuration

### Pin Definitions (Heltec V3)
```cpp
RADIO_SCLK_PIN    9
RADIO_MISO_PIN    11
RADIO_MOSI_PIN    10
RADIO_CS_PIN      8
RADIO_RST_PIN     12
RADIO_DIO1_PIN    14
RADIO_BUSY_PIN    13
```

### Default LoRA Settings
- **Frequency**: 915.0 MHz (configurable for 433.0/868.0/915.0 MHz)
- **Bandwidth**: 125.0 kHz
- **Spreading Factor**: 7
- **Coding Rate**: 4/5
- **TX Power**: 20 dBm
- **Default SyncWord**: 0x12

### Important Note
The Heltec V3 board uses the **SX1262** LoRA chip (not SX127x). This requires the **RadioLib** library instead of the older sandeepmistry/LoRa library. The API is similar but not identical.

**CRITICAL**: You must call `heltec_setup()` in your `main.cpp` setup() function **before** calling `loraManager.init()`. The `heltec_setup()` function initializes the SPI bus and radio hardware that the LoRAManager depends on.

## Usage

```cpp
#include "LoRAManager.h"
#include <heltec_unofficial.h>

LoRAManager loraManager;

void setup() {
    // Initialize Heltec hardware first (includes radio SPI setup)
    heltec_setup();
    
    // Now initialize LoRA
    if (!loraManager.init()) {
        Serial.println("LoRA init failed!");
    }
    
    // Set custom syncWord
    loraManager.setSyncWord(0xAB);
}

void loop() {
    // Send message
    loraManager.sendMessage("Hello LoRA!");
    
    // Receive message
    String msg = loraManager.receiveMessage();
    if (msg.length() > 0) {
        Serial.println(msg);
        int rssi = loraManager.getRSSI();
        float snr = loraManager.getSNR();
    }
}
```

## API Reference

### `bool init()`
Initialize the LoRA transceiver with default settings.

**Returns:** `true` if successful, `false` if initialization failed

### `void setSyncWord(uint8_t syncWord)`
Set the LoRA sync word for network separation. Only devices with the same syncWord can communicate.

**Parameters:**
- `syncWord`: Byte value (0x00-0xFF)

### `uint8_t getSyncWord()`
Get current sync word value.

**Returns:** Current sync word

### `bool sendMessage(const String& message)`
Send a message via LoRA.

**Parameters:**
- `message`: String message to transmit

**Returns:** `true` if sent successfully, `false` otherwise

The radio is put in continuous receive mode at initialization. This method checks if a packet has been received and returns it immediately without blocking.

**Returns:** Received message string, empty if no message available

**Note:** After receiving a message, the radio automatically returns to receive mode.
Check for and receive incoming LoRA messages (non-blocking).
000000 for 915 MHz)

**Note:** Frequency is converted to MHz internally (e.g., 915E6 → 915.0
**Returns:** Received message string, empty if no message available

### `void setFrequency(long frequency)`
Set LoRA frequency.

**Parameters:**
- `frequency`: Frequency in Hz (e.g., 915E6 for 915 MHz)

### `void setSpreadingFactor(int sf)`
Set spreading factor (affects range vs data rate).

**Parameters:**
- `sf`: Spreading factor (6-12). Higher = longer range, lower data rate

### `void setTxPower(int power)`
Set transmission power.

**Parameters:**
- `power`: Power in dBm (2-20)

### `int getRSSI()`
Get RSSI (Received Signal Strength Indicator) of last received packet.

**Returns:** RSSI value in dBm

### `float getSNR()`
Get SNR (Signal-to-Noise Ratio) of last received packet.
.0 MHz**: Europe, Asia (ISM band)
- **868.0 MHz**: Europe
- **915.0 MHz**: North America, Australia, South America

**Important**: Check local regulations before transmitting!

## RadioLib vs LoRa Library

This module uses **RadioLib** which is required for the SX1262 chip. Key differences from the old sandeepmistry/LoRa library:

- **Initialization**: `SX1262.begin()` instead of `LoRa.begin()`
- **Frequency**: In MHz (float) instead of Hz (long)
- **Receive**: Continuous receive mode with `start Receive()` and `readData()`
- **Error codes**: Returns specific error codes instead of boolean
- **More features**: Better support for advanced LoRa features
Choose the appropriate frequency for your region:
- **433 MHz**: Europe, Asia (ISM band)
- **868 MHz**: Europe
- **915 MHz**: North America, Australia, South America

**Important**: Check local regulations before transmitting!
