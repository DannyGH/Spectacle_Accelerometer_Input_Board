/****************************************************************************
main.c
Main code file for Spectacle Accelerometer input board project
Mike Hord @ SparkFun Electronics
24 Jan 2017
https://github.com/sparkfun/Spectacle_Accelerometer_Input_Board

This file includes main(), which configures the hardware for the system as
well as monitoring the data coming from the director board.

Development environment specifics:
Developed in PSoC Creator 4.0

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!
****************************************************************************/

#include <project.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "ADXL345.h"
#include "programming.h"
#include "spectacle.h"
#include "xl.h"

// systemTimer is incremented in the tickISR, which occurs once a millisecond.
//  It's the timebase for the entire firmware, upon which all other timing
//  is based.
volatile int32 systemTimer = 0;
CY_ISR_PROTO(tickISR);

// servo is a struct which tracks the desired behavior of a single channel. A
// single motor may have more than one servo struct associated with it, but
// only one servo struct should associate with a channel coming into the board.
// behaviors is the list of behaviors that the director board has passed into
// the servo board.
struct xl *behaviors;

// behaviorListLen is the variable which tracks the number of behaviors that
// the director board has passed into the servo board. During normal operation,
// we will iterate over the object 'behaviors' 'behaviorListLen' times every 10
// milliseconds.
int behaviorListLen = 0;

// mailboxes is where our channel data comes in. A spectacle system can have
// up to 64 channels of behaviors, each of which is an int16.
int16 mailboxes[128];

int16 scale(int16 N);
volatile uint8 *I2C_Mem;

#define I2C_BUFFER_SIZE 256
#define I2C_BUFFER_RW_BOUNDARY 256

int main()
{
  CyGlobalIntEnable; /* Enable global interrupts. */
  
  EZI2C_Start();
  EZI2C_EzI2CSetBuffer1(I2C_BUFFER_SIZE, I2C_BUFFER_RW_BOUNDARY, (uint8*)mailboxes);
  
  I2C_Start();
  
  UART_Start();
  
  CyIntSetSysVector((SysTick_IRQn + 16), tickISR);
  SysTick_Config(48000);

  behaviors = malloc(64*sizeof(struct xl));
  I2C_Mem = (uint8*)mailboxes;
  
  I2C_Mem[PROG_ENABLE_REG] = 0;
  I2C_Mem[PROG_READY_REG] = 0;
  I2C_Mem[DATA_READY_REG] = 0;
  I2C_Mem[BOARD_ID_REG] = BOARD_ID;

  int32 _100HzTick = 0;
  uint8 temp;
  int32 _2HzTick = 0;
  char buffer[64];
  int i = 0;
  
  int16 xMag;
  int16 yMag;
  int16 zMag;
  uint8 maxAxis = 0;
  uint8 active = 0;
  uint8 inactive = 1;
  UART_UartPutString("Hello world");
  configureXl();
  LED_Write(1);
/*
  behaviorListLen = 9;
  behaviors[0] = xlInit(0, ACTIVE, 1);
  behaviors[1] = xlInit(1, ACTIVE, 0);
  behaviors[2] = xlInit(2, WRITE_X, 0);
  behaviors[3] = xlInit(3, WRITE_Y, 0);
  behaviors[4] = xlInit(4, WRITE_Z, 0);
  behaviors[5] = xlInit(5, INACTIVE, 1);
  behaviors[6] = xlInit(6, INACTIVE, 0);
  behaviors[7] = xlInit(7, SIDE_A_UP, 1);
  behaviors[8] = xlInit(8, SIDE_A_UP, 0);
 */ 
  for(;;)
  {
    // One hundred times per second,
    if ((systemTimer - 10) > _100HzTick)
    {
      _100HzTick = systemTimer;
      if (I2C_Mem[PROG_ENABLE_REG] == 1)
      {
        program();
      }
      if (I2C_Mem[CONFIGURED_REG] == 1)
      {
        EZI2C_EzI2CSetAddress1(I2C_Mem[I2C_ADDR_REG]);
        I2C_Mem[CONFIGURED_REG] = 0;
        I2C_OUT_EN_Write(1);
      }
      if (INT1_Read() != 0)
      {
        //UART_UartPutString("INACTIVE\n");
        read8Bits(INT_SOURCE, &temp);
        active = 0;
        inactive = 1;
      }
      if (INT2_Read() != 0)
      {
        //UART_UartPutString("ACTIVE\n");
        read8Bits(INT_SOURCE, &temp);
        active = 1;
        inactive = 0;
      }
      readXYZ(&xMag, &yMag, &zMag);
      maxAxis = findMaxAxis(xMag, yMag, zMag);

      // behaviors loop: now that we've collected our data from the
      // accelerometer and parsed it into usable chunks, we have to determine
      // what to do with that information.
      for (i = 0; i < behaviorListLen; i++)
      {
        switch (behaviors[i].mode)
        {
          case WRITE_X:
          mailboxes[behaviors[i].channel] = scale(xMag);
          break;

          case WRITE_Y:
          mailboxes[behaviors[i].channel] = scale(yMag);
          break;
        
          case WRITE_Z:
          mailboxes[behaviors[i].channel] = scale(zMag);
          break;

          case ACTIVE:
          if (active == 0) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].active = 0;
          } // if (active == 0)
          else // active == 1
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].active == 0)
              {
                behaviors[i].active = 1;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else // local active flag is SET
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;

          case INACTIVE:
          if (inactive == 0) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].inactive = 0;
          } // if (active == 0)
          else // active == 1
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].inactive == 0)
              {
                behaviors[i].inactive = 1;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else // local active flag is SET
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;

          case SIDE_A_UP:
          if (maxAxis != Aup) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].orientation = maxAxis;
          }
          else// if (maxAxis == Aup)
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].orientation != Aup)
              {
                behaviors[i].orientation = Aup;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else 
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;
          
          case SIDE_B_UP:
          if (maxAxis != Bup) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].orientation = maxAxis;
          }
          else// if (maxAxis == Bup)
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].orientation != Bup)
              {
                behaviors[i].orientation = Bup;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else 
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;
          
          case SIDE_C_UP:
          if (maxAxis != Cup) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].orientation = maxAxis;
          }
          else// if (maxAxis == Cup)
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].orientation != Cup)
              {
                behaviors[i].orientation = Cup;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else 
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;
         
          case SIDE_D_UP:
          if (maxAxis != Dup) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].orientation = maxAxis;
          }
          else// if (maxAxis == Dup)
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].orientation != Dup)
              {
                behaviors[i].orientation = Dup;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else 
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;
          
          case SIDE_TOP_UP:
          if (maxAxis != Topup) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].orientation = maxAxis;
          }
          else// if (maxAxis == Topup)
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].orientation != Topup)
              {
                behaviors[i].orientation = Topup;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else 
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;
          
          case SIDE_BOTTOM_UP:
          if (maxAxis != Botup) 
          {
            mailboxes[behaviors[i].channel] = 0;
            behaviors[i].orientation = maxAxis;
          }
          else// if (maxAxis == Botup)
          {
            if (behaviors[i].momentary == 0) // continuous output desired
            {
              mailboxes[behaviors[i].channel] = 1000;
            }
            else // momentary output desired
            {
              if (behaviors[i].orientation != Botup)
              {
                behaviors[i].orientation = Botup;
                behaviors[i].xlTimer = systemTimer;
                mailboxes[behaviors[i].channel] = 1000;
              }
              else 
              {
                if (systemTimer - behaviors[i].xlTimer > 100)
                {
                  mailboxes[behaviors[i].channel] = 0;
                }
                else // less than 100ms have elapsed since output was set
                {
                  mailboxes[behaviors[i].channel] = 1000;
                }
              }
            }
          }
          break;          
        }


      }
      
    } // End of 100Hz loop
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

int16 scale(int16 N)
{
  if (N > 250) N = 250;
  if (N < -250) N = -250;
  return (N*2) + 500;
}

CY_ISR(tickISR)
{
  systemTimer++;
}



/* [] END OF FILE */
