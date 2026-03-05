#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <heltec_unofficial.h>

// Display dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

/**
 * DisplayManager - Manages OLED display operations
 * 
 * Handles initialization and display of status messages,
 * received LoRA data, BLE connection status, and sync word info.
 */
class DisplayManager {
public:
    DisplayManager();
    
    /**
     * Initialize OLED display
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Clear the display
     */
    void clear();
    
    /**
     * Display a message (typically received LoRA message)
     * @param message Message text to display
     * @param rssi Optional RSSI value to show
     * @param snr Optional SNR value to show
     */
    void showMessage(const String& message, int rssi = 0, float snr = 0.0);
    
    /**
     * Display status text
     * @param status Status message to display
     */
    void showStatus(const String& status);
    
    /**
     * Show BLE connection status
     * @param connected true if BLE client connected, false otherwise
     */
    void showBLEStatus(bool connected);
    
    /**
     * Show current sync word on display
     * @param syncWord Current sync word value
     */
    void showSyncWord(uint8_t syncWord);
    
    /**
     * Show startup screen with app name and version
     */
    void showStartupScreen();
    
    /**
     * Update display with current system state
     * @param bleConnected BLE connection status
     * @param syncWord Current sync word
     * @param lastMessage Last received message (optional)
     */
    void updateStatus(bool bleConnected, uint8_t syncWord, const String& lastMessage = "");

private:
    unsigned long lastUpdate;
};

#endif // DISPLAY_MANAGER_H
