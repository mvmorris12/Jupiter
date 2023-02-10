#include "spi.h"
#include "accelerometer.h"

volatile uint8_t      m_tx_buf[10];                 /**< TX buffer. */
         uint8_t      m_rx_buf[4];                  /**< RX buffer. */
volatile uint8_t      m_length = sizeof(m_tx_buf);  /**< Transfer length. */

extern volatile bool spi_xfer_done;

const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
/*
SPI event handler
*/
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}


/*
Initializes SPI comms
*/
void spi_init(void){
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = ACC_SS;
    spi_config.miso_pin = ACC_MISO;
    spi_config.mosi_pin = ACC_MOSI;
    spi_config.sck_pin  = ACC_SCK;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));
}