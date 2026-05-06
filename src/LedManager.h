#pragma once

#include "HttpControl.h"
#include <Adafruit_NeoPixel.h>
#include "common.h"

class LedManager
{
public:
    LedManager(int ledCountParam, int brightnessParam, int pin);
    void displayLeds(float redPercentage, float greenPercentage, float bluePercentage);
private:

    int colorValue(float percentage);
    void displaySingleColor(float redPercentage, float greenPercentage, float bluePercentage);
    void displayGrowFromCenter(float redPercentage, float greenPercentage, float bluePercentage);

    int ledCount;
    int brightness;
    Adafruit_NeoPixel* rgbWS;
    Color foregroundColor;
};