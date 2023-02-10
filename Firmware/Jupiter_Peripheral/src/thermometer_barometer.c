#include "led.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "thermometer_barometer.h"
#include "main.h"

extern struct Sensor_Data sensor_data;

void thermometer_barometer_init(void){
    uint8_t tx_buffer = 0b00000000;
    twi_write(TWI_ADDR_THERMOMETER_BAROMETER, TB_REG_FIFO_CONFIG_1, &tx_buffer, sizeof(tx_buffer));	
    tx_buffer = 0b00101101;
    twi_write(TWI_ADDR_THERMOMETER_BAROMETER, TB_REG_OSR, &tx_buffer, sizeof(tx_buffer));	  
}



void thermometer_barometer_take_measurement(void){
    uint8_t tx_buffer = 0b00100011;
    twi_write(TWI_ADDR_THERMOMETER_BAROMETER, TB_REG_PWR_CTRL, &tx_buffer, sizeof(tx_buffer));
    //NRF_LOG_INFO("Temperature & pressure sensors enabled");
}


//void thermometer_barometer_disable_sensors(void){
//    uint8_t tx_buffer = 0b00000000;
//    twi_write(TWI_ADDR_THERMOMETER_BAROMETER, TB_REG_PWR_CTRL, &tx_buffer, sizeof(tx_buffer));
//    //NRF_LOG_INFO("Temperature & pressure sensors disabled");
//}


void thermometer_barometer_read_measurement(void){
    uint8_t measurement_data[6];
    char date_str[20];
    twi_read(TWI_ADDR_THERMOMETER_BAROMETER, TB_REG_PRESSURE_DATA_0, measurement_data, sizeof(measurement_data));
    sensor_data.thermometer_barometer.pressure = (measurement_data[2] << 16) + (measurement_data[1] << 8) + (measurement_data[0]);
    sensor_data.thermometer_barometer.temperature = (measurement_data[5] << 16) + (measurement_data[4] << 8) + (measurement_data[3]);
    NRF_LOG_INFO("   Temperature: %x", sensor_data.thermometer_barometer.temperature);
    NRF_LOG_INFO("   Pressure: %x", sensor_data.thermometer_barometer.pressure);
}


void thermometer_barometer_get_measurement(void){
    thermometer_barometer_take_measurement();
    nrf_delay_ms(50);
    thermometer_barometer_read_measurement();
}
