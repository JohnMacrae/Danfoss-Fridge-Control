#include <Arduino.h>

#define TEMPSIG 21
#define GPIO2 2
#define TX 1
#define RX 3
#define GPIO1 5
#define JTAG1 14
#define JTAG2 16
#define JTAG3 13
#define JTAG4 23
#define GPIO3 16
#define GPIO4 17
#define GPIO5 18
#define GPIO6 19
//#define GPIO7 33 now ow bus
#define VSENS 22
#define SDA 23
#define SCL 25
#define FAULTLED 26
#define FAULTIN 27
#define BUZZER 32
#define PWM 33
#define TEMPSIG2 35

void setPins()
{
pinMode(FAULTIN, INPUT);
pinMode(FAULTLED, OUTPUT);
digitalWrite(FAULTLED, LOW);

pinMode(PWM, OUTPUT);
digitalWrite(PWM, HIGH);

pinMode(VSENS, INPUT);
}

