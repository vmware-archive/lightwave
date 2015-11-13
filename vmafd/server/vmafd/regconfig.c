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
VmAfdRegGetConfig(
    PCSTR               pszSubKey,
    PVMAFD_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    );

static
VOID
VmAfdRegConfigTableFreeContents(
    PVMAFD_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    );

DWORD
VmAfdSrvUpdateConfig(
    PVMAFD_GLOBALS pGlobals
    )
{
    DWORD dwError = 0;
    VMAFD_CONFIG_ENTRY initTable[] = VMAFD_CONFIG_INIT_TABLE_INITIALIZER;
    DWORD dwNumEntries = sizeof(initTable)/sizeof(initTable[0]);
    DWORD iEntry = 0;
    PSTR pszKrb5Config = NULL;
    PSTR pszKrb5Keytab = NULL;

    dwError = VmAfdRegGetConfig(
                VMAFD_CONFIG_PARAMETER_KEY_PATH,
                initTable,
                dwNumEntries);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMAFD_CONFIG_ENTRY pEntry = &initTable[iEntry];

        if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_LOG_FILE,
                TRUE))
        {
            dwError = VmAfdAllocateStringA(pEntry->cfgValue.pszValue,
                                           &pGlobals->pszLogFile);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_KRB5_CONF,
                TRUE))
        {
            dwError = VmAfdAllocateStringA(pEntry->cfgValue.pszValue,
                                           &pGlobals->pszKrb5Config);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_KEYTAB_PATH,
                TRUE))
        {
            dwError = VmAfdAllocateStringA(pEntry->cfgValue.pszValue,
                                           &pGlobals->pszKrb5Keytab);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_DOMAIN_STATE,
                TRUE))
        {
            pGlobals->domainState = (VMAFD_DOMAIN_STATE)pEntry->cfgValue.dwValue;
        }
        else if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_CERT_SEC,
                TRUE))
        {
            pGlobals->dwCertCheckSec = pEntry->cfgValue.dwValue;
        }
        else if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_MAX_OLD_LOGS,
                TRUE))
        {
            pGlobals->dwMaxOldLogs = pEntry->cfgValue.dwValue;
        }
        else if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_MAX_LOG_SIZE,
                TRUE))
        {
            pGlobals->dwMaxLogSize = pEntry->cfgValue.dwValue;
        }
        else if (!VmAfdStringCompareA(
                pEntry->pszName,
                VMAFD_REG_KEY_RPC_CONFIG,
                TRUE))
        {
            pGlobals->bEnableRPC = pEntry->cfgValue.dwValue?TRUE:FALSE;
        }
    }

#ifdef USE_DEFAULT_KRB5_PATHS
    /*
     * Force use of MIT Kerberos default file locations
     */
    dwError = VmAfdAllocateStringA(VMAFD_KRB5_CONF,
                                   &pszKrb5Config);
    BAIL_ON_VMAFD_ERROR(dwError);
    dwError = VmAfdAllocateStringA(VMAFD_KEYTAB_PATH,
                                   &pszKrb5Keytab);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pGlobals->pszKrb5Config)
    {
        VMAFD_SAFE_FREE_STRINGA(pGlobals->pszKrb5Config);
    }
    if (pGlobals->pszKrb5Keytab)
    {
        VMAFD_SAFE_FREE_STRINGA(pGlobals->pszKrb5Keytab);
    }

    pGlobals->pszKrb5Config = pszKrb5Config;
    pGlobals->pszKrb5Keytab = pszKrb5Keytab;
    pszKrb5Config = NULL;
    pszKrb5Keytab = NULL;
#endif

cleanup:

    VmAfdRegConfigTableFreeContents(initTable, dwNumEntries);

    return dwError;

error:
    VMAFD_SAFE_FREE_STRINGA(pszKrb5Config);
    VMAFD_SAFE_FREE_STRINGA(pszKrb5Keytab);

    goto cleanup;
}

static
DWORD
VmAfdRegGetConfig(
    PCSTR               pszSubKey,
    PVMAFD_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;
    PVMAF_CFG_CONNECTION pCfgHandle = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;

    dwError = VmAfConfigOpenConnection(&pCfgHandle);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(pCfgHandle,
                                    "HKEY_LOCAL_MACHINE",
                                    0,
                                    KEY_READ,
                                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMAFD_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        switch (pEntry->Type)
        {
            case VMAFD_CONFIG_VALUE_TYPE_STRING:

                dwError = VmAfConfigReadStringValue(
                            pRootKey,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.pszValue);
                if (dwError != 0)
                {   // use default value
                    dwError = VmAfdAllocateStringA(
                                    pEntry->defaultValue.pszDefault,
                                    &pEntry->cfgValue.pszValue);
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case VMAFD_CONFIG_VALUE_TYPE_DWORD:

                dwError = VmAfConfigReadDWORDValue(
                            pRootKey,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.dwValue);
                if (dwError != 0)
                {   // use default value
                    pEntry->cfgValue.dwValue =
                    pEntry->defaultValue.dwDefault;
                }

                if (pCfgTable[iEntry].cfgValue.dwValue > pCfgTable[iEntry].dwMax)
                {
                    VmAfdLog(VMAFD_DEBUG_ANY,
                            "Config [%s] value (%d) too big, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMax);

                    pEntry->cfgValue.dwValue = pEntry->dwMax;

                }

                if (pEntry->cfgValue.dwValue < pEntry->dwMin)
                {
                    VmAfdLog(
                            VMAFD_DEBUG_ANY,
                            "Config [%s] value (%d) too small, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMin);

                    pEntry->cfgValue.dwValue = pEntry->dwMin;
                }

                break;

            case VMAFD_CONFIG_VALUE_TYPE_BOOLEAN:

                dwError = VmAfConfigReadDWORDValue(
                            pRootKey,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.dwValue);

                if (dwError != 0)
                {   // use default value
                    pEntry->cfgValue.dwValue =
                    pEntry->defaultValue.dwDefault;
                }

                pEntry->cfgValue.dwValue =
                        pEntry->cfgValue.dwValue == 0 ? FALSE : TRUE;

                break;

            default:

                VmAfdLog(
                        VMAFD_DEBUG_ANY,
                        "VmAfdRegConfigProcess key [%s] type (%d) not supported.",
                        pEntry->pszName,
                        pEntry->Type);

                break;
        }
    }

    dwError = 0;

cleanup:

    if (pCfgHandle)
    {
        VmAfConfigCloseConnection(pCfgHandle);
    }

    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }

    return dwError;

error:

    goto cleanup;
}

static
VOID
VmAfdRegConfigTableFreeContents(
    PVMAFD_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMAFD_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        if (pEntry->Type == VMAFD_CONFIG_VALUE_TYPE_STRING)
        {
            VMAFD_SAFE_FREE_STRINGA(pEntry->cfgValue.pszValue);
        }
    }
}
