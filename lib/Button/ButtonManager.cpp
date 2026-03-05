#include "ButtonManager.h"

ButtonManager::ButtonManager()
    : buttonState(HIGH),
      lastButtonState(HIGH),
      lastDebounceTime(0),
      buttonPressTime(0),
      lastReleaseTime(0),
      longPressTriggered(false),
      waitingForDoublePress(false),
      pressCount(0),
      longPressCallback(nullptr),
      doublePressCallback(nullptr) {
}

void ButtonManager::init() {
    pinMode(PRG_BUTTON_PIN, INPUT_PULLUP);
    buttonState = digitalRead(PRG_BUTTON_PIN);
    lastButtonState = buttonState;
    
    Serial.println("ButtonManager initialized");
    Serial.print("PRG Button pin: ");
    Serial.println(PRG_BUTTON_PIN);
}

void ButtonManager::update() {
    int reading = digitalRead(PRG_BUTTON_PIN);
    
    // Check if button state changed (with debounce)
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // If state has changed after debounce
        if (reading != buttonState) {
            buttonState = reading;
            
            // Button pressed (LOW because of pull-up)
            if (buttonState == LOW) {
                handlePress();
            }
            // Button released
            else {
                handleRelease();
            }
        }
    }
    
    // Check for long press while button is held
    if (buttonState == LOW && !longPressTriggered) {
        if (millis() - buttonPressTime >= LONG_PRESS_TIME) {
            longPressTriggered = true;
            Serial.println("Long press detected (3s)!");
            
            if (longPressCallback != nullptr) {
                longPressCallback();
            }
        }
    }
    
    // Check if double press window has expired
    if (waitingForDoublePress && (millis() - lastReleaseTime > DOUBLE_PRESS_GAP)) {
        waitingForDoublePress = false;
        pressCount = 0;
    }
    
    lastButtonState = reading;
}

void ButtonManager::handlePress() {
    buttonPressTime = millis();
    longPressTriggered = false;
    
    Serial.println("Button pressed");
}

void ButtonManager::handleRelease() {
    unsigned long pressDuration = millis() - buttonPressTime;
    lastReleaseTime = millis();
    
    Serial.print("Button released after ");
    Serial.print(pressDuration);
    Serial.println("ms");
    
    // Only count as press if not a long press and was quick enough
    if (!longPressTriggered && pressDuration < LONG_PRESS_TIME) {
        pressCount++;
        
        if (pressCount == 1) {
            // First press - wait for potential second press
            waitingForDoublePress = true;
        } else if (pressCount == 2) {
            // Double press detected!
            Serial.println("Double press detected!");
            waitingForDoublePress = false;
            pressCount = 0;
            
            if (doublePressCallback != nullptr) {
                doublePressCallback();
            }
        }
    }
}

void ButtonManager::setLongPressCallback(void (*callback)()) {
    longPressCallback = callback;
}

void ButtonManager::setDoublePressCallback(void (*callback)()) {
    doublePressCallback = callback;
}

bool ButtonManager::isPressed() {
    return buttonState == LOW;
}
