#ifndef __servo_h_
#define __servo_h_
#include <project.h>  
#include <stdbool.h>

#pragma pack(1)
struct xl
{
  uint8 channel;
  uint8 mode;
  uint8 momentary;
  uint8 orientation;
  uint8 active;
  uint8 inactive;
  int16 X;
  int16 Y;
  int16 Z;
  int32 xlTimer;
};
#pragma pack()

bool read16Bits(uint8 address, int16 *read);
bool read8Bits(uint8 address, uint8 *read);
bool write8Bits(uint8 address, uint8 write);
uint8 findMaxAxis(int16 x, int16 y, int16 z);
struct xl xlInit(uint8 channel, uint8 mode, uint8 momentary);

enum {ACTIVE, INACTIVE, SIDE_A_UP, SIDE_B_UP, SIDE_C_UP, SIDE_D_UP,
      SIDE_TOP_UP, SIDE_BOTTOM_UP, WRITE_X, WRITE_Y, WRITE_Z};

#endif
  
