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

/*
 * Module Name: heartbeat.c
 *
 * Filename: heartbeat.c
 *
 * Abstract:
 *
 * Heartbeat functions
 *
 */

#include "includes.h"

// WARNING, WARNING, WARNING. It is awkward to get VECS headers via source tree
// structure.
#define wstring_t wstring_t_outoftheway
#include "../../../vmafd/include/public/vmafdclient.h"
#undef wstring_t_outoftheway

static VMDNS_LIB_HANDLE gVmAfdHeartbeatLibrary = NULL;

static PVMAFD_HB_HANDLE gHeartbeatHandle = NULL;

static
DWORD
_VmDnsCreateHeartbeatThread(
    )
{
    DWORD dwError = 0;
    DWORD (*fpStart)(PCSTR, DWORD, PVMAFD_HB_HANDLE*) = NULL;

    if (gHeartbeatHandle != NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    fpStart = (DWORD (*)(PCSTR, DWORD, PVMAFD_HB_HANDLE*))VmDnsGetLibSym(gVmAfdHeartbeatLibrary, "VmAfdStartHeartbeatA");
    if (fpStart == NULL)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = fpStart("VMware DNS-Service", 2015, &gHeartbeatHandle);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
VOID
_VmDnsKillHeartbeatThread(
    )
{
    DWORD (*fpStop)(PVMAFD_HB_HANDLE) = NULL;
    if (gHeartbeatHandle)
    {
        fpStop = (DWORD (*)(PVMAFD_HB_HANDLE)) VmDnsGetLibSym(gVmAfdHeartbeatLibrary, "VmAfdStopHeartbeat");
        if (fpStop)
        {
            fpStop(gHeartbeatHandle);
            gHeartbeatHandle = NULL;
        }
    }
}

DWORD
VmDnsCreateHeartbeatThread(
    )
{
    DWORD dwError = 0;

    if (!gVmAfdHeartbeatLibrary)
    {
        dwError = VmDnsOpenVmAfdClientLib(&gVmAfdHeartbeatLibrary);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = _VmDnsCreateHeartbeatThread();
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsKillHeartbeatThread();
    goto cleanup;
}

VOID
VmDnsKillHeartbeatThread(
    )
{
    if (gVmAfdHeartbeatLibrary)
    {
        _VmDnsKillHeartbeatThread();
        if (!gHeartbeatHandle)
        {
            VmDnsCloseLibrary(gVmAfdHeartbeatLibrary);
        }
    }
}
