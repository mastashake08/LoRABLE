#include "LoRAManager.h"
// Note: Do not include heltec_unofficial.h here to avoid multiple definition errors
// The radio object is declared as extern in LoRAManager.h and defined in main.cpp

LoRAManager::LoRAManager() 
    : currentSyncWord(DEFAULT_SYNC_WORD),
      lastRSSI(0),
      lastSNR(0.0),
      radioInitialized(false) {
}

bool LoRAManager::init() {
    Serial.println("Initializing LoRA...");
    
    // Note: heltec_setup() must be called in main.cpp before this
    // to initialize the SPI bus and radio hardware
    
    // Initialize radio first with simple begin()
    int state = radio.begin();
    
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("LoRA radio.begin() failed, code: ");
        Serial.println(state);
        return false;
    }
    
    // Now configure LoRA parameters
    state = radio.setFrequency(LORA_FREQUENCY);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set frequency, code: ");
        Serial.println(state);
        return false;
    }
    
    state = radio.setBandwidth(LORA_BANDWIDTH);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set bandwidth, code: ");
        Serial.println(state);
        return false;
    }
    
    state = radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set spreading factor, code: ");
        Serial.println(state);
        return false;
    }
    
    state = radio.setCodingRate(LORA_CODING_RATE);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set coding rate, code: ");
        Serial.println(state);
        return false;
    }
    
    state = radio.setOutputPower(LORA_TX_POWER);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set TX power, code: ");
        Serial.println(state);
        return false;
    }
    
    state = radio.setSyncWord(currentSyncWord);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set sync word, code: ");
        Serial.println(state);
        return false;
    }
    
    // Configure additional settings
    radio.setCRC(true);
    
    // Set preamble length
    state = radio.setPreambleLength(8);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set preamble length, code: ");
        Serial.println(state);
    }
    
    // Set current limit for PA (Power Amplifier) - important for SX1262
    state = radio.setCurrentLimit(140.0);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to set current limit, code: ");
        Serial.println(state);
    }
    
    // Start listening for packets
    state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to start receive mode, code: ");
        Serial.println(state);
    }
    
    radioInitialized = true;
    
    Serial.println("LoRA initialized successfully");
    Serial.print("Frequency: ");
    Serial.print(LORA_FREQUENCY);
    Serial.println(" MHz");
    Serial.print("Spreading Factor: ");
    Serial.println(LORA_SPREADING_FACTOR);
    Serial.print("Bandwidth: ");
    Serial.print(LORA_BANDWIDTH);
    Serial.println(" kHz");
    Serial.print("TX Power: ");
    Serial.print(LORA_TX_POWER);
    Serial.println(" dBm");
    Serial.print("Sync Word: 0x");
    Serial.println(currentSyncWord, HEX);
    
    return true;
}

void LoRAManager::setSyncWord(uint8_t syncWord) {
    currentSyncWord = syncWord;
    
    if (radioInitialized) {
        int state = radio.setSyncWord(currentSyncWord);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("Failed to set sync word, code: ");
            Serial.println(state);
        }
    }
    
    Serial.print("LoRA sync word updated to: 0x");
    Serial.println(currentSyncWord, HEX);
}

uint8_t LoRAManager::getSyncWord() {
    return currentSyncWord;
}

bool LoRAManager::sendMessage(const String& message) {
    if (message.length() == 0 || !radioInitialized) {
        Serial.println("Cannot send: LoRA not initialized or empty message");
        return false;
    }
    
    // Check message length (SX1262 max payload is 255 bytes)
    if (message.length() > 255) {
        Serial.println("Message too long! Max 255 bytes");
        return false;
    }
    
    Serial.print("Sending LoRA message (");
    Serial.print(message.length());
    Serial.print(" bytes): ");
    Serial.println(message);
    
    // Stop receiving before transmitting
    int state = radio.standby();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("Failed to enter standby mode, code: ");
        Serial.println(state);
    }
    delay(10);  // Small delay to ensure radio is ready
    
    // Transmit the message
    state = radio.transmit(message.c_str());
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("✓ Message sent successfully");
        
        // Restart listening for incoming messages
        radio.startReceive();
        
        return true;
    } else {
        Serial.print("✗ Message send failed, code: ");
        Serial.println(state);
        Serial.print("Error details: ");
        
        // Decode common error codes
        switch(state) {
            case RADIOLIB_ERR_PACKET_TOO_LONG:
                Serial.println("Packet too long");
                break;
            case RADIOLIB_ERR_TX_TIMEOUT:
                Serial.println("TX timeout - check antenna and frequency");
                break;
            case RADIOLIB_ERR_CHIP_NOT_FOUND:
                Serial.println("Chip not found - hardware issue");
                break;
            case RADIOLIB_ERR_CRC_MISMATCH:
                Serial.println("CRC mismatch");
                break;
            case RADIOLIB_ERR_INVALID_BANDWIDTH:
                Serial.println("Invalid bandwidth");
                break;
            case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
                Serial.println("Invalid spreading factor");
                break;
            case RADIOLIB_ERR_INVALID_CODING_RATE:
                Serial.println("Invalid coding rate");
                break;
            default:
                Serial.print("Unknown error (");
                Serial.print(state);
                Serial.println(")");
                break;
        }
        
        // Try to restart listening anyway
        radio.startReceive();
        
        return false;
    }
}

String LoRAManager::receiveMessage() {
    if (!radioInitialized) {
        return "";
    }
    
    String message = "";
    
    // Check if packet is available (non-blocking)
    int state = radio.readData(message);
    
    if (state == RADIOLIB_ERR_NONE) {
        // Packet received successfully
        lastRSSI = radio.getRSSI();
        lastSNR = radio.getSNR();
        
        Serial.print("Received LoRA message: ");
        Serial.println(message);
        Serial.print("RSSI: ");
        Serial.print(lastRSSI);
        Serial.print(" dBm, SNR: ");
        Serial.print(lastSNR);
        Serial.println(" dB");
        
        // Restart listening
        radio.startReceive();
        
        return message;
    } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
        // No packet received (timeout)
        return "";
    } else {
        // Some other error occurred
        return "";
    }
}

void LoRAManager::setFrequency(long frequency) {
    if (!radioInitialized) return;
    
    float freqMHz = frequency / 1E6;
    radio.setFrequency(freqMHz);
    
    Serial.print("LoRA frequency set to: ");
    Serial.print(freqMHz);
    Serial.println(" MHz");
}

void LoRAManager::setSpreadingFactor(int sf) {
    if (sf < 6 || sf > 12 || !radioInitialized) {
        Serial.println("Invalid spreading factor (must be 6-12) or radio not initialized");
        return;
    }
    
    radio.setSpreadingFactor(sf);
    
    Serial.print("LoRA spreading factor set to: ");
    Serial.println(sf);
}

void LoRAManager::setTxPower(int power) {
    if (power < 2 || power > 20 || !radioInitialized) {
        Serial.println("Invalid TX power (must be 2-20 dBm) or radio not initialized");
        return;
    }
    
    radio.setOutputPower(power);
    
    Serial.print("LoRA TX power set to: ");
    Serial.print(power);
    Serial.println(" dBm");
}

int LoRAManager::getRSSI() {
    return lastRSSI;
}

float LoRAManager::getSNR() {
    return lastSNR;
}
