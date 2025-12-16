# Aladdin Lamp - Test Suite

This directory contains unit tests for the Aladdin Lamp project.

## Test Structure

```
test/
├── test_config/          # Configuration validation tests
│   └── test_config.cpp
├── test_flicker/         # Flicker algorithm unit tests
│   └── test_flicker.cpp
└── README.md             # This file
```

## Running Tests

### Quick Start

```bash
# Run all tests on native platform (desktop)
make test

# Run tests on embedded device (ESP32 must be connected)
make test-embedded

# Run tests on all platforms
make test-all
```

### Using PlatformIO Directly

```bash
# Native platform tests (runs on your computer)
pio test -e test_native

# Embedded tests (runs on ESP32)
pio test -e test_embedded
```

## Test Suites

### test_config

Validates configuration constants defined in `include/config.h`:

- **Pin Definitions**: Verifies GPIO pins are valid and unique
- **LED Configuration**: Checks LED count and strip configuration
- **Default Settings**: Validates HSV default values are in range
- **Flicker Parameters**: Ensures smoothing and variation values are reasonable
- **Button Settings**: Checks debounce delay is appropriate

**Example Output**:
```
test_config/test_config.cpp:22:test_pin_definitions	[PASSED]
test_config/test_config.cpp:39:test_pin_uniqueness	[PASSED]
test_config/test_config.cpp:51:test_led_configuration	[PASSED]
...
-----------------------
13 Tests 0 Failures 0 Ignored
OK
```

### test_flicker

Tests the candle flicker algorithm and LED calculations:

- **Smoothing Algorithm**: Tests exponential moving average convergence and stability
- **LED Count Calculation**: Validates brightness-to-LED-count mapping
- **Fractional Brightness**: Tests fractional LED calculations
- **Utility Functions**: Tests constrain() and map() behavior

**Example Output**:
```
test_flicker/test_flicker.cpp:24:test_smoothing_convergence	[PASSED]
test_flicker/test_flicker.cpp:41:test_smoothing_stability	[PASSED]
test_flicker/test_flicker.cpp:51:test_smoothing_direction	[PASSED]
...
-----------------------
16 Tests 0 Failures 0 Ignored
OK
```

## Test Platforms

### Native Platform (test_native)

- **Runs on**: Your development computer (Linux, macOS, Windows)
- **Purpose**: Fast unit testing without hardware
- **Limitations**: Cannot test hardware-specific features (WiFi, HomeKit, LED output)
- **Best for**: Algorithm validation, configuration checks, logic tests

### Embedded Platform (test_embedded)

- **Runs on**: Actual ESP32 hardware
- **Purpose**: Hardware validation and integration testing
- **Requirements**: ESP32 must be connected via USB
- **Best for**: Pin configuration, peripheral testing, real-world validation

## Writing New Tests

### Test File Template

Create a new test file in `test/test_<name>/test_<name>.cpp`:

```cpp
#include <Arduino.h>
#include <unity.h>
#include "config.h"

void test_example(void)
{
    TEST_ASSERT_EQUAL(expected, actual);
}

void setUp(void)
{
    // Called before each test
}

void tearDown(void)
{
    // Called after each test
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_example);
    UNITY_END();
}

void loop()
{
    // Tests run once in setup()
}
```

### Unity Assertions

Common assertions available:

```cpp
// Equality
TEST_ASSERT_EQUAL(expected, actual);
TEST_ASSERT_EQUAL_FLOAT(expected, actual);

// Comparisons
TEST_ASSERT_GREATER_THAN(threshold, actual);
TEST_ASSERT_LESS_THAN(threshold, actual);
TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual);
TEST_ASSERT_LESS_OR_EQUAL(threshold, actual);

// Ranges
TEST_ASSERT_INT_WITHIN(delta, expected, actual);
TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual);

// Boolean
TEST_ASSERT_TRUE(condition);
TEST_ASSERT_FALSE(condition);

// Inequality
TEST_ASSERT_NOT_EQUAL(expected, actual);
```

### Adding Tests to Build

Update `platformio.ini` to include your new test:

```ini
[env:test_native]
test_filter = test_config, test_flicker, test_your_new_test
```

## Continuous Integration

Tests can be automated in CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Run Tests
  run: |
    pio test -e test_native
```

## Troubleshooting

### "Test port not found"

**Problem**: Cannot find embedded device for testing.

**Solution**:
- Ensure ESP32 is connected via USB
- Check device appears in: `pio device list`
- Specify port manually: `pio test -e test_embedded --upload-port /dev/ttyUSB0`

### "Test timeout"

**Problem**: Test doesn't complete on embedded device.

**Solution**:
- Increase timeout in platformio.ini:
  ```ini
  test_timeout = 120
  ```
- Check serial connection speed matches `test_speed`

### Tests fail on native but pass on embedded

**Problem**: Platform-specific behavior differences.

**Solution**:
- Use preprocessor directives for platform-specific code:
  ```cpp
  #ifdef ARDUINO
      // Embedded-only code
  #else
      // Native platform code
  #endif
  ```

## References

- [PlatformIO Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html)
- [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity)
- [Arduino Unit Testing Guide](https://docs.platformio.org/en/latest/tutorials/espressif32/arduino_debugging_unit_testing.html)
