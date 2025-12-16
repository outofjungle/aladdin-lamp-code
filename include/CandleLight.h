/**
 * @file CandleLight.h
 * @brief HomeKit candle light service with dual synchronized LED strips
 *
 * This file implements a HomeKit LightBulb service that controls two
 * APA102 LED strips with realistic candle flicker animation.
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

#ifndef CANDLELIGHT_H
#define CANDLELIGHT_H

#include <Arduino.h>
#include "HomeSpan.h"
#include <FastLED.h>
#include "config.h"

/**
 * @class DEV_CandleLight
 * @brief HomeKit LightBulb service with candle flicker effect
 *
 * Features:
 * - Dual synchronized APA102 LED strips
 * - Brightness control via LED count (0-100% → 0-8 LEDs)
 * - Fractional brightness on last LED for smooth transitions
 * - Exponential smoothing for natural flicker
 * - Manual power button with debouncing
 * - HomeKit HSV color control
 */
struct DEV_CandleLight : Service::LightBulb
{
    // ========================================================================
    // HOMEKIT CHARACTERISTICS
    // ========================================================================

    SpanCharacteristic *power;        // On/Off state
    SpanCharacteristic *hue;          // Color hue (0-360°)
    SpanCharacteristic *saturation;   // Color saturation (0-100%)
    SpanCharacteristic *brightness;   // Brightness (0-100%)

    // ========================================================================
    // BUTTON STATE TRACKING
    // ========================================================================

    uint32_t buttonDebounceTimer;     // Debounce timer (milliseconds)
    bool buttonLastStableState;       // Last confirmed stable button state
    bool buttonCurrentReading;        // Current GPIO reading
    bool buttonPreviousReading;       // Previous GPIO reading

    // ========================================================================
    // FLICKER STATE
    // ========================================================================

    /**
     * Previous brightness values for exponential smoothing
     * Indexed as: previousBrightness[strip][led]
     */
    float previousBrightness[NUM_STRIPS][LED_LENGTH];

    // ========================================================================
    // LED ARRAYS
    // ========================================================================

    /**
     * LED color arrays for both strips
     * Global arrays defined in main.cpp: CRGB leds[NUM_STRIPS][LED_LENGTH]
     */

    // ========================================================================
    // CONSTRUCTOR
    // ========================================================================

    /**
     * Initialize HomeKit service and hardware
     *
     * Sets up:
     * - HomeKit characteristics with default values
     * - FastLED for both APA102 strips
     * - Power button with internal pullup
     * - Smoothing state arrays
     */
    DEV_CandleLight();

    // ========================================================================
    // HOMEKIT CALLBACKS
    // ========================================================================

    /**
     * Called when HomeKit characteristics are updated
     *
     * Logs changes to serial monitor.
     * @return true to indicate successful update
     */
    boolean update() override;

    /**
     * Main animation loop
     *
     * Called continuously by HomeSpan.
     * Handles:
     * - Button polling and debouncing
     * - Flicker animation updates
     * - LED brightness smoothing
     * - FastLED output
     */
    void loop() override;

    // ========================================================================
    // PRIVATE METHODS
    // ========================================================================

private:
    /**
     * Handle power button press with debouncing
     *
     * Uses stable-state detection algorithm:
     * 1. Read current button state
     * 2. If changed from previous, reset timer
     * 3. If stable for DEBOUNCE_DELAY ms, accept new state
     * 4. Only trigger on button press (HIGH→LOW transition)
     */
    void handlePowerButton();

    /**
     * Apply candle flicker effect to active LEDs
     *
     * @param fullLEDs Number of fully-lit LEDs
     * @param fraction Fractional brightness for last LED (0.0-1.0)
     * @param baseHue Base color hue from HomeKit
     * @param baseSat Base color saturation from HomeKit
     */
    void applyFlicker(int fullLEDs, float fraction, int baseHue, int baseSat);

    /**
     * Calculate smoothed brightness using exponential moving average
     *
     * Formula: smoothed = (alpha × previous) + ((1-alpha) × target)
     * Where alpha = FLICKER_SMOOTHING
     *
     * @param target Target brightness value
     * @param previous Previous smoothed value
     * @return Smoothed brightness value
     */
    float calculateSmoothedBrightness(float target, float previous);
};

#endif // CANDLELIGHT_H
