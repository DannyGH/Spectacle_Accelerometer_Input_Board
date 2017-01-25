/****************************************************************************
ADXL345.c
Functions specific to the ADXL345
Mike Hord @ SparkFun Electronics
24 Jan 2017
https://github.com/sparkfun/Spectacle_Accelerometer_Input_Board

This file contains functions specific to the ADXL345. The accelerometer
used is fairly easy to change, as only the ADXL345.h and .c files need to be
changed.

Development environment specifics:
Developed in PSoC Creator 4.0

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!
****************************************************************************/

#include <project.h>
#include "ADXL345.h"
#include "xl.h"

void readXYZ(int16 *x, int16 *y, int16 *z)
{
  read16Bits(DATAX0, x);
  read16Bits(DATAY0, y);
  read16Bits(DATAZ0, z);
}

void configureXl(void)
{
  write8Bits(POWER_CTL, 0x08);   // Enable the chip.
  write8Bits(DATA_FORMAT, 0x00); // 2g, interrupts active high, right justified
                                 //  data, no self test 
  write8Bits(THRESH_ACT, 4);     // Threshold for detecting activity, in units
                                 //  62.5mg/LSB
  write8Bits(THRESH_INACT, 8);   // Threshold for detecting inactivity, in 
                                 //  units 62.5mg/LSB
  write8Bits(TIME_INACT, 3);     // Time device must be still for inactive int
                                 //  to trip, in sec/LSB
  write8Bits(ACT_INACT_CTL, 0xff);// AC activity/inactivity on all axes 
  write8Bits(INT_MAP, 0x10);     // Send interrupt trigger to INT1 or INT2.
                                 //  Here we send activity interrupt to INT2
                                 //  and inactivity to INT1.
  write8Bits(INT_ENABLE, 0x18);  // Enable activity and inactivity interrupts
}
