#include <project.h>
#include <stdio.h>
#include "xl.h"
#include "debug.h"

extern int behaviorListLen;
extern struct xl *behaviors;

void printBehaviors(void)

{
  char buffer[64];
  int i = 0;
  int listLen = behaviorListLen;
  if (listLen == 0) listLen = 5;
  sprintf(buffer, "Sizeof struct servo: %d\n", sizeof(struct xl));
  UART_UartPutString(buffer);
  sprintf(buffer, "location of behaviors: %p\n", behaviors);
  UART_UartPutString(buffer);
  for (i = 0; i < listLen; i++)
  {
    //sprintf(buffer, "Behavior %x: Ch %x, T %x, S %x, M %x\n", i, behaviors[i].channel,
    //        behaviors[i].threshold, behaviors[i].servoID, behaviors[i].mode);
    UART_UartPutString(buffer);
  }
}

