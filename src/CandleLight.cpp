/**
 * @file CandleLight.cpp
 * @brief Implementation of DEV_CandleLight and DEV_Identify HomeKit services
 *
 * Contains all logic for candle flicker animation, button handling,
 * HomeKit characteristic updates, and identify functionality.
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
    buttonState = BTN_IDLE;
    buttonStateTimer = 0;
    buttonPressStartTime = 0;
    buttonLastReading = HIGH;

    // Initialize smoothing arrays to midpoint of flicker range
    // This prevents large jumps on first animation frame
    for (int strip = 0; strip < NUM_STRIPS; strip++)
    {
        for (int i = 0; i < LED_LENGTH; i++)
        {
            previousBrightness[strip][i] = (FLICKER_BRIGHTNESS_MIN + FLICKER_BRIGHTNESS_MAX) / 2.0;
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
    bool currentReading = digitalRead(POWER_BUTTON_PIN);
    uint32_t now = millis();

    switch (buttonState)
    {
    case BTN_IDLE:
        // Monitor for button press (HIGH→LOW edge)
        if (buttonLastReading == HIGH && currentReading == LOW)
        {
            buttonState = BTN_DEBOUNCING_PRESS;
            buttonStateTimer = now;
        }
        break;

    case BTN_DEBOUNCING_PRESS:
        // Wait for stable LOW reading
        if (currentReading == HIGH)
        {
            // Button bounced back to HIGH, return to idle
            buttonState = BTN_IDLE;
        }
        else if ((now - buttonStateTimer) >= DEBOUNCE_DELAY)
        {
            // Stable LOW confirmed, press accepted
            buttonState = BTN_PRESSED;
            buttonPressStartTime = now;  // Record when press was confirmed
        }
        break;

    case BTN_PRESSED:
        // Monitor for release or long press threshold
        if (currentReading == HIGH)
        {
            // Release detected, begin debouncing short release
            buttonState = BTN_DEBOUNCING_SHORT_RELEASE;
            buttonStateTimer = now;  // Start debounce timer for release
        }
        else if ((now - buttonPressStartTime) >= LONG_PRESS_DURATION)
        {
            // Long press threshold reached, trigger WiFi AP mode
            buttonState = BTN_LONG_PRESS_ACTIVE;

            homeSpan.processSerialCommand("A");
            Serial.println("\n*** LONG PRESS DETECTED ***");
            Serial.println("WiFi AP mode enabled for 5 minutes");
            Serial.print("Connect to: ");
            Serial.println(WIFI_AP_SSID);
            Serial.println("AP will auto-disable after timeout");
            Serial.println("***************************\n");
        }
        break;

    case BTN_LONG_PRESS_ACTIVE:
        // Wait for button release
        if (currentReading == HIGH)
        {
            // Release detected, begin debouncing long release
            buttonState = BTN_DEBOUNCING_LONG_RELEASE;
            buttonStateTimer = now;
        }
        break;

    case BTN_DEBOUNCING_SHORT_RELEASE:
        // Wait for stable HIGH reading after short press
        if (currentReading == LOW)
        {
            // Bounced back to LOW, return to pressed state
            buttonState = BTN_PRESSED;
        }
        else if ((now - buttonStateTimer) >= DEBOUNCE_DELAY)
        {
            // Stable HIGH confirmed, execute short press action
            power->setVal(!power->getVal());
            Serial.print("Power button pressed - Lamp ");
            Serial.println(power->getVal() ? "ON" : "OFF");

            buttonState = BTN_IDLE;
        }
        break;

    case BTN_DEBOUNCING_LONG_RELEASE:
        // Wait for stable HIGH reading after long press
        if (currentReading == LOW)
        {
            // Bounced back to LOW, return to long press active
            buttonState = BTN_LONG_PRESS_ACTIVE;
        }
        else if ((now - buttonStateTimer) >= DEBOUNCE_DELAY)
        {
            // Stable HIGH confirmed, release accepted (no action needed)
            buttonState = BTN_IDLE;
        }
        break;
    }

    // Save current reading for next iteration
    buttonLastReading = currentReading;
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
        // Wrap to 0-360 range (handles negative values correctly)
        flickerHue = ((flickerHue % 360) + 360) % 360;

        // Convert to FastLED CHSV format (0-255 range)
        uint8_t finalHue = map(flickerHue, 0, 360, 0, 255);
        uint8_t finalBrightness = map((int)smoothedBrightness, FLICKER_BRIGHTNESS_MIN, FLICKER_BRIGHTNESS_MAX, 0, 255);
        uint8_t finalSaturation = map(baseSat, 0, 100, 0, 255);
        CHSV color(finalHue, finalSaturation, finalBrightness);

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
        // Wrap to 0-360 range (handles negative values correctly)
        flickerHue = ((flickerHue % 360) + 360) % 360;

        // Scale by fractional amount and convert to FastLED CHSV format (0-255 range)
        uint8_t finalHue = map(flickerHue, 0, 360, 0, 255);
        int scaledBrightness = (int)(smoothedBrightness * fraction);
        uint8_t finalBrightness = map(constrain(scaledBrightness, FLICKER_BRIGHTNESS_MIN, FLICKER_BRIGHTNESS_MAX), FLICKER_BRIGHTNESS_MIN, FLICKER_BRIGHTNESS_MAX, 0, 255);
        uint8_t finalSaturation = map(baseSat, 0, 100, 0, 255);
        CHSV color(finalHue, finalSaturation, finalBrightness);

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

// ============================================================================
// DEV_IDENTIFY - CONSTRUCTOR
// ============================================================================

DEV_Identify::DEV_Identify() : Service::AccessoryInformation()
{
    // Create required characteristics
    new Characteristic::Identify();
    new Characteristic::Manufacturer(HOMEKIT_MANUFACTURER);
    new Characteristic::Model(HOMEKIT_MODEL);
    new Characteristic::SerialNumber(HOMEKIT_SERIAL);

    Serial.println("Identify service initialized");
}

// ============================================================================
// DEV_IDENTIFY - HOMEKIT CALLBACKS
// ============================================================================

boolean DEV_Identify::update()
{
    // Called when user taps "Identify" in Home app
    // Flash LEDs to visually identify the device

    Serial.println("\n*** IDENTIFY REQUEST ***");
    Serial.println("Flashing LEDs to identify device");

    // Flash all LEDs white 3 times (1.8 seconds total)
    for (int flash = 0; flash < 3; flash++)
    {
        // Turn all LEDs white (full brightness)
        fill_solid(leds[0], LED_LENGTH, CRGB::White);
        fill_solid(leds[1], LED_LENGTH, CRGB::White);
        FastLED.show();
        delay(300);

        // Turn off all LEDs
        fill_solid(leds[0], LED_LENGTH, CRGB::Black);
        fill_solid(leds[1], LED_LENGTH, CRGB::Black);
        FastLED.show();
        delay(300);
    }

    Serial.println("Identify complete\n");
    return true;
}
