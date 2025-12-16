/**
 * @file test_flicker.cpp
 * @brief Flicker algorithm unit tests
 *
 * Tests for brightness smoothing and LED count calculations.
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
    #include <math.h>

    // Mock Arduino functions for native platform
    void delay(unsigned long ms) {}

    // Arduino utility functions
    inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    template<typename T>
    inline T constrain(T x, T a, T b) {
        if (x < a) return a;
        if (x > b) return b;
        return x;
    }
#else
    // Embedded platform - use real Arduino
    #include <Arduino.h>
    #include <unity.h>
    #include "config.h"
#endif

// ============================================================================
// SMOOTHING ALGORITHM TESTS
// ============================================================================

/**
 * Calculate smoothed brightness using exponential moving average
 * This is the same algorithm used in DEV_CandleLight::calculateSmoothedBrightness
 */
float calculateSmoothedBrightness(float target, float previous)
{
    return (FLICKER_SMOOTHING * previous) + ((1.0 - FLICKER_SMOOTHING) * target);
}

void test_smoothing_convergence(void)
{
    // Starting from 0, should converge toward target value
    float current = 0.0;
    float target = 100.0;

    // After many iterations, should approach target
    for (int i = 0; i < 100; i++)
    {
        current = calculateSmoothedBrightness(target, current);
    }

    // Should be very close to target after 100 iterations
    TEST_ASSERT_FLOAT_WITHIN(1.0, target, current);
}

void test_smoothing_stability(void)
{
    // If current equals target, should remain stable
    float current = 50.0;
    float target = 50.0;

    float result = calculateSmoothedBrightness(target, current);

    TEST_ASSERT_EQUAL_FLOAT(current, result);
}

void test_smoothing_direction(void)
{
    // Should always move toward target
    float current = 50.0;
    float target = 100.0;

    float result = calculateSmoothedBrightness(target, current);

    // Result should be between current and target
    TEST_ASSERT_GREATER_THAN(current, result);
    TEST_ASSERT_LESS_THAN(target, result);
}

void test_smoothing_bounds(void)
{
    // Even with extreme inputs, output should be bounded
    float current = -1000.0;
    float target = 1000.0;

    float result = calculateSmoothedBrightness(target, current);

    // Result should be between inputs
    TEST_ASSERT_GREATER_THAN(current, result);
    TEST_ASSERT_LESS_THAN(target, result);
}

// ============================================================================
// LED COUNT CALCULATION TESTS
// ============================================================================

void test_brightness_to_led_count_zero(void)
{
    // 0% brightness = 0 LEDs
    int brightnessPercent = 0;
    float numLEDsFloat = brightnessPercent * LED_LENGTH / 100.0;
    int fullLEDs = floor(numLEDsFloat);

    TEST_ASSERT_EQUAL(0, fullLEDs);
}

void test_brightness_to_led_count_full(void)
{
    // 100% brightness = all LEDs
    int brightnessPercent = 100;
    float numLEDsFloat = brightnessPercent * LED_LENGTH / 100.0;
    int fullLEDs = floor(numLEDsFloat);

    TEST_ASSERT_EQUAL(LED_LENGTH, fullLEDs);
}

void test_brightness_to_led_count_half(void)
{
    // 50% brightness = half LEDs
    int brightnessPercent = 50;
    float numLEDsFloat = brightnessPercent * LED_LENGTH / 100.0;
    int fullLEDs = floor(numLEDsFloat);

    TEST_ASSERT_EQUAL(LED_LENGTH / 2, fullLEDs);
}

void test_brightness_to_led_count_fractional(void)
{
    // 37.5% brightness = 3 full LEDs + 0.5 fractional
    int brightnessPercent = 37;
    float numLEDsFloat = brightnessPercent * LED_LENGTH / 100.0;
    int fullLEDs = floor(numLEDsFloat);
    float fraction = numLEDsFloat - fullLEDs;

    // With 8 LEDs: 37% = 2.96 LEDs = 2 full + 0.96 fractional
    TEST_ASSERT_EQUAL(2, fullLEDs);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 0.96, fraction);
}

void test_brightness_monotonic(void)
{
    // Higher brightness should always result in more LEDs (or equal)
    for (int brightness = 0; brightness < 100; brightness++)
    {
        float leds1 = brightness * LED_LENGTH / 100.0;
        float leds2 = (brightness + 1) * LED_LENGTH / 100.0;

        TEST_ASSERT_GREATER_OR_EQUAL(leds1, leds2);
    }
}

// ============================================================================
// CONSTRAIN FUNCTION TESTS
// ============================================================================

void test_constrain_within_bounds(void)
{
    // Value within bounds should be unchanged
    int value = 50;
    int result = constrain(value, 0, 100);
    TEST_ASSERT_EQUAL(50, result);
}

void test_constrain_below_min(void)
{
    // Value below min should be clamped to min
    int value = -10;
    int result = constrain(value, 0, 100);
    TEST_ASSERT_EQUAL(0, result);
}

void test_constrain_above_max(void)
{
    // Value above max should be clamped to max
    int value = 150;
    int result = constrain(value, 0, 100);
    TEST_ASSERT_EQUAL(100, result);
}

// ============================================================================
// MAP FUNCTION TESTS
// ============================================================================

void test_map_zero_to_zero(void)
{
    // 0 in 0-100 range should map to 0 in 0-255 range
    int result = map(0, 0, 100, 0, 255);
    TEST_ASSERT_EQUAL(0, result);
}

void test_map_max_to_max(void)
{
    // 100 in 0-100 range should map to 255 in 0-255 range
    int result = map(100, 0, 100, 0, 255);
    TEST_ASSERT_EQUAL(255, result);
}

void test_map_midpoint(void)
{
    // 50 in 0-100 range should map to ~127 in 0-255 range
    int result = map(50, 0, 100, 0, 255);
    TEST_ASSERT_INT_WITHIN(1, 127, result);
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

    // Smoothing tests
    RUN_TEST(test_smoothing_convergence);
    RUN_TEST(test_smoothing_stability);
    RUN_TEST(test_smoothing_direction);
    RUN_TEST(test_smoothing_bounds);

    // LED count calculation tests
    RUN_TEST(test_brightness_to_led_count_zero);
    RUN_TEST(test_brightness_to_led_count_full);
    RUN_TEST(test_brightness_to_led_count_half);
    RUN_TEST(test_brightness_to_led_count_fractional);
    RUN_TEST(test_brightness_monotonic);

    // Utility function tests
    RUN_TEST(test_constrain_within_bounds);
    RUN_TEST(test_constrain_below_min);
    RUN_TEST(test_constrain_above_max);
    RUN_TEST(test_map_zero_to_zero);
    RUN_TEST(test_map_max_to_max);
    RUN_TEST(test_map_midpoint);

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
