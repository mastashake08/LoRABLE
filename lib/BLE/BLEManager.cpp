#include "BLEManager.h"

// Server callbacks for connection/disconnection events
class BLEManager::ServerCallbacks: public BLEServerCallbacks {
private:
    BLEManager* manager;
    
public:
    ServerCallbacks(BLEManager* mgr) : manager(mgr) {}
    
    void onConnect(BLEServer* pServer) {
        manager->deviceConnected = true;
        Serial.println("BLE Client connected");
    }
    
    void onDisconnect(BLEServer* pServer) {
        manager->deviceConnected = false;
        Serial.println("BLE Client disconnected");
        
        // Restart advertising
        BLEDevice::startAdvertising();
        Serial.println("BLE Advertising restarted");
    }
};

// Characteristic callbacks for read/write events
class BLEManager::CharacteristicCallbacks: public BLECharacteristicCallbacks {
private:
    BLEManager* manager;
    bool isMessageCharacteristic;
    
public:
    CharacteristicCallbacks(BLEManager* mgr, bool isMessage = false) 
        : manager(mgr), isMessageCharacteristic(isMessage) {}
    
    void onWrite(BLECharacteristic *pCharacteristic) {
        String value = pCharacteristic->getValue();
        
        if (value.length() > 0) {
            if (isMessageCharacteristic) {
                // Handle message characteristic
                Serial.print("Received message via BLE: ");
                Serial.println(value);
                
                // Trigger message callback if set
                if (manager->messageCallback != nullptr) {
                    manager->messageCallback(value);
                }
            } else {
                // Handle syncWord characteristic
                uint8_t syncWord = (uint8_t)value[0];
                manager->currentSyncWord = syncWord;
                
                Serial.print("Received new syncWord via BLE: 0x");
                Serial.println(syncWord, HEX);
                
                // Trigger callback if set
                if (manager->syncWordCallback != nullptr) {
                    manager->syncWordCallback(syncWord);
                }
            }
        }
    }
    
    void onRead(BLECharacteristic *pCharacteristic) {
        if (!isMessageCharacteristic) {
            Serial.print("SyncWord read via BLE: 0x");
            Serial.println(manager->currentSyncWord, HEX);
        }
    }
};

BLEManager::BLEManager() 
    : pServer(nullptr), 
      pSyncWordCharacteristic(nullptr),
      pMessageCharacteristic(nullptr),
      deviceConnected(false),
      currentSyncWord(0x12),  // Default syncWord
      syncWordCallback(nullptr),
      messageCallback(nullptr) {
}

bool BLEManager::init() {
    Serial.println("Initializing BLE...");
    
    // Initialize BLE Device
    BLEDevice::init("LoRABLE");
    
    // Create BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));
    
    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // Create SyncWord Characteristic
    pSyncWordCharacteristic = pService->createCharacteristic(
        SYNCWORD_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    // Set characteristic callbacks
    pSyncWordCharacteristic->setCallbacks(new CharacteristicCallbacks(this, false));
    
    // Set initial value
    pSyncWordCharacteristic->setValue(&currentSyncWord, 1);
    
    // Create Message Characteristic
    pMessageCharacteristic = pService->createCharacteristic(
        MESSAGE_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Set message characteristic callbacks
    pMessageCharacteristic->setCallbacks(new CharacteristicCallbacks(this, true));
    
    // Start service
    pService->start();
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Functions for iPhone connection issues
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE initialized successfully");
    Serial.println("Waiting for BLE client connection...");
    
    return true;
}

void BLEManager::setSyncWordCallback(void (*callback)(uint8_t)) {
    syncWordCallback = callback;
}

void BLEManager::setMessageCallback(void (*callback)(const String&)) {
    messageCallback = callback;
}

bool BLEManager::isConnected() {
    return deviceConnected;
}

uint8_t BLEManager::getSyncWord() {
    return currentSyncWord;
}

void BLEManager::setSyncWord(uint8_t syncWord) {
    currentSyncWord = syncWord;
    if (pSyncWordCharacteristic != nullptr) {
        pSyncWordCharacteristic->setValue(&currentSyncWord, 1);
        pSyncWordCharacteristic->notify();
    }
}
