#include "ClockDialMinute.h"

ClockDialMinute::ClockDialMinute(int brightnessParam, int pin) : ClockDialBase(60, brightnessParam, pin)
{
    tickStep = 5; // ticks every 5 minutes (12 markers)
}
