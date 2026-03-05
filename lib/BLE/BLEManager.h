#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Heltec V3 Battery pins (from heltec_unofficial.h)
#define VBAT_CTRL GPIO_NUM_37  // Battery voltage control pin
#define VBAT_ADC  GPIO_NUM_1   // Battery ADC pin

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SYNCWORD_CHAR_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define MESSAGE_CHAR_UUID      "a3c87500-8ed3-4bdf-8a39-a01bebede295"
#define WIFI_SSID_CHAR_UUID    "7c8a8e48-68c9-4e3f-a3e6-8f6e7b8c9d0a"
#define WIFI_PASSWORD_CHAR_UUID "8d9b9f59-79da-4f40-b4f7-9f7e8c9d0e1b"

// Battery Service (Standard Bluetooth SIG UUID)
#define BATTERY_SERVICE_UUID   BLEUUID((uint16_t)0x180F)
#define BATTERY_LEVEL_CHAR_UUID BLEUUID((uint16_t)0x2A19)

/**
 * BLEManager - Manages Bluetooth Low Energy operations
 * 
 * Handles BLE server initialization, characteristic management,
 * and provides callbacks for syncWord configuration changes.
 */
class BLEManager {
public:
    BLEManager();
    
    /**
     * Initialize BLE server with service and characteristics
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Set callback function to be called when syncWord is changed via BLE
     * @param callback Function pointer that takes uint8_t syncWord parameter
     */
    void setSyncWordCallback(void (*callback)(uint8_t));
    
    /**
     * Set callback function to be called when message is received via BLE
     * @param callback Function pointer that takes String message parameter
     */
    void setMessageCallback(void (*callback)(const String&));
    
    /**
     * Set callback function to be called when WiFi credentials are changed via BLE
     * @param callback Function pointer that takes SSID and password
     */
    void setWiFiCallback(void (*callback)(const String&, const String&));
    
    /**
     * Check if a BLE client is currently connected
     * @return true if connected, false otherwise
     */
    bool isConnected();
    
    /**
     * Get current syncWord value
     * @return Current syncWord byte value
     */
    uint8_t getSyncWord();
    
    /**
     * Set syncWord value (updates BLE characteristic)
     * @param syncWord New syncWord byte value
     */
    void setSyncWord(uint8_t syncWord);
    
    /**
     * Update battery level
     * @param level Battery level percentage (0-100)
     */
    void updateBatteryLevel(uint8_t level);
    
    /**
     * Get current battery level from ADC
     * @return Battery level percentage (0-100)
     */
    uint8_t getBatteryLevel();

private:
    BLEServer* pServer;
    BLECharacteristic* pSyncWordCharacteristic;
    BLECharacteristic* pMessageCharacteristic;
    BLECharacteristic* pBatteryLevelCharacteristic;
    BLECharacteristic* pWiFiSSIDCharacteristic;
    BLECharacteristic* pWiFiPasswordCharacteristic;
    bool deviceConnected;
    uint8_t currentSyncWord;
    uint8_t batteryLevel;
    String wifiSSID;
    String wifiPassword;
    void (*syncWordCallback)(uint8_t);
    void (*messageCallback)(const String&);
    void (*wifiCallback)(const String&, const String&);
    
    // Callback classes for BLE events
    class ServerCallbacks;
    class CharacteristicCallbacks;
    
    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;
};

#endif // BLE_MANAGER_H
