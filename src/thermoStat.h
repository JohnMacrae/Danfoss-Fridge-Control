#include <Arduino.h>

/*
State machine switches on temperature in global tempC
*/

void thermoStat()
{
    if (!timeOverride)
    {

        if (tempC > upperLimit)
        {
            //Turn on



        }
        if (tempC < lowerLimit)
        {
            //Turn ff

        }
    }
}