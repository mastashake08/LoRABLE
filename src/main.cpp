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
#include <U8g2lib.h>
#include <Wire.h>
#include "BLEManager.h"
#include "LoRAManager.h"
#include "ConfigManager.h"

// Heltec V3 Display pins
#define OLED_SDA 17  // GPIO 17 for SDA
#define OLED_SCL 18  // GPIO 18 for SCL
#define OLED_RST 21
#define VEXT_CTRL 36  // Display power control

// U8g2 Constructor for SSD1306 128x64 I2C display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RST, OLED_SCL, OLED_SDA);

// Module instances
BLEManager bleManager;
LoRAManager loraManager;
ConfigManager configManager;

// Application state
String lastReceivedMessage = "";
String lastSentMessage = "";
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // Update display every 1 second

// Display helper functions
void initDisplay() {
    // Power on display
    pinMode(VEXT_CTRL, OUTPUT);
    digitalWrite(VEXT_CTRL, LOW);  // LOW = ON for Heltec V3
    delay(100);
    
    // Initialize I2C with custom pins
    Wire.begin(OLED_SDA, OLED_SCL);
    delay(100);
    
    // Initialize U8g2
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
}

void showStartupScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_inb16_mr);  // Large font
    u8g2.drawStr(20, 15, "LoRABLE");
    u8g2.setFont(u8g2_font_6x10_tf);   // Small font
    u8g2.drawStr(20, 45, "Initializing...");
    u8g2.sendBuffer();
}

void showStatus(const String& message) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tf);
    
    // Center text
    int width = u8g2.getStrWidth(message.c_str());
    int x = (128 - width) / 2;
    u8g2.drawStr(x, 24, message.c_str());
    
    u8g2.sendBuffer();
}

void showMessage(const String& message, int rssi, float snr) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Header
    u8g2.drawStr(0, 0, "Received:");
    
    // Message (truncate if too long)
    u8g2.setFont(u8g2_font_8x13_tf);
    String displayMsg = message.substring(0, min(16, (int)message.length()));
    u8g2.drawStr(0, 15, displayMsg.c_str());
    
    // Signal quality
    u8g2.setFont(u8g2_font_6x10_tf);
    String rssiStr = "RSSI: " + String(rssi) + " dBm";
    u8g2.drawStr(0, 38, rssiStr.c_str());
    
    String snrStr = "SNR: " + String(snr, 1) + " dB";
    u8g2.drawStr(0, 50, snrStr.c_str());
    
    u8g2.sendBuffer();
}

void showSentMessage(const String& message) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Header
    u8g2.drawStr(0, 0, "Sent via LoRA:");
    
    // Message (word wrap for long messages)
    u8g2.setFont(u8g2_font_8x13_tf);
    int y = 15;
    int maxChars = 16;
    
    for (int i = 0; i < message.length(); i += maxChars) {
        String line = message.substring(i, min(i + maxChars, (int)message.length()));
        u8g2.drawStr(0, y, line.c_str());
        y += 15;
        if (y > 50) break;  // Don't overflow screen
    }
    
    u8g2.sendBuffer();
}

void updateStatusDisplay(bool bleConnected, uint8_t syncWord, const String& lastMsg) {
    Serial.print("Updating display - SyncWord: 0x");
    Serial.print(syncWord, HEX);
    Serial.print(", BLE: ");
    Serial.println(bleConnected ? "Connected" : "Disconnected");
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // BLE status
    String bleStatus = "BLE: " + String(bleConnected ? "Connected" : "Waiting...");
    u8g2.drawStr(0, 0, bleStatus.c_str());
    
    // SyncWord
    String syncStr = "SyncWord: 0x" + String(syncWord, HEX);
    syncStr.toUpperCase();  // Ensure hex is uppercase
    u8g2.drawStr(0, 12, syncStr.c_str());
    
    // Last message preview
    u8g2.drawStr(0, 24, "Last msg:");
    if (lastMsg.length() > 0) {
        String preview = lastMsg.substring(0, min(16, (int)lastMsg.length()));
        u8g2.drawStr(0, 36, preview.c_str());
    } else {
        u8g2.drawStr(0, 36, "(none)");
    }
    
    // Listening indicator
    u8g2.drawStr(0, 54, "Listening for LoRA...");
    
    u8g2.sendBuffer();
}

/**
 * Callback function called when message is received via BLE
 * Sends the message via LoRA
 */
void onMessageReceived(const String& message) {
    Serial.println("=== Message Received via BLE ===");
    Serial.print("Message: ");
    Serial.println(message);
    
    // Send via LoRA
    showStatus("Sending...");
    
    if (loraManager.sendMessage(message)) {
        Serial.println("Message sent via LoRA successfully");
        lastSentMessage = message;
        
        // Show sent message
        showSentMessage(message);
        delay(2000);
    } else {
        Serial.println("Failed to send message via LoRA");
        showStatus("Send Failed!");
        delay(1500);
    }
    
    // Return to status display
    updateStatusDisplay(
        bleManager.isConnected(), 
        loraManager.getSyncWord(), 
        lastReceivedMessage
    );
    
    // Reset display update timer to prevent immediate overwrite
    lastDisplayUpdate = millis();
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
    
    // Reset display update timer to prevent immediate overwrite
    lastDisplayUpdate = millis();
}

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n====================================");
    Serial.println("        LoRABLE Starting...        ");
    Serial.println("====================================");
    
    // Initialize Display
    Serial.println("Initializing display...");
    initDisplay();
    
    // Show startup screen
    Serial.println("Showing startup screen...");
    showStartupScreen();
    Serial.println("Startup screen displayed");
    delay(2000);  // Show startup screen for 2 seconds
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
