#include <Arduino.h>

void setupPWM()
{
    const int freq = 5000;
    const int ledChannel = 0;
    const int resolution = 8;

    // configure LED PWM functionalitites
    ledcSetup(ledChannel, freq, resolution);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(PWM, ledChannel);

    int dutyCycle = 254;
    ledcWrite(ledChannel, dutyCycle);
}
