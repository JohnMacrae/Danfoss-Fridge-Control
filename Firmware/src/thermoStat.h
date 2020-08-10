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
            runled(compSpeed);
            runStart = esp_timer_get_time();
            runFlag = 1;
            //coolingStartTemp = tempC;
        }
        if (tempC < lowerLimit)
        {
            //Turn off
            runled(0);
            runFlag = 0;
            if (runStart>0)
            {// calculate the runtime
                runTime = (esp_timer_get_time()-runStart)/1000; // 1Khz clock;
                runStart = 0;
            }
        }
    }
}