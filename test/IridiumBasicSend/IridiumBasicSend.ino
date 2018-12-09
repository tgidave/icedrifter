#include <IridiumSBD.h>
#include <SoftwareSerial.h>

SoftwareSerial nss(12, 13);
IridiumSBD isbd(nss, 44);
static const int ledPin = 18;
static const int ROCKBLOCK_POWER = 15;

void setup() 
{
  int signalQuality = -1;

  pinMode(ledPin, OUTPUT);
  
  pinMode(ROCKBLOCK_POWER, OUTPUT);
  digitalWrite(ROCKBLOCK_POWER, HIGH);
  delay(1000);

  Serial.begin(115200);
  nss.begin(19200);

  isbd.attachConsole(Serial);
  isbd.setPowerProfile(1);
  isbd.begin();

  int err = isbd.getSignalQuality(signalQuality);
  if (err != 0)
  {
    Serial.print("SignalQuality failed: error ");
    Serial.println(err);
    return;
  }

  Serial.print("Signal quality is ");
  Serial.println(signalQuality);

  err = isbd.sendSBDText("Hello, world!");
  if (err != 0)
  {
    Serial.print("sendSBDText failed: error ");
    Serial.println(err);
    return;
  }

  Serial.println("Hey, it worked!");
  Serial.print("Messages left: ");
  Serial.println(isbd.getWaitingMessageCount());
}

void loop()
{
   digitalWrite(ledPin, HIGH);
}

bool ISBDCallback()
{
   digitalWrite(ledPin, (millis() / 1000) % 2 == 1 ? HIGH : LOW);
   return true;
}
