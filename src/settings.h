#include <Arduino.h>

struct SettingsStruct
{
  char WifiSSID[32];
  char WifiKey[64];
  char WifiAPKey[64];
  char ControllerUser[26];
  char ControllerPassword[64];
  char Host[26];
  char Broker[4];
  char BrokerIP[16];
  char Version[16];
} Settings;

/********************************************************************************************\
  Save data into config file on SPIFFS
  \*********************************************************************************************/
void SaveToFile(char *fname, int index, byte *memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToSave = memAddress;
    for (int x = 0; x < datasize; x++)
    {
      f.write(*pointerToByteToSave);
      pointerToByteToSave++;
    }
    f.close();
    String log = F("FILE : File saved");
  }
  else
  {
    Serial.println("Not Opened R+");
  }
}

/********************************************************************************************\
  Load data from config file on SPIFFS
  \*********************************************************************************************/
void LoadFromFile(char *fname, int index, byte *memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToRead = memAddress;
    for (int x = 0; x < datasize; x++)
    {
      *pointerToByteToRead = f.read();
      pointerToByteToRead++; // next byte
    }
    f.close();
  }
}

/********************************************************************************************\
  Save settings to SPIFFS
  \*********************************************************************************************/
void SaveSettings(void)
{
  SaveToFile((char *)"/config.txt", 0, (byte *)&Settings, sizeof(struct SettingsStruct));
  Serial.print("Setting Saved");
}

void ResetSettings()
{
  Serial.println(Settings.WifiSSID);
  Serial.println(ssid);
  strcpy(Settings.WifiSSID, ssid);
  strcpy(Settings.WifiKey, password);
  strcpy(Settings.Host, host);
  strcpy(Settings.ControllerUser, MQ_user);
  strcpy(Settings.ControllerPassword, MQ_pass);
  strcpy(Settings.Host, MQ_client);
  strcpy(Settings.BrokerIP, server);
  strcpy(Settings.Version, vers);

  Serial.print("In Settings");
  Serial.println(Settings.Version);
  Serial.println(vers);
  Serial.println(Settings.WifiSSID);
  Serial.println(Settings.WifiKey);
  Serial.println(Settings.Host);
  Serial.println(Settings.ControllerUser);
  Serial.println(Settings.ControllerPassword);
  Serial.println(Settings.BrokerIP);

  SaveSettings();

  //SPIFFS.end();
  //ESP.restart();
}

void fileSystemCheck()
{
  if (SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount successful");
    File f = SPIFFS.open("/config.txt", "r");
    if (!f)
    {
      Serial.println("formatting...");
      SPIFFS.format();
      Serial.println("format done!");
      File f = SPIFFS.open("/config.txt", "w");
      if (f)
      {
        for (int x = 0; x < 32768; x++)
          f.write(0);
        f.close();
        ResetSettings();
      }
    }
  }
  else
  {
    Serial.println("SPIFFS Mount failed");
  }
}

/********************************************************************************************\
  Load settings from SPIFFS
  \*********************************************************************************************/
void LoadSettings()
{
  LoadFromFile((char *)"/config.txt", 0, (byte *)&Settings, sizeof(struct SettingsStruct));
  Serial.print("Struct Size: ");
  Serial.println(sizeof(struct SettingsStruct));
  Serial.print("Load Settings");

  Serial.println(Settings.Version);
  Serial.println(vers);
  Serial.println(Settings.WifiSSID);
  Serial.println(Settings.WifiKey);
  Serial.println(Settings.Host);
  Serial.println(Settings.ControllerUser);
  Serial.println(Settings.ControllerPassword);
  Serial.println(Settings.BrokerIP);
}