/**
 * LoRABLE - LoRA + BLE Application
 * 
 * Main application file that orchestrates all modules:
 * - BLE: Receive syncWord configuration via Bluetooth
 * - LoRA: Transmit/receive messages with configured syncWord
 * - Display: Show messages and status on OLED
 * - Config: Persist settings across reboots
 * 
 * Hardware: Heltec WiFi LoRA 32 V3
 * Based on: https://randomnerdtutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
 */

#include <Arduino.h>
#include <heltec_unofficial.h>
#include "BLEManager.h"
#include "LoRAManager.h"
#include "DisplayManager.h"
#include "ConfigManager.h"

// Module instances
BLEManager bleManager;
LoRAManager loraManager;
DisplayManager displayManager;
ConfigManager configManager;

// Application state
String lastReceivedMessage = "";
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // Update display every 1 second

/**
 * Callback function called when syncWord is changed via BLE
 * Updates LoRA configuration and saves to persistent storage
 */
void onSyncWordChanged(uint8_t newSyncWord) {
    Serial.println("=== SyncWord Changed ===");
    Serial.print("New SyncWord: 0x");
    Serial.println(newSyncWord, HEX);
    
    // Update LoRA module
    loraManager.setSyncWord(newSyncWord);
    
    // Save to configuration
    configManager.saveSyncWord(newSyncWord);
    
    // Update display
    displayManager.showStatus("SyncWord Updated!");
    delay(1500);
    displayManager.updateStatus(bleManager.isConnected(), newSyncWord, lastReceivedMessage);
}

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);  // Wait for serial to initialize
    
    Serial.println("\n\n");
    Serial.println("====================================");
    Serial.println("        LoRABLE Starting...        ");
    Serial.println("====================================");
    
    // Initialize Heltec hardware (display, LoRA radio, etc.)
    heltec_setup();
    Serial.println("Heltec hardware initialized");
    
    // Initialize Display (already initialized by heltec_setup, just configure it)
    if (!displayManager.init()) {
        Serial.println("ERROR: Display initialization failed!");
    }
    displayManager.showStartupScreen();
    delay(2000);
    
    // Initialize Config Manager
    if (!configManager.init()) {
        Serial.println("ERROR: Config initialization failed!");
    }
    
    // Load saved configuration
    uint8_t savedSyncWord = configManager.loadSyncWord();
    
    // Initialize LoRA
    if (!loraManager.init()) {
        Serial.println("ERROR: LoRA initialization failed!");
        displayManager.showStatus("LoRA Failed!");
        while(1) {
            delay(1000);  // Halt if LoRA fails
        }
    }
    
    // Set saved syncWord
    loraManager.setSyncWord(savedSyncWord);
    
    // Initialize BLE
    if (!bleManager.init()) {
        Serial.println("ERROR: BLE initialization failed!");
        displayManager.showStatus("BLE Failed!");
    }
    
    // Set BLE callback
    bleManager.setSyncWordCallback(onSyncWordChanged);
    
    // Set initial syncWord in BLE characteristic
    bleManager.setSyncWord(savedSyncWord);
    
    // Print current configuration
    configManager.printSettings();
    
    // Show ready screen
    displayManager.updateStatus(bleManager.isConnected(), savedSyncWord, "");
    
    Serial.println("====================================");
    Serial.println("     LoRABLE Ready!                ");
    Serial.println("====================================");
    Serial.println("Listening for LoRA messages...");
    Serial.println("Connect via BLE to change SyncWord");
    Serial.println();
}

void loop() {
    // Check for incoming LoRA messages
    String message = loraManager.receiveMessage();
    
    if (message.length() > 0) {
        // Store message
        lastReceivedMessage = message;
        
        // Get signal quality
        int rssi = loraManager.getRSSI();
        float snr = loraManager.getSNR();
        
        // Display message with signal info
        displayManager.showMessage(message, rssi, snr);
        
        // Keep message on screen for 5 seconds
        delay(5000);
        
        // Return to status display
        displayManager.updateStatus(
            bleManager.isConnected(), 
            loraManager.getSyncWord(), 
            lastReceivedMessage
        );
    }
    
    // Periodically update status display (to show BLE connection changes)
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        displayManager.updateStatus(
            bleManager.isConnected(), 
            loraManager.getSyncWord(), 
            lastReceivedMessage
        );
        lastDisplayUpdate = millis();
    }
    
    // Call heltec_loop for display and button handling
    heltec_loop();
    
    // Small delay to prevent CPU hogging
    delay(10);
}
