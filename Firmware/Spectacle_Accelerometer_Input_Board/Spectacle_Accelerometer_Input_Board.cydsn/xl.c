#include <project.h>
#include <stdlib.h>
#include "ADXL345.h"
#include "xl.h"

extern volatile int32 systemTimer;

extern int16 mailboxes[64];

uint8 findMaxAxis(int16 x, int16 y, int16 z)
{
  int16 xAbs = abs(x);
  int16 yAbs = abs(y);
  int16 zAbs = abs(z); 

  if (xAbs > yAbs)
  {
    if (zAbs > xAbs) 
    {
      if (z < 0) return Botup;
      else       return Topup;
    }
    else
    {
      if (x < 0) return Dup;
      else       return Cup;
    }
  }
  else
  {
    if (zAbs > yAbs) 
    {
      if (z < 0) return Botup;
      else        return Topup;
    }
    else
    {
      if (y < 0) return Bup;
      else       return Aup;
    }
  }
}

struct xl xlInit(uint8 channel, uint8 mode, uint8 momentary)
{
  struct xl temp = {channel, mode, momentary, 0, 0, 0, 0, 0, 0, 0};
  return temp;
}

bool read16Bits(uint8 address, int16 *read)
{
  I2C_I2CMasterWriteBuf(ADXL345_ADDR, &address, 1, I2C_I2C_MODE_COMPLETE_XFER);
  while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
  I2C_I2CMasterReadBuf(ADXL345_ADDR, (uint8*)read, 2, I2C_I2C_MODE_COMPLETE_XFER);
  while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
  return true;
}

bool read8Bits(uint8 address, uint8 *read)
{
  I2C_I2CMasterWriteBuf(ADXL345_ADDR, &address, 1, I2C_I2C_MODE_COMPLETE_XFER);
  while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
  I2C_I2CMasterReadBuf(ADXL345_ADDR, (uint8*)read, 1, I2C_I2C_MODE_COMPLETE_XFER);
  while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
  return true;
}

bool write8Bits(uint8 address, uint8 write)
{

  uint8 temp[2];
  temp[0] = address;
  temp[1] = write;
  I2C_I2CMasterWriteBuf(ADXL345_ADDR, temp, 2, I2C_I2C_MODE_COMPLETE_XFER);
  while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
  return true;
}
