#include "stdint.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define ACC_IRQ                         11
#define ACC_MISO                        12
#define ACC_MOSI                        15
#define ACC_SCK                         16
#define ACC_SS                          17

#define SPI_READ_BIT                    0x80 // 0b10000000 | data: sets first bit to 1
#define SPI_WRITE_BIT                   0x7F // 0b01111111 & data: sets first bit to 0

// Registers
#define ISM330_REG_WHOAMI               0x0F
#define ISM330_REG_FIFO_CTRL1           0x06
#define ISM330_REG_FIFO_CTRL2           0x07
#define ISM330_REG_FIFO_CTRL3           0x08
#define ISM330_REG_FIFO_CTRL4           0x09
#define ISM330_REG_FIFO_CTRL5           0x0A
#define ISM330_REG_CTRL3_C              0x12
#define ISM330_REG_CTRL4_C              0x13
#define ISM330_REG_CTRL5_C              0x14
#define ISM330_REG_INT1_CTRL            0x0D
#define ISM330_REG_FIFO_STATUS1         0x3A
#define ISM330_REG_FIFO_STATUS2         0x3B
#define ISM330_REG_CTRL1_XL             0x10
#define ISM330_REG_CTRL9_XL             0x18
#define ISM330_REG_FIFO_DATA_OUT_L      0x3E
#define ISM330_REG_FIFO_DATA_OUT_H      0x3F
#define ISM330_REG_OUTX_L_XL            0x28
#define ISM330_REG_OUTX_H_XL            0x29

// Values for various settings
#define ISM330_VAL_FIFO_ODR_12_5HZ      0b0001
#define ISM330_VAL_FIFO_ODR_26HZ        0b0010
#define ISM330_VAL_FIFO_ODR_52HZ        0b0011
#define ISM330_VAL_FIFO_ODR_104HZ       0b0100
#define ISM330_VAL_FIFO_ODR_208HZ       0b0101
#define ISM330_VAL_FIFO_ODR_416HZ       0b0110
#define ISM330_VAL_FIFO_ODR_833HZ       0b0111
#define ISM330_VAL_FIFO_ODR_1_66KHZ     0b1000
#define ISM330_VAL_FIFO_ODR_3_33KHZ     0b1001
#define ISM330_VAL_FIFO_ODR_6_66KHZ     0b1010
#define ISM330_VAL_FIFO                 0b001
#define ISM330_VAL_CONTINUOUS           0b011
#define ISM330_VAL_BYPASS               0b100
#define ISM330_VAL_CONTINUOUS_OVRW      0b110
#define ISM330_VAL_2G                   0b00
#define ISM330_VAL_4G                   0b10
#define ISM330_VAL_8G                   0b11
#define ISM330_VAL_16G                  0b01

// Reference settings
#define ISM330_SET_FIFO_MODE            ISM330_VAL_FIFO
#define ISM330_SET_FIFO_ODR             ISM330_VAL_FIFO_ODR_6_66KHZ
#define ISM330_SET_ACC_G                ISM330_VAL_2G       

uint16_t ism_read(uint8_t reg);
void ism_write(uint8_t reg, uint8_t data);
void ism_init(void);
void ism_reset_fifo(void);
void ism_start_measurement(void);
void ism_remaining_fifo_samples(void);
void ism_read_fifo(void);