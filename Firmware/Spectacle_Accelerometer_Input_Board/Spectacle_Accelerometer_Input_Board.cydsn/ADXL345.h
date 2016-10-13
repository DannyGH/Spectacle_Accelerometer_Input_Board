// #ifndef __adxl345_h__
// #define __adxl345_h__

// This is the right justified address of the accelerometer, when the SDO pin
//  is grounded (as it is in our application).
#define ADXL345_ADDR 0x53

// Register map
#define DEVID           0      // Reads 11100101/0xE5
#define THRESH_TAP      0x1D   // Tap detection threshold
#define OFSX            0x1E   // X-axis offset
#define OFSY            0x1F   // Y-axis offset
#define OFSZ            0x20   // Z-axis offset
#define DUR             0x21   // Tap duration
#define LATENT          0x22   // Tap latency
#define WINDOW          0x23   // Tap window
#define THRESH_ACT      0x24   // Activity threshold
#define THRESH_INACT    0x25   // Inactivity threshold
#define TIME_INACT      0x26   // Inactivity time
#define ACT_INACT_CTL   0x27   // Axis enable control for activity and
                               //  inactivity detection
#define THRESH_FF       0x28   // Free-fall threshold
#define TIME_FF         0x29   // Free-fall time
#define TAP_AXES        0x2A   // Axis control for tap/double tap
#define ACT_TAP_STATUS  0x2B   // Source of tap/double tap
#define BW_RATE         0x2C   // Data rate and power mode control
#define POWER_CTL       0x2D   // Power-saving features control
#define INT_ENABLE      0x2E   // Interrupt enable control
#define INT_MAP         0x2F   // Interrupt mapping control
#define INT_SOURCE      0x30   // Source of interrupts
#define DATA_FORMAT     0x31   // Data format control
#define DATAX0          0x32   // X-axis Data 0
#define DATAX1          0x33   // X-axis Data 1
#define DATAY0          0x34   // Y-axis Data 0
#define DATAY1          0x35   // Y-axis Data 1
#define DATAZ0          0x36   // Z-axis Data 0
#define DATAZ1          0x37   // Z-axis Data 1
#define FIFO_CTL        0x38   // FIFO control
#define FIFO_STATUS     0x39   // FIFO status

enum {Aup, Bup, Cup, Dup, Topup, Botup};

bool read16Bits(uint8 address, int16 *read);
void readXYZ(int16 *x, int16 *y, int16 *z);
bool read8Bits(uint8 address, uint8 *read);
bool write8Bits(uint8 address, uint8 write);

// #endif
