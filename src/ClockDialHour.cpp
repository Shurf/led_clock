#include "ClockDialHour.h"

ClockDialHour::ClockDialHour(int brightnessParam, int pin) : ClockDialBase(12, brightnessParam, pin)
{
    //tickStep = 3; // ticks at 0, 3, 6, 9
    tickStep = 0;
}
