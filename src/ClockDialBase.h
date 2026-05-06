#pragma once

#include <Adafruit_NeoPixel.h>
#include "HttpControl.h"
#include "common.h"


class ClockDialBase {
public: 
    ClockDialBase(int ledCountParam, int brightnessParam, int pin);
    void display(int value);

    void setForegroundColor(int red, int green, int blue);
    void setBackgroundColor(int red, int green, int blue);
    void setTickColor(int red, int green, int blue);

protected:

    void setColor(Color & color, int red, int green, int blue);
    virtual void displayValue(int value);
    int getLedIndex(int value);
    Color dimmed(const Color & c) const;
    int ledCount;
    int brightness;
    int tickStep;
    Adafruit_NeoPixel* rgbWS;
    int previousValue;
    bool dirty;

    Color backgroundColor;
    Color foregroundColor;
    Color tickColor;
};