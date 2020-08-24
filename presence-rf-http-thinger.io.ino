
#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>

#include <RCSwitch.h>
#include <ESP8266HTTPClient.h>

#include "define.ignore.h"

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);
RCSwitch mySwitch = RCSwitch();

#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 2 * 3600);

#include "lib.h"

// Global Vars
int presence = 0;
unsigned long int t_turnOff = 0;
unsigned long int nextStream = 0;
unsigned long int nextCheck = 0;

// Main functions
void setup()
{
  Serial.begin(serialRate);
  pinMode(PIN_PRESENCE, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  mySwitch.enableTransmit(PIN_RF);
  thing.add_wifi(SSID, SSID_PASSWORD);

  thing["action"] << [](pson &in) {
    _sendRF(mySwitch, (int)in["protocol"], (int)in["pulseLength"], (int)in["code"], (int)in["repeatTransmit"]);
  };

  thing["state"] << [](pson &request) {
    if (!request.is_empty())
    {
      digitalWrite(BUILTIN_LED, LOW);
      String powerState = request["powerState"];
      String endpointId = request["endpointId"];
      int percentageState = request["brightness"];

      if (endpointId == "a2-nightstands")
      {
        if (powerState == "ON")
        {
          sendRF(mySwitch, 11767553); //nightstand lamps ON
          if (percentageState < 25)
          {
            sendRF(mySwitch, 11767562);
          }
          else if (percentageState < 50)
          {
            sendRF(mySwitch, 11767563);
          }
          else
          {
            sendRF(mySwitch, 11767564);
          }
          t_turnOff = millis() + STATE_REQUEST_DURATION;
        }
        else if (powerState == "OFF")
        {
          sendRF(mySwitch, 11767555); //nightstand lamps OFF
        }
      }
      digitalWrite(BUILTIN_LED, HIGH);
    };
  };

  thing["presence"] >> [](pson &out) {
    out = (t_turnOff > 0 || presence > 0) ? 1 : -1;
  };
  thing["httpCode"] >> [](pson &out) {
    out = httpCode;
  };
  timeClient.begin();
  Serial.println("\nWorking...");
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);
  delay(250);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop()
{
  timeClient.update();
  delay(100);
  thing.handle();
  presence = digitalRead(PIN_PRESENCE);

  //presence detection
  if (presence)
  {
    char dayMode = timeClient.getHours() < NIGHT_HOUR_INIT && timeClient.getHours() > NIGHT_HOUR_END;
    //it is off, turning on
    if (t_turnOff == 0)
    {
      digitalWrite(BUILTIN_LED, LOW);
      leds(); //switch leds on
      if (dayMode)
      {
        sendRF(mySwitch, 11767553); //nightstand lamps ON
      }
      digitalWrite(BUILTIN_LED, HIGH);
    }
    //setting turning off time
    unsigned long int _t_turnOff = dayMode ? millis() + TIME_ON_DAY : millis() + TIME_ON_NIGHT;
    if (_t_turnOff > t_turnOff)
    {
      t_turnOff = _t_turnOff;
    }
  }
  //turning off time out
  if (t_turnOff > 0 && t_turnOff < millis())
  {
    t_turnOff = 0;
    sendRF(mySwitch, 11767555); //nightstand lamps OFF
    leds();                     //switch leds off
  }

  //streaming
  if (millis() > nextStream)
  {
    thing.stream(thing["presence"]);
    nextStream = millis() + STREAMING_TIME;
  }
}
