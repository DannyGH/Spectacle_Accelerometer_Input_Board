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
