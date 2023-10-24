/* ***************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "st_dev.h"
#include "device_control.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "iot_uart_cli.h"
#include "iot_cli_cmd.h"

#include "caps_illuminanceMeasurement.h"

// onboarding_config_start is null-terminated string
extern const uint8_t onboarding_config_start[]    asm("_binary_onboarding_config_json_start");
extern const uint8_t onboarding_config_end[]    asm("_binary_onboarding_config_json_end");

// device_info_start is null-terminated string
extern const uint8_t device_info_start[]    asm("_binary_device_info_json_start");
extern const uint8_t device_info_end[]        asm("_binary_device_info_json_end");

static iot_status_t g_iot_status = IOT_STATUS_IDLE;
static iot_stat_lv_t g_iot_stat_lv;

IOT_CTX* ctx = NULL;


static int noti_led_mode = LED_ANIMATION_MODE_IDLE;

static caps_illuminanceMeasurement_data_t *cap_sensor_data;
int monitor_period_ms = 20000;  // start with 20sec and switch to 5min once initialized


static void capability_init()
{
    cap_sensor_data = caps_illuminanceMeasurement_initialize(ctx, "main", NULL, NULL);
    if (cap_sensor_data) {
        cap_sensor_data->set_illuminance_value(cap_sensor_data, 0);

        cap_sensor_data->set_illuminance_unit(cap_sensor_data, caps_helper_illuminanceMeasurement.attr_illuminance.unit_lux);
    }

}

static void iot_status_cb(iot_status_t status,
                          iot_stat_lv_t stat_lv, void *usr_data)
{
    g_iot_status = status;
    g_iot_stat_lv = stat_lv;

    printf("status: %d, stat: %d\n", g_iot_status, g_iot_stat_lv);

    switch(status)
    {
        case IOT_STATUS_NEED_INTERACT:
            noti_led_mode = LED_ANIMATION_MODE_FAST;
            break;
        case IOT_STATUS_IDLE:
        case IOT_STATUS_CONNECTING:
            noti_led_mode = LED_ANIMATION_MODE_IDLE;
            //change_switch_state(get_switch_state());
            break;
        default:
            break;
    }
}


static void connection_start(void)
{
    iot_pin_t *pin_num = NULL;
    int err;

    // process on-boarding procedure. There is nothing more to do on the app side than call the API.
    err = st_conn_start(ctx, (st_status_cb)&iot_status_cb, IOT_STATUS_ALL, NULL, pin_num);
    if (err) {
        printf("fail to start connection. err:%d\n", err);
    }
    if (pin_num) {
        free(pin_num);
    }
}


static void iot_noti_cb(iot_noti_data_t *noti_data, void *noti_usr_data)
{
    printf("Notification message received\n");

    if (noti_data->type == IOT_NOTI_TYPE_DEV_DELETED) {
        printf("[device deleted]\n");
    } else if (noti_data->type == IOT_NOTI_TYPE_RATE_LIMIT) {
        printf("[rate limit] Remaining time:%d, sequence number:%d\n",
               noti_data->raw.rate_limit.remainingTime, noti_data->raw.rate_limit.sequenceNumber);
    }
}


static void app_main_task(void *arg)
{
    TimeOut_t monitor_timeout;
    TickType_t monitor_period_tick = pdMS_TO_TICKS(monitor_period_ms);
    double previous_1 = 0;
    double previous_2 = 0;

    for (;;) {
        if (noti_led_mode != LED_ANIMATION_MODE_IDLE) {
            change_led_mode(noti_led_mode);
        }

        if ((xTaskCheckForTimeOut(&monitor_timeout, &monitor_period_tick) != pdFALSE)) {
            vTaskSetTimeOutState(&monitor_timeout);
            monitor_period_tick = pdMS_TO_TICKS(monitor_period_ms);

            uint16_t adc_data;
            esp_err_t err = adc_read(&adc_data);
            //printf("\nAnalog error: %i\n", (int)err);
            printf("Analog Read: %i =  %f lumins\n", adc_data, (double)(1024-adc_data)/1024*100000);

            double curLumins = ((double)(1024-adc_data)/1024*100000);
            printf("cur: %f,  prev: %f,  prevx2: %f\n", curLumins, previous_1, previous_2);

            double avgLumins = (curLumins + previous_1 + previous_2) / 300;
            avgLumins = round(avgLumins);
            avgLumins *= 100;
            printf("avg: %f\n", avgLumins);

            if (previous_2 != 0)
            {
                printf("sending data: %f\n\n", avgLumins);
                cap_sensor_data->set_illuminance_value(cap_sensor_data, avgLumins);
                cap_sensor_data->attr_illuminance_send(cap_sensor_data);

                // switch to longer wait
                monitor_period_ms = 300000; // 5mins
            }

            previous_2 = previous_1;
            previous_1 = curLumins;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
    /**
      SmartThings Device SDK(STDK) aims to make it easier to develop IoT devices by providing
      additional st_iot_core layer to the existing chip vendor SW Architecture.

      That is, you can simply develop a basic application
      by just calling the APIs provided by st_iot_core layer like below.

      // create a iot context
      1. st_conn_init();

      // create a handle to process capability
      2. st_cap_handle_init(); (called in function 'capability_init')

      // register a callback function to process capability command when it comes from the SmartThings Server.
      3. st_cap_cmd_set_cb(); (called in function 'capability_init')

      // process on-boarding procedure. There is nothing more to do on the app side than call the API.
      4. st_conn_start(); (called in function 'connection_start')
     */

    unsigned char *onboarding_config = (unsigned char *) onboarding_config_start;
    unsigned int onboarding_config_len = onboarding_config_end - onboarding_config_start;
    unsigned char *device_info = (unsigned char *) device_info_start;
    unsigned int device_info_len = device_info_end - device_info_start;

    int iot_err;

    // create a iot context
    ctx = st_conn_init(onboarding_config, onboarding_config_len, device_info, device_info_len);
    if (ctx != NULL) {
        iot_err = st_conn_set_noti_cb(ctx, iot_noti_cb, NULL);
        if (iot_err)
            printf("fail to set notification callback function\n");
    } else {
        printf("fail to create the iot_context\n");
    }

    // create a handle to process capability and initialize capability info
    capability_init();

    iot_gpio_init();
    iot_adc_init();
    register_iot_cli_cmd();
    uart_cli_main();
    xTaskCreate(app_main_task, "app_main_task", 4096, NULL, 10, NULL);

    // connect to server
    connection_start();
}

