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

//#define CONFIG_TARGET_WITTY_CLOUD
//#define CONFIG_TARGET_ESP8266_DEVKITC_V1
#if defined(CONFIG_TARGET_WITTY_CLOUD)

#define GPIO_INPUT_BUTTON 4

#define GPIO_OUTPUT_MAINLED 15
#define GPIO_OUTPUT_MAINLED_0 16 /* use as ground */

#define GPIO_OUTPUT_NOUSE1 12
#define GPIO_OUTPUT_NOUSE2 13

#elif defined(CONFIG_TARGET_ESP8266_DEVKITC_V1)

#define GPIO_INPUT_BUTTON 0

#define GPIO_OUTPUT_MAINLED 14
#define GPIO_OUTPUT_MAINLED_0 15 /* use as ground */

#define GPIO_OUTPUT_NOUSE1 12
#define GPIO_OUTPUT_NOUSE2 13

#else //default

#define GPIO_OUTPUT_NOTIFICATION_LED 2
#define GPIO_INPUT_BUTTON 5

#define GPIO_OUTPUT_MAINLED 16
#define GPIO_OUTPUT_MAINLED_0 13 /* use as ground */

#define GPIO_OUTPUT_NOUSE1 14
#define GPIO_OUTPUT_NOUSE2 12

#endif

enum switch_onoff_state {
    SWITCH_OFF = 0,
    SWITCH_ON = 1,
};

enum main_led_gpio_state {
    MAINLED_GPIO_ON = 1,
    MAINLED_GPIO_OFF = 0,
};

enum led_animation_mode_list {
    LED_ANIMATION_MODE_IDLE = 0,
    LED_ANIMATION_MODE_FAST,
    LED_ANIMATION_MODE_SLOW,
};

enum button_gpio_state {
    BUTTON_GPIO_RELEASED = 1,
    BUTTON_GPIO_PRESSED = 0,
};


//void change_switch_state(int switch_state);
void led_blink(int switch_state, int delay, int count);
void change_led_mode(int noti_led_mode);
void iot_gpio_init(void);


void iot_adc_init(void);