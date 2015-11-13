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

DWORD
VmwDeployStartService(
    PCSTR pszName
    )
{
    DWORD dwError = 0;
    PWSTR pwszName = NULL;
    LW_SERVICE_HANDLE hService = NULL;

    dwError = VmAfdAllocateStringWFromA(
                    pszName,
                    &pwszName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = LwSmAcquireServiceHandle(
                  pwszName,
                  &hService);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = LwSmStartService(hService);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (hService)
    {
        LwSmReleaseServiceHandle(hService);
    }
    if (pwszName)
    {
        VmAfdFreeMemory(pwszName);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmwDeployStopService(
    PCSTR pszName
    )
{
    DWORD dwError = 0;
    PWSTR pwszName = NULL;
    LW_SERVICE_HANDLE hService = NULL;

    dwError = VmAfdAllocateStringWFromA(
                    pszName,
                    &pwszName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = LwSmAcquireServiceHandle(
                  pwszName,
                  &hService);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = LwSmStopService(hService);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (hService)
    {
        LwSmReleaseServiceHandle(hService);
    }
    if (pwszName)
    {
        VmAfdFreeMemory(pwszName);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmwDeployRestartService(
    PCSTR pszName
    )
{
    DWORD dwError = 0;

    dwError = VmwDeployStopService(pszName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmwDeployStartService(pszName);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}


DWORD
VMwDeployStartServiceClient(VOID)
{
    DWORD dwError = 0;
    int iSvc = 0;
    PCSTR ppszServices[]=
    {
        VMW_DCERPC_SVC_NAME,
        VMW_VMAFD_SVC_NAME
    };

    for (iSvc=0; iSvc < sizeof(ppszServices)/sizeof(ppszServices[0]); iSvc++)
    {
        PCSTR pszService = ppszServices[iSvc];

        VmAfdLog(VMAFD_DEBUG_ANY, "Starting service [%s]", pszService);

        dwError = VmwDeployStartService(pszService);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:
    return dwError;
}
