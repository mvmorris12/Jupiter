#include "stdint.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_timer.h"

#include "gpiote.h"

extern bool sync_time_flag;
extern uint8_t interrupt_count;
extern bool ism_fifo_full;
extern bool ism_take_measurement;
extern bool ism_measurement_armed;
/*
GPIOTE interrupt handler for comparator interrupt. Triggers ISM measurements on second interrupt as
the first interrupt occurs too quickly and is not synced with other device.
*/
void in_pin_handler_comp(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if ((sync_time_flag) && (interrupt_count++ >= 1)){
        if (ism_measurement_armed){
            ism_start_measurements();
            sync_time_flag = false;
            gpiote_comp_stop();
            ism_take_measurement = true;
            ism_measurement_armed = false;
        }
    }
}


/*
GPIOTE interrupt handler for accelerometer interrupt 1, essentially toggles pin 25 when
the accelerometer has finished filling its FIFO buffer, useful for measuring
measurement duration with oscilloscope
*/
void in_pin_handler_acc(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    pin_flip_quick(25);
    ism_fifo_full = true;
    NRF_LOG_INFO("Accelerometer FIFO full irq detected");
}


/*
Initializes GPIOTE for comparator and accelerometer interrupts. 
*/
void gpiote_init()
{
    ret_code_t err_code;

    nrf_drv_gpiote_init();

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;
    in_config.sense = GPIOTE_CONFIG_POLARITY_HiToLo;
    in_config.is_watcher = false;
    in_config.hi_accuracy = false;
    
    err_code = nrf_drv_gpiote_in_init(PIN_IN_COMPARATOR_IRQ, &in_config, in_pin_handler_comp);
    APP_ERROR_CHECK(err_code);

    in_config.pull = NRF_GPIO_PIN_PULLDOWN;
    in_config.sense = GPIOTE_CONFIG_POLARITY_LoToHi;
    in_config.is_watcher = false;
    in_config.hi_accuracy = false;
   
    err_code = nrf_drv_gpiote_in_init(PIN_IN_ACC_IRQ, &in_config, in_pin_handler_acc);
    APP_ERROR_CHECK(err_code);
    gpiote_acc_start();
    
    NRF_LOG_INFO("GPIOTE initialized");
}


/*
Starts GPIOTE interrupt detection for comparator interrupt
*/
void gpiote_comp_start(void){
    nrf_drv_gpiote_in_event_enable(PIN_IN_COMPARATOR_IRQ, true);
    NRF_LOG_INFO("GPIOTE started");
}


/*
Disables GPIOTE interrupt detection for comparator interrupt
*/
void gpiote_comp_stop(void){
    nrf_drv_gpiote_in_event_disable(PIN_IN_COMPARATOR_IRQ);
    NRF_LOG_INFO("GPIOTE stopped");
}


/*
Uninitializes GPIOTE
*/ //TODO check if this is right
void gpiote_uninit(void){
    if(!nrf_drv_gpiote_is_init()){
        nrf_drv_gpiote_uninit();
        NRF_LOG_INFO("GPIOTE uninitialized");
    } else { 
        nrf_drv_gpiote_in_event_disable(PIN_IN_COMPARATOR_IRQ);
    }
}

/*
Enables GPIOTE event for accelerometer interrupt 1
*/
void gpiote_acc_start(void){
    nrf_drv_gpiote_in_event_enable(PIN_IN_ACC_IRQ, true);
    NRF_LOG_INFO("Accelerometer interrupt GPIOTE enabled")
}


/*
Disables GPIOTE event for accelerometer interrupt 1
*/
void gpiote_acc_stop(void){
    nrf_drv_gpiote_in_event_disable(PIN_IN_ACC_IRQ);
}

