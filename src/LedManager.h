#pragma once

#include "HttpControl.h"
#include <Adafruit_NeoPixel.h>
#include "common.h"

// Per-channel multipliers applied after the AGC + envelope follower, before the
// [0, 255] clamp. <1 suppresses, >1 boosts. Defaults bias the palette toward red
// so highs glow bright, mids contribute orange-ish, bass barely tints blue.
#define RED_TINT   1.0f
#define GREEN_TINT 0.35f
#define BLUE_TINT  0.4f

class LedManager
{
public:
    LedManager(int ledCountParam, int brightnessParam, int pin);

    // Spatial spectrum: each LED corresponds to a frequency band based on its
    // index. Band group decides the color channel (0..4 highs → red, 5..7
    // mids → green, 8..10 bass → blue), tinted by RED/GREEN/BLUE_TINT.
    void displaySpectrum(const float* bands, int bandCount);

    // Beat-driven flash: paint every pixel with (r, g, b) scaled by `intensity`
    // in [0, 1]. Bypasses the tint pipeline so the flash punches through the
    // warm palette as a clean white pop.
    void displayFlash(float intensity, int r, int g, int b);

private:
    int colorValue(float percentage);

    int ledCount;
    int brightness;
    Adafruit_NeoPixel* rgbWS;
};
