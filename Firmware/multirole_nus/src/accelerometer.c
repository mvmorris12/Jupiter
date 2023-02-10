#include "stdint.h"
#include "stdbool.h"
#include "spi.h"
#include "accelerometer.h"
#include "nrf_gpio.h"

extern uint16_t fifo_data[3][681];

extern volatile uint8_t m_tx_buf[10];
extern uint8_t m_rx_buf[4];
extern volatile uint8_t m_length;
extern bool ism_fifo_full;
extern bool ism_take_measurement;
extern bool ism_measurement_armed;
extern bool print_fifo_data;

extern volatile bool spi_xfer_done;
extern const nrf_drv_spi_t spi;


/*
SPI read of register (reg) on ISM330
*/
uint16_t ism_read(uint8_t reg){
    memset(m_rx_buf, 0, 4);
    spi_xfer_done = false;
    uint8_t test_rx[] = {(SPI_READ_BIT | reg), 0x00};
    m_length = sizeof(test_rx);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, test_rx, m_length, m_rx_buf, m_length));
    while (!spi_xfer_done)
    {
        //__WFE(); // this doesn't seem to be compatible with the softdevice?
    }
    // determines if ISM330 is connected properly
    if (reg == ISM330_REG_WHOAMI){
        if (m_rx_buf[1] == 0x6A){
            NRF_LOG_INFO("Accelerometer recognized");
            return m_rx_buf[1];
        } else { 
            NRF_LOG_INFO("Accelerometer Off");
            return 0x00;
        } 
    }
}

/*
SPI write to register (reg) on ISM330
*/
void ism_write(uint8_t reg, uint8_t data){
    memset(m_rx_buf, 0, 4);
    spi_xfer_done = false;
    uint8_t test_rx[] = {(SPI_WRITE_BIT & reg), data};
    m_length = sizeof(test_rx);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, test_rx, (m_length), m_rx_buf, m_length));
}



/*
Configures ISM330 and primes for measurements
*/
void ism_init(void){
    if (ism_read(ISM330_REG_WHOAMI) == 0x6A){
        nrf_gpio_cfg_output(19);
        nrf_gpio_pin_set(19);     // enables ISM330 power supply
        ism_write(ISM330_REG_CTRL1_XL, ((ISM330_SET_FIFO_ODR << 4) | (ISM330_SET_ACC_G << 2)));
        ism_write(ISM330_REG_FIFO_CTRL1, 0b11111111);
        ism_write(ISM330_REG_FIFO_CTRL2, 0b00000111);
        ism_write(ISM330_REG_FIFO_CTRL3, 0b00000001);
        ism_write(ISM330_REG_FIFO_CTRL4, 0b00000000);
        ism_write(ISM330_REG_CTRL3_C, 0b01000100);
        ism_write(ISM330_REG_CTRL4_C, 0b10010100);
        ism_write(ISM330_REG_CTRL5_C, 0b00000000);
        ism_write(ISM330_REG_INT1_CTRL, 0b00100000);
        ism_write(ISM330_REG_CTRL9_XL, 0b10000000);
        ism_write(ISM330_REG_FIFO_CTRL5, (ISM330_SET_FIFO_ODR << 3)); // stops measurements from being stored in FIFO buffer
        NRF_LOG_INFO("Accelerometer initialized");
        //ism_write(ISM330_REG_FIFO_CTRL5, ((ISM330_SET_FIFO_ODR << 4) | (ISM330_SET_FIFO_MODE)));  // starts measurement storage in FIFO
        ism_fifo_full = false;
    }
}

/*
Triggers ISM330 to stop filling FIFO buffer, sets all existing measurements in FIFO buffer to 0
*/
void ism_reset_fifo(void){
    ism_write(ISM330_REG_FIFO_CTRL5, ((ISM330_SET_FIFO_ODR << 3) | (0x00))); // stop measurements, reset FIFO buffer data
    ism_fifo_full = false;
    NRF_LOG_INFO("FIFO measurements reset, accelerometer armed");
    print_fifo_data = true;
}


/*
Triggers ISM330 to begin filling FIFO buffer with measurements
*/
void ism_start_measurements(void){
    pin_flip_quick(25);// used for measurement duration measurements with oscilloscope
    ism_write(ISM330_REG_FIFO_CTRL5, ((ISM330_SET_FIFO_ODR << 3) | (ISM330_SET_FIFO_MODE)));  // start measurements
    NRF_LOG_INFO("Starting ISM measurements...");
}


/*
Reads ISM330 FIFO_STATUS1 reg which contains how many samples are currently stored in the FIFO buffer
*/
void ism_remaining_fifo_samples(void){
    NRF_LOG_INFO("tabulating FIFO buffer..");
    uint8_t test_rx[] = {(SPI_READ_BIT | ISM330_REG_FIFO_STATUS1), 0x00, 0x00};
    m_length = sizeof(test_rx);
    memset(m_rx_buf, 0, m_length);
    spi_xfer_done = false;
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, test_rx, m_length, m_rx_buf, m_length));
    while (!spi_xfer_done)
    {
        __WFE();
    }
    printf("0x%02x 0x%02x : %d\n", m_rx_buf[2], m_rx_buf[1], (((m_rx_buf[2] & 0x07) << 8) | (m_rx_buf[1])));    
}




/*
Performs multiple sequential reads of ISM330 FIFO buffer and stores all 682 measurements in fifo_data[]
*/
void ism_read_fifo(void){
    NRF_LOG_INFO("reading FIFO buffer..");
    uint8_t fifo_rx[] = {(SPI_READ_BIT | ISM330_REG_FIFO_DATA_OUT_L), 0x00, 0x00};
    m_length = sizeof(fifo_rx);
    
    // ISM330 FIFO buffer: 4kB = 4096B = 32768b
    // XYZ measurement triplet = 16b*3 = 48b
    // FIFO buffer only has room for 682 entries with 4 bytes leftover
    for (uint16_t i=0; i<682; i++){ 
        for (uint8_t j=0; j<3; j++){
            memset(m_rx_buf, 0, m_length);
            spi_xfer_done = false;
            APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, fifo_rx, m_length, m_rx_buf, m_length));
            while (!spi_xfer_done)
            {
                //__WFE(); // this doesn't seem to be compatible with the softdevice
            }
            fifo_data[j][i] = (uint16_t*)((m_rx_buf[2] << 8) | (m_rx_buf[1])); // reads SPI rx buffer into array
        }
    }
    NRF_LOG_INFO("FIFO data collected");
    //ism_reset_fifo();
}
