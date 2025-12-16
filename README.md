# Aladdin Lamp - HomeKit Candle Light

A realistic candle light simulator for ESP32 with HomeKit integration, featuring two synchronized APA102 LED strips with dramatic flicker effects.

![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Framework](https://img.shields.io/badge/framework-Arduino-00979D)
![HomeKit](https://img.shields.io/badge/HomeKit-Compatible-000000)

## Features

🕯️ **Realistic Candle Flicker**
- Dramatic ±40% brightness variation
- Exponential smoothing for natural transitions
- Per-LED randomness for organic effect
- Adjustable smoothing parameter (0.0-1.0)

🏠 **HomeKit Integration**
- Full Siri control ("Hey Siri, turn on Aladdin Lamp")
- Color selection (HSV)
- Brightness control via LED count (0-8 LEDs per strip)
- Syncs across all Apple devices

💡 **Dual LED Strips**
- Two synchronized APA102 strips (8 LEDs each)
- Fractional brightness for smooth transitions
- Sequential lighting from index 0

🎛️ **Manual Control**
- Physical power button (GPIO 0)
- Factory reset button (GPIO 39)
- Status LED indicator (GPIO 22)

## Hardware Requirements

- **ESP32 PICO32** board
- **2x APA102 LED strips** (8 LEDs each)
- **Momentary push buttons** (2x)
- **Status LED** (any color)
- **220Ω-1kΩ resistor** for status LED

### Wiring

| Component | GPIO Pin |
|-----------|----------|
| Strip 1 Data | 26 |
| Strip 1 Clock | 25 |
| Strip 2 Data | 19 |
| Strip 2 Clock | 18 |
| Power Button | 0 (to GND) |
| Factory Reset | 39 (to GND) |
| Status LED | 22 (+ resistor to GND) |

## Quick Start

### 1. Install PlatformIO

```bash
# Using PlatformIO CLI
pip install platformio

# Or use VS Code with PlatformIO IDE extension
```

### 2. Build and Upload

**Using Makefile (Recommended)**:

```bash
# Build firmware
make build

# Upload to ESP32
make upload

# Build, upload, and monitor in one command
make flash-monitor

# Run tests
make test
```

**Using PlatformIO CLI**:

```bash
# Build firmware
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

See `make help` for all available commands.

### 3. WiFi Setup

1. Power on the device
2. Connect to **"Aladdin-Setup"** WiFi network (no password)
3. Configure your home WiFi via captive portal
4. Device will auto-connect and store credentials

### 4. HomeKit Pairing

1. Open **Apple Home** app on iPhone/iPad
2. Tap **"+"** → **"Add Accessory"**
3. Enter the **Setup Code** shown in serial monitor
4. Device appears as **"Aladdin Lamp"**

## Usage

### HomeKit Control

- **"Hey Siri, turn on Aladdin Lamp"**
- **"Hey Siri, set Aladdin Lamp to 50%"** (4 LEDs per strip)
- **"Hey Siri, set Aladdin Lamp to orange"**
- Use Home app for precise color/brightness control

### Physical Buttons

- **GPIO 0**: Press to toggle ON/OFF
- **GPIO 39**: Long press (>3 sec) for factory reset

### Status LED

- **Blinking**: WiFi not connected or pairing mode
- **Solid**: Connected and paired

## Customization

### Adjust Flicker Effect

Edit `include/config.h`:

```cpp
#define FLICKER_SMOOTHING 0.75  // 0.0 = hard, 1.0 = smooth
```

**Recommended values**:
- `0.50-0.65`: Erratic, windblown candle
- `0.70-0.80`: Natural indoor candle
- `0.85-0.95`: Calm, meditative glow

### Change Default Settings

Edit `include/config.h`:

```cpp
#define DEFAULT_HUE 25           // Orange flame
#define DEFAULT_SATURATION 100   // Full color
#define DEFAULT_BRIGHTNESS 100   // All LEDs on
```

### Serial Commands

While connected via serial monitor, use HomeSpan CLI:
- `W <ssid> <password>` - Configure WiFi
- `S` - Show status
- `A` - Start pairing mode
- `U` - Unpair from HomeKit
- `H` - Help (full command list)

## Project Structure

```
aladdin-lamp-code/
├── include/
│   ├── config.h              # Configuration constants
│   └── CandleLight.h         # DEV_CandleLight class declaration
├── src/
│   ├── main.cpp              # Application entry point
│   └── CandleLight.cpp       # DEV_CandleLight implementation
├── test/
│   ├── test_config/          # Configuration validation tests
│   ├── test_flicker/         # Flicker algorithm tests
│   └── README.md             # Testing documentation
├── Makefile                  # Build automation
├── platformio.ini            # Build configuration
├── LICENSE                   # MIT License
├── README.md                 # This file
└── CLAUDE.md                 # Developer documentation
```

## Technical Details

### Memory Usage

- **Flash**: ~1.47 MB (46.6% of 3 MB partition)
- **RAM**: ~60 KB (18.5% of 320 KB)

### Dependencies

- [FastLED](https://github.com/FastLED/FastLED) v3.10.3 - LED control
- [HomeSpan](https://github.com/HomeSpan/HomeSpan) v2.1.6+ - HomeKit framework

### Algorithms

**Brightness Control**:
- Brightness % → LED count: `numLEDs = brightness × 8 / 100`
- Fractional LED brightness for smooth transitions

**Flicker Smoothing**:
- Exponential moving average: `smoothed = (α × previous) + ((1-α) × target)`
- Where `α = FLICKER_SMOOTHING`

**Button Debouncing**:
- Stable-state detection with 50ms requirement
- Prevents false triggers from mechanical bounce

## Troubleshooting

**Build fails with "program too large"**
- Ensure `board_build.partitions = huge_app.csv` in `platformio.ini`

**WiFi won't connect**
- Check 2.4GHz WiFi (5GHz not supported)
- Factory reset via button on GPIO 39
- Look for "Aladdin-Setup" AP

**HomeKit pairing fails**
- Verify Setup Code from serial output
- Try `A` command in serial monitor
- Check status LED is blinking

**LEDs don't light**
- Verify APA102 wiring (data + clock pins)
- Check power supply to LED strips
- Ensure strips are BGR color order

**Power button doesn't work**
- Check GPIO 0 to GND connection
- Verify button is normally-open
- Check serial for "Power button pressed" messages

## Testing

The project includes comprehensive unit tests for configuration validation and flicker algorithms.

### Run Tests

```bash
# Run all tests (native platform - fast)
make test

# Run tests on embedded device (requires ESP32 connected)
make test-embedded

# Run tests on all platforms
make test-all
```

### Test Suites

- **test_config**: Validates configuration constants and pin assignments
- **test_flicker**: Tests smoothing algorithm and LED calculations

See [test/README.md](test/README.md) for detailed testing documentation.

## Development

For detailed development information, see [CLAUDE.md](CLAUDE.md).

### Building from Source

```bash
# Clean build
pio run --target clean

# Build with verbose output
pio run -v

# Upload and monitor
pio run --target upload && pio device monitor
```

## Credits

Built with:
- [HomeSpan](https://github.com/HomeSpan/HomeSpan) by HomeSpan
- [FastLED](https://github.com/FastLED/FastLED) by FastLED
- [PlatformIO](https://platformio.org/)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Copyright (c) 2025 @outofjungle

## Support

For issues or questions:
1. Check [CLAUDE.md](CLAUDE.md) for detailed documentation
2. Review HomeSpan documentation
3. Check FastLED documentation for LED control issues
