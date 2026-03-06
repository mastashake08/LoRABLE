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
public:
    enum CharType { SYNCWORD, MESSAGE, WIFI_SSID, WIFI_PASSWORD, DEVICE_NAME, GPIO_CONTROL, LED_CONTROL };
    
    CharacteristicCallbacks(BLEManager* mgr, CharType type) 
        : manager(mgr), charType(type) {}
    
private:
    BLEManager* manager;
    CharType charType;
    
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string stdValue = pCharacteristic->getValue();
        String value = String(stdValue.c_str());
        
        if (value.length() > 0) {
            switch(charType) {
                case MESSAGE:
                    Serial.print("Received message via BLE: ");
                    Serial.println(value);
                    if (manager->messageCallback != nullptr) {
                        manager->messageCallback(value);
                    }
                    break;
                    
                case SYNCWORD:
                    {
                        uint8_t syncWord = (uint8_t)value[0];
                        manager->currentSyncWord = syncWord;
                        Serial.print("Received new syncWord via BLE: 0x");
                        Serial.println(syncWord, HEX);
                        if (manager->syncWordCallback != nullptr) {
                            manager->syncWordCallback(syncWord);
                        }
                    }
                    break;
                    
                case WIFI_SSID:
                    manager->wifiSSID = value;
                    Serial.print("Received WiFi SSID via BLE: ");
                    Serial.println(value);
                    // Trigger callback only when both SSID and password are set
                    if (manager->wifiPassword.length() > 0 && manager->wifiCallback != nullptr) {
                        manager->wifiCallback(manager->wifiSSID, manager->wifiPassword);
                    }
                    break;
                    
                case WIFI_PASSWORD:
                    manager->wifiPassword = value;
                    Serial.println("Received WiFi Password via BLE");
                    // Trigger callback only when both SSID and password are set
                    if (manager->wifiSSID.length() > 0 && manager->wifiCallback != nullptr) {
                        manager->wifiCallback(manager->wifiSSID, manager->wifiPassword);
                    }
                    break;
                    
                case DEVICE_NAME:
                    manager->deviceName = value;
                    Serial.print("Received new device name via BLE: ");
                    Serial.println(value);
                    // Update the BLE device name
                    BLEDevice::init(value.c_str());
                    if (manager->deviceNameCallback != nullptr) {
                        manager->deviceNameCallback(value);
                    }
                    break;
                    
                case GPIO_CONTROL:
                    Serial.print("Received GPIO control via BLE: ");
                    Serial.println(value);
                    if (manager->gpioCallback != nullptr) {
                        manager->gpioCallback(value);
                    }
                    break;
                    
                case LED_CONTROL:
                    {
                        uint8_t ledState = (uint8_t)value[0];
                        Serial.print("Received LED control via BLE: ");
                        Serial.println(ledState ? "ON" : "OFF");
                        if (manager->ledCallback != nullptr) {
                            manager->ledCallback(ledState);
                        }
                    }
                    break;
            }
        }
    }
    
    void onRead(BLECharacteristic *pCharacteristic) {
        if (charType == SYNCWORD) {
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
      pWiFiSSIDCharacteristic(nullptr),
      pWiFiPasswordCharacteristic(nullptr),
      pDeviceNameCharacteristic(nullptr),
      pGPIOControlCharacteristic(nullptr),
      pLEDControlCharacteristic(nullptr),
      deviceConnected(false),
      currentSyncWord(0x12),  // Default syncWord
      batteryLevel(100),     // Default battery level
      wifiSSID(""),
      wifiPassword(""),
      deviceName("LoRABLE"),  // Default device name
      syncWordCallback(nullptr),
      messageCallback(nullptr),
      wifiCallback(nullptr),
      deviceNameCallback(nullptr),
      gpioCallback(nullptr),
      ledCallback(nullptr) {
}

bool BLEManager::init() {
    Serial.println("Initializing BLE...");
    
    // Initialize BLE Device with current device name
    BLEDevice::init(deviceName.c_str());
    
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
    
    // Set characteristic callbacks - use SYNCWORD enum
    pSyncWordCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CharacteristicCallbacks::SYNCWORD));
    
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
    
    // Set message characteristic callbacks - use MESSAGE enum
    pMessageCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CharacteristicCallbacks::MESSAGE));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pMessageDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pMessageDescriptor->setValue("LoRA Message");
    pMessageCharacteristic->addDescriptor(pMessageDescriptor);
    
    // Set initial empty value
    pMessageCharacteristic->setValue("");
    
    Serial.println("Message characteristic created");
    
    // Create WiFi SSID Characteristic
    pWiFiSSIDCharacteristic = pService->createCharacteristic(
        WIFI_SSID_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Set WiFi SSID characteristic callbacks
    pWiFiSSIDCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CharacteristicCallbacks::WIFI_SSID));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pSSIDDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pSSIDDescriptor->setValue("WiFi SSID");
    pWiFiSSIDCharacteristic->addDescriptor(pSSIDDescriptor);
    pWiFiSSIDCharacteristic->setValue("");
    
    Serial.println("WiFi SSID characteristic created");
    
    // Create WiFi Password Characteristic
    pWiFiPasswordCharacteristic = pService->createCharacteristic(
        WIFI_PASSWORD_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Set WiFi Password characteristic callbacks
    pWiFiPasswordCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CharacteristicCallbacks::WIFI_PASSWORD));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pPasswordDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pPasswordDescriptor->setValue("WiFi Password");
    pWiFiPasswordCharacteristic->addDescriptor(pPasswordDescriptor);
    pWiFiPasswordCharacteristic->setValue("");
    
    Serial.println("WiFi Password characteristic created");
    
    // Create Device Name Characteristic
    pDeviceNameCharacteristic = pService->createCharacteristic(
        DEVICE_NAME_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Set Device Name characteristic callbacks
    pDeviceNameCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CharacteristicCallbacks::DEVICE_NAME));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pDeviceNameDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pDeviceNameDescriptor->setValue("Device Name");
    pDeviceNameCharacteristic->addDescriptor(pDeviceNameDescriptor);
    pDeviceNameCharacteristic->setValue(deviceName.c_str());
    
    Serial.println("Device Name characteristic created");
    
    // Create GPIO Control Characteristic
    pGPIOControlCharacteristic = pService->createCharacteristic(
        GPIO_CONTROL_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Set GPIO Control characteristic callbacks
    pGPIOControlCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CharacteristicCallbacks::GPIO_CONTROL));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pGPIODescriptor = new BLEDescriptor((uint16_t)0x2901);
    pGPIODescriptor->setValue("GPIO Control");
    pGPIOControlCharacteristic->addDescriptor(pGPIODescriptor);
    pGPIOControlCharacteristic->setValue("0,0");  // Default: GPIO 0, state 0
    
    Serial.println("GPIO Control characteristic created");
    
    // Create LED Control Characteristic
    pLEDControlCharacteristic = pService->createCharacteristic(
        LED_CONTROL_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Set LED Control characteristic callbacks
    pLEDControlCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CharacteristicCallbacks::LED_CONTROL));
    
    // Add descriptor for user-friendly name
    BLEDescriptor *pLEDDescriptor = new BLEDescriptor((uint16_t)0x2901);
    pLEDDescriptor->setValue("LED Control");
    pLEDControlCharacteristic->addDescriptor(pLEDDescriptor);
    
    uint8_t ledDefaultState = 0;  // Default: LED OFF
    pLEDControlCharacteristic->setValue(&ledDefaultState, 1);
    
    Serial.println("LED Control characteristic created");
    
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
    
    // Add manufacturer data "Mastashake"
    // Format: Company ID (2 bytes, little-endian) + Data
    // Using 0xFFFF as custom company ID
    std::string manufacturerData = "";
    manufacturerData += (char)0xFF;  // Company ID low byte
    manufacturerData += (char)0xFF;  // Company ID high byte
    manufacturerData += "Mastashake"; // Custom data
    BLEAdvertisementData advertisementData;
    advertisementData.setManufacturerData(manufacturerData);
    advertisementData.setCompleteServices(BLEUUID(SERVICE_UUID));
    advertisementData.setName(deviceName.c_str());
    pAdvertising->setAdvertisementData(advertisementData);
    
    BLEDevice::startAdvertising();
    
    Serial.println("BLE initialized successfully");
    Serial.println("Manufacturer data: Mastashake");
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

void BLEManager::setWiFiCallback(void (*callback)(const String&, const String&)) {
    wifiCallback = callback;
}

void BLEManager::setDeviceNameCallback(void (*callback)(const String&)) {
    deviceNameCallback = callback;
}

void BLEManager::setGPIOCallback(void (*callback)(const String&)) {
    gpioCallback = callback;
}

void BLEManager::setLEDCallback(void (*callback)(uint8_t)) {
    ledCallback = callback;
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
    // Heltec V3 battery reading
    // VBAT_CTRL (GPIO37) enables battery voltage measurement
    // VBAT_ADC (GPIO1) reads the battery voltage through voltage divider
    
    #ifdef VBAT_ADC
        // Enable battery voltage reading
        pinMode(VBAT_CTRL, OUTPUT);
        digitalWrite(VBAT_CTRL, LOW);  // LOW enables battery reading
        delay(10);  // Wait for voltage to stabilize
        
        // Configure ADC with proper attenuation for 0-3.3V range
        analogSetAttenuation(ADC_11db);  // Full 0-3.6V range
        
        // Take multiple readings for stability
        int total = 0;
        const int numReadings = 10;
        for (int i = 0; i < numReadings; i++) {
            total += analogRead(VBAT_ADC);
            delay(1);
        }
        int adcValue = total / numReadings;
        
        // Disable battery reading to save power
        pinMode(VBAT_CTRL, INPUT);
        
        // Convert ADC to voltage
        // ADC is 12-bit (0-4095), voltage divider ratio is 390k/(390k+100k) = 0.796
        // Vbat = (ADC / 4095) * 3.3V * (490k/100k) = ADC * 0.00394
        float vbat = adcValue * 0.00394;
        
        Serial.print("Battery ADC: ");
        Serial.print(adcValue);
        Serial.print(", Voltage: ");
        Serial.print(vbat, 2);
        Serial.print("V");
        
        // Convert voltage to percentage
        // LiPo voltage range: 3.0V (empty) to 4.2V (full)
        float percentage = ((vbat - 3.0) / (4.2 - 3.0)) * 100.0;
        
        // Clamp to 0-100
        if (percentage < 0) percentage = 0;
        if (percentage > 100) percentage = 100;
        
        Serial.print(", Percentage: ");
        Serial.print((uint8_t)percentage);
        Serial.println("%");
        
        return (uint8_t)percentage;
    #else
        // If no battery pin defined, return 100%
        Serial.println("Battery pins not defined, returning 100%");
        return 100;
    #endif
}

String BLEManager::getDeviceName() {
    return deviceName;
}

void BLEManager::setDeviceName(const String& name) {
    deviceName = name;
    if (pDeviceNameCharacteristic != nullptr) {
        pDeviceNameCharacteristic->setValue(deviceName.c_str());
        pDeviceNameCharacteristic->notify();
    }
    // Update the BLE device name
    BLEDevice::init(deviceName.c_str());
    Serial.print("Device name set to: ");
    Serial.println(deviceName);
}
