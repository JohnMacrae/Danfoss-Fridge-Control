#include <Arduino.h>

void setupPWM()
{

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0 8

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT 13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)

    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(PWM, LEDC_CHANNEL_0);
}

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
    // calculate duty, 8191 from 2 ^ 13 - 1
    uint32_t duty = (8191 / valueMax) * min(value, valueMax);

    // write duty to LEDC
    ledcWrite(channel, duty);
}

void runled(int level)
{
    ledcAnalogWrite(LEDC_CHANNEL_0, level);

    // change the compSpeed for next time through the loop:
    compSpeed = compSpeed + fadeAmount;

    // reverse the direction of the fading at the ends of the fade:
    if (compSpeed <= 0 || compSpeed >= 255)
    {
        fadeAmount = -fadeAmount;
    }
    // wait for 30 milliseconds to see the dimming effect

    delay(30);
}


