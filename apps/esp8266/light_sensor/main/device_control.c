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


#include "device_control.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"


void change_led_mode(int noti_led_mode)
{
    static TimeOut_t led_timeout;
    static TickType_t led_tick = -1;
    static int last_led_mode = -1;
    static int led_state = SWITCH_OFF;

    if (last_led_mode != noti_led_mode) {
        last_led_mode = noti_led_mode;
        vTaskSetTimeOutState(&led_timeout);
        led_tick = 0;
    }

    switch (noti_led_mode)
    {
        case LED_ANIMATION_MODE_IDLE:
            break;
        case LED_ANIMATION_MODE_SLOW:
            if (xTaskCheckForTimeOut(&led_timeout, &led_tick ) != pdFALSE) {
                led_state = 1 - led_state;
                //change_switch_state(led_state);
                vTaskSetTimeOutState(&led_timeout);
                if (led_state == SWITCH_ON) {
                    led_tick = pdMS_TO_TICKS(200);
                } else {
                    led_tick = pdMS_TO_TICKS(800);
                }
            }
            break;
        case LED_ANIMATION_MODE_FAST:
            if (xTaskCheckForTimeOut(&led_timeout, &led_tick ) != pdFALSE) {
                led_state = 1 - led_state;
                //change_switch_state(led_state);
                vTaskSetTimeOutState(&led_timeout);
                led_tick = pdMS_TO_TICKS(100);
            }
            break;
        default:
            break;
    }
}

void iot_gpio_init(void)
{
	gpio_config_t io_conf;

	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 1 << GPIO_OUTPUT_MAINLED;
	io_conf.pull_down_en = 1;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	io_conf.pin_bit_mask = 1 << GPIO_OUTPUT_MAINLED_0;
	gpio_config(&io_conf);

	io_conf.pin_bit_mask = 1 << GPIO_OUTPUT_NOUSE1;
	gpio_config(&io_conf);
	io_conf.pin_bit_mask = 1 << GPIO_OUTPUT_NOUSE2;
	gpio_config(&io_conf);


	io_conf.intr_type = GPIO_INTR_ANYEDGE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = 1 << GPIO_INPUT_BUTTON;
	io_conf.pull_down_en = (BUTTON_GPIO_RELEASED == 0);
	io_conf.pull_up_en = (BUTTON_GPIO_RELEASED == 1);
	gpio_config(&io_conf);

	gpio_set_intr_type(GPIO_INPUT_BUTTON, GPIO_INTR_ANYEDGE);

	gpio_install_isr_service(0);

	gpio_set_level(GPIO_OUTPUT_MAINLED, MAINLED_GPIO_ON);
	gpio_set_level(GPIO_OUTPUT_MAINLED_0, 0);
}

void iot_adc_init(void)
{
    adc_config_t adc_config;

    // Depend on menuconfig->Component config->PHY->vdd33_const value
    // When measuring system voltage(ADC_READ_VDD_MODE), vdd33_const must be set to 255.
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8; // ADC sample collection clock = 80MHz/clk_div = 10MHz
    adc_init(&adc_config);

}
