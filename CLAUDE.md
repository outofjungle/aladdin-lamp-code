# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a HomeKit-controlled ESP32 candle light simulator with two synchronized APA102 LED strips. The device appears in Apple Home as "Aladdin Lamp" and features realistic candle flicker animation.

## Hardware Configuration

- **Board**: ESP32 PICO32
- **LED Strips**: 2x APA102 strips (8 LEDs each)
- **Strip 1 Pins**:
  - Clock: GPIO 25
  - Data: GPIO 26
- **Strip 2 Pins**:
  - Clock: GPIO 18
  - Data: GPIO 19
- **Control Pins**:
  - Status LED: GPIO 22 (WiFi/HomeKit status indicator)
  - Power Button: GPIO 0 (manual on/off toggle)
  - Factory Reset Button: GPIO 39 (long press for factory reset)
- **Serial Monitor**: 115200 baud
- **WiFi**: 2.4GHz required for HomeKit

## Build System

This project uses PlatformIO (not Arduino IDE).

### Common Commands

**Using Makefile (Recommended)**:

```bash
# Show all available commands
make help

# Build the project
make build

# Upload to device
make upload

# Build, upload, and monitor in one step
make flash-monitor

# Run tests
make test

# Clean build files
make clean
```

**Using PlatformIO CLI**:

```bash
# Build the project
pio run

# Upload to device
pio run --target upload

# Open serial monitor (to see HomeKit pairing code)
pio device monitor

# Build, upload, and monitor
pio run --target upload && pio device monitor

# Clean build files
pio run --target clean
```

### Environments

- **pico32**: Main firmware build (ESP32 PICO32 board)
  - Partition Scheme: `huge_app.csv` (required for HomeSpan - uses ~1.5MB of 3MB available)
- **test_native**: Unit tests running on native platform (your computer)
  - Fast feedback for algorithm and logic testing
- **test_embedded**: Unit tests running on actual ESP32 hardware
  - Hardware validation and integration testing

### Testing

The project includes comprehensive unit tests:

```bash
# Run all tests on native platform (fast)
make test

# Run tests on embedded device (ESP32 must be connected)
make test-embedded

# Run tests on all platforms
make test-all
```

**Test Suites**:
- `test_config`: Validates configuration constants, pin assignments, and parameter ranges
- `test_flicker`: Tests exponential smoothing algorithm, LED count calculations, and utility functions

See [test/README.md](test/README.md) for detailed testing documentation.

## Dependencies

- **FastLED**: v3.10.3 - LED control library for APA102 strips
- **HomeSpan**: v2.1.0+ - HomeKit accessory framework
- **Arduino-ESP32**: v3.1.0 (required for HomeSpan compatibility)

## Code Architecture

### HomeKit Integration

The project uses HomeSpan to create a HomeKit LightBulb accessory with full HSV color control:

1. **HomeSpan Service**: `DEV_CandleLight` struct extends `Service::LightBulb`
2. **Characteristics**:
   - **On/Off**: Power control
   - **Hue**: 0-360° color selection
   - **Saturation**: 0-100% color intensity
   - **Brightness**: 0-100% (mapped to LED count, not intensity)

### Brightness Control via LED Count

Unique feature: Brightness controls the number of active LEDs, not their intensity:

- **0%**: All LEDs off
- **37.5%**: 3 LEDs on + 1 LED at 50% brightness (3.5 effective LEDs)
- **50%**: 4 LEDs on (per strip)
- **100%**: All 8 LEDs on (per strip)
- **Calculation**: `numLEDsFloat = brightness * 8 / 100`
  - Full LEDs: `floor(numLEDsFloat)`
  - Fractional LED brightness: `(numLEDsFloat - floor(numLEDsFloat)) * 100%`

**LED Selection Pattern**: Sequential from index 0
- LEDs light starting from index 0 upward
- Example: At 50% brightness, LEDs 0-3 are active
- Fractional brightness applied to the last LED for smooth transitions

### Candle Flicker Algorithm

Located in `DEV_CandleLight::loop()` method:

- **Update Rate**: 60ms (~17 FPS) for smooth animation
- **Dramatic Intensity**: ±40% brightness variation (range 30-120%)
- **Hue Variation**: ±8-15° toward yellow/orange for warmth
- **Smoothing**: Exponential smoothing with configurable transition curve
  - **Parameter**: `FLICKER_SMOOTHING` (0.0-1.0) in include/config.h
  - **Default**: 0.75 (natural candle effect)
  - **Lower values** (0.5-0.7): Faster, more erratic flicker
  - **Higher values** (0.85-0.95): Slower, gentler transitions
  - **Formula**: `smoothed = (alpha × previous) + ((1-alpha) × target)`
- **Synchronization**: Both strips display identical flickering
- **Per-LED Randomness**: Each active LED gets independent flicker values
- **Fractional LED**: Fractional brightness LED also flickers with scaled intensity

### Dual Strip Control (DRY Architecture)

- **LED Arrays**: `CRGB leds[2][8]` - Two strips, 8 LEDs each
- **Synchronized Updates**: Single flicker calculation applied to both strips
- **Single FastLED.show()**: Updates all strips simultaneously
- **Pin Configuration**: Defined as constants at top of main.cpp

### Default Settings

- **Default Color**: Orange flame (Hue: 25°, Saturation: 100%)
- **Default Brightness**: 100% (all 8 LEDs active)
- **Power-On State**: Always ON at 100% brightness with default color after power cycle
- **Persistence**: Settings persist through reboots via HomeKit, but power cycle resets to defaults

### Manual Control

- **Power Button (GPIO 0)**: Multi-function button with short and long press detection
  - **Short Press** (< 3 seconds): Toggle lamp ON/OFF
    - Works independently of HomeKit control
    - State syncs with HomeKit automatically
    - Serial monitor shows "Power button pressed - Lamp ON/OFF"
  - **Long Press** (≥ 3 seconds): Enable WiFi AP mode for 5 minutes
    - Activates "Aladdin-Setup" WiFi access point
    - Allows WiFi reconfiguration without factory reset
    - AP automatically disables after 5-minute timeout
    - Serial monitor shows long press confirmation and AP status
  - **Robust software debouncing**: Requires 50ms stable state before accepting input
  - **Long press detection**: Monitors button hold duration, triggers at 3 seconds
  - **Smart triggering**: Prevents power toggle if long press was detected
  - Prevents false triggers from mechanical bounce and electrical noise

## HomeKit Setup Process

### First-Time Setup

1. **Upload firmware**: `pio run --target upload`
2. **Open serial monitor**: `pio device monitor`
3. **WiFi Configuration** (Automatic Captive Portal):
   - On first boot, ESP32 creates "Aladdin-Setup" access point
   - Connect to AP and configure WiFi credentials via captive portal web interface
   - Device auto-connects afterward and stores credentials
4. **Status LED Indication** (GPIO 22):
   - **Blinking**: Not connected to WiFi or in HomeKit pairing mode
   - **Solid**: Connected to WiFi and paired with HomeKit
5. **HomeKit Pairing**:
   - Setup Code: **792-00-981** (configured in include/config.h)
   - Open Apple Home app → "+" → "Add Accessory"
   - Scan QR code (in README.md) or manually enter Setup Code: **792-00-981**
   - Device appears as "Aladdin Lamp"

### WiFi Reconfiguration (Without Factory Reset)

To change WiFi credentials without losing HomeKit pairing:
1. **Long press** the power button (GPIO 0) for **3 seconds**
2. "Aladdin-Setup" WiFi AP will activate for 5 minutes
3. Connect to the AP and reconfigure WiFi via captive portal
4. HomeKit pairing remains intact
5. AP automatically disables after 5-minute timeout

### Factory Reset

To reset WiFi and HomeKit pairing:
- **Long press** the control button (GPIO 39) for **>3 seconds**
- Status LED will indicate reset in progress
- Device will restart and create "HomeSpan-Setup" AP again for reconfiguration

### HomeSpan Commands (via Serial)

While connected to serial monitor, HomeSpan provides CLI commands:
- `W <ssid> <password>`: Configure WiFi
- `S`: Show status
- `A`: Start HomeKit pairing mode
- `U`: Unpair from HomeKit
- Full command list: [HomeSpan CLI Reference](https://github.com/HomeSpan/HomeSpan/blob/master/docs/CLI.md)

## Development Notes

### File Structure

```
aladdin-lamp-code/
├── include/
│   ├── config.h              # All configuration constants
│   └── CandleLight.h         # DEV_CandleLight class declaration
├── src/
│   ├── main.cpp              # Application entry point (setup/loop)
│   └── CandleLight.cpp       # DEV_CandleLight implementation
├── test/
│   ├── test_config/          # Configuration validation tests
│   │   └── test_config.cpp
│   ├── test_flicker/         # Flicker algorithm tests
│   │   └── test_flicker.cpp
│   └── README.md             # Testing documentation
├── Makefile                  # Build automation (make help)
├── platformio.ini            # Build & test configuration
├── LICENSE                   # MIT License
├── README.md                 # User documentation
└── CLAUDE.md                 # Developer documentation
```

**File Responsibilities**:
- **include/config.h**: Hardware pins, LED configuration, default settings, flicker parameters
- **include/CandleLight.h**: Class declaration with method signatures and documentation
- **src/CandleLight.cpp**: All DEV_CandleLight logic (constructor, HomeKit callbacks, flicker animation, button handling)
- **src/main.cpp**: Minimal entry point (global LED arrays, setup(), loop())
- **test/**: Unit tests with Unity framework for validation
- **Makefile**: Convenient targets for build, test, upload, monitor

### Memory Usage

- **Flash**: ~1.46 MB (46.5% of 3 MB partition)
- **RAM**: ~60 KB (18.5% of 320 KB)
- **Note**: HomeSpan adds ~50-80 KB RAM overhead for WiFi/HomeKit stack

### Important Implementation Details

1. **Non-Blocking Animation**: Flicker runs in `DEV_CandleLight::loop()` override, doesn't block `homeSpan.poll()`
2. **Color Mapping**: HomeKit HSV (0-360, 0-100, 0-100) mapped to FastLED CHSV (0-255, 0-255, 0-255)
3. **Sequential LED Pattern**: LEDs light from index 0 upward (see `DEV_CandleLight::applyFlicker()`)
4. **Fractional Brightness**: Last LED uses fractional brightness for smooth transitions
5. **Exponential Smoothing**: Each LED maintains previous brightness state for smooth transitions (see `calculateSmoothedBrightness()`)
6. **Both Strips Synchronized**: Same color/flicker applied to `leds[0][i]` and `leds[1][i]`
7. **Power Button**: GPIO 0 with dual-function (short/long press detection)
   - Short press (< 3 sec): Toggle power with stable-state debouncing
   - Long press (≥ 3 sec): Trigger WiFi AP mode for 5 minutes
   - See `handlePowerButton()` in CandleLight.cpp for implementation
8. **Factory Reset Button**: GPIO 39 supports factory reset (long press >3 sec, handled by HomeSpan)
9. **Status LED**: GPIO 22 indicates WiFi/HomeKit connection status (managed by HomeSpan)
10. **Global LED Arrays**: `CRGB leds[NUM_STRIPS][LED_LENGTH]` defined in main.cpp, accessed via extern in CandleLight.cpp
11. **WiFi AP Timeout**: 5-minute timeout configured via `homeSpan.setApTimeout()` in main.cpp
12. **Custom Setup Code**: HomeKit pairing code **792-00-981** configured in include/config.h

### Adjusting Flicker Behavior

To customize the candle effect, edit `FLICKER_SMOOTHING` in include/config.h:

```cpp
#define FLICKER_SMOOTHING 0.75  // Change this value
```

**Examples**:
- **0.50**: Fast, jittery flicker (unstable candle in wind)
- **0.70**: Moderate smooth flicker
- **0.75**: Natural candle (default - recommended)
- **0.90**: Very smooth, gentle transitions (calm candle)
- **0.95**: Almost static with slow drift

### Troubleshooting

- **Build fails with "program too large"**: Ensure `board_build.partitions = huge_app.csv` in platformio.ini
- **HomeKit won't pair**:
  - Check WiFi is 2.4GHz
  - Verify Setup Code from serial output
  - Try `A` command in serial monitor to restart pairing
  - Check status LED - should be blinking during pairing
- **Can't connect to WiFi**:
  - Long press power button (GPIO 0) for 3 seconds to enable WiFi AP mode
  - Alternatively, factory reset via control button (long press >3 sec on GPIO 39)
  - Look for "Aladdin-Setup" AP and reconfigure via captive portal
  - WiFi AP mode auto-disables after 5 minutes
  - Check status LED for connection status
- **Strips don't match**: Verify both are same orientation; if mirrored, may need to reverse one strip's indices
- **No flicker visible**: Check that power is ON in Home app and brightness > 0%
- **Lamp doesn't turn on after power cycle**: Should auto-start at 75% brightness with orange color - check serial output for errors
- **Status LED not working**: Verify GPIO 22 is not conflicting with other peripherals
- **Power button not responding**:
  - Check button is connected between GPIO 0 and GND (active LOW)
  - Verify button wiring - should be normally open, closes to GND when pressed
  - **Short press** (< 3 sec) should toggle lamp ON/OFF - check serial for "Power button pressed" messages
  - **Long press** (≥ 3 sec) should enable WiFi AP - check serial for long press confirmation
  - GPIO 0 has internal pullup enabled - external pullup not needed
  - Debouncing requires 50ms stable state - ensure clean button contacts
