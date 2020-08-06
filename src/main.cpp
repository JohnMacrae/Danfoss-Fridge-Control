#include <Arduino.h>

#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "globals.h"
#include "credentials.h"
#include "config.h"
#include "mqtt.h"
#include "pins.h"
#include "pwm.h"
#include "thermoStat.h"

#define RESOLUTION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(TEMPSIG);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

volatile int interruptCounter = 0;
int totalInterruptCounter = 0;

struct Fault
{
  uint32_t interval;
  uint16_t code;
  unsigned long fTimerStart;
  bool startTimer;
};

volatile Fault fault = {0, 0, 0, false};

hw_timer_t *timer = NULL;
hw_timer_t *thermoTimer = NULL;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile int thermoCounter = 0;

void IRAM_ATTR onThermoTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);

  thermoCounter++;

  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR onFaultTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  /* if (fault.fTimerStart == 0)
  {
    fault.fTimerStart = esp_timer_get_time();
  }
  interruptCounter++;
  */
  portEXIT_CRITICAL_ISR(&timerMux);
}

struct Button
{
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {0, false};

void IRAM_ATTR isr()
{
  portENTER_CRITICAL_ISR(&timerMux);
  button1.pressed = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void OTA_Setup()
{
  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
    SPIFFS.end();
  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end

  });

  ArduinoOTA.onError([](ota_error_t error) {
    ESP.restart();
  });

  /* setup the OTA server */
  ArduinoOTA.begin();
}

/*
 * Setup function. Here we do the basics
 */
void setup(void)
{
  // start serial port
  Serial.begin(115200);

  thermoTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(thermoTimer, &onThermoTimer, true);
  timerAlarmWrite(thermoTimer, 10000000, true);
  timerAlarmEnable(thermoTimer);

  setPins();

  attachInterrupt(FAULTIN, isr, RISING);

  fileSystemCheck();

  loadConfig();
  WiFi.mode(WIFI_STA);
  WiFi.begin(Settings.WifiSSID, Settings.WifiKey);

  int count = 0;
  while ((count < 10) && (WiFi.status() != WL_CONNECTED))
  {
    delay(500);
    count++;
    Serial.print(WiFi.status());
    Serial.print(".");
  }
  Serial.println("");

  Serial.println(WiFi.localIP());

  OTA_Setup();

  Serial.println("Dallas Temperature IC Control Library Demo");

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode())
    Serial.println("ON");
  else
    Serial.println("OFF");

  if (!sensors.getAddress(insideThermometer, 0))
    Serial.println("Unable to find address for Device 0");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, RESOLUTION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();
  sensors.requestTemperatures(); // Send the command to get temperatures
  vTaskDelay(300);
  float tempC = -99;
  tempC = sensors.getTempC(insideThermometer);
  Serial.println(tempC);

  setupPWM();

  myreconnect();
}

void handleFault()
{
  if (button1.pressed)
  {
    portENTER_CRITICAL(&timerMux);
    button1.pressed = false;
    portEXIT_CRITICAL(&timerMux);

    if (fault.fTimerStart > 0) //Already running counter
    {
      if ((esp_timer_get_time() - fault.fTimerStart) >= 4000000)
      { //end it after 4 s
        fault.fTimerStart = 0;
        Serial.println("Fault reset");
        lasttime = 0;
      }
      else
      {
        if ((esp_timer_get_time() - lasttime) >= 300000)
        {
          fault.code++;
          Serial.printf("Flash %d\n", fault.code);
          lasttime = esp_timer_get_time();
        }
      }
    }
    else
    {
      {
        //Not yet counting
        fault.fTimerStart = esp_timer_get_time(); //Start timer
        fault.code = 1;
        Serial.println("Flash 1");
        lasttime = fault.fTimerStart;
      }
    }
  }
  //Handle fault.code here
}
void handleFaultCode()
{
}

void resetFaultTimer()
{
  if (((esp_timer_get_time() - fault.fTimerStart) >= 4000000) && fault.fTimerStart != 0)
  { //end it after 4 s
    fault.fTimerStart = 0;
    Serial.println("Fault reset");
    if (fault.code > 0)
    {
      handleFaultCode();
    }
  }
}
long lastReconnectAttempt = 0;

boolean reconnect()
{
  if (client.connect(Settings.Host, Settings.ControllerUser, Settings.ControllerPassword))
  {
    // Once connected, publish an announcement...
    client.publish("outTopic", "hello world");
    // ... and resubscribe
    //client.subscribe("inTopic");
  }
  return client.connected();
}
// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  do
  {
    sensors.requestTemperatures();
    vTaskDelay(750 / (1 << (12 - RESOLUTION)));
    tempC = sensors.getTempCByIndex(0);
  } while ((tempC < -10) || (tempC > 50));

  // get temperature
  //Serial.println();
  if (tempC > -100)
  {
    Serial.print("Temperature: ");
    Serial.println(tempC);

    strcpy(pTopic, "");
    strcat(pTopic, Settings.Host);
    strcat(pTopic, "/tempc");
    snprintf(msg, 63, "%f", tempC);

    myreconnect();

    if (!client.connected())
    {
      long now = millis();
      if (now - lastReconnectAttempt > 5000)
      {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (reconnect())
        {
          lastReconnectAttempt = 0;
        }
      }
    }
    if (client.connect(Settings.Host, Settings.ControllerUser, Settings.ControllerPassword))
    {
      client.publish(pTopic, msg);
      /*
      Serial.print(Settings.Host);
      Serial.print(" : ");
      Serial.print(pTopic);
      Serial.print(" : ");
      Serial.println(msg);
      */
    }
  }
}

void runpins()
{
  digitalWrite(BUZZER, LOW);
  vTaskDelay(100);
  digitalWrite(BUZZER, HIGH);
  vTaskDelay(100);
  digitalWrite(BUZZER, LOW);
}

/*
 * Main function. It will request the tempC from the sensors and display on Serial.
 */
void loop(void)
{
  ArduinoOTA.handle();
  client.loop();

  if (thermoCounter > 0)
  {
    portENTER_CRITICAL(&timerMux);
    thermoCounter--;
    portEXIT_CRITICAL(&timerMux);

    do
    {
      printTemperature(insideThermometer); // Use a simple function to print out the data
    } while ((tempC < -10) && (tempC > 50));

    runled(compSpeed);

    runpins();
  }

  handleFault();

  resetFaultTimer();

  boot = false;
}