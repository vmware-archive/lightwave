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

// TODO: remove once registy change is ported to windows ...
#ifndef _WIN32

static
DWORD
VmAuthsvcRegGetConfig(
    PCSTR               pszSubKey,
    PVMAUTHSVC_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    );

static
DWORD
VmAuthsvcRegConfigHandleOpen(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE *ppCfgHandle
    );

static
DWORD
VmAuthsvcRegConfigGetDword(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    );

static
DWORD
VmAuthsvcRegConfigGetString(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
VOID
VmAuthsvcRegConfigHandleClose(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle
    );

static
VOID
VmAuthsvcRegConfigTableFreeContents(
    PVMAUTHSVC_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    );

DWORD
VmAuthsvcSrvUpdateConfig(
    VOID
    )
{
    DWORD dwError = 0;
    VMAUTHSVC_CONFIG_ENTRY initTable[] = VMAUTHSVC_CONFIG_INIT_TABLE_INITIALIZER;
    DWORD dwNumEntries = sizeof(initTable)/sizeof(initTable[0]);
    DWORD iEntry = 0;

    dwError = VmAuthsvcRegGetConfig(
                VMAUTHSVC_CONFIG_PARAMETER_KEY_PATH,
                initTable,
                dwNumEntries);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMAUTHSVC_CONFIG_ENTRY pEntry = &initTable[iEntry];

        if (!VmAuthsvcStringCompareA(
                pEntry->pszName,
                VMAUTHSVC_REG_KEY_PORT,
                TRUE))
        {
            gVmauthsvcGlobals.iListenPort = pEntry->cfgValue.dwValue;
        }
    }

cleanup:

    VmAuthsvcRegConfigTableFreeContents(initTable, dwNumEntries);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAuthsvcConfigSetAdminCredentials(
    PCSTR pszUserDN,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCSTR pszParamsKeyPath      = VMAUTHSVC_CONFIG_PARAMETER_KEY_PATH;
    PCSTR pszValueAdminDN       = VMAUTHSVC_REG_KEY_ADMIN_DN;
    PCSTR pszCredsKeyPath       = VMAUTHSVC_CONFIG_CREDS_KEY_PATH;
    PCSTR pszValueAdminPassword = VMAUTHSVC_REG_KEY_ADMIN_PASSWD;
    HANDLE hConnection = NULL;

    if (IsNullOrEmptyString(pszUserDN) || !pszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    dwError = RegOpenServer(&hConnection);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = RegUtilSetValue(
                hConnection,
                HKEY_THIS_MACHINE,
                pszParamsKeyPath,
                NULL,
                pszValueAdminDN,
                REG_SZ,
                (PVOID)pszUserDN,
                VmAuthsvcStringLenA(pszUserDN));
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = RegUtilSetValue(
                hConnection,
                HKEY_THIS_MACHINE,
                pszCredsKeyPath,
                NULL,
                pszValueAdminPassword,
                REG_SZ,
                (PVOID)pszPassword,
                VmAuthsvcStringLenA(pszPassword));
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hConnection)
    {
        RegCloseServer(hConnection);
    }

    return dwError;
}

static
DWORD
VmAuthsvcRegGetConfig(
    PCSTR               pszSubKey,
    PVMAUTHSVC_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    dwError = VmAuthsvcRegConfigHandleOpen(&pCfgHandle);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMAUTHSVC_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        switch (pEntry->Type)
        {
            case VMAUTHSVC_CONFIG_VALUE_TYPE_STRING:

                dwError = VmAuthsvcRegConfigGetString(
                            pCfgHandle,
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.pszValue);
                if (dwError != 0)
                {   // use default value
                    dwError = VmAuthsvcAllocateStringA(
                                    pEntry->defaultValue.pszDefault,
                                    &pEntry->cfgValue.pszValue);
                    BAIL_ON_VMAUTHSVC_ERROR(dwError);
                }

                break;

            case VMAUTHSVC_CONFIG_VALUE_TYPE_DWORD:

                dwError = VmAuthsvcRegConfigGetDword(
                            pCfgHandle,
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
                    VmAuthsvcLog(VMAUTHSVC_DEBUG_ANY,
                            "Config [%s] value (%d) too big, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMax);

                    pEntry->cfgValue.dwValue = pEntry->dwMax;

                }

                if (pEntry->cfgValue.dwValue < pEntry->dwMin)
                {
                    VmAuthsvcLog(
                            VMAUTHSVC_DEBUG_ANY,
                            "Config [%s] value (%d) too small, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMin);

                    pEntry->cfgValue.dwValue = pEntry->dwMin;
                }

                break;

            case VMAUTHSVC_CONFIG_VALUE_TYPE_BOOLEAN:

                dwError = VmAuthsvcRegConfigGetDword(
                            pCfgHandle,
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

                VmAuthsvcLog(
                        VMAUTHSVC_DEBUG_ANY,
                        "VmAuthsvcRegConfigProcess key [%s] type (%d) not supported.",
                        pEntry->pszName,
                        pEntry->Type);

                break;
        }
    }

    dwError = 0;

cleanup:

    if (pCfgHandle)
    {
        VmAuthsvcRegConfigHandleClose(pCfgHandle);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAuthsvcRegConfigHandleOpen(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE *ppCfgHandle)
{
    DWORD dwError = 0;
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    dwError = VmAuthsvcAllocateMemory(
                sizeof(VMAUTHSVC_CONFIG_CONNECTION_HANDLE),
                (PVOID*)&pCfgHandle);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = RegOpenServer(&pCfgHandle->hConnection);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = RegOpenKeyExA(
                pCfgHandle->hConnection,
                NULL,
                HKEY_THIS_MACHINE,
                0,
                KEY_READ,
                &pCfgHandle->hKey);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    *ppCfgHandle = pCfgHandle;

cleanup:

    return dwError;

error:

    *ppCfgHandle = NULL;

    if (pCfgHandle)
    {
        VmAuthsvcRegConfigHandleClose(pCfgHandle);
    }

    goto cleanup;
}


static
DWORD
VmAuthsvcRegConfigGetDword(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    )
{
    DWORD dwError =0;
    DWORD dwValue = 0;
    DWORD dwValueSize = sizeof(dwValue);

    dwError = RegGetValueA(
                pCfgHandle->hConnection,
                pCfgHandle->hKey,
                pszSubKey,
                pszKeyName,
                RRF_RT_REG_DWORD,
                NULL,
                (PVOID)&dwValue,
                &dwValueSize);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

static
DWORD
VmAuthsvcRegConfigGetString(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue)
{
    DWORD dwError = 0;
    char szValue[VMAUTHSVC_MAX_CONFIG_VALUE_LENGTH] = {0};
    DWORD dwszValueSize = sizeof(szValue);
    PSTR pszValue = NULL;

    dwError = RegGetValueA(
                pCfgHandle->hConnection,
                pCfgHandle->hKey,
                pszSubKey,
                pszKeyName,
                RRF_RT_REG_SZ,
                NULL,
                szValue,
                &dwszValueSize);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = VmAuthsvcAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    VMAUTHSVC_SAFE_FREE_STRINGA(pszValue);

    goto cleanup;
}

static
VOID
VmAuthsvcRegConfigHandleClose(
    PVMAUTHSVC_CONFIG_CONNECTION_HANDLE pCfgHandle
    )
{
    if (pCfgHandle->hConnection)
    {
        if (pCfgHandle->hKey)
        {
            DWORD dwError = RegCloseKey(
                        pCfgHandle->hConnection,
                        pCfgHandle->hKey);
            if (dwError != 0)
            {   // Do not bail, best effort to cleanup.
                VmAuthsvcLog(
                        VMAUTHSVC_DEBUG_ANY,
                        "RegCloseKey failed, Error code: (%u)(%s)",
                        dwError,
                        VMAUTHSVC_SAFE_STRING(LwWin32ErrorToName(dwError)));
            }
        }

       RegCloseServer(pCfgHandle->hConnection);
    }

    VMAUTHSVC_SAFE_FREE_MEMORY(pCfgHandle);
}

static
VOID
VmAuthsvcRegConfigTableFreeContents(
    PVMAUTHSVC_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMAUTHSVC_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        if (pEntry->Type == VMAUTHSVC_CONFIG_VALUE_TYPE_STRING)
        {
            VMAUTHSVC_SAFE_FREE_STRINGA(pEntry->cfgValue.pszValue);
        }
    }
}

#else /* #ifndef _WIN32 */
//
// Save pszUserDN and pszPassword into registry
//

//*************************************************************
//
//  VmAuthsvcConfigSetAdminCredentials()
//
//  Purpose:    Save UserDN and Password into the registry
//
//  Parameters: pszUserDN    -  contains value of the UserDN
//              pszPassword     contains value of the password
//
//  Return:     ERROR_SUCCESS if successful.
//              otherwise if an error occurs.
//
//*************************************************************
DWORD
VmAuthsvcConfigSetAdminCredentials(
    PCSTR pszUserDN,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCTSTR pszParamsKeyPath      = VMAUTHSVC_CONFIG_PARAMETER_KEY_PATH;
    PCTSTR pszValueAdminDN       = VMAUTHSVC_REG_KEY_ADMIN_DN;
    PCTSTR pszCredsKeyPath       = VMAUTHSVC_CONFIG_CREDS_KEY_PATH;
    PCTSTR pszValueAdminPassword = VMAUTHSVC_REG_KEY_ADMIN_PASSWD;

    HKEY hKey;
    SIZE_T lengthCheck;
#define MSG_BUFFER_SIZE 2048
    _TCHAR msgBuffer[MSG_BUFFER_SIZE];

    // UserDN and Password can't be NULL or empty string
    if ( IsNullOrEmptyString(pszUserDN)
        || IsNullOrEmptyString(pszPassword)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    dwError = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,         //  __in        HKEY hKey,
                pszParamsKeyPath,           //  __in        LPCTSTR lpSubKey,
                0,                          //  __reserved  DWORD Reserved,
                NULL,                       //  __in_opt    LPTSTR lpClass,
                REG_OPTION_NON_VOLATILE,    //  __in        DWORD dwOptions,
                KEY_WRITE,                  //  __in        REGSAM samDesired,
                NULL,                       //  __in_opt    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                &hKey,                      //  __out       PHKEY phkResult,
                NULL                        //  __out_opt   LPDWORD lpdwDisposition
                );

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    lengthCheck = VmAuthsvcStringLenA(pszUserDN)+1;

    dwError = RegSetValueEx(
                hKey,                   // HKEY hKey,
                pszValueAdminDN,        // LPCTSTR lpValueName,
                0,                      // DWORD Reserved,
                REG_SZ,                 // DWORD dwType,
                (BYTE*)pszUserDN,       // const BYTE *lpData,
                (DWORD)lengthCheck      // DWORD cbData, the size in bytes, including the terminating character.
                );

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    RegCloseKey(hKey); // We don't care if this call fails.

    dwError = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,         //  __in        HKEY hKey,
                pszCredsKeyPath,           //  __in        LPCTSTR lpSubKey,
                0,                          //  __reserved  DWORD Reserved,
                NULL,                       //  __in_opt    LPTSTR lpClass,
                REG_OPTION_NON_VOLATILE,    //  __in        DWORD dwOptions,
                KEY_WRITE,                  //  __in        REGSAM samDesired,
                NULL,                       //  __in_opt    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                &hKey,                      //  __out       PHKEY phkResult,
                NULL                        //  __out_opt   LPDWORD lpdwDisposition
                );

    lengthCheck = VmAuthsvcStringLenA(pszUserDN)+1 ;
    if (lengthCheck > UINT32_MAX)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }
    else
    {
        dwError = RegSetValueEx(
                hKey,                   // HKEY hKey,
                pszValueAdminPassword,  // LPCTSTR lpValueName,
                0,                      // DWORD Reserved,
                REG_SZ,                 // DWORD dwType,
                (BYTE*)pszPassword,     // const BYTE *lpData,
                (DWORD)lengthCheck      // DWORD cbData, the size in bytes, including the terminating character.
                );
    }

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hKey)
        RegCloseKey(hKey);

    if (dwError != ERROR_SUCCESS)
    {
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM , //  __in      DWORD dwFlags,
            NULL,                   //  __in_opt  LPCVOID lpSource,
            dwError,                //  __in      DWORD dwMessageId,
            LANG_SYSTEM_DEFAULT,    //  __in      DWORD dwLanguageId,
            msgBuffer,              //  __out     LPTSTR lpBuffer,
            MSG_BUFFER_SIZE,        //  __in      DWORD nSize,
            NULL                    //  __in_opt  va_list *Arguments
            );
    }
    return dwError;
}

#endif
