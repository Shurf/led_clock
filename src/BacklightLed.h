#pragma once

#include <Adafruit_NeoPixel.h>
#include "common.h"

class BacklightLed {
public: 
    BacklightLed(int ledCountParam, int brightnessParam, int pin);
    void display(int brightnessParam);
    void setBrightness(int brightnessParam);

    void setForegroundColor(int red, int green, int blue);

protected:

    void setColor(Color & color, int red, int green, int blue);
    int ledCount;
    int brightness;
    Adafruit_NeoPixel* rgbWS;

    Color foregroundColor;
};