#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

// PRG button on Heltec V3 (GPIO 0)
#define PRG_BUTTON_PIN 0

// Button timing constants
#define DEBOUNCE_DELAY 50      // ms
#define LONG_PRESS_TIME 3000   // ms (3 seconds)
#define DOUBLE_PRESS_GAP 400   // ms

/**
 * ButtonManager - Manages button inputs for PRG button
 * 
 * Handles:
 * - Long press (3s): Toggle sleep mode
 * - Double press: Toggle between BLE and BLE+Serial modes
 */
class ButtonManager {
public:
    ButtonManager();
    
    /**
     * Initialize button with pull-up
     */
    void init();
    
    /**
     * Call this in loop() to process button events
     * Must be called frequently for accurate timing
     */
    void update();
    
    /**
     * Set callback for long press event (3s)
     * @param callback Function to call on long press
     */
    void setLongPressCallback(void (*callback)());
    
    /**
     * Set callback for double press event
     * @param callback Function to call on double press
     */
    void setDoublePressCallback(void (*callback)());
    
    /**
     * Check if button is currently pressed
     * @return true if pressed, false otherwise
     */
    bool isPressed();
    
private:
    // Button state
    int buttonState;
    int lastButtonState;
    
    // Timing
    unsigned long lastDebounceTime;
    unsigned long buttonPressTime;
    unsigned long lastReleaseTime;
    
    // Flags
    bool longPressTriggered;
    bool waitingForDoublePress;
    int pressCount;
    
    // Callbacks
    void (*longPressCallback)();
    void (*doublePressCallback)();
    
    // Helper methods
    void handlePress();
    void handleRelease();
};

#endif // BUTTON_MANAGER_H
