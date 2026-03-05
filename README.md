# LoRABLE - LoRA + BLE Application

A modular LoRA and Bluetooth Low Energy application for the **Heltec WiFi LoRA 32 V3** board that allows dynamic configuration of LoRA syncWord via BLE and displays received messages on an OLED screen.

## Features

- 📡 **LoRA Communication**: Send and receive LoRA messages using SX1262 chip
- 📱 **BLE Configuration**: Set LoRA syncWord wirelessly via Bluetooth
- 🖥️ **OLED Display**: View messages and status on built-in 128x64 display
- 💾 **Persistent Storage**: Save settings that survive reboots
- 🔧 **Modular Architecture**: Clean, maintainable code structure
- 📊 **Signal Quality**: Display RSSI and SNR for received messages

## Hardware Requirements

- **Heltec WiFi LoRA 32 V3** board
  - MCU: ESP32-S3FN8
  - LoRA: SX1262 (863-928 MHz)
  - Display: 0.96" OLED (128x64)
- USB-C cable for programming
- Antenna for LoRA (required!)

## Software Requirements

- [PlatformIO](https://platformio.org/) or Arduino IDE
- Python 3.x (for PlatformIO)
- nRF Connect app (for BLE testing)

## Installation

### Using PlatformIO (Recommended)

1. Clone this repository:
   ```bash
   git clone <repository-url>
   cd LoRABLE
   ```

2. Build the project:
   ```bash
   pio run
   ```

3. Upload to board:
   ```bash
   pio run --target upload
   ```

4. Monitor serial output:
   ```bash
   pio device monitor
   ```

### Using Arduino IDE

1. Install required libraries via Library Manager:
   - LoRa by Sandeep Mistry
   - ESP8266 and ESP32 OLED driver for SSD1306 displays

2. Open `src/main.cpp` in Arduino IDE

3. Select board: **Heltec WiFi LoRA 32 V3**

4. Upload to board

## Project Structure

```
LoRABLE/
├── src/
│   └── main.cpp              # Main application (orchestration only)
├── lib/
│   ├── BLE/                  # BLE module
│   │   ├── BLEManager.h
│   │   ├── BLEManager.cpp
│   │   └── README.md
│   ├── LoRA/                 # LoRA module
│   │   ├── LoRAManager.h
│   │   ├── LoRAManager.cpp
│   │   └── README.md
│   ├── Display/              # Display module
│   │   ├── DisplayManager.h
│   │   ├── DisplayManager.cpp
│   │   └── README.md
│   └── Config/               # Configuration module
│       ├── ConfigManager.h
│       ├── ConfigManager.cpp
│       └── README.md
├── platformio.ini            # PlatformIO configuration
├── .github/
│   ├── copilot-instructions.md
│   └── workflows/
│       └── build-firmware.yml
└── README.md
```

## Usage

### 1. Initial Setup

On first boot, the device:
- Initializes with default syncWord (0x12)
- Starts BLE advertising as "LoRABLE"
- Displays status on OLED

### 2. Configure SyncWord via BLE

1. Open **nRF Connect** app on your smartphone
2. Scan for devices and connect to "**LoRABLE**"
3. Navigate to service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
4. Find syncWord characteristic: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
5. Write new syncWord (1 byte, 0x00-0xFF)
6. Device will update LoRA configuration and save to flash

### 3. Send/Receive LoRA Messages

Use another Heltec device or LoRA module with the same syncWord to send messages. The device will:
- Receive messages automatically
- Display message content
- Show RSSI and SNR values
- Update status screen

### 4. Monitor via Serial

Connect via serial monitor (115200 baud) to see:
- Initialization status
- Configuration changes
- Received messages
- Signal quality metrics

## Module Documentation

Each module has detailed documentation:

- [BLE Module](lib/BLE/README.md) - Bluetooth Low Energy operations
- [LoRA Module](lib/LoRA/README.md) - LoRA transceiver operations
- [Display Module](lib/Display/README.md) - OLED display management
- [Config Module](lib/Config/README.md) - Persistent storage

## Configuration

### Default LoRA Settings

```cpp
Frequency:         915 MHz (adjust for region)
Bandwidth:         125 kHz
Spreading Factor:  7
Coding Rate:       4/5
TX Power:          20 dBm
Default SyncWord:  0x12
```

### Regional Frequencies

Choose the appropriate frequency for your region in `lib/LoRA/LoRAManager.h`:

- **433 MHz**: Europe, Asia (ISM band)
- **868 MHz**: Europe
- **915 MHz**: North America, Australia, South America

**⚠️ Important**: Check local regulations before transmitting!

## Development

### Adding New Features

The modular architecture makes it easy to extend:

1. **Add new BLE characteristics**: Modify `BLEManager`
2. **Change LoRA parameters**: Extend `LoRAManager`
3. **Customize display**: Update `DisplayManager`
4. **Add new settings**: Extend `ConfigManager`

Main application in `main.cpp` only orchestrates modules - keep it minimal!

### Building Firmware

Use GitHub Actions to build firmware automatically:

```bash
git tag v1.0.0
git push origin v1.0.0
```

This creates a GitHub Release with downloadable firmware binaries.

## Troubleshooting

### LoRA Not Working
- Check antenna is connected
- Verify frequency matches your region
- Ensure both devices have same syncWord

### BLE Not Visible
- Check Bluetooth is enabled on phone
- Device name is "LoRABLE"
- Try restarting the board

### Display Not Working
- VEXT_CTRL pin controls power (active LOW)
- Check I2C connections (pins 17, 18)
- Reset display if corrupted

### No Serial Output
- Check baud rate is 115200
- Ensure USB cable supports data
- Try different USB port

## Contributing

This project follows a modular architecture. When contributing:

1. Keep modules independent and focused
2. Follow existing code style (PascalCase for classes, camelCase for methods)
3. Add documentation for new features
4. Test modules independently
5. Update READMEs

## License

[Your License Here]

## Credits

- Based on tutorial: [ESP32 LoRA with Arduino IDE](https://randomnerdtutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/)
- Hardware: Heltec Automation
- Libraries: LoRa (Sandeep Mistry), SSD1306 (ThingPulse)

## References

- [Heltec WiFi LoRA 32 V3 Documentation](https://heltec.org/project/wifi-lora-32-v3/)
- [LoRa Library](https://github.com/sandeepmistry/arduino-LoRa)
- [SSD1306 Library](https://github.com/ThingPulse/esp8266-oled-ssd1306)
- [ESP32 BLE Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/index.html)
