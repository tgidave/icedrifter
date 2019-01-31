#include <SoftwareSerial.h>

SoftwareSerial schain(2, 3);

#define CHAIN_POWER 1

//char buffer[1024];

int charCount;

void setup()
{

  Serial.begin(115200);
  schain.begin(9600);

  pinMode(CHAIN_POWER, OUTPUT);
  digitalWrite(CHAIN_POWER, HIGH);
  delay(1000);

  schain.listen();

  Serial.print("Chain test.\r\n");

  //schain.print("+1::debug=1\n");

  schain.print("+1::measure\n");
  //delay(1000);

  //schain.print("+1::light\n");
  /*
  charCount = 0;

  while (schain.available() > 0) {
    char inByte = schain.read();
    Serial.write(inByte);
    ++charCount;
    if (inByte == '\n') {
      break;
    }
  }

  Serial.print("\r\nTotal characters read = \r\n");
  Serial.print(charCount);
  Serial.print("\r\n");
  */
}

void loop()
{

  if (Serial.available()) {
    char c = Serial.read();
    schain.write(c);
  }
  if (schain.available()) {
    char c = schain.read();
    Serial.write(c);
  }
}


