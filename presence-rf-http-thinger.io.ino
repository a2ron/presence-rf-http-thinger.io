
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
  digitalWrite(BUILTIN_LED, LOW);
  mySwitch.enableTransmit(PIN_RF);
  thing.add_wifi(SSID, SSID_PASSWORD);

  thing["action"] << [](pson &in) {
    _sendRF(mySwitch, (int)in["protocol"], (int)in["pulseLength"], (int)in["code"], (int)in["repeatTransmit"]);
  };

  thing["presence"] >> [](pson &out) {
    out = (t_turnOff > 0 || presence > 0) ? 1 : -1;
  };
  thing["httpCode"] >> [](pson &out) {
    out = httpCode;
  };
  timeClient.begin();
  digitalWrite(BUILTIN_LED, LOW);
  Serial.println("\nWorking...");
  delay(2000);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop()
{
  timeClient.update();
  delay(200);
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
    }
    //setting turning off time
    if (dayMode)
    {
      sendRF(mySwitch, 11767553); //nightstand lamps ON
      t_turnOff = millis() + TIME_ON_DAY;
    }
    else
    {
      t_turnOff = millis() + TIME_ON_NIGHT;
    }
  }
  //turning off time out
  if (t_turnOff > 0 && t_turnOff < millis())
  {
    digitalWrite(BUILTIN_LED, HIGH);
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
