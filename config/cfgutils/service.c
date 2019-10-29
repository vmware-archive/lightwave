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

    if (VmStringCompareA(pszName, "vmafd", FALSE) == 0)
    {
        dwError = system(VMW_START_VMAFDD);
    }
    else if (VmStringCompareA(pszName, "vmdir", FALSE) == 0)
    {
        dwError = system(VMW_START_VMDIRD);
    }
    else if (VmStringCompareA(pszName, "vmca", FALSE) == 0)
    {
        dwError = system(VMW_START_VMCAD);
    }
    else if (VmStringCompareA(pszName, "vmdns", FALSE) == 0)
    {
        dwError = system(VMW_START_VMDNSD);
    }
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

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

    if (VmStringCompareA(pszName, "vmafd", FALSE) == 0)
    {
        dwError = system(VMW_STOP_VMAFDD);
    }
    else if (VmStringCompareA(pszName, "vmdir", FALSE) == 0)
    {
        dwError = system(VMW_STOP_VMDIRD);
    }
    else if (VmStringCompareA(pszName, "vmca", FALSE) == 0)
    {
        dwError = system(VMW_STOP_VMCAD);
    }
    else if (VmStringCompareA(pszName, "vmdns", FALSE) == 0)
    {
        dwError = system(VMW_STOP_VMDNSD);
    }
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

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
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployStartService(pszName);
    BAIL_ON_DEPLOY_ERROR(dwError);

error:

    return dwError;
}
