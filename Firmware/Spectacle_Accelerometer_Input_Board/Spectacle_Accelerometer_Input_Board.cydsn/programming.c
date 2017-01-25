/****************************************************************************
programming.c
Receiving function from director board
Mike Hord @ SparkFun Electronics
24 Jan 2017
https://github.com/sparkfun/Spectacle_Accelerometer_Input_Board

Contains the function which receives configuration data from the director board
and stores it away for later use.

Development environment specifics:
Developed in PSoC Creator 4.0

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!
****************************************************************************/

#include <project.h>
#include <stdlib.h>
#include <stdio.h>
#include "programming.h"
#include "xl.h"

extern int behaviorListLen;
extern struct xl *behaviors;
extern volatile uint8 *I2C_Mem;

void program(void)
{
  uint8 i = 0;
  behaviorListLen = 0;
  I2C_Mem[DATA_READY_REG] = 0;
  I2C_Mem[PROG_READY_REG] = 1;
  // Loop here, waiting for data to come in from the director board.
  while (I2C_Mem[PROG_ENABLE_REG] == 1)
  {
    // This flag will be set by the director board when it has finished
    // loading the next data struct into the I2C memory for capture by this
    // board.
    while (I2C_Mem[DATA_READY_REG] == 0)
    {
      // While we're waiting, we want to make sure we don't miss the director
      // board telling us that it's done sending us packets. It does that by
      // clearing the PROG_ENABLE_REG.
      if (I2C_Mem[PROG_ENABLE_REG] == 0)
      {
        // Clear PROG_READ_REG, so we don't accidentally tell the director
        // board that we're ready to receive when we aren't, then return.
        I2C_Mem[PROG_READY_REG] = 0;
        return;
      }
    }

    // At this point, the director board has loaded a new struct into the
    // memory, and we can copy that struct into memory at a location determined
    // by the number of behaviors already in the list.
    // We can't use memcpy to copy our I2C data into behaviors, as memcpy will
    // discard the volatile qualifier and may fetch (bad) data from the cache.
    // We therefore must write our own copying method.
    for (i = 0; i < USER_DATA_LEN; ++i)
    {
      *((uint8_t*)(&behaviors[behaviorListLen])+i) =
              I2C_Mem[i+128];
    }     
    for (i = USER_DATA_LEN; i < sizeof(struct xl); ++i)
    {
      *((uint8_t*)(&behaviors[behaviorListLen])+i) = 0;
    } 
    behaviorListLen++;
    I2C_Mem[DATA_READY_REG] = 0;
  }
}

