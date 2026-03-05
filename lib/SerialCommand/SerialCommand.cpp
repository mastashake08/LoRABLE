#include "SerialCommand.h"

SerialCommand::SerialCommand()
    : enabled(false),
      inputBuffer(""),
      syncWordCallback(nullptr),
      messageCallback(nullptr),
      wifiCallback(nullptr),
      statusCallback(nullptr),
      gpioCallback(nullptr) {
}

void SerialCommand::init() {
    Serial.println("SerialCommand initialized (disabled by default)");
    Serial.println("Double-press PRG button to enable Serial mode");
}

void SerialCommand::update() {
    if (!enabled) {
        return;
    }
    
    while (Serial.available() > 0) {
        char inChar = Serial.read();
        
        // End of command
        if (inChar == '\n' || inChar == '\r') {
            if (inputBuffer.length() > 0) {
                processCommand(inputBuffer);
                inputBuffer = "";
            }
        }
        // Add to buffer
        else {
            inputBuffer += inChar;
            
            // Prevent buffer overflow
            if (inputBuffer.length() > 256) {
                Serial.println("ERROR: Command too long!");
                inputBuffer = "";
            }
        }
    }
}

void SerialCommand::processCommand(const String& command) {
    Serial.print("Serial command: ");
    Serial.println(command);
    
    // Convert to uppercase for case-insensitive comparison
    String cmd = command;
    cmd.toUpperCase();
    
    // Help command
    if (cmd == "HELP" || cmd == "?") {
        printHelp();
    }
    // Status command
    else if (cmd == "STATUS") {
        Serial.println("=== STATUS ===");
        if (statusCallback != nullptr) {
            statusCallback();
        } else {
            Serial.println("Status callback not set");
        }
    }
    // SyncWord command: SYNCWORD:0xXX
    else if (cmd.startsWith("SYNCWORD:")) {
        String value = command.substring(9);
        value.trim();
        
        uint8_t syncWord = parseHexByte(value);
        
        Serial.print("Setting syncWord to: 0x");
        Serial.println(syncWord, HEX);
        
        if (syncWordCallback != nullptr) {
            syncWordCallback(syncWord);
        } else {
            Serial.println("ERROR: SyncWord callback not set");
        }
    }
    // Message command: MESSAGE:text
    else if (command.startsWith("MESSAGE:") || command.startsWith("message:")) {
        String message = command.substring(8);
        message.trim();
        
        if (message.length() == 0) {
            Serial.println("ERROR: Empty message");
            return;
        }
        
        Serial.print("Sending message: ");
        Serial.println(message);
        
        if (messageCallback != nullptr) {
            messageCallback(message);
        } else {
            Serial.println("ERROR: Message callback not set");
        }
    }
    // WiFi SSID command: WIFI_SSID:ssid
    else if (command.startsWith("WIFI_SSID:") || command.startsWith("wifi_ssid:")) {
        String ssid = command.substring(10);
        ssid.trim();
        
        if (ssid.length() == 0) {
            Serial.println("ERROR: Empty SSID");
            return;
        }
        
        Serial.print("WiFi SSID set to: ");
        Serial.println(ssid);
        Serial.println("Note: Use WIFI_PASSWORD command next, then both will be saved");
    }
    // WiFi Password command: WIFI_PASSWORD:password
    else if (command.startsWith("WIFI_PASSWORD:") || command.startsWith("wifi_password:")) {
        String password = command.substring(14);
        password.trim();
        
        Serial.println("WiFi password received");
        Serial.println("Note: Use WIFI_SSID command first to set credentials");
    }
    // GPIO command: GPIO:pin,state
    else if (command.startsWith("GPIO:") || command.startsWith("gpio:")) {
        String gpioCmd = command.substring(5);
        gpioCmd.trim();
        
        if (gpioCmd.length() == 0) {
            Serial.println("ERROR: Empty GPIO command");
            Serial.println("Format: GPIO:pin,state (e.g., GPIO:5,1)");
            return;
        }
        
        Serial.print("GPIO control: ");
        Serial.println(gpioCmd);
        
        if (gpioCallback != nullptr) {
            gpioCallback(gpioCmd);
        } else {
            Serial.println("ERROR: GPIO callback not set");
        }
    }
    // Unknown command
    else {
        Serial.print("ERROR: Unknown command: ");
        Serial.println(command);
        Serial.println("Type HELP for command list");
    }
}

void SerialCommand::printHelp() {
    Serial.println("\n=== LoRABLE Serial Commands ===");
    Serial.println("Commands (case-insensitive):");
    Serial.println("  HELP or ?                    - Show this help");
    Serial.println("  STATUS                       - Show current status");
    Serial.println("  SYNCWORD:0xXX                - Set LoRA syncword (hex)");
    Serial.println("  MESSAGE:text                 - Send message via LoRA");
    Serial.println("  WIFI_SSID:your_ssid          - Set WiFi SSID");
    Serial.println("  WIFI_PASSWORD:your_password  - Set WiFi password");
    Serial.println("  GPIO:pin,state               - Control GPIO (e.g., GPIO:5,1)");
    Serial.println("\nExamples:");
    Serial.println("  SYNCWORD:0x12");
    Serial.println("  MESSAGE:Hello from serial!");
    Serial.println("  WIFI_SSID:MyNetwork");
    Serial.println("  WIFI_PASSWORD:MyPassword123");
    Serial.println("  GPIO:5,1     (GPIO 5 HIGH)");
    Serial.println("  GPIO:13,0    (GPIO 13 LOW)");
    Serial.println("\nNote: WiFi is only used for OTA updates on boot");
    Serial.println("==============================\n");
}

uint8_t SerialCommand::parseHexByte(const String& hexStr) {
    String str = hexStr;
    str.trim();
    
    // Remove 0x prefix if present
    if (str.startsWith("0x") || str.startsWith("0X")) {
        str = str.substring(2);
    }
    
    // Parse hex string
    long value = strtol(str.c_str(), NULL, 16);
    
    return (uint8_t)(value & 0xFF);
}

void SerialCommand::setSyncWordCallback(void (*callback)(uint8_t)) {
    syncWordCallback = callback;
}

void SerialCommand::setMessageCallback(void (*callback)(const String&)) {
    messageCallback = callback;
}

void SerialCommand::setWiFiCallback(void (*callback)(const String&, const String&)) {
    wifiCallback = callback;
}

void SerialCommand::setStatusCallback(void (*callback)()) {
    statusCallback = callback;
}

void SerialCommand::setGPIOCallback(void (*callback)(const String&)) {
    gpioCallback = callback;
}

void SerialCommand::setEnabled(bool _enabled) {
    enabled = _enabled;
    inputBuffer = "";  // Clear buffer when toggling
    
    if (enabled) {
        Serial.println("\n*** Serial Command Mode ENABLED ***");
        Serial.println("Type HELP for command list");
        Serial.println("Double-press PRG button to disable");
    } else {
        Serial.println("\n*** Serial Command Mode DISABLED ***");
        Serial.println("Double-press PRG button to enable");
    }
}

bool SerialCommand::isEnabled() {
    return enabled;
}
