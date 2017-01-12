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



#include "includes.h"


static
DWORD
_VmKdcInitKdcServiceThread(
    PVMKDC_GLOBALS pGlobals
	);

DWORD
VmKdcServiceStartup(
    VOID
    )
{
    DWORD    dwError = 0;

    /*
     * Load the server configuration from the registry.
     * Note that this may create a new thread.
     */
    dwError = VmKdcSrvUpdateConfig(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcInit();
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = _VmKdcInitKdcServiceThread(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcSrvInit");

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ERROR: vmkdc VmKdcServiceStartup failed (%d)", dwError);
    goto cleanup;
}

VOID
VmKdcServiceShutdown(
    VOID
    )
{
    VmKdcdStateSet(VMKDC_STOPPING);
    VmKdcShutdown();
    //VmKdcLogTerminate();

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Vmkdcd: stop");

    return;
}

static
DWORD
_VmKdcInitKdcServiceThread(
    PVMKDC_GLOBALS pGlobals
	)
{
    DWORD dwError = 0;
    int   sts = 0;
    void*(*pThrFn)(void*) = (void*(*)(void*))VmKdcInitLoop;

    sts = pthread_create(
            &pGlobals->thread,
            NULL,
            pThrFn,
            pGlobals);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmKdcInitKdcServiceThread: failed (%u)", dwError);
    goto cleanup;
}
