
#define PIN_IN_COMPARATOR_IRQ           31
#define PIN_IN_ACC_IRQ                  11

void in_pin_handler_comp(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void in_pin_handler_acc(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void gpiote_init();
void gpiote_comp_start(void);
void gpiote_comp_stop(void);
void gpiote_uninit(void);
void gpiote_acc_start(void);
void gpiote_acc_stop(void);