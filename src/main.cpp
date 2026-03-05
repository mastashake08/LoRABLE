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
#define HELTEC_POWER_BUTTON   // Required for proper display initialization
#include <heltec_unofficial.h>
#include "BLEManager.h"
#include "LoRAManager.h"
#include "ConfigManager.h"

// Module instances
BLEManager bleManager;
LoRAManager loraManager;
ConfigManager configManager;

// Application state
String lastReceivedMessage = "";
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // Update display every 1 second

// Display helper functions
void showStartupScreen() {
    display.clear();
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, "LoRABLE");
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 45, "Initializing...");
    display.display();
}

void showStatus(const String& message) {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, message);
    display.display();
}

void showMessage(const String& message, int rssi, float snr) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Received:");
    
    // Display message (word wrap if needed)
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 15, message);
    
    // Show signal quality
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 38, "RSSI: " + String(rssi) + " dBm");
    display.drawString(0, 50, "SNR: " + String(snr, 1) + " dB");
    
    display.display();
}

void updateStatusDisplay(bool bleConnected, uint8_t syncWord, const String& lastMsg) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    
    // BLE status
    display.drawString(0, 0, "BLE: " + String(bleConnected ? "Connected" : "Waiting..."));
    
    // SyncWord
    display.drawString(0, 12, "SyncWord: 0x" + String(syncWord, HEX));
    
    // Last message preview
    display.drawString(0, 24, "Last msg:");
    if (lastMsg.length() > 0) {
        String preview = lastMsg.substring(0, min(16, (int)lastMsg.length()));
        display.drawString(0, 36, preview);
    } else {
        display.drawString(0, 36, "(none)");
    }
    
    // Listening indicator
    display.drawString(0, 54, "Listening for LoRA...");
    
    display.display();
}

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
    showStatus("SyncWord Updated!");
    delay(1500);
    updateStatusDisplay(bleManager.isConnected(), newSyncWord, lastReceivedMessage);
}

void setup() {
    // Initialize Heltec hardware first (Serial at 115200, display, button)
    heltec_setup();
    
    delay(500);  // Brief delay for hardware stabilization
    
    Serial.println("\n\n");
    Serial.println("====================================");
    Serial.println("        LoRABLE Starting...        ");
    Serial.println("====================================");
    
    // Show startup screen
    showStartupScreen();
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
        showStatus("LoRA Failed!");
        while(1) {
            heltec_loop();
            delay(1000);  // Halt if LoRA fails
        }
    }
    
    // Set saved syncWord
    loraManager.setSyncWord(savedSyncWord);
    
    // Initialize BLE
    if (!bleManager.init()) {
        Serial.println("ERROR: BLE initialization failed!");
        showStatus("BLE Failed!");
    }
    
    // Set BLE callback
    bleManager.setSyncWordCallback(onSyncWordChanged);
    
    // Set initial syncWord in BLE characteristic
    bleManager.setSyncWord(savedSyncWord);
    
    // Print current configuration
    configManager.printSettings();
    
    // Show ready screen
    updateStatusDisplay(bleManager.isConnected(), savedSyncWord, "");
    
    Serial.println("====================================");
    Serial.println("     LoRABLE Ready!                ");
    Serial.println("====================================");
    Serial.println("Listening for LoRA messages...");
    Serial.println("Connect via BLE to change SyncWord");
    Serial.println();
}

void loop() {
    // Call heltec_loop() for display/button management
    heltec_loop();
    
    // Check for incoming LoRA messages
    String message = loraManager.receiveMessage();
    
    if (message.length() > 0) {
        // Store message
        lastReceivedMessage = message;
        
        // Get signal quality
        int rssi = loraManager.getRSSI();
        float snr = loraManager.getSNR();
        
        // Display message with signal info
        showMessage(message, rssi, snr);
        
        // Keep message on screen for 5 seconds
        delay(5000);
        
        // Return to status display
        updateStatusDisplay(
            bleManager.isConnected(), 
            loraManager.getSyncWord(), 
            lastReceivedMessage
        );
    }
    
    // Periodically update status display (to show BLE connection changes)
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        updateStatusDisplay(
            bleManager.isConnected(), 
            loraManager.getSyncWord(), 
            lastReceivedMessage
        );
        lastDisplayUpdate = millis();
    }
    
    // Small delay to prevent CPU hogging
    delay(10);
}
