#include <Arduino.h>

char pTopic[64] = {0};
char fridgeStatus[3];
float tempC = -99;
float coolingStartTemp = 0;
char temp[8];
char msg[64] = {0};

float lowerLimit = 2.5;
float upperLimit = 5.0;

bool timeOverride = false;

int brightness = 120; // how bright the LED is
int fadeAmount = 5;   // how many points to fade the LED by