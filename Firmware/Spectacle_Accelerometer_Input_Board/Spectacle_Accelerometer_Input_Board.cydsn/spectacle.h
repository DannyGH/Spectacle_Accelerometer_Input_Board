#ifndef __spectacle_h__
#define __spectacle_h__

typedef struct
{
  uint8 boardID;
  uint8 type;
  uint8 change;
  uint8 data[32];
} Board;

#define I2C_BUFFER_SIZE 36
#define I2C_BUFFER_RW_BOUNDARY 36

#endif

