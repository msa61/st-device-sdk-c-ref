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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_uart_cli.h"
#include "device_control.h"

#include "st_dev.h"

extern IOT_CTX *ctx;

static void _cli_cmd_cleanup(char *string)
{
    printf("clean-up data with reboot option");
    st_conn_cleanup(ctx, true);
}


static struct cli_command cmd_list[] = {
    {"cleanup", "clean-up data with reboot option", _cli_cmd_cleanup},
};

void register_iot_cli_cmd(void) {
    for (int i = 0; i < ARRAY_SIZE(cmd_list); i++)
        cli_register_command(&cmd_list[i]);
}
