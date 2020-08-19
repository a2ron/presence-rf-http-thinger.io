
int httpCode = -1;

int _leds()
{
  HTTPClient http;
  String url = LEDS_URL;
  httpCode = -1;

  if (http.begin(url))
  {
    httpCode = http.GET();

    if (httpCode > 0)
    {

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString();
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  else
  {
    Serial.printf("[HTTP} Unable to connect\n");
  }

  return httpCode;
}

void leds()
{
  int httpCode = _leds();
  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
  {
    thing.stream(thing["httpCode"]);
  }
  else
  {
    delay(2000);
    _leds();
    thing.stream(thing["httpCode"]);
  }
}

void _sendRF(RCSwitch mySwitch, int protocol, int pulseLength, int code, int repeatTransmit)
{

  if (protocol <= 0)
  {
    protocol = 1;
  }
  if (pulseLength <= 0)
  {
    pulseLength = 393;
  }
  mySwitch.setProtocol(protocol, pulseLength);
  if (repeatTransmit <= 0)
  {
    repeatTransmit = 10;
  }
  mySwitch.setRepeatTransmit(repeatTransmit);
  mySwitch.send(code, 24);
}

void sendRF(RCSwitch mySwitch, int code)
{
  _sendRF(mySwitch, 0, 0, code, 0);
}
