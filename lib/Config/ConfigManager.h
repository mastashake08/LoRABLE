#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

// Preferences namespace
#define PREFS_NAMESPACE "lorable"

// Configuration keys
#define KEY_SYNC_WORD   "syncWord"
#define KEY_FREQUENCY   "frequency"
#define KEY_SF          "spreadingFactor"
#define KEY_BANDWIDTH   "bandwidth"
#define KEY_TX_POWER    "txPower"
#define KEY_WIFI_SSID   "wifiSSID"
#define KEY_WIFI_PASSWORD "wifiPassword"
#define KEY_DEVICE_NAME "deviceName"

/**
 * ConfigManager - Manages persistent configuration storage
 * 
 * Uses ESP32 Preferences to store and retrieve configuration
 * values like syncWord, frequency, spreading factor, etc.
 */
class ConfigManager {
public:
    ConfigManager();
    
    /**
     * Initialize configuration manager
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Save sync word to persistent storage
     * @param syncWord Sync word byte value
     */
    void saveSyncWord(uint8_t syncWord);
    
    /**
     * Load sync word from persistent storage
     * @param defaultValue Default value if not found
     * @return Stored sync word or default value
     */
    uint8_t loadSyncWord(uint8_t defaultValue = 0x12);
    
    /**
     * Save LoRA frequency
     * @param frequency Frequency in Hz
     */
    void saveFrequency(long frequency);
    
    /**
     * Load LoRA frequency
     * @param defaultValue Default frequency if not found
     * @return Stored frequency or default value
     */
    long loadFrequency(long defaultValue = 915E6);
    
    /**
     * Save spreading factor
     * @param sf Spreading factor (6-12)
     */
    void saveSpreadingFactor(int sf);
    
    /**
     * Load spreading factor
     * @param defaultValue Default value if not found
     * @return Stored spreading factor or default value
     */
    int loadSpreadingFactor(int defaultValue = 7);
    
    /**
     * Save transmission power
     * @param power TX power in dBm
     */
    void saveTxPower(int power);
    
    /**
     * Load transmission power
     * @param defaultValue Default value if not found
     * @return Stored TX power or default value
     */
    int loadTxPower(int defaultValue = 20);
    
    /**
     * Save WiFi credentials
     * @param ssid WiFi network SSID
     * @param password WiFi network password
     */
    void saveWiFiCredentials(const String& ssid, const String& password);
    
    /**
     * Load WiFi SSID
     * @return Stored SSID or empty string
     */
    String loadWiFiSSID();
    
    /**
     * Load WiFi password
     * @return Stored password or empty string
     */
    String loadWiFiPassword();
    
    /**
     * Check if WiFi credentials are configured
     * @return true if both SSID and password are stored
     */
    bool hasWiFiCredentials();
    
    /**
     * Clear WiFi credentials from storage
     */
    void clearWiFiCredentials();
    
    /**
     * Save device name to persistent storage
     * @param name Device name
     */
    void saveDeviceName(const String& name);
    
    /**
     * Load device name from persistent storage
     * @param defaultValue Default device name if not found
     * @return Stored device name or default value
     */
    String loadDeviceName(const String& defaultValue = "LoRABLE");
    
    /**
     * Clear all stored preferences
     */
    void clearAll();
    
    /**
     * Print all current settings to Serial
     */
    void printSettings();

private:
    Preferences preferences;
};

#endif // CONFIG_MANAGER_H
