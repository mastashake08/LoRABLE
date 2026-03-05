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
    
    // Configure LoRA parameters using the global 'radio' object from heltec_unofficial.h
    int state = radio.begin(LORA_FREQUENCY, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, 
                            LORA_CODING_RATE, currentSyncWord, LORA_TX_POWER);
    
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("LoRA initialization failed, code: ");
        Serial.println(state);
        return false;
    }
    
    // Configure additional settings
    radio.setCRC(true);
    
    // Start listening for packets
    radio.startReceive();
    
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
    Serial.print("Sync Word: 0x");
    Serial.println(currentSyncWord, HEX);
    
    return true;
}

void LoRAManager::setSyncWord(uint8_t syncWord) {
    currentSyncWord = syncWord;
    
    if (radioInitialized) {
        radio.setSyncWord(currentSyncWord, 0x44);  // Private network, standard preamble
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
    
    Serial.print("Sending LoRA message: ");
    Serial.println(message);
    
    // Transmit the message
    int state = radio.transmit(message.c_str());
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Message sent successfully");
        
        // Restart listening for incoming messages
        radio.startReceive();
        
        return true;
    } else {
        Serial.print("Message send failed, code: ");
        Serial.println(state);
        
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
