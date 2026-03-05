#ifndef SERIAL_COMMAND_H
#define SERIAL_COMMAND_H

#include <Arduino.h>

/**
 * SerialCommand - Handles serial commands that mimic BLE functionality
 * 
 * Commands:
 * - SYNCWORD:0xXX - Set syncword (hex value)
 * - MESSAGE:text - Send LoRA message
 * - WIFI_SSID:ssid - Set WiFi SSID
 * - WIFI_PASSWORD:password - Set WiFi password
 * - STATUS - Show current status
 */
class SerialCommand {
public:
    SerialCommand();
    
    /**
     * Initialize serial command parser
     */
    void init();
    
    /**
     * Process incoming serial data
     * Call this in loop() when serial mode is enabled
     */
    void update();
    
    /**
     * Set callback for syncword command
     * @param callback Function that takes uint8_t syncWord
     */
    void setSyncWordCallback(void (*callback)(uint8_t));
    
    /**
     * Set callback for message command
     * @param callback Function that takes String message
     */
    void setMessageCallback(void (*callback)(const String&));
    
    /**
     * Set callback for WiFi credentials command
     * @param callback Function that takes SSID and password
     */
    void setWiFiCallback(void (*callback)(const String&, const String&));
    
    /**
     * Set callback for status command
     * @param callback Function with no parameters
     */
    void setStatusCallback(void (*callback)());
    
    /**
     * Enable or disable serial command processing
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled);
    
    /**
     * Check if serial commands are enabled
     * @return true if enabled, false otherwise
     */
    bool isEnabled();
    
private:
    bool enabled;
    String inputBuffer;
    
    // Callbacks
    void (*syncWordCallback)(uint8_t);
    void (*messageCallback)(const String&);
    void (*wifiCallback)(const String&, const String&);
    void (*statusCallback)();
    
    // Command parsing
    void processCommand(const String& command);
    void printHelp();
    uint8_t parseHexByte(const String& hexStr);
};

#endif // SERIAL_COMMAND_H
