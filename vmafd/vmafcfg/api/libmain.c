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
VmAfCfgInitCfgPackage(
    VOID
    );

static
VOID
VmAfCfgFreeCfgPackage(
    VOID
    );

DWORD
VmAfCfgInit(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmAfCfgInitCfgPackage();
    BAIL_ON_VMAF_CFG_ERROR(dwError);

error:

    return dwError;
}

VOID
VmAfCfgShutdown(
    VOID
    )
{
    VmAfCfgFreeCfgPackage();
}

static
DWORD
VmAfCfgInitCfgPackage(
    VOID
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_PACKAGE pCfgPackage = NULL;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
#ifdef _WIN32
        dwError = VmAfWinCreateConfigPackage(&pCfgPackage);
#else
        dwError = VmAfPosixCreateConfigPackage(&pCfgPackage);
#endif
        if (!dwError)
        {
            gpVmAfCfgApiGlobals->pCfgPackage = pCfgPackage;
        }
    }

    return dwError;
}

static
VOID
VmAfCfgFreeCfgPackage(
    VOID
    )
{
    if (gpVmAfCfgApiGlobals->pCfgPackage)
    {
#ifdef _WIN32
        VmAfWinFreeConfigPackage(gpVmAfCfgApiGlobals->pCfgPackage);
#else
        VmAfPosixFreeConfigPackage(gpVmAfCfgApiGlobals->pCfgPackage);
#endif

        gpVmAfCfgApiGlobals->pCfgPackage = NULL;
    }
}
