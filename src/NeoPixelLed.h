#pragma once

#include <Adafruit_NeoPixel.h>
#include "HttpControl.h"

#define SECTOR_MAX_CYCLES 1
#define CURRENT_SECTOR_PERCENT_LENGTH 10
#define NO_BLANKS true

class NeoPixelLed {
public: 
    NeoPixelLed(int ledCountParam, int brightnessParam, int pin, bool useSymmetricParam);
    void display(Arguments & arguments);

private:

    void displaySecondaryAnimation(Arguments & arguments);
    void displayPrimaryAnimation(Arguments & arguments);
    void displaySymmetricAnimation(Arguments & arguments);
    int getLedIndex(int percentage);
    int getMaxSecondarySymmetricLedIndex(int percentage);

    int ledCount;
    int brightness;
    bool useSymmetric;
    Adafruit_NeoPixel* rgbWS;

    int currentSectorPercentEnd = 100;
    int currentSectorCycles = 0;
};