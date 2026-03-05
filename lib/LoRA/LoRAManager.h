#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H

#include <Arduino.h>
#include <LoRa.h>

// LoRA SX1262 Pins for Heltec WiFi LoRA 32 V3
#define LORA_SCK     9
#define LORA_MISO    11
#define LORA_MOSI    10
#define LORA_NSS     8
#define LORA_RST     12
#define LORA_DIO1    14
#define LORA_BUSY    13

// Default LoRA Configuration
#define LORA_FREQUENCY      915E6   // 915 MHz (adjust for your region: 433E6, 868E6, 915E6)
#define LORA_BANDWIDTH      125E3   // 125 kHz
#define LORA_SPREADING_FACTOR  7    // SF7 (6-12)
#define LORA_CODING_RATE    5       // 4/5 (5-8 for 4/5-4/8)
#define LORA_TX_POWER       20      // 20 dBm
#define DEFAULT_SYNC_WORD   0x12    // Default sync word

/**
 * LoRAManager - Manages LoRA transceiver operations
 * 
 * Handles initialization of SX1262 LoRA chip, configuration,
 * and message transmission/reception.
 */
class LoRAManager {
public:
    LoRAManager();
    
    /**
     * Initialize LoRA transceiver with default settings
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Set the LoRA sync word for network separation
     * @param syncWord Byte value (0x00-0xFF)
     */
    void setSyncWord(uint8_t syncWord);
    
    /**
     * Get current sync word
     * @return Current sync word value
     */
    uint8_t getSyncWord();
    
    /**
     * Send a message via LoRA
     * @param message String message to send
     * @return true if message sent successfully, false otherwise
     */
    bool sendMessage(const String& message);
    
    /**
     * Check for and receive incoming LoRA message
     * @return Received message string, empty if no message available
     */
    String receiveMessage();
    
    /**
     * Set LoRA frequency
     * @param frequency Frequency in Hz (e.g., 915E6 for 915 MHz)
     */
    void setFrequency(long frequency);
    
    /**
     * Set LoRA spreading factor
     * @param sf Spreading factor (6-12)
     */
    void setSpreadingFactor(int sf);
    
    /**
     * Set LoRA transmission power
     * @param power Power in dBm (2-20)
     */
    void setTxPower(int power);
    
    /**
     * Get RSSI of last received packet
     * @return RSSI value in dBm
     */
    int getRSSI();
    
    /**
     * Get SNR of last received packet
     * @return SNR value in dB
     */
    float getSNR();

private:
    uint8_t currentSyncWord;
    int lastRSSI;
    float lastSNR;
};

#endif // LORA_MANAGER_H
