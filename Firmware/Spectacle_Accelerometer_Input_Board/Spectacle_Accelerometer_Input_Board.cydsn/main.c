#include <project.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "ADXL345.h"
#include "spectacle.h"

volatile int32 systemTimer = 0;

CY_ISR_PROTO(tickISR);

Board registerSpace;

int main()
{
  CyGlobalIntEnable; /* Enable global interrupts. */
  
  EZI2C_Start();
  EZI2C_EzI2CSetBuffer1(I2C_BUFFER_SIZE, I2C_BUFFER_RW_BOUNDARY, (uint8*)&registerSpace);
  
  I2C_Start();
  
  UART_Start();
  
  CyIntSetSysVector((SysTick_IRQn + 16), tickISR);
  SysTick_Config(48000);
  
  int32 _100HzTick = 0;
  uint8 temp;
  int32 _2HzTick = 0;
  char buffer[64];
  
  uint8 *boardID = 0;
  uint8 *boardType = 0;
  uint8 *change = 0;
  int16 *xMag = 0;
  int16 *yMag = 0;
  int16 *zMag = 0;
  uint8 *maxAxis = 0;
  int16 *magnitude = 0;
  uint8 *active = 0;
  boardID = &registerSpace.boardID;
  boardType = &registerSpace.type;
  change = &registerSpace.change;
  active = &registerSpace.data[0];
  maxAxis = &registerSpace.data[1];
  magnitude = (int16*)&registerSpace.data[2];
  xMag = (int16*)&registerSpace.data[4];
  yMag = (int16*)&registerSpace.data[6];
  zMag = (int16*)&registerSpace.data[8];

  *boardID = 0xa3;
  *boardType = 1;
  *change = 0;
  *active = 0;
  *maxAxis = 0;
  *magnitude = 0;
  *xMag = 0;
  *yMag = 0;
  *zMag = 0;

  
  UART_UartPutString("Hello world");
  write8Bits(POWER_CTL, 0x08);
  write8Bits(DATA_FORMAT, 0x00);
  write8Bits(THRESH_ACT, 8);
  write8Bits(THRESH_INACT, 8);
  write8Bits(TIME_INACT, 3);
  write8Bits(ACT_INACT_CTL, 0xff); 
  write8Bits(INT_MAP, 0x10);
  write8Bits(INT_ENABLE, 0x18);
  LED_Write(1);
  
  for(;;)
  {
    // One hundred times per second,
    if ((systemTimer - 10) > _100HzTick)
    {
      _100HzTick = systemTimer;
      if (INT1_Read() != 0)
      {
        //UART_UartPutString("INACTIVE\n");
        read8Bits(INT_SOURCE, &temp);
        *active = 0;
      }
      if (INT2_Read() != 0)
      {
        //UART_UartPutString("ACTIVE\n");
        read8Bits(INT_SOURCE, &temp);
        *active = 1;
      }
      readXYZ(xMag, yMag, zMag);
      int16 xAbs = abs(*xMag);
      int16 yAbs = abs(*yMag);
      int16 zAbs = abs(*zMag); 
      uint8 oldMaxAxis = *maxAxis;
      if (xAbs > yAbs)
      {
        if (zAbs > xAbs) 
        {
          if (zMag < 0) *maxAxis = Botup;
          else          *maxAxis = Topup;
        }
        else
        {
          if (xMag < 0) *maxAxis = Dup;
          else          *maxAxis = Bup;
        }
      }
      else
      {
        if (zAbs > yAbs) 
        {
          if (zMag < 0) *maxAxis = Botup;
          else          *maxAxis = Topup;
        }
        else
        {
          if (yMag < 0) *maxAxis = Bup;
          else          *maxAxis = Aup;
        }
      }
      if ((~oldMaxAxis & *maxAxis) != 0)
      {
        *change = 1;
      }
    }
    // One Hz blinking LED heartbeat. Will probably be removed from production
    //  code.
    if ((systemTimer - 500) > _2HzTick)
    {
      _2HzTick = systemTimer;
      if (LED_Read() != 0)
      {
        LED_Write(0);
      }
      else
      {
        LED_Write(1);
      }
      readXYZ(xMag, yMag, zMag);
      float vectorMagnitude = sqrt((float)*xMag*(float)*xMag + 
                                   (float)*yMag*(float)*yMag +
                                   (float)*zMag*(float)*zMag);
      *magnitude = (int)vectorMagnitude;
      sprintf(buffer, "%d\n", (int)vectorMagnitude);
      UART_UartPutString(buffer);
      sprintf(buffer, "X: %d Y: %d Z: %d\n", *xMag, *yMag, *zMag);
      UART_UartPutString(buffer);
      switch (*maxAxis)
      {
        case Aup:
          sprintf(buffer, "A edge is up\n");
          break;
        case Bup:
          sprintf(buffer, "B edge is up\n");
          break;
        case Cup:
          sprintf(buffer, "C edge is up\n");
          break;
        case Dup:
          sprintf(buffer, "D edge is up\n");
          break;
        case Topup:
          sprintf(buffer, "Top face is up\n");
          break;
        case Botup:
          sprintf(buffer, "Bottom face is up\n");
          break;
      }
      UART_UartPutString(buffer);
    }
  }
}

void readXYZ(int16 *x, int16 *y, int16 *z)
{
  read16Bits(DATAX0, x);
  read16Bits(DATAY0, y);
  read16Bits(DATAZ0, z);
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

CY_ISR(tickISR)
{
  systemTimer++;
}



/* [] END OF FILE */
