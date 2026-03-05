#include "ConfigManager.h"

ConfigManager::ConfigManager() {
}

bool ConfigManager::init() {
    Serial.println("Initializing Config Manager...");
    
    // Preferences will be opened when needed
    Serial.println("Config Manager initialized successfully");
    
    return true;
}

void ConfigManager::saveSyncWord(uint8_t syncWord) {
    preferences.begin(PREFS_NAMESPACE, false);  // Read-write mode
    preferences.putUChar(KEY_SYNC_WORD, syncWord);
    preferences.end();
    
    Serial.print("Saved sync word to config: 0x");
    Serial.println(syncWord, HEX);
}

uint8_t ConfigManager::loadSyncWord(uint8_t defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);  // Read-only mode
    uint8_t syncWord = preferences.getUChar(KEY_SYNC_WORD, defaultValue);
    preferences.end();
    
    Serial.print("Loaded sync word from config: 0x");
    Serial.println(syncWord, HEX);
    
    return syncWord;
}

void ConfigManager::saveFrequency(long frequency) {
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.putLong(KEY_FREQUENCY, frequency);
    preferences.end();
    
    Serial.print("Saved frequency to config: ");
    Serial.print(frequency / 1E6);
    Serial.println(" MHz");
}

long ConfigManager::loadFrequency(long defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    long frequency = preferences.getLong(KEY_FREQUENCY, defaultValue);
    preferences.end();
    
    Serial.print("Loaded frequency from config: ");
    Serial.print(frequency / 1E6);
    Serial.println(" MHz");
    
    return frequency;
}

void ConfigManager::saveSpreadingFactor(int sf) {
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.putInt(KEY_SF, sf);
    preferences.end();
    
    Serial.print("Saved spreading factor to config: ");
    Serial.println(sf);
}

int ConfigManager::loadSpreadingFactor(int defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    int sf = preferences.getInt(KEY_SF, defaultValue);
    preferences.end();
    
    Serial.print("Loaded spreading factor from config: ");
    Serial.println(sf);
    
    return sf;
}

void ConfigManager::saveTxPower(int power) {
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.putInt(KEY_TX_POWER, power);
    preferences.end();
    
    Serial.print("Saved TX power to config: ");
    Serial.print(power);
    Serial.println(" dBm");
}

int ConfigManager::loadTxPower(int defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    int power = preferences.getInt(KEY_TX_POWER, defaultValue);
    preferences.end();
    
    Serial.print("Loaded TX power from config: ");
    Serial.print(power);
    Serial.println(" dBm");
    
    return power;
}

void ConfigManager::saveWiFiCredentials(const String& ssid, const String& password) {
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.putString(KEY_WIFI_SSID, ssid);
    preferences.putString(KEY_WIFI_PASSWORD, password);
    preferences.end();
    
    Serial.println("Saved WiFi credentials to config");
    Serial.print("SSID: ");
    Serial.println(ssid);
}

String ConfigManager::loadWiFiSSID() {
    preferences.begin(PREFS_NAMESPACE, true);
    String ssid = preferences.getString(KEY_WIFI_SSID, "");
    preferences.end();
    
    return ssid;
}

String ConfigManager::loadWiFiPassword() {
    preferences.begin(PREFS_NAMESPACE, true);
    String password = preferences.getString(KEY_WIFI_PASSWORD, "");
    preferences.end();
    
    return password;
}

bool ConfigManager::hasWiFiCredentials() {
    String ssid = loadWiFiSSID();
    String password = loadWiFiPassword();
    return (ssid.length() > 0 && password.length() > 0);
}

void ConfigManager::clearWiFiCredentials() {
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.remove(KEY_WIFI_SSID);
    preferences.remove(KEY_WIFI_PASSWORD);
    preferences.end();
    
    Serial.println("WiFi credentials cleared from config");
}

void ConfigManager::saveDeviceName(const String& name) {
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.putString(KEY_DEVICE_NAME, name);
    preferences.end();
    
    Serial.print("Saved device name to config: ");
    Serial.println(name);
}

String ConfigManager::loadDeviceName(const String& defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    String name = preferences.getString(KEY_DEVICE_NAME, defaultValue);
    preferences.end();
    
    Serial.print("Loaded device name from config: ");
    Serial.println(name);
    
    return name;
}

void ConfigManager::clearAll() {
    preferences.begin(PREFS_NAMESPACE, false);
    preferences.clear();
    preferences.end();
    
    Serial.println("All configuration cleared");
}

void ConfigManager::printSettings() {
    Serial.println("=== Current Configuration ===");
    
    preferences.begin(PREFS_NAMESPACE, true);
    
    uint8_t syncWord = preferences.getUChar(KEY_SYNC_WORD, 0x12);
    long frequency = preferences.getLong(KEY_FREQUENCY, 915E6);
    int sf = preferences.getInt(KEY_SF, 7);
    int power = preferences.getInt(KEY_TX_POWER, 20);
    String ssid = preferences.getString(KEY_WIFI_SSID, "");
    bool hasPassword = preferences.getString(KEY_WIFI_PASSWORD, "").length() > 0;
    String deviceName = preferences.getString(KEY_DEVICE_NAME, "LoRABLE");
    
    preferences.end();
    
    Serial.print("Device Name: ");
    Serial.println(deviceName);
    Serial.print("Sync Word: 0x");
    Serial.println(syncWord, HEX);
    Serial.print("Frequency: ");
    Serial.print(frequency / 1E6);
    Serial.println(" MHz");
    Serial.print("Spreading Factor: ");
    Serial.println(sf);
    Serial.print("TX Power: ");
    Serial.print(power);
    Serial.println(" dBm");
    
    // WiFi credentials (show SSID only, not password)
    if (ssid.length() > 0) {
        Serial.print("WiFi SSID: ");
        Serial.println(ssid);
        Serial.println("WiFi Password: [CONFIGURED]");
    } else {
        Serial.println("WiFi: Not configured");
    }
    
    Serial.println("============================");
}
