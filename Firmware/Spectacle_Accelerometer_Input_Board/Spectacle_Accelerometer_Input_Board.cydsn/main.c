/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "ADXL345.h"

#define I2C_BUFFER_SIZE 37
#define I2C_BUFFER_RW_BOUNDARY 37

volatile int32 systemTimer = 0;

CY_ISR_PROTO(tickISR);

int main()
{
  CyGlobalIntEnable; /* Enable global interrupts. */
  
  uint8 registerSpace[I2C_BUFFER_SIZE];
  EZI2C_Start();
  EZI2C_EzI2CSetBuffer1(I2C_BUFFER_SIZE, I2C_BUFFER_RW_BOUNDARY, registerSpace);
  
  I2C_Start();
  
  UART_Start();
  
  CyIntSetSysVector((SysTick_IRQn + 16), tickISR);
  SysTick_Config(48000);
  
  int32 _100HzTick = 0;
  uint8 temp;
  int32 _2HzTick = 0;
  uint32 rxBuffer = 0;
  int i = 0;
  char buffer[64];
  
  int16 xMag = 0;
  int16 yMag = 0;
  int16 zMag = 0;
  int16 maxAxis;
  uint8 *active;
  active = &registerSpace[0];
  
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
      rxBuffer = UART_UartGetChar();
      if (rxBuffer == 'r')
      {
        for (i=0; i < I2C_BUFFER_SIZE; i++)
        {
          UART_UartPutChar(registerSpace[i]);
        }
      }
      if (INT1_Read() != 0)
      {
        UART_UartPutString("INACTIVE\n");
        read8Bits(INT_SOURCE, &temp);
        *active = 0;
      }
      if (INT2_Read() != 0)
      {
        UART_UartPutString("ACTIVE\n");
        read8Bits(INT_SOURCE, &temp);
        *active = 1;
      }
      readXYZ(&xMag, &yMag, &zMag);
      int16 xAbs = abs(xMag);
      int16 yAbs = abs(yMag);
      int16 zAbs = abs(zMag); 
      if (xAbs > yAbs)
      {
        if (zAbs > xAbs) 
        {
          if (zMag < 0) maxAxis = Botup;
          else          maxAxis = Topup;
        }
        else
        {
          if (xMag < 0) maxAxis = Dup;
          else          maxAxis = Bup;
        }
      }
      else
      {
        if (zAbs > yAbs) 
        {
          if (zMag < 0) maxAxis = Botup;
          else          maxAxis = Topup;
        }
        else
        {
          if (yMag < 0) maxAxis = Bup;
          else          maxAxis = Aup;
        }
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
      readXYZ(&xMag, &yMag, &zMag);
      float vectorMagnitude = sqrt((float)xMag*(float)xMag + 
                                   (float)yMag*(float)yMag +
                                   (float)zMag*(float)zMag);
      sprintf(buffer, "%d\n", (int)vectorMagnitude);
      UART_UartPutString(buffer);
      sprintf(buffer, "X: %d Y: %d Z: %d\n", xMag, yMag, zMag);
      UART_UartPutString(buffer);
      switch (maxAxis)
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

bool readXYZ(int16 *x, int16 *y, int16 *z)
{
  read16Bits(DATAX0, x);
  read16Bits(DATAY0, y);
  read16Bits(DATAZ0, z);
  return true;
}

bool read16Bits(int address, int16 *read)
{
  I2C_I2CMasterClearStatus();
  I2C_I2CMasterSendStart(ADXL345_ADDR, I2C_I2C_WRITE_XFER_MODE);
  I2C_I2CMasterWriteByte(address);
  I2C_I2CMasterSendStop();
  I2C_I2CMasterSendStart(ADXL345_ADDR, I2C_I2C_READ_XFER_MODE);
  *read = I2C_I2CMasterReadByte(I2C_I2C_ACK_DATA);
  *read |= I2C_I2CMasterReadByte(I2C_I2C_NAK_DATA)<< 8;
  I2C_I2CMasterSendStop();
  return true;
}

bool read8Bits(int address, uint8 *read)
{
  I2C_I2CMasterClearStatus();
  I2C_I2CMasterSendStart(ADXL345_ADDR, I2C_I2C_WRITE_XFER_MODE);
  I2C_I2CMasterWriteByte(address);
  I2C_I2CMasterSendStop();
  I2C_I2CMasterSendStart(ADXL345_ADDR, I2C_I2C_READ_XFER_MODE);
  *read = I2C_I2CMasterReadByte(I2C_I2C_NAK_DATA);
  I2C_I2CMasterSendStop();
  return true;
}

bool write8Bits(int address, uint8 write)
{
  I2C_I2CMasterClearStatus();
  bool retVal = false; // Assume a failed read.
  if ( I2C_I2CMasterSendStart(ADXL345_ADDR, I2C_I2C_WRITE_XFER_MODE) !=
       I2C_I2C_MSTR_NO_ERROR) return retVal;
  if ( I2C_I2CMasterWriteByte(address) != I2C_I2C_MSTR_NO_ERROR) return retVal;
  I2C_I2CMasterWriteByte(write);
  if ( I2C_I2CMasterSendStop() != I2C_I2C_MSTR_NO_ERROR) return retVal;
  return true;
}

CY_ISR(tickISR)
{
  systemTimer++;
}



/* [] END OF FILE */
