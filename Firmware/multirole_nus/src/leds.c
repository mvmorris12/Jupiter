#include "leds.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"



/*
Configures LED pins as output
*/
void leds_init(void){
    nrf_gpio_cfg_output(LED_PIN_GREEN);
    nrf_gpio_cfg_output(LED_PIN_RED);
}


/*
Turns green LED on
*/
void led_green_on(void){
    nrf_gpio_pin_set(LED_PIN_GREEN);
}


/*
Turns green LED off
*/
void led_green_off(void){
    nrf_gpio_pin_clear(LED_PIN_GREEN);
}


/*
Turns red LED on
*/
void led_red_on(void){
    nrf_gpio_pin_set(LED_PIN_RED);
}

/*
Turns red LED off
*/
void led_red_off(void){
    nrf_gpio_pin_clear(LED_PIN_RED);
}


/*
Function for flashing leds:
    led_pin       which LED to flash
    on_time_ms    Duration in ms that LED is on
    flashes       # of times to flash
*/
void led_flash(uint8_t led_pin, uint16_t on_time_ms, uint8_t flashes){
    for (uint8_t i=0; i<flashes; i++){
        switch(led_pin){
            case LED_PIN_RED:
                led_red_on();
                nrf_delay_ms(on_time_ms);
                led_red_off();
                break;
            case LED_PIN_GREEN:
                led_green_on();
                nrf_delay_ms(on_time_ms);
                led_green_off();
                break;
            default:
                led_red_on();
                led_green_on();
                nrf_delay_ms(on_time_ms);
                led_red_off();
                led_green_off();
        }
        if (flashes > 1){
            nrf_delay_ms(125);
        }
    }
}

