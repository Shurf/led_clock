#include "LedManager.h"

LedManager::LedManager(int ledCountParam, int brightnessParam, int pin)
{
    ledCount = ledCountParam;
    brightness = brightnessParam;

    foregroundColor.red = 255;
    foregroundColor.green = 255;
    foregroundColor.blue = 0;

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

void LedManager::displaySingleColor(float redPercentage, float greenPercentage, float bluePercentage)
{
    int r = colorValue(redPercentage * RED_TINT);
    int g = colorValue(greenPercentage * GREEN_TINT);
    int b = colorValue(bluePercentage * BLUE_TINT);

    for (auto i = 0; i < ledCount; i++)
        rgbWS->setPixelColor(i, r, g, b);
    // Send each frame twice. The second show() waits for the WS2812 latch period
    // automatically, so a single-bit data glitch in the first transmission is
    // overwritten before the eye can register it. Mitigates the occasional
    // stray-color pixel caused by 3.3 V data driving a 5 V strip.
    rgbWS->show();
    rgbWS->show();
}

void LedManager::displayGrowFromCenter(float redPercentage, float greenPercentage, float bluePercentage)
{
    /*int numActiveLeds = int(NUM_LEDS * (redPercentage + greenPercentage + bluePercentage) / 3.0);
    numActiveLeds *= LED_SCALING;
    if (numActiveLeds > NUM_LEDS)
        numActiveLeds = NUM_LEDS;

    if (numActiveLeds == 0)
        return;

    auto numAlteredLeds = numActiveLeds / 3;

    auto initialColor = CRGB(colorValue(redPercentage), colorValue(greenPercentage), colorValue(bluePercentage));
    auto noneColor = CRGB(0, 0, 0);

    auto redIncrement = (1.0 - redPercentage) / numAlteredLeds;
    auto greenIncrement = (1.0 - greenPercentage) / numAlteredLeds;
    auto blueIncrement = (1.0 - bluePercentage) / numAlteredLeds;

    auto currentRedPercentage = redPercentage;
    auto currentGreenPercentage = greenPercentage;
    auto currentBluePercentage = bluePercentage;


    for(int i = 0; i < numActiveLeds/2; i++)
    {
        if(i < (numActiveLeds - numAlteredLeds)/2)
        {
        leds[NUM_LEDS/2 + i] = initialColor;
        leds[NUM_LEDS/2 - i - 1]= initialColor;
        continue;
        }

        auto currentColor = CRGB(colorValue(currentRedPercentage), colorValue(currentGreenPercentage), colorValue(currentBluePercentage));

        leds[NUM_LEDS/2 + i] = currentColor;
        leds[NUM_LEDS/2 - i - 1]= currentColor;

        currentRedPercentage += redIncrement;
        currentGreenPercentage += greenIncrement;
        currentBluePercentage += blueIncrement;

    }

    for(int i = numActiveLeds/2; i < NUM_LEDS/2; i++)
    {
        leds[NUM_LEDS/2 + i] = noneColor;
        leds[NUM_LEDS/2 - i - 1]= noneColor;
    }

    FastLED.show(arguments.brightness);*/
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

void LedManager::displayLeds(float redPercentage, float greenPercentage, float bluePercentage)
{
    //if(arguments.mode == MODE_SINGLE_COLOR)
        displaySingleColor(redPercentage, greenPercentage, bluePercentage);
    /*if(arguments.mode == MODE_GROW_FROM_CENTER)
        displayGrowFromCenter(redPercentage, greenPercentage, bluePercentage, arguments);*/
}