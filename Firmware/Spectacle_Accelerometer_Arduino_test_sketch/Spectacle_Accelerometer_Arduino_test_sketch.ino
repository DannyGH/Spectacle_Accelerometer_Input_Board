#include <Wire.h>

void setup() 
{
  pinMode(7, OUTPUT);
  Wire.begin();
}

void loop() 
{
  if (isActive())
  {
    digitalWrite(7, HIGH);
  }
  else
  {
    digitalWrite(7, LOW);
  }
}

boolean isActive()
{
  Wire.beginTransmission(0x09);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(0x09, 1);
  if (Wire.read())
  {
    return true;
  }
  else
  {
    return false;
  }
}

