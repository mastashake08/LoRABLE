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
    
    preferences.end();
    
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
    Serial.println("============================");
}
