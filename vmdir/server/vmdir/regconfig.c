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
VmDirRegConfigHandleOpen(
    PVMDIR_CONFIG_CONNECTION_HANDLE *ppCfgHandle
    );

static
DWORD
VmDirRegConfigGetDword(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    );

static
DWORD
VmDirRegConfigGetString(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
DWORD
VmDirRegConfigGetMultiString(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
VOID
VmDirRegConfigHandleClose(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle
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
                    VMDIR_REG_KEY_REST_LISTEN_PORT,
                    TRUE))
        {
            dwError = VmDirAllocateStringA(
                        pEntry->pszValue,
                        &gVmdirGlobals.pszRestListenPort);
            BAIL_ON_VMDIR_ERROR(dwError);
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
                    VMDIR_REG_KEY_MAX_SIZELIMIT_SCAN,
                    TRUE))
        {
            gVmdirGlobals.dwMaxSizelimitScan = VMDIR_MAX(pEntry->dwValue, 0);
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
                    VMDIR_REG_KEY_PAGED_SEARCH_READ_AHEAD,
                    TRUE))
        {
            gVmdirGlobals.bPagedSearchReadAhead = !!pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_COPY_DB_WRITES_MIN,
                    TRUE))
        {
            gVmdirGlobals.dwCopyDbWritesMin = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_COPY_DB_INTERVAL_IN_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwCopyDbIntervalInSec = pEntry->dwValue;
        }
        else if (!VmDirStringCompareA(
                    pEntry->pszName,
                    VMDIR_REG_KEY_COPY_DB_BLOCK_WRITE_IN_SEC,
                    TRUE))
        {
            gVmdirGlobals.dwCopyDbBlockWriteInSec = pEntry->dwValue;
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

    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    if (!pszKeyPath || !pszKeyName || !ppStrList)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRegConfigHandleOpen(&pCfgHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegConfigGetMultiString(
                            pCfgHandle,
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
    if (pCfgHandle)
    {
        VmDirRegConfigHandleClose(pCfgHandle);
    }
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
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    dwError = VmDirRegConfigHandleOpen(&pCfgHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMDIR_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        switch (pEntry->Type)
        {
            case VMDIR_CONFIG_VALUE_TYPE_STRING:

                dwError = VmDirRegConfigGetString(
                            pCfgHandle,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->pszValue);
                if (dwError != 0)
                {   // use default value
                    dwError = VmDirAllocateStringA(
                                    pEntry->pszDefault,
                                    &pEntry->pszValue);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                break;

            case VMDIR_CONFIG_VALUE_TYPE_MULTISTRING:

                dwError = VmDirRegConfigGetMultiString(
                            pCfgHandle,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->pszValue);
                if (dwError != 0)
                {   // use default value

                    dwError = VmDirAllocateMultiStringA(
                                    pEntry->pszDefault,
                                    &pEntry->pszValue);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
                break;


            case VMDIR_CONFIG_VALUE_TYPE_DWORD:

                dwError = VmDirRegConfigGetDword(
                            pCfgHandle,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->dwValue);
                if (dwError != 0)
                {   // use default value
                    pEntry->dwValue = pEntry->dwDefault;
                }

                if (pCfgTable[iEntry].dwValue > pCfgTable[iEntry].dwMax)
                {
                    VmDirLog(LDAP_DEBUG_ANY,
                            "Config [%s] value (%d) too big, using (%d).",
                            pEntry->pszName,
                            pEntry->dwValue,
                            pEntry->dwMax);

                    pEntry->dwValue = pEntry->dwMax;

                }

                if (pEntry->dwValue < pEntry->dwMin)
                {
                    VmDirLog(
                            LDAP_DEBUG_ANY,
                            "Config [%s] value (%d) too small, using (%d).",
                            pEntry->pszName,
                            pEntry->dwValue,
                            pEntry->dwMin);

                    pEntry->dwValue = pEntry->dwMin;
                }

                break;

            case VMDIR_CONFIG_VALUE_TYPE_BOOLEAN:

                dwError = VmDirRegConfigGetDword(
                            pCfgHandle,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->dwValue);

                if (dwError != 0)
                {   // use default value
                    pEntry->dwValue = pEntry->dwDefault;
                }

                pEntry->dwValue = pEntry->dwValue == 0 ? FALSE : TRUE;

                break;

            default:

                VmDirLog(
                        LDAP_DEBUG_ANY,
                        "VmDirRegConfigProcess key [%s] type (%d) not supported.",
                        pEntry->pszName,
                        pEntry->Type);

                break;
        }
    }

    dwError = 0;

cleanup:

    if (pCfgHandle)
    {
        VmDirRegConfigHandleClose(pCfgHandle);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDirRegConfigHandleOpen(
    PVMDIR_CONFIG_CONNECTION_HANDLE *ppCfgHandle)
{
    DWORD dwError = 0;
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    dwError = VmDirAllocateMemory(
                sizeof(VMDIR_CONFIG_CONNECTION_HANDLE),
                (PVOID*)&pCfgHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    dwError = RegOpenServer(&pCfgHandle->hConnection);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

#ifndef _WIN32
    dwError = RegOpenKeyExA(
                pCfgHandle->hConnection,
                NULL,
                HKEY_THIS_MACHINE,
                0,
                KEY_READ,
                &pCfgHandle->hKey);
    BAIL_ON_VMDIR_ERROR(dwError);
#else
        dwError = RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                NULL,
                0,
                KEY_READ,
                &pCfgHandle->hKey);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    *ppCfgHandle = pCfgHandle;

cleanup:

    return dwError;

error:

    *ppCfgHandle = NULL;

    if (pCfgHandle)
    {
        VmDirRegConfigHandleClose(pCfgHandle);
    }

    goto cleanup;
}


static
DWORD
VmDirRegConfigGetDword(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    )
{
    DWORD dwError =0;
    DWORD dwValue = 0;
    DWORD dwValueSize = sizeof(dwValue);

    dwError = RegGetValueA(
#ifndef _WIN32
                pCfgHandle->hConnection,
#endif
                pCfgHandle->hKey,
                pszSubKey,
                pszKeyName,
                RRF_RT_REG_DWORD,
                NULL,
                (PVOID)&dwValue,
                &dwValueSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

static
DWORD
VmDirRegConfigGetString(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue)
{
    DWORD dwError = 0;
    char szValue[VMDIR_MAX_CONFIG_VALUE_LENGTH] = {0};
    DWORD dwszValueSize = sizeof(szValue);
    PSTR pszValue = NULL;

    dwError = RegGetValueA(
#ifndef _WIN32
                pCfgHandle->hConnection,
#endif
                pCfgHandle->hKey,
                pszSubKey,
                pszKeyName,
                RRF_RT_REG_SZ,
                NULL,
                szValue,
                &dwszValueSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    VMDIR_SAFE_FREE_STRINGA(pszValue);

    goto cleanup;
}

static
DWORD
VmDirRegConfigGetMultiString(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    )
{
    DWORD dwError = 0;
    char szValue[VMDIR_MAX_CONFIG_VALUE_LENGTH] = {0};
    DWORD dwszValueSize = sizeof(szValue);
    PSTR pszValue = NULL;

    dwError = RegGetValueA(
#ifndef _WIN32
                pCfgHandle->hConnection,
#endif
                pCfgHandle->hKey,
                pszSubKey,
                pszKeyName,
                RRF_RT_REG_MULTI_SZ,
                NULL,
                szValue,
                &dwszValueSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(dwszValueSize, (PVOID)&pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(pszValue, dwszValueSize, szValue, dwszValueSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    VMDIR_SAFE_FREE_MEMORY(pszValue);

    goto cleanup;
}
static
VOID
VmDirRegConfigHandleClose(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle
    )
{
#ifndef _WIN32
    if (pCfgHandle->hConnection)
    {
        if (pCfgHandle->hKey)
        {
            DWORD dwError = RegCloseKey(
                        pCfgHandle->hConnection,
                        pCfgHandle->hKey);
            if (dwError != 0)
            {   // Do not bail, best effort to cleanup.
                VmDirLog(
                        LDAP_DEBUG_ANY,
                        "RegCloseKey failed, Error code: (%u)(%s)",
                        dwError,
                        VDIR_SAFE_STRING(LwWin32ErrorToName(dwError)));
            }
        }

        RegCloseServer(pCfgHandle->hConnection);
    }
#else
    if (pCfgHandle->hKey)
    {
        RegCloseKey(pCfgHandle->hKey);
    }
#endif

    VMDIR_SAFE_FREE_MEMORY(pCfgHandle);
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

    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    dwError = VmDirRegConfigHandleOpen(&pCfgHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegConfigGetDword(
                            pCfgHandle,
                            VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
                            VMDIR_REG_KEY_MAXIMUM_DB_SIZE_MB,
                            &keyValue);

    BAIL_ON_VMDIR_ERROR(dwError);
    *pMaxDbSizeMb = keyValue;

cleanup:
    if (pCfgHandle)
    {
        VmDirRegConfigHandleClose(pCfgHandle);
    }
    return dwError;
error:
    goto cleanup;
}

DWORD
_VmDirDbCpReadRegistry(
    PDWORD pdwCopyDbWritesMin,
    PDWORD pdwCopyDbIntervalInSec,
    PDWORD pdwCopyDbBlockWriteInSec
    )
{
#ifndef WIN32
    DWORD dwError = 0;
    DWORD dwValue = 0;
    PSTR  pszLocalErrorMsg = NULL;
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    dwError = VmDirRegConfigHandleOpen(&pCfgHandle);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
      "_VmDirDbCpReadRegistry: VmDirRegConfigHandleOpen error %d", dwError);

    dwError = VmDirRegConfigGetDword(pCfgHandle, VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
            VMDIR_REG_KEY_COPY_DB_INTERVAL_IN_SEC, &dwValue);
    if (dwError)
    {
        if (dwError != LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirDbCpReadRegistry %s error %d",
                  VMDIR_REG_KEY_COPY_DB_INTERVAL_IN_SEC, dwError);
        }
    } else
    {
        *pdwCopyDbIntervalInSec = dwValue;
    }

    dwError = VmDirRegConfigGetDword(pCfgHandle, VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
                VMDIR_REG_KEY_COPY_DB_WRITES_MIN, &dwValue);
    if (dwError)
    {
        if (dwError != LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirDbCpReadRegistry %s error %d",
                  VMDIR_REG_KEY_COPY_DB_WRITES_MIN, dwError);
        }
    } else
    {
        *pdwCopyDbWritesMin = dwValue;
    }

    dwError = VmDirRegConfigGetDword(pCfgHandle, VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
            VMDIR_REG_KEY_COPY_DB_BLOCK_WRITE_IN_SEC, &dwValue);
    if (dwError)
    {
        if (dwError != LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirDbCpReadRegistry %s error %d",
                  VMDIR_REG_KEY_COPY_DB_BLOCK_WRITE_IN_SEC, dwError);
        }
    } else
    {
        *pdwCopyDbBlockWriteInSec = dwValue;
    }

cleanup:
    if (pCfgHandle)
    {
        VmDirRegConfigHandleClose(pCfgHandle);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto cleanup;
#else
    return 0;
#endif
}
