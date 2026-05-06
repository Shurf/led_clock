#include "BacklightLed.h"

BacklightLed::BacklightLed(int ledCountParam, int brightnessParam, int pin)
{
    ledCount = ledCountParam;
    brightness = brightnessParam;

    foregroundColor.red = 255;
    foregroundColor.green = 0;
    foregroundColor.blue = 0;

    rgbWS = new Adafruit_NeoPixel(ledCount, pin, NEO_GRB + NEO_KHZ800);

    rgbWS->begin();
    rgbWS->setBrightness(brightness);
}


void BacklightLed::display(int brightnessParam)
{
    setBrightness(brightnessParam);
    for(int i = 0; i < ledCount; i++)
        rgbWS->setPixelColor(i, foregroundColor.red, foregroundColor.green, foregroundColor.blue);
    rgbWS->show();
}

void BacklightLed::setBrightness(int brightnessParam)
{    
    brightness = brightnessParam;
    rgbWS->setBrightness(brightness);
}

void BacklightLed::setColor(Color & color, int red, int green, int blue)
{
    color.red = red;
    color.green = green;
    color.blue = blue;
}

void BacklightLed::setForegroundColor(int red, int green, int blue)
{
    setColor(foregroundColor, red, green, blue);
}
