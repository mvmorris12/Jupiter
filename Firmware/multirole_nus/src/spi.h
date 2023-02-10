#include "stdint.h"
#include "nrf_drv_spi.h"


#define SPI_INSTANCE  0 /**< SPI instance index. */

#ifndef _SPI_H
#define _SPI_H
#endif


//static volatile bool spi_xfer_done;                 /**< Flag used to indicate that SPI instance completed the transfer. */
void spi_init(void);