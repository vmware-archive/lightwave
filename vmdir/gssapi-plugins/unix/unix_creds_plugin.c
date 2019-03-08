/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
#include <errno.h>
#include "unix_crypt.h"
#include <gssapi_creds_plugin.h>

/*
 * plugin load
*/
int
creds_plugin_load_interface(
    PCREDS_PLUGIN_INTERFACE *ppInterface
    )
{
    int sts = 0;
    PCREDS_PLUGIN_INTERFACE pInterface = NULL;

    if(!ppInterface)
    {
        sts = EINVAL;
        goto error;
    }

    pInterface = (PCREDS_PLUGIN_INTERFACE)
                 malloc(sizeof(CREDS_PLUGIN_INTERFACE));
    if(!pInterface)
    {
        sts = ENOMEM;
        goto error;
    }

    pInterface->pfnGetHashedCreds = get_salt_and_v_value;
    *ppInterface = pInterface;

cleanup:
    return sts;

error:
    if(ppInterface)
    {
        *ppInterface = NULL;
    }
    if(pInterface)
    {
        free(pInterface);
    }
    goto cleanup;
}
