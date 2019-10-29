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



#if defined(_WIN32) && !defined(WIN2008)
#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#define WINVER _WIN32_WINNT
#endif

#include "includes.h"

static
DWORD
VmKdcRegGetConfig(
    PCSTR               pszSubKey,
    PVMKDC_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    );

static
DWORD
VmKdcRegConfigGetDword(
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    );

static
DWORD
VmKdcRegConfigGetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
VOID
VmKdcRegConfigTableFreeContents(
    PVMKDC_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    );

DWORD
VmKdcSrvUpdateConfig(
    PVMKDC_GLOBALS pGlobals
    )
{
    DWORD dwError = 0;
    VMKDC_CONFIG_ENTRY initTable[] = VMKDC_CONFIG_INIT_TABLE_INITIALIZER;
    DWORD dwNumEntries = sizeof(initTable)/sizeof(initTable[0]);
    DWORD iEntry = 0;

    dwError = VmKdcRegGetConfig(
                VMKDC_CONFIG_PARAMETER_KEY_PATH,
                initTable,
                dwNumEntries);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMKDC_CONFIG_ENTRY pEntry = &initTable[iEntry];

        if (!VmKdcStringCompareA(
                pEntry->pszName,
                VMKDC_REG_KEY_KERBEROS_PORT,
                TRUE))
        {
            pGlobals->iListenPort = pEntry->cfgValue.dwValue;
        }
        else if (!VmKdcStringCompareA(
                pEntry->pszName,
                VMKDC_REG_KEY_DEFAULT_REALM,
                TRUE))
        {
            dwError = VmKdcAllocateStringA(pEntry->cfgValue.pszValue,
                                           &pGlobals->pszDefaultRealm);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        else if (!VmKdcStringCompareA(
                pEntry->pszName,
                VMKDC_REG_KEY_CLOCK_SKEW,
                TRUE))
        {
            pGlobals->iClockSkew = pEntry->cfgValue.dwValue;
        }
        else if (!VmKdcStringCompareA(
                pEntry->pszName,
                VMKDC_REG_KEY_MAX_LIFE,
                TRUE))
        {
            pGlobals->iMaxLife = pEntry->cfgValue.dwValue;
        }
        else if (!VmKdcStringCompareA(
                pEntry->pszName,
                VMKDC_REG_KEY_MAX_RENEWABLE_LIFE,
                TRUE))
        {
            pGlobals->iMaxRenewableLife = pEntry->cfgValue.dwValue;
        }
    }

cleanup:

    VmKdcRegConfigTableFreeContents(initTable, dwNumEntries);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmKdcRegGetConfig(
    PCSTR               pszSubKey,
    PVMKDC_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMKDC_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        switch (pEntry->Type)
        {
            case VMKDC_CONFIG_VALUE_TYPE_STRING:

                dwError = VmKdcRegConfigGetString(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.pszValue);
                if (dwError != 0)
                {   // use default value
                    dwError = VmKdcAllocateStringA(
                                    pEntry->defaultValue.pszDefault,
                                    &pEntry->cfgValue.pszValue);
                    BAIL_ON_VMKDC_ERROR(dwError);
                }
                break;

            case VMKDC_CONFIG_VALUE_TYPE_DWORD:

                dwError = VmKdcRegConfigGetDword(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.dwValue);
                if (dwError != 0)
                {   // use default value
                    pEntry->cfgValue.dwValue = pEntry->defaultValue.dwDefault;
                }

                if (pCfgTable[iEntry].cfgValue.dwValue > pCfgTable[iEntry].dwMax)
                {
                    VMDIR_LOG_WARNING(
                            VMDIR_LOG_MASK_ALL,
                            "Config [%s] value (%d) too big, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMax);

                    pEntry->cfgValue.dwValue = pEntry->dwMax;

                }

                if (pEntry->cfgValue.dwValue < pEntry->dwMin)
                {
                    VMDIR_LOG_WARNING(
                            VMDIR_LOG_MASK_ALL,
                            "Config [%s] value (%d) too small, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMin);

                    pEntry->cfgValue.dwValue = pEntry->dwMin;
                }

                break;

            case VMKDC_CONFIG_VALUE_TYPE_BOOLEAN:

                dwError = VmKdcRegConfigGetDword(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.dwValue);

                if (dwError != 0)
                {   // use default value
                    pEntry->cfgValue.dwValue = pEntry->defaultValue.dwDefault;
                }

                pEntry->cfgValue.dwValue =
                        pEntry->cfgValue.dwValue == 0 ? FALSE : TRUE;

                break;

            default:

                VMDIR_LOG_ERROR(
                        VMDIR_LOG_MASK_ALL,
                        "VmKdcRegConfigProcess key [%s] type (%d) not supported.",
                        pEntry->pszName,
                        pEntry->Type);

                break;
        }
    }

    dwError = 0;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmKdcRegConfigGetDword(
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    )
{
    DWORD dwError =0;
    DWORD dwValue = 0;

    dwError = VmRegCfgGetKeyDword(
                pszSubKey,
                pszKeyName,
                &dwValue,
                0);
    BAIL_ON_VMKDC_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

static
DWORD
VmKdcRegConfigGetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue)
{
    DWORD   dwError = 0;
    char    szValue[VMKDC_MAX_CONFIG_VALUE_LENGTH] = {0};
    size_t  dwszValueSize = sizeof(szValue);
    PSTR    pszValue = NULL;

    dwError = VmRegCfgGetKeyStringA(pszSubKey, pszKeyName, szValue, dwszValueSize);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    VMKDC_SAFE_FREE_STRINGA(pszValue);

    goto cleanup;
}

static
VOID
VmKdcRegConfigTableFreeContents(
    PVMKDC_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMKDC_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        if (pEntry->Type == VMKDC_CONFIG_VALUE_TYPE_STRING)
        {
            VMKDC_SAFE_FREE_STRINGA(pEntry->cfgValue.pszValue);
        }
    }
}
