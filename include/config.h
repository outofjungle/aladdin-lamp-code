/**
 * @file config.h
 * @brief Configuration constants for Aladdin Lamp
 *
 * This file contains all hardware pin assignments and configuration
 * parameters for the HomeKit candle light simulator.
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

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// HARDWARE PIN ASSIGNMENTS
// ============================================================================

// LED Strip 1 (APA102)
#define STRIP1_DATA_PIN 26
#define STRIP1_CLOCK_PIN 25

// LED Strip 2 (APA102)
#define STRIP2_DATA_PIN 19
#define STRIP2_CLOCK_PIN 18

// Control Pins
#define STATUS_LED_PIN 22     // WiFi/HomeKit status indicator
#define CONTROL_BUTTON_PIN 39 // Factory reset button (long press >3 sec)
#define POWER_BUTTON_PIN 0    // Manual power toggle button

// ============================================================================
// LED CONFIGURATION
// ============================================================================

#define LED_LENGTH 8 // Number of LEDs per strip
#define NUM_STRIPS 2 // Number of LED strips

// ============================================================================
// DEFAULT SETTINGS (Power-On State)
// ============================================================================

// Default candle color (orange flame)
#define DEFAULT_HUE 25         // HSV Hue: 0-360 degrees
#define DEFAULT_SATURATION 100 // HSV Saturation: 0-100%
#define DEFAULT_BRIGHTNESS 100 // Brightness: 0-100% (maps to LED count)

// ============================================================================
// FLICKER ALGORITHM PARAMETERS
// ============================================================================

/**
 * Flicker smoothing factor (0.0 - 1.0)
 *
 * Controls the transition curve between brightness changes.
 * Uses exponential moving average: smoothed = (alpha × previous) + ((1-alpha) × target)
 *
 * Recommended values:
 * - 0.50-0.65: Fast, erratic flicker (candle in wind)
 * - 0.70-0.80: Natural candle movement
 * - 0.75: Default - balanced natural effect
 * - 0.85-0.95: Calm, gentle transitions
 */
#define FLICKER_SMOOTHING 0.75

/**
 * Animation update interval (milliseconds)
 *
 * How often the flicker animation updates.
 * 60ms = ~17 FPS (smooth animation)
 */
#define UPDATE_INTERVAL 60

/**
 * Brightness variation range
 *
 * Random brightness variation for flicker effect.
 * Base 100% with ±40% variation = range of 60-140%
 * Clamped to 30-120% to prevent too dim/bright.
 */
#define FLICKER_VARIATION_MIN -40
#define FLICKER_VARIATION_MAX 20
#define FLICKER_BRIGHTNESS_MIN 30
#define FLICKER_BRIGHTNESS_MAX 120

/**
 * Hue variation range (degrees)
 *
 * Random hue shift toward yellow/orange for warmth.
 * Creates subtle color temperature changes in the flame.
 */
#define FLICKER_HUE_MIN -8
#define FLICKER_HUE_MAX 15

// ============================================================================
// BUTTON DEBOUNCING
// ============================================================================

/**
 * Button debounce delay (milliseconds)
 *
 * How long button state must remain stable before accepting input.
 * 50ms handles most mechanical switch bounce.
 */
#define DEBOUNCE_DELAY 50

/**
 * Long press duration for WiFi AP mode (milliseconds)
 *
 * How long POWER_BUTTON_PIN must be held to trigger WiFi AP mode.
 * 3000ms (3 seconds) prevents accidental activation.
 */
#define LONG_PRESS_DURATION 3000

/**
 * WiFi AP timeout (seconds)
 *
 * How long the WiFi setup AP remains active after long press.
 * 300 seconds = 5 minutes.
 */
#define WIFI_AP_TIMEOUT 300

// ============================================================================
// HOMEKIT CONFIGURATION
// ============================================================================

// WiFi Access Point for setup (no password)
#define WIFI_AP_SSID "Aladdin-Setup"

// HomeKit Device Information
#define HOMEKIT_NAME "Aladdin Lamp"
#define HOMEKIT_MANUFACTURER "Custom"
#define HOMEKIT_MODEL "Dual-Strip Candle v1.1"
#define HOMEKIT_SERIAL "ALC-001"

// HomeKit Setup Code
#define HOMEKIT_SETUP_CODE "79200981"

#endif // CONFIG_H
