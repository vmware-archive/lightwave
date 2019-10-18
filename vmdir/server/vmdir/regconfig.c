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
VmDirRegGetConfig(
    PCSTR               pszSubKey,
    PVMDIR_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    );

static
DWORD
VmDirRegConfigGetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
DWORD
VmDirRegConfigGetMultiString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
VOID
VmDirRegConfigTableFreeContents(
    PVMDIR_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    );

static
DWORD
VmDirRegConfigMultiStringToDwords(
    PCSTR   pszValues,
    PDWORD* ppdwValues,
    DWORD*  pdwValues
    );

static
DWORD
VmDirRegConfigMultiStringToStrList(
    PCSTR               pszValues,
    PVMDIR_STRING_LIST* ppStrList
    );

DWORD
VmDirSrvUpdateConfig(
    VOID
    )
{
    DWORD dwError = 0;
    VMDIR_CONFIG_ENTRY initTable[] = VMDIR_CONFIG_INIT_TABLE_INITIALIZER;

    DWORD dwNumEntries = sizeof(initTable)/sizeof(initTable[0]);
    DWORD iEntry = 0;

    dwError = VmDirRegGetConfig(
            VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
            initTable,
            dwNumEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMDIR_CONFIG_ENTRY pEntry = &initTable[iEntry];

        if (!VmDirStringCompareA(
                     pEntry->pszName,
                     VMDIR_REG_KEY_ALLOW_INSECURE_AUTH,
                     TRUE))
        {
            gVmdirGlobals.bAllowInsecureAuth = pEntry->dwValue ? TRUE : FALSE;
        }
        else if (!VmDirStringCompareA(
                     pEntry->pszName,
                     VMDIR_REG_KEY_DISABLE_VECS,
                     TRUE))
        {
            gVmdirGlobals.bDisableVECSIntegration = pEntry->dwValue ? TRUE : FALSE;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_LDAP_LISTEN_PORTS,
                    TRUE))
        {
            dwError = VmDirRegConfigMultiStringToDwords(
                        pEntry->pszValue,
                        &gVmdirGlobals.pdwLdapListenPorts,
                        &gVmdirGlobals.dwLdapListenPorts);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_LDAPS_LISTEN_PORTS,
                    TRUE))
        {
            dwError = VmDirRegConfigMultiStringToDwords(
                        pEntry->pszValue,
                        &gVmdirGlobals.pdwLdapsListenPorts,
                        &gVmdirGlobals.dwLdapsListenPorts);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_LDAP_CONNECT_PORTS,
                    TRUE))
        {
            dwError = VmDirRegConfigMultiStringToDwords(
                        pEntry->pszValue,
                        &gVmdirGlobals.pdwLdapConnectPorts,
                        &gVmdirGlobals.dwLdapConnectPorts);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_LDAPS_CONNECT_PORTS,
                    TRUE))
        {
            dwError = VmDirRegConfigMultiStringToDwords(
                        pEntry->pszValue,
                        &gVmdirGlobals.pdwLdapsConnectPorts,
                        &gVmdirGlobals.dwLdapsConnectPorts);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_HTTP_LISTEN_PORT,
                    TRUE))
        {
            gVmdirGlobals.dwHTTPListenPort = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_HTTPS_LISTEN_PORT,
                    TRUE))
        {
            gVmdirGlobals.dwHTTPSListenPort = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_HTTPS_API_LISTEN_PORT,
                    TRUE))
        {
            gVmdirGlobals.dwHTTPSApiListenPort = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_LDAP_RECV_TIMEOUT_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwLdapRecvTimeoutSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_MAX_OP_THREADS,
                    TRUE))
        {
            gVmdirGlobals.dwMaxFlowCtrlThr = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_MAX_INDEX_SCAN,
                    TRUE))
        {
            gVmdirGlobals.dwMaxIndexScan = VMDIR_MAX(pEntry->dwValue, 512);
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_EFFICIENT_READ_OP,
                    TRUE))
        {
            gVmdirServerGlobals.dwEfficientReadOpTimeMS = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_EFFICIENT_WRITE_OP,
                    TRUE))
        {
            gVmdirServerGlobals.dwEfficientWriteOpTimeMS = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_SMALL_CANDIDATE_SET,
                    TRUE))
        {
            gVmdirGlobals.dwSmallCandidateSet = VMDIR_MAX(pEntry->dwValue, 32);
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_ALLOW_ADMIN_LOCKOUT,
                    TRUE))
        {
            gVmdirGlobals.bAllowAdminLockout = pEntry->dwValue ? TRUE : FALSE;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_ALLOW_IMPORT_OP_ATTR,
                    TRUE))
        {
            gVmdirGlobals.bAllowImportOpAttrs = pEntry->dwValue ? TRUE : FALSE;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_LDAP_SEARCH_TIMEOUT_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwLdapSearchTimeoutSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_TRACK_LAST_LOGIN_TIME,
                    TRUE))
        {
            gVmdirGlobals.bTrackLastLoginTime = pEntry->dwValue ? TRUE : FALSE;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_TOMBSTONE_EXPIRATION_IN_SEC,
                    TRUE))
        {
            gVmdirServerGlobals.dwTombstoneExpirationPeriod = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_TOMBSTONE_REAPING_FREQ_IN_SEC,
                    TRUE))
        {
            gVmdirServerGlobals.dwTombstoneThreadFrequency = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_LDAP_CONNECT_TIMEOUT_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwLdapConnectTimeoutSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_MAX_INTERNAL_SEARCH,
                    TRUE))
        {
            gVmdirServerGlobals.dwMaxInternalSearchLimit = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_OPERATIONS_THREAD_TIMEOUT_IN_MILLI_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwOperationsThreadTimeoutInMilliSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_REPL_CONSUMER_THREAD_TIMEOUT_IN_MILLI_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwReplConsumerThreadTimeoutInMilliSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_SUPPLIER_THREAD_TIMEOUT_IN_MILLI_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwSupplierThrTimeoutInMilliSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_WRITE_TIMEOUT_IN_MILLI_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwWriteTimeoutInMilliSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_ENABLE_REGIONAL_MASTER,
                    TRUE))
        {
            gVmdirGlobals.bEnableRegionalMaster = pEntry->dwValue ? TRUE : FALSE;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_WARN_PWD_EXPIRING_SEC,
                    TRUE))
        {
            gVmdirGlobals.iWarnPwdExpiring = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_MAX_SEARCH_ITERATION,
                    TRUE))
        {
            gVmdirGlobals.dwMaxSearchIteration = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_MAX_SEARCH_ITERATION_TXN,
                    TRUE))
        {
            gVmdirGlobals.dwMaxSearchIterationTxn = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_ENABLE_SEARCH_OPTIMIZATION,
                    TRUE))
        {
            gVmdirGlobals.dwEnableSearchOptimization = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_REST_WORKER,
                    TRUE))
        {
            gVmdirGlobals.dwRESTWorker = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_REST_CLIENT,
                    TRUE))
        {
            gVmdirGlobals.dwRESTClient = pEntry->dwValue;
        }
    }

cleanup:

    VmDirRegConfigTableFreeContents(initTable, dwNumEntries);

    return dwError;

error:

    VmDirSrvFreeConfig();

    goto cleanup;
}

VOID
VmDirSrvFreeConfig(
    VOID
    )
{
    VMDIR_SAFE_FREE_MEMORY(gVmdirGlobals.pdwLdapListenPorts);
    gVmdirGlobals.dwLdapListenPorts = 0;

    VMDIR_SAFE_FREE_MEMORY(gVmdirGlobals.pdwLdapsListenPorts);
    gVmdirGlobals.dwLdapsListenPorts = 0;

    VMDIR_SAFE_FREE_MEMORY(gVmdirGlobals.pdwLdapConnectPorts);
    gVmdirGlobals.dwLdapConnectPorts = 0;

    VMDIR_SAFE_FREE_MEMORY(gVmdirGlobals.pdwLdapsConnectPorts);
    gVmdirGlobals.dwLdapsConnectPorts = 0;
}

DWORD
VmDirRegGetMultiSZ(
    PCSTR   pszKeyPath,
    PCSTR   pszKeyName,
    PVMDIR_STRING_LIST* ppStrList
    )
{
    DWORD               dwError = 0;
    PSTR                pszValue = NULL;
    PVMDIR_STRING_LIST  pStrList = NULL;

    if (!pszKeyPath || !pszKeyName || !ppStrList)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRegConfigGetMultiString(
                            pszKeyPath,
                            pszKeyName,
                            &pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegConfigMultiStringToStrList(pszValue, &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    // bail if there is no content in pStrList
    if (!pStrList || pStrList->dwCount == 0)
    {
        dwError = VMDIR_ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppStrList = pStrList; pStrList = NULL;

cleanup:
    if (pStrList)
    {
        VmDirStringListFree(pStrList);
    }
    VMDIR_SAFE_FREE_MEMORY(pszValue);

    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmDirRegGetConfig(
    PCSTR               pszSubKey,
    PVMDIR_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMDIR_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        switch (pEntry->Type)
        {
            case VMDIR_CONFIG_VALUE_TYPE_STRING:

                dwError = VmDirRegConfigGetString(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->pszValue);
                if (dwError != 0)
                {
                    dwError = VmDirAllocateStringA(
                                    pEntry->pszDefault,
                                    &pEntry->pszValue);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s, string key (%s) value(%s)", pEntry->pszName, pEntry->pszValue);
                break;

            case VMDIR_CONFIG_VALUE_TYPE_MULTISTRING:

                dwError = VmDirRegConfigGetMultiString(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->pszValue);
                if (dwError != 0)
                {
                    dwError = VmDirAllocateMultiStringA(
                                    pEntry->pszDefault,
                                    &pEntry->pszValue);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s, multi-string key (%s) value(%s)", pEntry->pszName, pEntry->pszValue);
                break;

            case VMDIR_CONFIG_VALUE_TYPE_DWORD:

                dwError = VmRegCfgGetKeyDword(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->dwValue,
                            0);
                if (dwError != 0)
                {
                    pEntry->dwValue = pEntry->dwDefault;
                }

                if (pCfgTable[iEntry].dwValue > pCfgTable[iEntry].dwMax)
                {
                    pEntry->dwValue = pEntry->dwMax;

                }
                else if (pEntry->dwValue < pEntry->dwMin)
                {
                    pEntry->dwValue = pEntry->dwMin;
                }

                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s, dword key (%s) value(%lu)", pEntry->pszName, pEntry->dwValue);
                break;

            case VMDIR_CONFIG_VALUE_TYPE_BOOLEAN:

                dwError = VmRegCfgGetKeyDword(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->dwValue,
                            0);

                if (dwError != 0)
                {
                    pEntry->dwValue = pEntry->dwDefault;
                }

                pEntry->dwValue = pEntry->dwValue == 0 ? FALSE : TRUE;

                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s, bool key (%s) value(%d)", pEntry->pszName, pEntry->dwValue);
                break;

            default:

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
VmDirRegConfigGetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    )
{
    DWORD   dwError = 0;
    CHAR    szValue[VMDIR_MAX_CONFIG_VALUE_LENGTH] = {0};
    size_t  dwszValueSize = sizeof(szValue);
    PSTR    pszValue = NULL;

    dwError = VmRegCfgGetKeyStringA(pszSubKey, pszKeyName, szValue, dwszValueSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszValue);

    goto cleanup;
}

static
DWORD
VmDirRegConfigGetMultiString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    )
{
    DWORD   dwError = 0;
    CHAR    szValue[VMDIR_MAX_CONFIG_VALUE_LENGTH] = {0};
    size_t  dwszValueSize = sizeof(szValue);
    PSTR    pszValue = NULL;

    dwError = VmRegCfgGetKeyMultiSZA(pszSubKey, pszKeyName, szValue, &dwszValueSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateAndCopyMemory(szValue, dwszValueSize+1, (PVOID*)&pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszValue);
    goto cleanup;
}

static
VOID
VmDirRegConfigTableFreeContents(
    PVMDIR_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMDIR_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        if (pEntry->Type == VMDIR_CONFIG_VALUE_TYPE_STRING)
        {
            VMDIR_SAFE_FREE_STRINGA(pEntry->pszValue);
        }
        else if (pEntry->Type == VMDIR_CONFIG_VALUE_TYPE_MULTISTRING)
        {
            VMDIR_SAFE_FREE_MEMORY(pEntry->pszValue);
        }
    }
}

static
DWORD
VmDirRegConfigMultiStringToDwords(
    PCSTR   pszValues,
    PDWORD* ppdwValues,
    DWORD*  pdwValuesLen
    )
{
    DWORD dwError = 0;
    PDWORD pdwValues = NULL;
    DWORD dwValuesLen = 0;
    DWORD dwCount = 0;
    PCSTR pszIter = NULL;

    if (pszValues)
    {
        pszIter = pszValues;
        while (pszIter != NULL && *pszIter != '\0')
        {
            dwValuesLen++;

            pszIter += VmDirStringLenA(pszIter) + 1;
        }

        /* Allocate space for one even if no space is really needed,
         * that way we have a valid pointer.
         */
        dwError = VmDirAllocateMemory(sizeof(DWORD) * (dwValuesLen == 0 ? 1 : dwValuesLen), (PVOID)&pdwValues);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszIter = pszValues;
        while (pszIter != NULL && *pszIter != '\0')
        {
            DWORD dwVal = atoi(pszIter);
            pdwValues[dwCount++] = dwVal;
            pszIter += VmDirStringLenA(pszIter) + 1;
        }
    }

    *ppdwValues = pdwValues;
    *pdwValuesLen = dwValuesLen;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pdwValues);
    goto cleanup;
}

static
DWORD
VmDirRegConfigMultiStringToStrList(
    PCSTR               pszValues,
    PVMDIR_STRING_LIST* ppStrList
    )
{
    DWORD               dwError = 0;
    DWORD               dwValuesLen = 0;
    PCSTR               pszIter = NULL;
    PVMDIR_STRING_LIST  pStrList = NULL;

    if (pszValues)
    {
        pszIter = pszValues;
        while (pszIter != NULL && *pszIter != '\0')
        {
            dwValuesLen++;

            pszIter += VmDirStringLenA(pszIter) + 1;
        }

        dwError = VmDirStringListInitialize(&pStrList, dwValuesLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszIter = pszValues;
        while (pszIter != NULL && *pszIter != '\0')
        {
            dwError = VmDirStringListAddStrClone(pszIter, pStrList);
            BAIL_ON_VMDIR_ERROR(dwError);

            pszIter += VmDirStringLenA(pszIter) + 1;
        }

        *ppStrList = pStrList; pStrList = NULL;
    }

cleanup:
    if (pStrList)
    {
        VmDirStringListFree(pStrList);
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirGetMaxDbSizeMb(
    PDWORD pMaxDbSizeMb
    )
{
    DWORD keyValue = 0;
    DWORD dwError = 0;

    dwError = VmRegCfgGetKeyDword(
            VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
            VMDIR_REG_KEY_MAXIMUM_DB_SIZE_MB,
            &keyValue,
            20*1024);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pMaxDbSizeMb = keyValue;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirGetMdbWalEnable(
    BOOLEAN *pbMdbEnableWal
    )
{
    DWORD keyValue = 1;
    DWORD dwError = 0;

    if (pbMdbEnableWal==NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pbMdbEnableWal = FALSE;

    dwError = VmRegCfgGetKeyDword(
            VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
            VMDIR_REG_KEY_MDB_ENABLE_WAL,
            &keyValue,
            1);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbMdbEnableWal = (BOOLEAN)(keyValue!=0);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirGetMdbChkptInterval(
    DWORD *pdwMdbChkptInterval
    )
{
    DWORD keyValue = 0;
    DWORD dwError = 0;

    if (pdwMdbChkptInterval==NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwMdbChkptInterval = VMDIR_REG_KEY_MDB_CHKPT_INTERVAL_DEFAULT;

    dwError = VmRegCfgGetKeyDword(
            VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
            VMDIR_REG_KEY_MDB_CHKPT_INTERVAL,
            &keyValue,
            VMDIR_REG_KEY_MDB_CHKPT_INTERVAL_DEFAULT);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (keyValue < VMDIR_REG_KEY_MDB_CHKPT_INTERVAL_MIN ||
        keyValue > VMDIR_REG_KEY_MDB_CHKPT_INTERVAL_MAX)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwMdbChkptInterval = keyValue;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirGetLdapCopyEnable(
    BOOLEAN *pbLdapCopyEnable
    )
{
    DWORD keyValue = 0;
    DWORD dwError = 0;

    if (pbLdapCopyEnable == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pbLdapCopyEnable = FALSE;

    dwError = VmRegCfgGetKeyDword(
            VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
            VMDIR_REG_KEY_LDAP_COPY_ENABLE,
            &keyValue,
            0);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbLdapCopyEnable = (BOOLEAN)(keyValue!=0);

cleanup:
    return dwError;

error:
    goto cleanup;
}
