#include "LoRAManager.h"

LoRAManager::LoRAManager() 
    : radio(nullptr),
      currentSyncWord(DEFAULT_SYNC_WORD),
      lastRSSI(0),
      lastSNR(0.0) {
}

bool LoRAManager::init() {
    Serial.println("Initializing LoRA...");
    
    // Create SX1262 instance with pin configuration
    radio = new SX1262(new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN));
    
    // Initialize SX1262
    SX1262* lora = (SX1262*)radio;
    int state = lora->begin(LORA_FREQUENCY, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, 
                            LORA_CODING_RATE, currentSyncWord, LORA_TX_POWER);
    
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print("LoRA initialization failed, code: ");
        Serial.println(state);
        return false;
    }
    
    // Configure additional settings
    lora->setCRC(true);
    
    // Start listening for packets
    lora->startReceive();
    
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
    
    if (radio != nullptr) {
        SX1262* lora = (SX1262*)radio;
        lora->setSyncWord(currentSyncWord, 0x44);  // Private network, standard preamble
    }
    
    Serial.print("LoRA sync word updated to: 0x");
    Serial.println(currentSyncWord, HEX);
}

uint8_t LoRAManager::getSyncWord() {
    return currentSyncWord;
}

bool LoRAManager::sendMessage(const String& message) {
    if (message.length() == 0 || radio == nullptr) {
        return false;
    }
    
    Serial.print("Sending LoRA message: ");
    Serial.println(message);
    
    SX1262* lora = (SX1262*)radio;
    
    // Transmit the message
    int state = lora->transmit(message.c_str());
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Message sent successfully");
        
        // Restart listening for incoming messages
        lora->startReceive();
        
        return true;
    } else {
        Serial.print("Message send failed, code: ");
        Serial.println(state);
        
        // Try to restart listening anyway
        lora->startReceive();
        
        return false;
    }
}

String LoRAManager::receiveMessage() {
    if (radio == nullptr) {
        return "";
    }
    
    SX1262* lora = (SX1262*)radio;
    String message = "";
    
    // Check if packet is available (non-blocking)
    int state = lora->readData(message);
    
    if (state == RADIOLIB_ERR_NONE) {
        // Packet received successfully
        lastRSSI = lora->getRSSI();
        lastSNR = lora->getSNR();
        
        Serial.print("Received LoRA message: ");
        Serial.println(message);
        Serial.print("RSSI: ");
        Serial.print(lastRSSI);
        Serial.print(" dBm, SNR: ");
        Serial.print(lastSNR);
        Serial.println(" dB");
        
        // Restart listening
        lora->startReceive();
        
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
    if (radio == nullptr) return;
    
    SX1262* lora = (SX1262*)radio;
    float freqMHz = frequency / 1E6;
    lora->setFrequency(freqMHz);
    
    Serial.print("LoRA frequency set to: ");
    Serial.print(freqMHz);
    Serial.println(" MHz");
}

void LoRAManager::setSpreadingFactor(int sf) {
    if (sf < 6 || sf > 12 || radio == nullptr) {
        Serial.println("Invalid spreading factor (must be 6-12)");
        return;
    }
    
    SX1262* lora = (SX1262*)radio;
    lora->setSpreadingFactor(sf);
    
    Serial.print("LoRA spreading factor set to: ");
    Serial.println(sf);
}

void LoRAManager::setTxPower(int power) {
    if (power < 2 || power > 20 || radio == nullptr) {
        Serial.println("Invalid TX power (must be 2-20 dBm)");
        return;
    }
    
    SX1262* lora = (SX1262*)radio;
    lora->setOutputPower(power);
    
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
