#pragma once

#include "HttpControl.h"
#include <Adafruit_NeoPixel.h>
#include "common.h"

// Per-channel multipliers applied after the AGC + envelope follower, before the
// [0, 255] clamp. <1 suppresses, >1 boosts. Defaults bias the palette toward red
// so highs glow bright, mids contribute orange-ish, bass barely tints blue.
#define RED_TINT   1.0f
#define GREEN_TINT 0.35f
#define BLUE_TINT  0.15f

class LedManager
{
public:
    LedManager(int ledCountParam, int brightnessParam, int pin);
    void displayLeds(float redPercentage, float greenPercentage, float bluePercentage);
    // Paint every pixel with (r, g, b) scaled by `intensity` in [0, 1]. Used
    // for the beat-driven flash on the center dot — soundLoop holds intensity
    // at 1.0 on a beat, then decays it each frame.
    void displayFlash(float intensity, int r, int g, int b);
private:

    int colorValue(float percentage);
    void displaySingleColor(float redPercentage, float greenPercentage, float bluePercentage);
    void displayGrowFromCenter(float redPercentage, float greenPercentage, float bluePercentage);

    int ledCount;
    int brightness;
    Adafruit_NeoPixel* rgbWS;
    Color foregroundColor;
};