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
      pBatteryLevelCharacteristic(nullptr),
      deviceConnected(false),
      currentSyncWord(0x12),  // Default syncWord
      batteryLevel(100),     // Default battery level
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
    
    Serial.println("Creating characteristics...");
    
    // Create SyncWord Characteristic
    pSyncWordCharacteristic = pService->createCharacteristic(
        SYNCWORD_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    // Add BLE2902 descriptor for notifications
    pSyncWordCharacteristic->addDescriptor(new BLE2902());
    
    // Set characteristic callbacks
    pSyncWordCharacteristic->setCallbacks(new CharacteristicCallbacks(this, false));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pSyncWordDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pSyncWordDescriptor->setValue("Sync Word");
    pSyncWordCharacteristic->addDescriptor(pSyncWordDescriptor);
    
    // Set initial value
    pSyncWordCharacteristic->setValue(&currentSyncWord, 1);
    
    Serial.println("SyncWord characteristic created");
    
    // Create Message Characteristic
    pMessageCharacteristic = pService->createCharacteristic(
        MESSAGE_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Set message characteristic callbacks
    pMessageCharacteristic->setCallbacks(new CharacteristicCallbacks(this, true));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pMessageDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pMessageDescriptor->setValue("LoRA Message");
    pMessageCharacteristic->addDescriptor(pMessageDescriptor);
    
    // Set initial empty value
    pMessageCharacteristic->setValue("");
    
    Serial.println("Message characteristic created");
    
    // Start LoRABLE service
    pService->start();
    Serial.println("LoRABLE service started");
    
    // Create Battery Service (Standard Bluetooth SIG service)
    BLEService *pBatteryService = pServer->createService(BATTERY_SERVICE_UUID);
    
    // Create Battery Level Characteristic
    pBatteryLevelCharacteristic = pBatteryService->createCharacteristic(
        BATTERY_LEVEL_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    // Add BLE2902 descriptor for notifications
    pBatteryLevelCharacteristic->addDescriptor(new BLE2902());
    
    // Add descriptor for battery level
    BLEDescriptor *pBatteryDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pBatteryDescriptor->setValue("Battery Level");
    pBatteryLevelCharacteristic->addDescriptor(pBatteryDescriptor);
    
    // Set initial battery level
    pBatteryLevelCharacteristic->setValue(&batteryLevel, 1);
    
    // Start battery service
    pBatteryService->start();
    Serial.println("Battery service created");
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->addServiceUUID(BATTERY_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Functions for iPhone connection issues
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE initialized successfully");
    Serial.println("Waiting for BLE client connection...");
    Serial.print("Service UUID: ");
    Serial.println(SERVICE_UUID);
    Serial.print("Battery Service UUID: 0x180F");
    Serial.println();
    
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

void BLEManager::updateBatteryLevel(uint8_t level) {
    if (level > 100) level = 100;
    batteryLevel = level;
    
    if (pBatteryLevelCharacteristic != nullptr) {
        pBatteryLevelCharacteristic->setValue(&batteryLevel, 1);
        pBatteryLevelCharacteristic->notify();
        
        Serial.print("Battery level updated: ");
        Serial.print(batteryLevel);
        Serial.println("%");
    }
}

uint8_t BLEManager::getBatteryLevel() {
    // Read battery voltage from ADC (GPIO1 on Heltec V3)
    // Heltec V3 has Vbat connected to GPIO1 through voltage divider
    // ADC range: 0-3.3V, Battery range: 3.0-4.2V (through divider)
    
    #ifdef ADC_VBAT
        int adcValue = analogRead(ADC_VBAT);
        
        // Convert ADC value to voltage (ESP32-S3 12-bit ADC, 0-4095)
        // Voltage divider: R1=390K, R2=100K (divides by 4.9)
        float voltage = (adcValue / 4095.0) * 3.3 * 4.9;
        
        // Convert voltage to percentage (3.0V = 0%, 4.2V = 100%)
        float percentage = ((voltage - 3.0) / (4.2 - 3.0)) * 100.0;
        
        // Clamp to 0-100
        if (percentage < 0) percentage = 0;
        if (percentage > 100) percentage = 100;
        
        return (uint8_t)percentage;
    #else
        // If no battery pin defined, return 100%
        return 100;
    #endif
}
