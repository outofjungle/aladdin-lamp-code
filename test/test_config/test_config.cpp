/**
 * @file test_config.cpp
 * @brief Configuration validation tests
 *
 * Tests to verify configuration constants are properly defined
 * and within valid ranges.
 *
 * @license MIT License
 *
 * Copyright (c) 2025 @outofjungle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef UNIT_TEST
    // Native platform - provide Arduino compatibility
    #include <unity.h>
    #include "config.h"

    // Mock Arduino functions for native platform
    void delay(unsigned long ms) {}
#else
    // Embedded platform - use real Arduino
    #include <Arduino.h>
    #include <unity.h>
    #include "config.h"
#endif

// ============================================================================
// PIN CONFIGURATION TESTS
// ============================================================================

void test_pin_definitions(void)
{
    // Verify all pins are defined within valid ESP32 range
    TEST_ASSERT_LESS_THAN(40, STRIP1_DATA_PIN);
    TEST_ASSERT_LESS_THAN(40, STRIP1_CLOCK_PIN);
    TEST_ASSERT_LESS_THAN(40, STRIP2_DATA_PIN);
    TEST_ASSERT_LESS_THAN(40, STRIP2_CLOCK_PIN);
    TEST_ASSERT_LESS_THAN(40, STATUS_LED_PIN);
    TEST_ASSERT_LESS_THAN(40, CONTROL_BUTTON_PIN);
    TEST_ASSERT_LESS_THAN(40, POWER_BUTTON_PIN);

    // Verify pins are non-negative
    TEST_ASSERT_GREATER_OR_EQUAL(0, STRIP1_DATA_PIN);
    TEST_ASSERT_GREATER_OR_EQUAL(0, STRIP1_CLOCK_PIN);
    TEST_ASSERT_GREATER_OR_EQUAL(0, STRIP2_DATA_PIN);
    TEST_ASSERT_GREATER_OR_EQUAL(0, STRIP2_CLOCK_PIN);
}

void test_pin_uniqueness(void)
{
    // Verify no pin conflicts (each pin should be unique)
    TEST_ASSERT_NOT_EQUAL(STRIP1_DATA_PIN, STRIP1_CLOCK_PIN);
    TEST_ASSERT_NOT_EQUAL(STRIP1_DATA_PIN, STRIP2_DATA_PIN);
    TEST_ASSERT_NOT_EQUAL(STRIP1_DATA_PIN, STRIP2_CLOCK_PIN);
    TEST_ASSERT_NOT_EQUAL(STRIP1_CLOCK_PIN, STRIP2_DATA_PIN);
    TEST_ASSERT_NOT_EQUAL(STRIP1_CLOCK_PIN, STRIP2_CLOCK_PIN);
    TEST_ASSERT_NOT_EQUAL(STRIP2_DATA_PIN, STRIP2_CLOCK_PIN);
}

// ============================================================================
// LED CONFIGURATION TESTS
// ============================================================================

void test_led_configuration(void)
{
    // Verify LED count is reasonable
    TEST_ASSERT_GREATER_THAN(0, LED_LENGTH);
    TEST_ASSERT_LESS_OR_EQUAL(256, LED_LENGTH);

    // Verify strip count
    TEST_ASSERT_EQUAL(2, NUM_STRIPS);
}

// ============================================================================
// DEFAULT SETTINGS TESTS
// ============================================================================

void test_default_hue(void)
{
    // Hue should be in valid HSV range (0-360)
    TEST_ASSERT_GREATER_OR_EQUAL(0, DEFAULT_HUE);
    TEST_ASSERT_LESS_OR_EQUAL(360, DEFAULT_HUE);
}

void test_default_saturation(void)
{
    // Saturation should be in valid range (0-100)
    TEST_ASSERT_GREATER_OR_EQUAL(0, DEFAULT_SATURATION);
    TEST_ASSERT_LESS_OR_EQUAL(100, DEFAULT_SATURATION);
}

void test_default_brightness(void)
{
    // Brightness should be in valid range (0-100)
    TEST_ASSERT_GREATER_OR_EQUAL(0, DEFAULT_BRIGHTNESS);
    TEST_ASSERT_LESS_OR_EQUAL(100, DEFAULT_BRIGHTNESS);
}

// ============================================================================
// FLICKER ALGORITHM TESTS
// ============================================================================

void test_flicker_smoothing(void)
{
    // Smoothing factor should be in valid range (0.0-1.0)
    TEST_ASSERT_GREATER_OR_EQUAL(0.0, FLICKER_SMOOTHING);
    TEST_ASSERT_LESS_OR_EQUAL(1.0, FLICKER_SMOOTHING);
}

void test_update_interval(void)
{
    // Update interval should be reasonable (not too fast, not too slow)
    TEST_ASSERT_GREATER_THAN(0, UPDATE_INTERVAL);
    TEST_ASSERT_LESS_OR_EQUAL(1000, UPDATE_INTERVAL);
}

void test_brightness_range(void)
{
    // Min should be less than max
    TEST_ASSERT_LESS_THAN(FLICKER_BRIGHTNESS_MAX, FLICKER_BRIGHTNESS_MIN);

    // Values should be reasonable percentages
    TEST_ASSERT_GREATER_OR_EQUAL(0, FLICKER_BRIGHTNESS_MIN);
    TEST_ASSERT_LESS_OR_EQUAL(200, FLICKER_BRIGHTNESS_MAX);
}

void test_hue_variation(void)
{
    // Min should be less than or equal to max
    TEST_ASSERT_LESS_OR_EQUAL(FLICKER_HUE_MAX, FLICKER_HUE_MIN);
}

// ============================================================================
// BUTTON CONFIGURATION TESTS
// ============================================================================

void test_debounce_delay(void)
{
    // Debounce delay should be reasonable (20-200ms typical)
    TEST_ASSERT_GREATER_OR_EQUAL(10, DEBOUNCE_DELAY);
    TEST_ASSERT_LESS_OR_EQUAL(500, DEBOUNCE_DELAY);
}

// ============================================================================
// TEST RUNNER
// ============================================================================

void setUp(void)
{
    // Called before each test
}

void tearDown(void)
{
    // Called after each test
}

void run_tests(void)
{
    UNITY_BEGIN();

    // Pin tests
    RUN_TEST(test_pin_definitions);
    RUN_TEST(test_pin_uniqueness);

    // LED tests
    RUN_TEST(test_led_configuration);

    // Default settings tests
    RUN_TEST(test_default_hue);
    RUN_TEST(test_default_saturation);
    RUN_TEST(test_default_brightness);

    // Flicker algorithm tests
    RUN_TEST(test_flicker_smoothing);
    RUN_TEST(test_update_interval);
    RUN_TEST(test_brightness_range);
    RUN_TEST(test_hue_variation);

    // Button tests
    RUN_TEST(test_debounce_delay);

    UNITY_END();
}

#ifdef UNIT_TEST
// Native platform - use main()
int main(int argc, char **argv)
{
    run_tests();
    return 0;
}
#else
// Embedded platform - use setup()/loop()
void setup()
{
    delay(2000); // Wait for serial monitor
    run_tests();
}

void loop()
{
    // Tests run once in setup()
}
#endif
