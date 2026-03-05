#include "LoRAManager.h"

LoRAManager::LoRAManager() 
    : currentSyncWord(DEFAULT_SYNC_WORD),
      lastRSSI(0),
      lastSNR(0.0) {
}

bool LoRAManager::init() {
    Serial.println("Initializing LoRA...");
    
    // Setup LoRA pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO1);
    
    // Initialize LoRA with frequency
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("LoRA initialization failed!");
        return false;
    }
    
    // Configure LoRA parameters
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSyncWord(currentSyncWord);
    
    // Enable CRC
    LoRa.enableCrc();
    
    Serial.println("LoRA initialized successfully");
    Serial.print("Frequency: ");
    Serial.print(LORA_FREQUENCY / 1E6);
    Serial.println(" MHz");
    Serial.print("Spreading Factor: ");
    Serial.println(LORA_SPREADING_FACTOR);
    Serial.print("Bandwidth: ");
    Serial.print(LORA_BANDWIDTH / 1E3);
    Serial.println(" kHz");
    Serial.print("Sync Word: 0x");
    Serial.println(currentSyncWord, HEX);
    
    return true;
}

void LoRAManager::setSyncWord(uint8_t syncWord) {
    currentSyncWord = syncWord;
    LoRa.setSyncWord(currentSyncWord);
    
    Serial.print("LoRA sync word updated to: 0x");
    Serial.println(currentSyncWord, HEX);
}

uint8_t LoRAManager::getSyncWord() {
    return currentSyncWord;
}

bool LoRAManager::sendMessage(const String& message) {
    if (message.length() == 0) {
        return false;
    }
    
    Serial.print("Sending LoRA message: ");
    Serial.println(message);
    
    // Begin packet
    LoRa.beginPacket();
    LoRa.print(message);
    
    // End packet and transmit
    bool success = LoRa.endPacket();
    
    if (success) {
        Serial.println("Message sent successfully");
    } else {
        Serial.println("Message send failed");
    }
    
    return success;
}

String LoRAManager::receiveMessage() {
    // Check if packet available
    int packetSize = LoRa.parsePacket();
    
    if (packetSize == 0) {
        return "";  // No packet available
    }
    
    // Read packet
    String message = "";
    while (LoRa.available()) {
        message += (char)LoRa.read();
    }
    
    // Store RSSI and SNR
    lastRSSI = LoRa.packetRssi();
    lastSNR = LoRa.packetSnr();
    
    Serial.print("Received LoRA message: ");
    Serial.println(message);
    Serial.print("RSSI: ");
    Serial.print(lastRSSI);
    Serial.print(" dBm, SNR: ");
    Serial.print(lastSNR);
    Serial.println(" dB");
    
    return message;
}

void LoRAManager::setFrequency(long frequency) {
    LoRa.setFrequency(frequency);
    
    Serial.print("LoRA frequency set to: ");
    Serial.print(frequency / 1E6);
    Serial.println(" MHz");
}

void LoRAManager::setSpreadingFactor(int sf) {
    if (sf >= 6 && sf <= 12) {
        LoRa.setSpreadingFactor(sf);
        
        Serial.print("LoRA spreading factor set to: ");
        Serial.println(sf);
    } else {
        Serial.println("Invalid spreading factor (must be 6-12)");
    }
}

void LoRAManager::setTxPower(int power) {
    if (power >= 2 && power <= 20) {
        LoRa.setTxPower(power);
        
        Serial.print("LoRA TX power set to: ");
        Serial.print(power);
        Serial.println(" dBm");
    } else {
        Serial.println("Invalid TX power (must be 2-20 dBm)");
    }
}

int LoRAManager::getRSSI() {
    return lastRSSI;
}

float LoRAManager::getSNR() {
    return lastSNR;
}
