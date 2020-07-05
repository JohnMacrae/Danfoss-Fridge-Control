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

int compSpeed = 120; // how bright the LED is
int fadeAmount = 5;   // how many points to fade the LED by

uint64_t runStart = 0; // time the compressor started
int level = 0; // PWM level
bool runFlag = true;
uint64_t runTime = 0;

uint32_t diff = 0;
unsigned long lasttime=0;