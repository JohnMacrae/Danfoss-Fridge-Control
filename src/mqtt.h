#include <Arduino.h>

bool boot = true;

struct MQMessage
{
  char topic[32];
  char message[32];
};

//char msg[32] = {0};

void callback(char *topic, byte *payload, unsigned int length)
{
  //int pinstate = 1;
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i = 0;
  char buf[80] = {0};
  for (i = 0; i < length; i++)
  {
    buf[i] = payload[i];
  }
  buf[i] = '\0';

  if (strcmp(topic, "suspend") == 0)
  {
    //suspendTimer = millis() + 60 * 5 * 1000;
  }
}

WiFiClient ethClient;
PubSubClient client(Settings.BrokerIP, 1883, callback, ethClient);

void myreconnect()
{
  // Loop until we're reconnected
  int count = 0;
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(Settings.WifiSSID, Settings.WifiKey);
    Serial.println("Reconnecting: ");
    Serial.println(Settings.WifiSSID);
    Serial.println(Settings.WifiKey);
    Serial.println("");

    while ((count < 5) && (WiFi.status() != WL_CONNECTED))
    {

      Serial.print(WiFi.status());
      Serial.println(" - Retrying connection...");
      vTaskDelay(500);
      count++;
    }

    if (WiFi.status() == WL_CONNECTED)
      Serial.println("Reconnected WiFi");
    {
      count = 0;
      while (!client.connected() && (count < 5))
      {

        if (client.connect(Settings.Host, Settings.ControllerUser, Settings.ControllerPassword))
        {
          if (boot)
          {
            client.publish("fridge/mqstatus", "1");
          }
          else
          {
            client.publish("fridge/mqstatus", "0");
          }
        }
          else
          {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            delay(500);
          }
          count++;
        }
      }
    }
    else
    { // WiFi IS Connected
      if (client.connect(Settings.Host, Settings.ControllerUser, Settings.ControllerPassword))
      {
        if(boot)
          {
          client.publish("fridge/mqstatus", "1");
        }else{
          client.publish("fridge/mqstatus", "0");
        }
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        // Serial.println(" try again in 1 second");
        // Wait a second before retrying
        delay(500);
      }
    }
  }

  void MQ_Publish(char *mytopic, char *mymsg)
  {
    char pTopic[64] = {0};
    strcat(pTopic, Settings.Host);
    strcat(pTopic, "/");
    strcat(pTopic, mytopic);
    //strcpy(mqMessage.message, mymsg);
    /*
  client.publish(pTopic, mymsg);
  Serial.println(pTopic);
  Serial.println(" : ");
  Serial.println(mymsg);
  */
  }

  void PrintSettings()
  {
    Serial.println("Printing...");
    char pTopic[30] = {0};
    strcat(pTopic, Settings.Host);
    strcat(pTopic, "/");
    strcat(pTopic, "outTopic");
    if (client.connect(host, MQ_user, MQ_pass))
    {
      client.publish(pTopic, Settings.WifiSSID);
      client.publish(pTopic, Settings.ControllerUser);
      client.publish(pTopic, Settings.Host);
      client.publish(pTopic, Settings.BrokerIP);
      client.publish(pTopic, Settings.Version);
      Serial.print(pTopic);
    }
  }
