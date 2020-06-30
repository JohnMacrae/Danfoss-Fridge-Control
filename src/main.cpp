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
#include "thermoStat.h"
#include "pwm.h"

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
  uint32_t numberKeyPresses;
  int64_t fTimerStart;
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
  if (fault.numberKeyPresses == 0)
  {
    fault.fTimerStart = esp_timer_get_time();
  }

  interruptCounter++;
  fault.numberKeyPresses++;

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

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onFaultTimer, true);
  timerAlarmWrite(timer, 4000000, true);
  timerAlarmEnable(timer);

  thermoTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(thermoTimer, &onThermoTimer, true);
  timerAlarmWrite(thermoTimer, 10000000, true);
  timerAlarmEnable(thermoTimer);

  setPins();
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

  //myreconnect();

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
  sensors.setResolution(insideThermometer, 12);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();
  sensors.requestTemperatures(); // Send the command to get temperatures
  vTaskDelay(300);
  float tempC = -99;
  tempC = sensors.getTempC(insideThermometer);
  Serial.println(tempC);
  setupPWM();
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  vTaskDelay(100);
  float tempC = -99;
  tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.println(tempC);

  Serial.println(sensors.getTempC(deviceAddress));

  strcpy(pTopic, "");
  strcat(pTopic, Settings.Host);
  strcat(pTopic, "/tempc");
  snprintf(msg, 63, "%f", tempC);

  if (client.connect(Settings.Host, Settings.ControllerUser, Settings.ControllerPassword))
  {
    client.publish(pTopic, msg);
  }
}
/*
 * Main function. It will request the tempC from the sensors and display on Serial.
 */
void loop(void)
{
  ArduinoOTA.handle();

  if (thermoCounter > 0)
  {
    portENTER_CRITICAL(&timerMux);
    thermoCounter--;
    portEXIT_CRITICAL(&timerMux);
    printTemperature(insideThermometer); // Use a simple function to print out the data
    myreconnect();
  }

  if (interruptCounter > 0)
  {
    uint32_t diff = esp_timer_get_time() - fault.fTimerStart;

    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

    if (fault.numberKeyPresses == 4)
    {
      fault.numberKeyPresses = 0;
    }
  }

  runled();
}