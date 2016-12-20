#include <Wire.h>

#define PROG_ENABLE_REG 200
#define PROG_READY_REG  201
#define DATA_READY_REG  202
#define INC_STRUCT_TOP  128

enum {ACTIVE, INACTIVE, SIDE_A_UP, SIDE_B_UP, SIDE_C_UP, SIDE_D_UP,
      SIDE_TOP_UP, SIDE_BOTTOM_UP, WRITE_X, WRITE_Y, WRITE_Z};

void setup() 
{
  pinMode(7, OUTPUT);
  Wire.begin();
  Serial.begin(115200);
  // Teach the daughter board about the behaviors we want it to use.
  sendByte(PROG_ENABLE_REG, 1);
  Serial.println("Prog enable!");
  while (progReady() == 0);
  Serial.println("Prog ready!");
  xlInit(0, ACTIVE, 1);
  xlInit(1, ACTIVE, 0);
  xlInit(2, WRITE_X, 0);
  xlInit(3, WRITE_Y, 0);
  xlInit(4, WRITE_Z, 0);
  xlInit(5, INACTIVE, 1);
  xlInit(6, INACTIVE, 0);
  xlInit(7, SIDE_A_UP, 1);
  xlInit(8, SIDE_A_UP, 0);
  sendByte(PROG_ENABLE_REG, 0);
}

void loop() 
{
  if (getMail(7) > 500)
  {
    digitalWrite(7, HIGH);
  }
  else
  {
    digitalWrite(7, LOW);
  }
}

int getMail(int address)
{
  Wire.beginTransmission(0x09);
  Wire.write(address * 2);
  Wire.endTransmission();
  Wire.requestFrom(0x09, 2);
  while (Wire.available() < 2);
  int temp = Wire.read();
  temp |= Wire.read()<<8;
  Serial.println(temp);
  return temp;
}

void sendCmd(byte offset, uint16_t value)
{
  Wire.beginTransmission(0x09);
  Wire.write(2*offset);
  Wire.write(value);
  Wire.write(value>>8);
  Wire.endTransmission();
} 

void sendByte(byte offset, byte value)
{
  Wire.beginTransmission(0x09);
  Wire.write(offset);
  Wire.write(value);
  Wire.endTransmission();
} 

void sendWord(byte offset, uint16_t value)
{
  Wire.beginTransmission(0x09);
  Wire.write(offset);
  Wire.write(value);
  Wire.write(value>>8);
  Wire.endTransmission();
}

void sendLWord(byte offset, uint32_t value)
{
  Wire.beginTransmission(0x09);
  Wire.write(offset);
  Wire.write(value);
  Wire.write(value>>8);
  Wire.write(value>>16);
  Wire.write(value>>24);
  Wire.endTransmission();
}

byte progReady()
{
  byte resp = 0;
  Wire.beginTransmission(0x09);
  Wire.write(PROG_READY_REG);
  Wire.endTransmission();
  Wire.requestFrom(0x09, 1);
  while (Wire.available() == 0);
  return Wire.read();
}

byte dataAccepted()
{
  Wire.beginTransmission(0x09);
  Wire.write(DATA_READY_REG);
  Wire.endTransmission();
  Wire.requestFrom(0x09, 1);
  while (Wire.available() == 0);
  return Wire.read();
}


void xlInit(uint8_t channel, uint8_t mode, uint8_t momentary)
{
  sendByte(INC_STRUCT_TOP, channel);
  sendWord(INC_STRUCT_TOP + 1, mode);
  sendByte(INC_STRUCT_TOP + 2, momentary);
  sendByte(INC_STRUCT_TOP + 3, 0);
  sendWord(INC_STRUCT_TOP + 4, 0);
  sendWord(INC_STRUCT_TOP + 5, 0);
  sendLWord(INC_STRUCT_TOP + 6, 0);
  sendLWord(INC_STRUCT_TOP + 8, 0);
  sendLWord(INC_STRUCT_TOP + 10, 0);
  sendLWord(INC_STRUCT_TOP + 12, 0);
  
  sendByte(DATA_READY_REG, 1);
  while (dataAccepted() == 1);
}

