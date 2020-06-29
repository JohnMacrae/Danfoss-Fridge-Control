#include <Arduino.h>
#include <ArduinoJson.h>

const bool defaultsettings = true;

struct SettingsStruct
{
    char WifiSSID[32];
    char WifiKey[64];
    char ControllerUser[26];
    char ControllerPassword[64];
    char Host[16];
    char Broker[16];
    char BrokerIP[16];
    char Version[16];
} Settings;

// Prints the content of a file to the Serial
void printFile(const char *filename)
{
    // Open file for reading
    File file = SPIFFS.open(filename);
    if (!file)
    {
        Serial.println(F("Failed to read file"));
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        //if(DEBUGLEVEL >0){
        Serial.print((char)file.read());
        //}
    }
    Serial.println();

    // Close the file (File's destructor doesn't close the file)
    file.close();
}

bool saveConfig()
{
    //  StaticJsonBuffer<600> jsonBuffer;
    //  JsonObject &json = jsonBuffer.createObject();

    StaticJsonDocument<600> json;

    Serial.print("--- Saving version: ");
    Serial.print(vers);
    Serial.println(" ---");

    Serial.println(ssid);
    Serial.println(password);
    Serial.println(MQ_user);
    Serial.println(MQ_pass);
    Serial.println(host);
    Serial.println(MQ_server);
    Serial.println(MQ_client);
    Serial.println(vers);
    json["WifiSSID"] = ssid;
    json["WifiKey"] = password;
    json["ControllerUser"] = MQ_user;
    json["ControllerPassword"] = MQ_pass;
    json["Host"] = host;
    json["BrokerIP"] = MQ_server;
    json["Version"] = vers;
    json["Broker"] = MQ_client;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    serializeJson(json, configFile);
    configFile.close();
    Serial.println("Saved Vesion");
    printFile("/config.json");

    return true;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }

    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

bool loadConfig()
{
    if (defaultsettings)
    {
        listDir(SPIFFS, "/", 0);
        SPIFFS.remove("/config.json");
        Serial.println("File Deleted");
        listDir(SPIFFS, "/", 0);
        saveConfig();
        //defaultsettings = false;
    }

    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file for reading");
        return false;
    }
    Serial.println("Loading Version");
    //printFile("/config.json");
    /*    size_t size = configFile.size();
    if (size > 1024)
    {
        Serial.println("Config file size is too large");
        return false;
    }
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);
*/
    StaticJsonDocument<600> json;
    //    JsonObject json = jsonBuffer.parseObject(buf.get());

    auto error = deserializeJson(json, configFile);
    if (error)
    {
        Serial.print(F("Failed to parse config file with code "));
        Serial.println(error.c_str());
        return false;
    }
    else
    {

        strlcpy(Settings.WifiSSID, json["WifiSSID"] | ssid, sizeof(Settings.WifiSSID));                                  // <- destination's capacity
        strlcpy(Settings.WifiKey, json["WifiKey"] | password, sizeof(Settings.WifiKey));                                 // <- destination's capacity
        strlcpy(Settings.ControllerUser, json["ControllerUser"] | MQ_user, sizeof(Settings.ControllerUser));             // <- destination's capacity
        strlcpy(Settings.ControllerPassword, json["ControllerPassword"] | MQ_pass, sizeof(Settings.ControllerPassword)); // <- destination's capacity
        strlcpy(Settings.Broker, json["Broker"] | MQ_client, sizeof(Settings.Broker));                                   // <- destination's capacity
        strlcpy(Settings.BrokerIP, json["BrokerIP"] | MQ_server, sizeof(Settings.BrokerIP));                             // <- destination's capacity
        strlcpy(Settings.Host, json["Host"] | host, sizeof(Settings.Host));                                              // <- destination's capacity
        strlcpy(Settings.Version, json["Version"] | vers, sizeof(Settings.Version));                                     // <- destination's capacity

        Serial.println("");
        Serial.println("--- Loading Config ---");
        Serial.println(Settings.Version);
        Serial.println(Settings.WifiSSID);
        Serial.println(Settings.WifiKey);
        Serial.println(Settings.ControllerUser);
        Serial.println(Settings.ControllerPassword);
        Serial.println(Settings.Host);
        Serial.println(Settings.Broker);
        Serial.println(Settings.BrokerIP);
        Serial.println("");
        return true;
    }
}

void fileSystemCheck()
{
    if (SPIFFS.begin())

    {
        Serial.println("SPIFFS Mount successful");
        File f = SPIFFS.open("/config.json", "r");
        if (!f)
        {
            Serial.println("formatting...");
            SPIFFS.format();
            Serial.println("format done!");
            /*
      File f = SPIFFS.open("/config.txt", "w");
      if (f)
      {
        for (int x = 0; x < 32768; x++)
          f.write(0);
        f.close();
        //ResetSettings();
      }
      */
        }
    }
    else
    {
        Serial.println("SPIFFS Mount failed");
        SPIFFS.format();
        Serial.println("SPIFFS Formatted");
    }
}
