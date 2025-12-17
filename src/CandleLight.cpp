/**
 * @file CandleLight.cpp
 * @brief Implementation of DEV_CandleLight HomeKit service
 *
 * Contains all logic for candle flicker animation, button handling,
 * and HomeKit characteristic updates.
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

#include "CandleLight.h"

// External LED arrays defined in main.cpp
extern CRGB leds[NUM_STRIPS][LED_LENGTH];

// ============================================================================
// CONSTRUCTOR
// ============================================================================

DEV_CandleLight::DEV_CandleLight() : Service::LightBulb()
{
    // Initialize HomeKit characteristics with defaults
    power = new Characteristic::On(1); // Start ON after power cycle
    hue = new Characteristic::Hue(DEFAULT_HUE);
    saturation = new Characteristic::Saturation(DEFAULT_SATURATION);
    brightness = new Characteristic::Brightness(DEFAULT_BRIGHTNESS);

    // Initialize FastLED for both APA102 strips
    FastLED.addLeds<APA102, STRIP1_DATA_PIN, STRIP1_CLOCK_PIN, BGR>(leds[0], LED_LENGTH);
    FastLED.addLeds<APA102, STRIP2_DATA_PIN, STRIP2_CLOCK_PIN, BGR>(leds[1], LED_LENGTH);
    FastLED.setBrightness(255); // Use full brightness, control via color values

    // Turn off all LEDs initially
    fill_solid(leds[0], LED_LENGTH, CRGB::Black);
    fill_solid(leds[1], LED_LENGTH, CRGB::Black);
    FastLED.show();

    // Initialize power button with internal pullup (active LOW)
    pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
    buttonDebounceTimer = 0;
    buttonLastStableState = HIGH;
    buttonCurrentReading = HIGH;
    buttonPreviousReading = HIGH;
    buttonPressStartTime = 0;
    longPressTriggered = false;

    // Initialize smoothing arrays
    for (int strip = 0; strip < NUM_STRIPS; strip++)
    {
        for (int i = 0; i < LED_LENGTH; i++)
        {
            previousBrightness[strip][i] = 0.0;
        }
    }

    // Log configuration
    Serial.print("Configured Candle Light with ");
    Serial.print(NUM_STRIPS);
    Serial.println(" strips");
    Serial.print("Flicker smoothing: ");
    Serial.println(FLICKER_SMOOTHING);
}

// ============================================================================
// HOMEKIT CALLBACKS
// ============================================================================

boolean DEV_CandleLight::update()
{
    // Log HomeKit characteristic changes
    if (power->updated())
    {
        Serial.print("Power: ");
        Serial.println(power->getNewVal() ? "ON" : "OFF");
    }

    if (hue->updated())
    {
        Serial.print("Hue: ");
        Serial.println(hue->getNewVal());
    }

    if (saturation->updated())
    {
        Serial.print("Saturation: ");
        Serial.println(saturation->getNewVal());
    }

    if (brightness->updated())
    {
        Serial.print("Brightness: ");
        Serial.println(brightness->getNewVal());
    }

    return true;
}

void DEV_CandleLight::loop()
{
    // Handle manual power button
    handlePowerButton();

    // Rate-limit animation updates
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < UPDATE_INTERVAL)
    {
        return;
    }
    lastUpdate = millis();

    // Turn off all LEDs if power is off
    if (!power->getVal())
    {
        fill_solid(leds[0], LED_LENGTH, CRGB::Black);
        fill_solid(leds[1], LED_LENGTH, CRGB::Black);
        FastLED.show();
        return;
    }

    // Calculate LED count from brightness percentage
    int brightnessPercent = brightness->getVal();
    float numLEDsFloat = brightnessPercent * LED_LENGTH / 100.0;
    int fullLEDs = floor(numLEDsFloat);
    float fraction = numLEDsFloat - fullLEDs;

    // Clamp to valid range
    fullLEDs = constrain(fullLEDs, 0, LED_LENGTH);

    // Clear all LEDs
    fill_solid(leds[0], LED_LENGTH, CRGB::Black);
    fill_solid(leds[1], LED_LENGTH, CRGB::Black);

    // Early exit if no LEDs should be on
    if (fullLEDs == 0 && fraction < 0.01)
    {
        FastLED.show();
        return;
    }

    // Apply flicker effect
    applyFlicker(fullLEDs, fraction, hue->getVal(), saturation->getVal());

    // Update all strips
    FastLED.show();
}

// ============================================================================
// BUTTON HANDLING
// ============================================================================

void DEV_CandleLight::handlePowerButton()
{
    // Read current button state
    buttonCurrentReading = digitalRead(POWER_BUTTON_PIN);

    // Reset timer if state changed
    if (buttonCurrentReading != buttonPreviousReading)
    {
        buttonDebounceTimer = millis();
    }

    // Check if state has been stable long enough
    if ((millis() - buttonDebounceTimer) > DEBOUNCE_DELAY)
    {
        // State has changed from last stable state
        if (buttonCurrentReading != buttonLastStableState)
        {
            buttonLastStableState = buttonCurrentReading;

            // Button press detected (HIGH→LOW transition)
            if (buttonLastStableState == LOW)
            {
                // Record press start time for long press detection
                buttonPressStartTime = millis();
                longPressTriggered = false;
            }
            // Button release detected (LOW→HIGH transition)
            else if (buttonLastStableState == HIGH)
            {
                // Only toggle power if it was a short press (not long press)
                if (!longPressTriggered)
                {
                    power->setVal(!power->getVal());
                    Serial.print("Power button pressed - Lamp ");
                    Serial.println(power->getVal() ? "ON" : "OFF");
                }
                // Reset long press flag for next press
                longPressTriggered = false;
            }
        }
        // Button is being held down (stable LOW state)
        else if (buttonLastStableState == LOW && !longPressTriggered)
        {
            // Check if long press duration exceeded
            if ((millis() - buttonPressStartTime) >= LONG_PRESS_DURATION)
            {
                longPressTriggered = true;

                // Trigger WiFi AP mode for 5 minutes
                homeSpan.setApTimeout(WIFI_AP_TIMEOUT);
                homeSpan.processSerialCommand("A"); // Start AP mode

                Serial.println("\n*** LONG PRESS DETECTED ***");
                Serial.println("WiFi AP mode enabled for 5 minutes");
                Serial.print("Connect to: ");
                Serial.println(WIFI_AP_SSID);
                Serial.println("AP will auto-disable after timeout");
                Serial.println("***************************\n");
            }
        }
    }

    // Save current reading for next iteration
    buttonPreviousReading = buttonCurrentReading;
}

// ============================================================================
// FLICKER ANIMATION
// ============================================================================

void DEV_CandleLight::applyFlicker(int fullLEDs, float fraction, int baseHue, int baseSat)
{
    // Apply flicker to fully-lit LEDs
    for (int i = 0; i < fullLEDs; i++)
    {
        // Generate random target brightness
        float targetBrightness = 100.0 + random(FLICKER_VARIATION_MIN, FLICKER_VARIATION_MAX);
        targetBrightness = constrain(targetBrightness, FLICKER_BRIGHTNESS_MIN, FLICKER_BRIGHTNESS_MAX);

        // Apply exponential smoothing
        float smoothedBrightness = calculateSmoothedBrightness(targetBrightness, previousBrightness[0][i]);

        // Store for next iteration
        previousBrightness[0][i] = smoothedBrightness;
        previousBrightness[1][i] = smoothedBrightness;

        // Generate hue variation (toward yellow/orange)
        int flickerHue = baseHue + random(FLICKER_HUE_MIN, FLICKER_HUE_MAX);
        flickerHue = (flickerHue + 360) % 360; // Wrap around

        // Convert to FastLED CHSV format
        uint8_t finalBrightness = map((int)smoothedBrightness, 0, 100, 0, 255);
        uint8_t finalSaturation = map(baseSat, 0, 100, 0, 255);
        CHSV color(flickerHue, finalSaturation, finalBrightness);

        // Apply to both strips (synchronized)
        leds[0][i] = color;
        leds[1][i] = color;
    }

    // Handle fractional LED (if any)
    if (fraction > 0.01 && fullLEDs < LED_LENGTH)
    {
        // Generate random target brightness
        float targetBrightness = 100.0 + random(FLICKER_VARIATION_MIN, FLICKER_VARIATION_MAX);
        targetBrightness = constrain(targetBrightness, FLICKER_BRIGHTNESS_MIN, FLICKER_BRIGHTNESS_MAX);

        // Apply smoothing
        float smoothedBrightness = calculateSmoothedBrightness(targetBrightness, previousBrightness[0][fullLEDs]);

        // Store for next iteration
        previousBrightness[0][fullLEDs] = smoothedBrightness;
        previousBrightness[1][fullLEDs] = smoothedBrightness;

        // Generate hue variation
        int flickerHue = baseHue + random(FLICKER_HUE_MIN, FLICKER_HUE_MAX);
        flickerHue = (flickerHue + 360) % 360;

        // Scale by fractional amount
        uint8_t finalBrightness = map((int)(smoothedBrightness * fraction), 0, 100, 0, 255);
        uint8_t finalSaturation = map(baseSat, 0, 100, 0, 255);
        CHSV color(flickerHue, finalSaturation, finalBrightness);

        // Apply to both strips
        leds[0][fullLEDs] = color;
        leds[1][fullLEDs] = color;
    }
}

float DEV_CandleLight::calculateSmoothedBrightness(float target, float previous)
{
    // Exponential moving average
    // smoothed = (alpha × previous) + ((1-alpha) × target)
    return (FLICKER_SMOOTHING * previous) + ((1.0 - FLICKER_SMOOTHING) * target);
}
