#include "stdint.h"
#include "i2c.h"

#define TB_REG_CHIP_ID		    0x00
#define TB_REG_REV_ID		    0x01
#define TB_REG_ERR_REG		    0x02
#define TB_REG_STATUS		    0x03
#define TB_REG_PRESSURE_DATA_0	    0x04
#define TB_REG_PRESSURE_DATA_1	    0x05
#define TB_REG_PRESSURE_DATA_2	    0x06
#define TB_REG_TEMPERATUER_DATA_3   0x07
#define TB_REG_TEMPERATUER_DATA_4   0x08
#define TB_REG_TEMPERATUER_DATA_5   0x09
#define TB_REG_SENSORTIME_0	    0x0C
#define TB_REG_SENSORTIME_1	    0x0D
#define TB_REG_SENSORTIME_2	    0x0E
#define TB_REG_EVENT		    0x10
#define TB_REG_INT_STATUS	    0x11
#define TB_REG_FIFO_LENGTH_0	    0x12
#define TB_REG_FIFO_LENGTH_1	    0x13
#define TB_REG_FIFO_DATA	    0x14
#define TB_REG_FIFO_WATERMARK_0	    0x15
#define TB_REG_FIFO_WATERMARK_1	    0x16
#define TB_REG_FIFO_CONFIG_1	    0x17
#define TB_REG_FIFO_CONFIG_2	    0x18
#define TB_REG_INT_CTRL		    0x19
#define TB_REG_IF_CONF		    0x1A
#define TB_REG_PWR_CTRL		    0x1B
#define TB_REG_OSR		    0x1C
#define TB_REG_ODR		    0x1D
#define TB_REG_CONFIG		    0x1F
#define TB_REG_NVM_PAR_T1_0	    0x31
#define TB_REG_NVM_PAR_T1_1	    0x32
#define TB_REG_NVM_PAR_T2_0	    0x33
#define TB_REG_NVM_PAR_T2_1	    0x34
#define TB_REG_NVM_PAR_T3	    0x35
#define TB_REG_NVM_PAR_P1_0	    0x36
#define TB_REG_NVM_PAR_P1_1	    0x37
#define TB_REG_NVM_PAR_P2_0	    0x38
#define TB_REG_NVM_PAR_P2_1	    0x39
#define TB_REG_NVM_PAR_P3	    0x3A
#define TB_REG_NVM_PAR_P4	    0x3B
#define TB_REG_NVM_PAR_P5_0	    0x3C
#define TB_REG_NVM_PAR_P5_1	    0x3D
#define TB_REG_NVM_PAR_P6_0	    0x3E
#define TB_REG_NVM_PAR_P6_1	    0x3F
#define TB_REG_NVM_PAR_P7	    0x40
#define TB_REG_NVM_PAR_P8	    0x41
#define TB_REG_NVM_PAR_P9_0	    0x42
#define TB_REG_NVM_PAR_P9_1	    0x43
#define TB_REG_NVM_PAR_P10	    0x44
#define TB_REG_NVM_PAR_P11	    0x45
#define TB_REG_CMD		    0x7E

void thermometer_barometer_init(void);
void thermometer_barometer_take_measurement(void);
void thermometer_barometer_read_measurement(void);
void thermometer_barometer_get_measurement(void);
