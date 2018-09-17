/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../testplugin.h"

#define TEST_PLUGIN_MESSAGE "Hello from testpluginvalid!"

static int      _get_number(void);
static char *   _get_message(void);

int32_t
LwCAPluginLoad(
    void    *pvTable
    )
{
    if (!pvTable)
    {
        return 1;
    }

    PLWCA_PLUGIN_VTABLE pVT = (PLWCA_PLUGIN_VTABLE)pvTable;

    pVT->pfnGetNumber   = &(_get_number);
    pVT->pfnGetMessage  = &(_get_message);

    return 0;
}

static int _get_number(void)
{
    return 0xCA2018;
}

static char * _get_message(void)
{
    return TEST_PLUGIN_MESSAGE;
}
