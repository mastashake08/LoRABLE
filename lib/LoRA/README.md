# LoRA Module

## Overview
The LoRA module manages all LoRA transceiver operations using the SX1262 chip on the Heltec WiFi LoRA 32 V3 board. It handles initialization, configuration, message transmission, and reception.

## Features
- SX1262 LoRA chip support (Heltec V3)
- Configurable syncWord for network separation
- Message transmission and reception
- RSSI and SNR measurement
- Configurable frequency, spreading factor, and power
- CRC error checking enabled

## Hardware Configuration

### Pin Definitions (Heltec V3)
```cpp
LORA_SCK     9
LORA_MISO    11
LORA_MOSI    10
LORA_NSS     8
LORA_RST     12
LORA_DIO1    14
LORA_BUSY    13
```

### Default LoRA Settings
- **Frequency**: 915 MHz (configurable for 433/868/915 MHz)
- **Bandwidth**: 125 kHz
- **Spreading Factor**: 7
- **Coding Rate**: 4/5
- **TX Power**: 20 dBm
- **Default SyncWord**: 0x12

## Usage

```cpp
#include "LoRAManager.h"

LoRAManager loraManager;

void setup() {
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

### `String receiveMessage()`
Check for and receive incoming LoRA messages (non-blocking).

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

**Returns:** SNR value in dB

## Regional Frequency Bands

Choose the appropriate frequency for your region:
- **433 MHz**: Europe, Asia (ISM band)
- **868 MHz**: Europe
- **915 MHz**: North America, Australia, South America

**Important**: Check local regulations before transmitting!
