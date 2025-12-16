/**
 * @file CandleLedArray.h
 * @brief LED array controller with candle flicker animation
 *
 * Manages dual synchronized APA102 LED strips with realistic
 * candle flicker effects using exponential smoothing.
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

#ifndef CANDLELEDARRAY_H
#define CANDLELEDARRAY_H

#include <FastLED.h>
#include <colorutils.h>
#include <math.h>
#include "config.h"

/**
 * @struct CandleLedArray
 * @brief Manages dual synchronized LED strips with candle flicker
 *
 * Encapsulates all LED control logic including brightness mapping,
 * candle flicker animation, and dual-strip synchronization.
 */
struct CandleLedArray
{
    CFastLED *fastled;
    CRGB (*leds)[LED_LENGTH];  // Pointer to 2D array [NUM_STRIPS][LED_LENGTH]
    bool state;
    uint8_t length;
    uint8_t hue;
    uint8_t saturation;
    uint8_t brightness;
    uint8_t fullLEDs;     // Number of fully-lit LEDs
    float fraction;       // Fractional brightness for last LED

    // Flicker state tracking
    float previousBrightness[NUM_STRIPS][LED_LENGTH];
    uint32_t lastUpdate;

    /**
     * Initialize LED array controller
     *
     * @param fastled Pointer to FastLED instance
     * @param leds Pointer to 2D LED array
     * @param length Number of LEDs per strip
     */
    CandleLedArray(CFastLED *fastled, CRGB (*leds)[LED_LENGTH], uint8_t length)
    {
        this->fastled = fastled;
        this->leds = leds;
        this->length = length;

        // Initialize to default candle state (ON, orange, 100% brightness)
        this->state = true;
        this->hue = map(DEFAULT_HUE, 0, 360, 0, 255);
        this->saturation = map(DEFAULT_SATURATION, 0, 100, 0, 255);
        transformSetBrightness(DEFAULT_BRIGHTNESS);

        this->lastUpdate = 0;

        // Initialize flicker arrays with random starting values
        for (int strip = 0; strip < NUM_STRIPS; strip++)
        {
            for (int i = 0; i < LED_LENGTH; i++)
            {
                previousBrightness[strip][i] = 70.0 + random(0, 31);
            }
        }

        // Start with LEDs off (animation will turn them on)
        fill_solid(leds[0], length, CRGB::Black);
        fill_solid(leds[1], length, CRGB::Black);
        fastled->show();
    }

    /**
     * Set power state
     */
    void SetPower(bool state)
    {
        this->state = state;
    }

    /**
     * Transform brightness percentage to LED count with fractional dimming
     *
     * @param brightness Brightness value (0-100)
     */
    void transformSetBrightness(int brightness)
    {
        float numLEDsFloat = float(length * brightness) / 100.0;
        this->fullLEDs = (uint8_t)floor(numLEDsFloat);
        this->fraction = numLEDsFloat - fullLEDs;

        // Clamp to valid range
        this->fullLEDs = constrain(this->fullLEDs, 0, length);
    }

    /**
     * Set HSV color and brightness values
     *
     * @param H Hue (0-360)
     * @param S Saturation (0-100)
     * @param V Brightness (0-100)
     */
    void SetValues(float H, float S, float V)
    {
        this->hue = (uint8_t)(H * 255 / 360);
        this->saturation = (uint8_t)(S * 2.55);
        this->brightness = (uint8_t)V;
        transformSetBrightness(V);
    }

    /**
     * Update LED display with current state and candle animation
     *
     * @return true on success
     */
    bool On()
    {
        // Rate-limit animation updates
        if (millis() - lastUpdate < UPDATE_INTERVAL)
        {
            return true;
        }
        lastUpdate = millis();

        // If powered off, turn off all LEDs
        if (!state)
        {
            fill_solid(leds[0], length, CRGB::Black);
            fill_solid(leds[1], length, CRGB::Black);
            fastled->show();
            return true;
        }

        // Clear all LEDs first
        fill_solid(leds[0], length, CRGB::Black);
        fill_solid(leds[1], length, CRGB::Black);

        // Early exit if no LEDs should be lit
        if (fullLEDs == 0 && fraction < 0.01)
        {
            fastled->show();
            return true;
        }

        // Apply candle flicker to fully-lit LEDs
        applyFlicker();

        // Update display
        fastled->show();
        return true;
    }

    /**
     * Blink animation for HomeKit identification
     *
     * @return true on success
     */
    bool Blink()
    {
        // Sweep white LED forward
        for (uint8_t i = 0; i < length; i++)
        {
            fill_solid(leds[0], length, CRGB::Black);
            fill_solid(leds[1], length, CRGB::Black);
            leds[0][i] = CRGB::White;
            leds[1][i] = CRGB::White;
            fastled->show();
            delay(50);
        }

        // Sweep white LED backward
        for (int i = length - 1; i >= 0; i--)
        {
            fill_solid(leds[0], length, CRGB::Black);
            fill_solid(leds[1], length, CRGB::Black);
            leds[0][i] = CRGB::White;
            leds[1][i] = CRGB::White;
            fastled->show();
            delay(50);
        }

        // Clear and restore normal state
        fill_solid(leds[0], length, CRGB::Black);
        fill_solid(leds[1], length, CRGB::Black);
        fastled->show();
        return true;
    }

private:
    /**
     * Apply candle flicker effect to active LEDs
     */
    void applyFlicker()
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
            int flickerHue = hue + random(FLICKER_HUE_MIN, FLICKER_HUE_MAX);
            flickerHue = (flickerHue + 255) % 255;

            // Convert to FastLED CHSV format
            uint8_t finalBrightness = map((int)smoothedBrightness, 0, 100, 0, 255);
            CHSV color(flickerHue, saturation, finalBrightness);

            // Apply to both strips (synchronized)
            leds[0][i] = color;
            leds[1][i] = color;
        }

        // Handle fractional LED (if any)
        if (fraction > 0.01 && fullLEDs < length)
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
            int flickerHue = hue + random(FLICKER_HUE_MIN, FLICKER_HUE_MAX);
            flickerHue = (flickerHue + 255) % 255;

            // Scale by fractional amount
            uint8_t finalBrightness = map((int)(smoothedBrightness * fraction), 0, 100, 0, 255);
            CHSV color(flickerHue, saturation, finalBrightness);

            // Apply to both strips
            leds[0][fullLEDs] = color;
            leds[1][fullLEDs] = color;
        }
    }

    /**
     * Calculate smoothed brightness using exponential moving average
     *
     * @param target Target brightness value
     * @param previous Previous smoothed value
     * @return Smoothed brightness value
     */
    float calculateSmoothedBrightness(float target, float previous)
    {
        return (FLICKER_SMOOTHING * previous) + ((1.0 - FLICKER_SMOOTHING) * target);
    }
};

#endif // CANDLELEDARRAY_H
