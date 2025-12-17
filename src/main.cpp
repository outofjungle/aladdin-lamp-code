/**
 * @file main.cpp
 * @brief Aladdin Lamp - HomeKit Candle Light Simulator
 *
 * A realistic candle light simulator for ESP32 with dual synchronized
 * APA102 LED strips and HomeKit integration.
 *
 * Features:
 * - HomeKit color and brightness control
 * - Dramatic candle flicker with exponential smoothing
 * - Manual power button control
 * - Factory reset via long button press
 * - Status LED for WiFi/HomeKit connection
 * - Open WiFi setup portal (no password)
 *
 * Hardware:
 * - ESP32 PICO32 board
 * - 2x APA102 LED strips (8 LEDs each)
 * - Power button on GPIO 0
 * - Factory reset button on GPIO 39
 * - Status LED on GPIO 22
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

// Third-party libraries
#include <Arduino.h>
#include "HomeSpan.h"
#include <FastLED.h>

// Project headers
#include "config.h"
#include "CandleLight.h"

// ============================================================================
// GLOBAL LED ARRAYS
// ============================================================================

/**
 * LED color arrays for both strips
 * Accessed globally by DEV_CandleLight service
 */
CRGB leds[NUM_STRIPS][LED_LENGTH];

// ============================================================================
// SETUP AND MAIN LOOP
// ============================================================================

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB
    }

    // Initialize random number generator with ESP32 hardware RNG
    // This ensures different flicker patterns on each power cycle
    randomSeed(esp_random());

    Serial.println("\n\n================================");
    Serial.println("Aladdin Lamp - HomeKit Candle");
    Serial.println("================================\n");

    // Configure HomeSpan before begin()
    homeSpan.setApSSID(WIFI_AP_SSID);
    homeSpan.setApPassword("");                  // Open network (no password)
    homeSpan.setApTimeout(WIFI_AP_TIMEOUT);      // 5-minute AP timeout
    homeSpan.setPairingCode(HOMEKIT_SETUP_CODE); // Custom pairing code
    homeSpan.setStatusPin(STATUS_LED_PIN);
    homeSpan.setControlPin(CONTROL_BUTTON_PIN);

    // Initialize HomeSpan
    homeSpan.begin(Category::Lighting, HOMEKIT_NAME);

    // Create HomeKit accessory
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Manufacturer(HOMEKIT_MANUFACTURER);
    new Characteristic::Model(HOMEKIT_MODEL);
    new Characteristic::SerialNumber(HOMEKIT_SERIAL);

    // Add candle light service
    new DEV_CandleLight();

    // Print setup instructions
    Serial.println("Setup complete!");
    Serial.println("\nWiFi Setup AP: '" WIFI_AP_SSID "' (OPEN - no password)");
    Serial.println("\nButtons:");
    Serial.println("  - GPIO 0:  Short press to toggle lamp ON/OFF");
    Serial.println("             Long press (3 sec) to enable WiFi AP for 5 min");
    Serial.println("  - GPIO 39: Long press (>10 sec) for factory reset");
    Serial.println("\nStatus LED (GPIO 22):");
    Serial.println("  - Blinking: Not connected/pairing");
    Serial.println("  - Solid: Connected and paired");
    Serial.println("\nTo pair with HomeKit:");
    Serial.println("1. Connect to '" WIFI_AP_SSID "' WiFi (no password)");
    Serial.println("2. Configure your WiFi credentials via captive portal");
    Serial.println("3. Open Home app on iPhone/iPad");
    Serial.println("4. Tap '+' > Add Accessory");
    Serial.println("5. Scan or enter the Setup Code shown above");
    Serial.println("\nTo reconfigure WiFi:");
    Serial.println("- Long press power button (GPIO 0) for 3 seconds");
    Serial.println("- WiFi AP will be enabled for 5 minutes");
    Serial.println("================================\n");
}

void loop()
{
    homeSpan.poll();
}
