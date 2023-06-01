#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "ble_lbs.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "main.h"
#include "gpiote.h"
#include "getdatetime.h"
#include "getdatetime_var.h"
#include "i2c.h"
#include "led.h"
#include "rtc.h"
#include "saadc.h"
#include "hygrometer.h"
#include "magnetometer.h"
#include "thermometer_barometer.h"


#define DEVICE_NAME                     "J1"                                    /**< Name of device. Will be included in the advertising data. */
#define DEVICE_BLE_MAC                  "E7:12:34:56:78:90"

#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_INTERVAL                1000                                    /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */
#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */


#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(1500, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(1500, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   10000                                    /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)          /**< Connection supervisory time-out (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)                    /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                        /**< Number of attempts before giving up the connection parameter negotiation. */

//#define DEAD_BEEF                       0xDEADBEEF                               /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


uint8_t measurement_count = 0;

struct Sensor_Data sensor_data;
struct Calib_Data calib_data;

bool take_measurement_flag = false;

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                         /**< Handle of the current connection. */

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;                    /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];                     /**< Buffer for storing an encoded advertising set. */
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];          /**< Buffer for storing an encoded scan data. */

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = m_enc_scan_response_data,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX

    }
};


static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


//static void gatt_init(void)
//{
//    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
//    APP_ERROR_CHECK(err_code);
//}


static void advertising_init(void)
{
    ret_code_t    err_code;
    ble_advdata_t advdata;
    ble_advdata_t srdata;
    ble_advdata_manuf_data_t manuf_data;
    //ble_uuid_t adv_uuids[] = {};
    uint8_t data[20]                      = {0xDE, 0xAD, 0xBE, 0xAA, 0x00, 0x01, 0x01, 0x03};
    //manuf_data.company_identifier       = 0x0059;
    manuf_data.data.p_data              = data;
    manuf_data.data.size                = 31;
    advdata.p_manuf_specific_data       = &manuf_data;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));


    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    //advdata.short_name_len     = 2;
    advdata.include_appearance = true;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    memset(&srdata, 0, sizeof(srdata));
    //srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    //srdata.uuids_complete.p_uuids  = adv_uuids;
    
    m_adv_data.adv_data.len = 32;

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len);
    APP_ERROR_CHECK(err_code);

    ble_gap_adv_params_t adv_params;

    // Set advertising parameters.
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;
    adv_params.duration        = APP_ADV_DURATION;
    adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    adv_params.p_peer_addr     = NULL;
    //adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    adv_params.interval        = APP_ADV_INTERVAL;

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    ret_code_t           err_code;

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_tx_power_set(1, m_adv_handle, 4);
    APP_ERROR_CHECK(err_code);

    //bsp_board_led_on(ADVERTISING_LED);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}


static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


void print_ble_mac(void){
    char InfoPacket[80];
    // Get actual BLE address in case it is different from hardware register BLE address
    ble_gap_addr_t device_addr;
    sd_ble_gap_addr_get(&device_addr);
    NRF_LOG_INFO("Bluetooth MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                    device_addr.addr[5], 
                    device_addr.addr[4], 
                    device_addr.addr[3], 
                    device_addr.addr[2], 
                    device_addr.addr[1], 
                    device_addr.addr[0]);
}


void set_ble_mac(void){
    static ble_gap_addr_t m_peripheral_addr;
    
    m_peripheral_addr.addr_type     = BLE_GAP_ADDR_TYPE_PUBLIC;
    for (uint8_t i=0; i<6; i++){
        uint8_t char_buf = 0;
        for (uint8_t j=0; j<2; j++){
            if (DEVICE_BLE_MAC[i*3+j] > 64){
                char_buf += (DEVICE_BLE_MAC[i*3+j] - 55);
            } 
            else if (DEVICE_BLE_MAC[i*3+j] < 58){
                char_buf += (DEVICE_BLE_MAC[i*3+j] - 48);
            }
            if (j == 0){
                char_buf <<= 4;
            } else {
                m_peripheral_addr.addr[5-i] = char_buf;
            }
        }
    }
    sd_ble_gap_addr_set(&m_peripheral_addr);
    print_ble_mac();
}


void sensor_data_init(void){
    sensor_data.battery.voltage = 0x0;
    sensor_data.hygrometer.humidity = 0x0;
    sensor_data.hygrometer.temperature = 0x0;
    sensor_data.magnetometer.x = 0x0;
    sensor_data.magnetometer.y = 0x0;
    sensor_data.magnetometer.z = 0x0;
    sensor_data.thermometer_barometer.pressure = 0x0;
    sensor_data.thermometer_barometer.temperature = 0x0;
    sensor_data.thermometer_barometer.ftemperature = 0.0;
    sensor_data.rtc.year = 0x0;
    sensor_data.rtc.month = 0x0;
    sensor_data.rtc.day = 0x0;
    sensor_data.rtc.hour = 0x0;
    sensor_data.rtc.minute = 0x0;
    sensor_data.rtc.second = 0x0;
    sensor_data.rtc.epoch_s = 0x0;
    calib_data.par_t1 = 0x0;
    calib_data.par_t2 = 0x0;
    calib_data.par_t3 = 0x0;
    calib_data.t_lin = 0x0;
    calib_data.fpar_t1 = 0.0;
    calib_data.fpar_t2 = 0.0;
    calib_data.fpar_t3 = 0.0;
    calib_data.par_p1 = 0x0;
    calib_data.par_p2 = 0x0;
    calib_data.par_p3 = 0x0;
    calib_data.par_p4 = 0x0;
    calib_data.par_p5 = 0x0;
    calib_data.par_p6 = 0x0;
    calib_data.par_p7 = 0x0;
    calib_data.par_p8 = 0x0;
    calib_data.par_p9 = 0x0;
    calib_data.par_p10 = 0x0;
    calib_data.par_p11 = 0x0;
    calib_data.fpar_p1 = 0.0;
    calib_data.fpar_p2 = 0.0;
    calib_data.fpar_p3 = 0.0;
    calib_data.fpar_p4 = 0.0;
    calib_data.fpar_p5 = 0.0;
    calib_data.fpar_p6 = 0.0;
    calib_data.fpar_p7 = 0.0;
    calib_data.fpar_p8 = 0.0;
    calib_data.fpar_p9 = 0.0;
    calib_data.fpar_p10 = 0.0;
    calib_data.fpar_p11 = 0.0;
}


void update_ble_adv_packet(void){
    ble_advdata_t advdata;    
    //m_adv_data.adv_data.p_data[0]  = 0x4A; // 'J' 
    //m_adv_data.adv_data.p_data[1]  = 0x01; // '01'
    m_adv_data.adv_data.p_data[0]  = (sensor_data.rtc.epoch_s>>24)&0xFF;
    m_adv_data.adv_data.p_data[1]  = (sensor_data.rtc.epoch_s>>16)&0xFF;
    m_adv_data.adv_data.p_data[2]  = (sensor_data.rtc.epoch_s>>8)&0xFF;
    m_adv_data.adv_data.p_data[3]  = (sensor_data.rtc.epoch_s>>0)&0xFF;
    m_adv_data.adv_data.p_data[4]  = (sensor_data.thermometer_barometer.temperature>>24)&0xFF;  // temperature [31:24]
    m_adv_data.adv_data.p_data[5]  = (sensor_data.thermometer_barometer.temperature>>16)&0xFF;  // temperature [23:16]
    m_adv_data.adv_data.p_data[6]  = (sensor_data.thermometer_barometer.temperature>>8)&0xFF;   // temperature [15:8]
    m_adv_data.adv_data.p_data[7]  = (sensor_data.thermometer_barometer.temperature>>0)&0xFF;   // temperature [7:0]
    m_adv_data.adv_data.p_data[8]  = (sensor_data.thermometer_barometer.pressure>>24)&0xFF;     // pressure [31:24]
    m_adv_data.adv_data.p_data[9]  = (sensor_data.thermometer_barometer.pressure>>16)&0xFF;     // pressure [23:16]
    m_adv_data.adv_data.p_data[10] = (sensor_data.thermometer_barometer.pressure>>8)&0xFF;      // pressure [15:8]
    m_adv_data.adv_data.p_data[11] = (sensor_data.thermometer_barometer.pressure>>8)&0xFF;      // pressure [7:0]
    m_adv_data.adv_data.p_data[12] = (sensor_data.hygrometer.humidity>>8)&0xFF;                 // humidity [16:8]
    m_adv_data.adv_data.p_data[13] = (sensor_data.hygrometer.humidity>>0)&0xFF;                 // humidity [8:0]
    m_adv_data.adv_data.p_data[14] = (sensor_data.battery.voltage>>8)&0xFF;                     // battery voltage [16:8]
    m_adv_data.adv_data.p_data[15] = (sensor_data.battery.voltage)&0xFF;                        // battery voltage [8:0]

    //ret_code_t err_code = sd_ble_gap_adv_data_set(NULL, 0, m_adv_data.adv_data.p_data, sizeof(m_adv_data.adv_data.p_data));
    //ret_code_t err_code = ble_advdata_encode(&m_adv_data, m_adv_data.adv_data.p_data, sizeof(m_adv_data.adv_data.p_data));
    //APP_ERROR_CHECK(err_code);
}


void take_measurements(void){
    take_measurement_flag = false;
    rtc_set_minute_alarm();
    rtc_read_time();
    thermometer_barometer_get_measurement();
    //magnetometer_get_measurement();
    hygrometer_get_measurement();
    if (++measurement_count >= 4){
        measurement_count = 0;
    }
    update_ble_adv_packet();
}

/* if this hangs on twi fx, run the twi scanner to reset */
void main(void){
    sensor_data_init();
    log_init();
    NRF_LOG_INFO("<~~~ Jupiter ~~~>");
    power_management_init();

    led_init();
    led_flash(LED_PIN_RED, 125, 1);
    twi_init();
    saadc_init();
    gpiote_init();
    twi_check_sensors();
    rtc_init();
    thermometer_barometer_init();
    thermometer_barometer_read_calibration();
    ble_stack_init();
    set_ble_mac();
    advertising_init();
    advertising_start();

    take_measurements();
    battery_get_percent();

    for(;;){
        idle_state_handle();
        if (take_measurement_flag){
            take_measurements();
        }

    }
}