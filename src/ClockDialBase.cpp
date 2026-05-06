#include "ClockDialBase.h"

ClockDialBase::ClockDialBase(int ledCountParam, int brightnessParam, int pin)
{
    ledCount = ledCountParam;
    brightness = brightnessParam;
    tickStep = 0;

    foregroundColor.red = 255;
    foregroundColor.green = 0;
    foregroundColor.blue = 0;

    backgroundColor.red = 225;
    backgroundColor.green = 35;
    backgroundColor.blue = 0;

    tickColor.red = 0;
    tickColor.green = 0;
    tickColor.blue = 255;

    previousValue = 0;
    dirty = true;

    rgbWS = new Adafruit_NeoPixel(ledCount, pin, NEO_GRB + NEO_KHZ800);

    rgbWS->begin();
    // Strip brightness stays at full; the highlight pixel emits at full color, while
    // background and tick pixels are dimmed per-pixel via `brightness` so the highlight
    // visually pops.
    rgbWS->setBrightness(255);
}


void ClockDialBase::display(int value)
{
    if(previousValue == value && !dirty)
        return;

    displayValue(value);

    rgbWS->show();

    previousValue = value;
    dirty = false;
}

Color ClockDialBase::dimmed(const Color & c) const
{
    Color result;
    result.red = (c.red * brightness) / 255;
    result.green = (c.green * brightness) / 255;
    result.blue = (c.blue * brightness) / 255;
    return result;
}

void ClockDialBase::displayValue(int value)
{
    Color bg = dimmed(backgroundColor);
    for(int i = 0; i < ledCount; i++)
        rgbWS->setPixelColor(getLedIndex(i), bg.red, bg.green, bg.blue);

    if(tickStep > 0)
    {
        Color tick = dimmed(tickColor);
        for(int i = 0; i < ledCount; i += tickStep)
            rgbWS->setPixelColor(getLedIndex(i), tick.red, tick.green, tick.blue);
    }

    rgbWS->setPixelColor(getLedIndex(value), foregroundColor.red, foregroundColor.green, foregroundColor.blue);
}

int ClockDialBase::getLedIndex(int value)
{
    return (ledCount/2  + value) % ledCount;
}

void ClockDialBase::setColor(Color & color, int red, int green, int blue)
{
    if(color.red == red && color.green == green && color.blue == blue)
        return;
    color.red = red;
    color.green = green;
    color.blue = blue;
    dirty = true;
}

void ClockDialBase::setForegroundColor(int red, int green, int blue)
{
    setColor(foregroundColor, red, green, blue);
}

void ClockDialBase::setBackgroundColor(int red, int green, int blue)
{
    setColor(backgroundColor, red, green, blue);
}

void ClockDialBase::setTickColor(int red, int green, int blue)
{
    setColor(tickColor, red, green, blue);
}
