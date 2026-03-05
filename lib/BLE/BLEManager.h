#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SYNCWORD_CHAR_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define MESSAGE_CHAR_UUID   "a3c87500-8ed3-4bdf-8a39-a01bebede295"

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

private:
    BLEServer* pServer;
    BLECharacteristic* pSyncWordCharacteristic;
    BLECharacteristic* pMessageCharacteristic;
    bool deviceConnected;
    uint8_t currentSyncWord;
    void (*syncWordCallback)(uint8_t);
    void (*messageCallback)(const String&);
    
    // Callback classes for BLE events
    class ServerCallbacks;
    class CharacteristicCallbacks;
    
    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;
};

#endif // BLE_MANAGER_H
