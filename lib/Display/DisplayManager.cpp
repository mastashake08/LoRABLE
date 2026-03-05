#include "DisplayManager.h"

DisplayManager::DisplayManager() 
    : lastUpdate(0) {
}

bool DisplayManager::init() {
    Serial.println("Initializing Display...");
    
    // Initialize Heltec display (library handles all pins and power)
    heltec_setup();
    
    // Clear and setup display
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.display();
    
    Serial.println("Display initialized successfully");
    
    return true;
}

void DisplayManager::clear() {
    display.clear();
    display.display();
}

void DisplayManager::showMessage(const String& message, int rssi, float snr) {
    display.clear();
    
    // Header
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Received Message:");
    display.drawLine(0, 12, 128, 12);
    
    // Message content (wrap if too long)
    display.setFont(ArialMT_Plain_16);
    int y = 16;
    
    if (message.length() > 16) {
        // Split into multiple lines
        for (size_t i = 0; i < message.length(); i += 16) {
            String line = message.substring(i, min(i + 16, message.length()));
            display.drawString(0, y, line);
            y += 16;
            if (y > 44) break;  // Don't overflow screen
        }
    } else {
        display.drawString(0, y, message);
    }
    
    // Show RSSI and SNR if provided
    if (rssi != 0) {
        display.setFont(ArialMT_Plain_10);
        String info = "RSSI:" + String(rssi) + " SNR:" + String(snr, 1);
        display.drawString(0, 54, info);
    }
    
    display.display();
}

void DisplayManager::showStatus(const String& status) {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 24, status);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.display();
}

void DisplayManager::showBLEStatus(bool connected) {
    // Draw BLE icon in top-right corner
    display.setColor(BLACK);
    display.fillRect(110, 0, 18, 10);
    display.setColor(WHITE);
    
    if (connected) {
        display.setFont(ArialMT_Plain_10);
        display.drawString(110, 0, "BLE");
    }
}

void DisplayManager::showSyncWord(uint8_t syncWord) {
    display.setFont(ArialMT_Plain_10);
    String swStr = "SW:0x" + String(syncWord, HEX);
    swStr.toUpperCase();
    display.drawString(0, 0, swStr);
}

void DisplayManager::showStartupScreen() {
    display.clear();
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, "LoRABLE");
    
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 38, "LoRA + BLE");
    display.drawString(64, 50, "Initializing...");
    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.display();
}

void DisplayManager::updateStatus(bool bleConnected, uint8_t syncWord, const String& lastMessage) {
    display.clear();
    
    // Top bar with sync word and BLE status
    display.setFont(ArialMT_Plain_10);
    showSyncWord(syncWord);
    showBLEStatus(bleConnected);
    
    display.drawLine(0, 12, 128, 12);
    
    // Status text
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 16, "Status: Listening");
    
    if (bleConnected) {
        display.drawString(0, 28, "BLE: Connected");
    } else {
        display.drawString(0, 28, "BLE: Waiting...");
    }
    
    // Last message preview
    if (lastMessage.length() > 0) {
        display.drawString(0, 40, "Last:");
        display.setFont(ArialMT_Plain_10);
        String preview = lastMessage.substring(0, min(16, (int)lastMessage.length()));
        display.drawString(0, 52, preview);
    } else {
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 48, "No messages yet");
    }
    
    display.display();
}
