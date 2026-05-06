#include "NeoPixelLed.h"

NeoPixelLed::NeoPixelLed(int ledCountParam, int brightnessParam, int pin, bool useSymmetricParam)
{
    ledCount = ledCountParam;
    brightness = brightnessParam;
    useSymmetric = useSymmetricParam;

    rgbWS = new Adafruit_NeoPixel(ledCount, pin, NEO_GRB + NEO_KHZ800);

    rgbWS->begin();
    rgbWS->setBrightness(brightness);
}

int NeoPixelLed::getLedIndex(int percentage)
{
  return percentage*ledCount/100 + ((percentage*ledCount % 100) == 0 ? 0 : 1);
}

int NeoPixelLed::getMaxSecondarySymmetricLedIndex(int percentage)
{
  int max = ledCount/2;
  return max - percentage*max/100 + ((percentage*ledCount % 100) == 0 ? 0 : 1);
}

void NeoPixelLed::display(Arguments & arguments)
{
    if(arguments.useSecondary)
        displaySecondaryAnimation(arguments);
    else if(useSymmetric)
        displaySymmetricAnimation(arguments);
    else
        displayPrimaryAnimation(arguments);

    rgbWS->show();
}

void NeoPixelLed::displaySecondaryAnimation(Arguments & arguments)
{  
    int sectorEnd = getLedIndex(currentSectorPercentEnd);
    int sectorStart = getLedIndex(currentSectorPercentEnd - CURRENT_SECTOR_PERCENT_LENGTH);
    if(sectorStart < 0)
      sectorStart = 0;
    for(int i = 0; i < ledCount; i++)
    {
      // < and not <= because sectorEnd contains the starting index of the next block
      if(i >= sectorStart && i < sectorEnd)
        rgbWS->setPixelColor(i, arguments.red, arguments.green, arguments.blue);  
      else
        rgbWS->setPixelColor(i, arguments.secondaryRed2, arguments.secondaryGreen2, arguments.secondaryBlue2);
      currentSectorCycles++;      
    }
    if(currentSectorCycles >= SECTOR_MAX_CYCLES)
    {
      currentSectorCycles = 0;
      currentSectorPercentEnd -= CURRENT_SECTOR_PERCENT_LENGTH;
      if(currentSectorPercentEnd <= 0)
        currentSectorPercentEnd = 100;
    }
}

void NeoPixelLed::displayPrimaryAnimation(Arguments & arguments)
{
    int currentMax = getLedIndex(arguments.percentage);
    for(int i = 0; i < currentMax; i++) {
      rgbWS->setPixelColor(i, arguments.red, arguments.green, arguments.blue);
    }

    for(int i = currentMax; i < ledCount; i++)
    {
#ifdef NO_BLANKS
      rgbWS->setPixelColor(i, arguments.secondaryRed2, arguments.secondaryGreen2, arguments.secondaryBlue2);
      //rgbWS.setPixelColor(i, 255, random(0, 255), random(0, 255));
#else
      rgbWS->setPixelColor(i, 0, 0, 0);
#endif
    }
    
}

void NeoPixelLed::displaySymmetricAnimation(Arguments & arguments)
{

    for(int i = 0; i < ledCount; i++) {
        rgbWS->setPixelColor(i, arguments.red, arguments.green, arguments.blue);
    }
    int secondaryIndex = getMaxSecondarySymmetricLedIndex(arguments.percentage);
    Serial.println(secondaryIndex);
    for(int i = 0; i < secondaryIndex; i++)
        rgbWS->setPixelColor(i, arguments.secondaryRed2, arguments.secondaryGreen2, arguments.secondaryBlue2);

    for(int i = ledCount - 1; i > ledCount - 1 - secondaryIndex; i--)
        rgbWS->setPixelColor(i, arguments.secondaryRed2, arguments.secondaryGreen2, arguments.secondaryBlue2);
}