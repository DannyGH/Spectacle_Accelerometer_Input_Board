#ifndef __spectacle_h__
#define __spectacle_h__

#define BOARD_ID        0x81 // unique identifier for this type of spectacle 
                             // board. The MSb signifies whether this is an
                             // input type board (1) or output type (0)
#define BOARD_ID_REG    0xff // I2C register holding the board ID.
#define CONFIGURED_REG  0xfd // register set by director when it has finished
                             // setting the I2C address for this board into
                             // the appropriate register, which is...
#define I2C_ADDR_REG    0xfe // ...this register.

#endif

