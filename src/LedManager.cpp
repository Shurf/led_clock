#include "LedManager.h"

LedManager::LedManager(int ledCountParam, int brightnessParam, int pin)
{
    ledCount = ledCountParam;
    brightness = brightnessParam;

    rgbWS = new Adafruit_NeoPixel(ledCount, pin, NEO_GRB + NEO_KHZ800);

    rgbWS->begin();
    rgbWS->setBrightness(brightness);
}

int LedManager::colorValue(float percentage)
{
    auto value = (int)(percentage * 255) + 1;
    if (value > 255)
        value = 255;
    return value;
}

void LedManager::displaySpectrum(const float* bands, int bandCount)
{
    for (int i = 0; i < ledCount; i++)
    {
        int bandIdx = (i * bandCount) / ledCount;
        if (bandIdx >= bandCount) bandIdx = bandCount - 1;

        float intensity = bands[bandIdx];
        int r = 0, g = 0, b = 0;
        // Band groups match FFT.cpp's grouping: 0..4 highs, 5..7 mids, 8..10 bass.
        if (bandIdx <= 4)
            r = colorValue(intensity * RED_TINT);
        else if (bandIdx <= 7)
            g = colorValue(intensity * GREEN_TINT);
        else
            b = colorValue(intensity * BLUE_TINT);

        rgbWS->setPixelColor(i, r, g, b);
    }
    // Send each frame twice. The second show() waits for the WS2812 latch
    // period automatically, so a single-bit data glitch in the first
    // transmission is overwritten before the eye can register it. Mitigates
    // the occasional stray-color pixel caused by 3.3 V data driving a 5 V strip.
    rgbWS->show();
    rgbWS->show();
}

void LedManager::displayFlash(float intensity, int r, int g, int b)
{
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;

    int sr = (int)(r * intensity);
    int sg = (int)(g * intensity);
    int sb = (int)(b * intensity);

    for (auto i = 0; i < ledCount; i++)
        rgbWS->setPixelColor(i, sr, sg, sb);
    rgbWS->show();
    rgbWS->show();
}
