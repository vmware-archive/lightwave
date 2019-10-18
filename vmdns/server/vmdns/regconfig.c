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


/*
 * Module Name:  regconfig.c
 *
 * Abstract: VMware Domain Name Service.
 *
 */

#include "includes.h"

// TODO: remove once registy change is ported to windows ...
#ifndef _WIN32

static
DWORD
VmDnsRegGetConfig(
    PCSTR               pszSubKey,
    PVMDNS_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    );

static
DWORD
VmDnsRegConfigGSetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    pszValue
    );

static
DWORD
VmDnsRegConfigGetDword(
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    );

static
DWORD
VmDnsRegConfigGetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
DWORD
VmDnsRegConfigGSetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue
    );

static
VOID
VmDnsRegConfigTableFreeContents(
    PVMDNS_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    );

DWORD
VmDnsSrvUpdateConfig(
    VOID
    )
{
    DWORD dwError = 0;
    VMDNS_CONFIG_ENTRY initTable[] = VMDNS_CONFIG_INIT_TABLE_INITIALIZER;
    DWORD dwNumEntries = sizeof(initTable)/sizeof(initTable[0]);
    DWORD iEntry = 0;

    dwError = VmDnsRegGetConfig(
                VMDNS_CONFIG_PARAMETER_KEY_PATH,
                initTable,
                dwNumEntries);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMDNS_CONFIG_ENTRY pEntry = &initTable[iEntry];

        if (!VmDnsStringCompareA(
                pEntry->pszName,
                VMDNS_REG_KEY_PORT,
                TRUE))
        {
            gVmdnsGlobals.iListenPort = pEntry->cfgValue.dwValue;
        }
    }

cleanup:

    VmDnsRegConfigTableFreeContents(initTable, dwNumEntries);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsConfigSetAdminCredentials(
    PCSTR pszUserDN,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszUserDN) || !pszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsRegConfigGSetString(
                VMDNS_CONFIG_PARAMETER_KEY_PATH,
                VMDNS_REG_KEY_ADMIN_DN,
                pszUserDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRegConfigGSetString(
                VMDNS_CONFIG_CREDS_KEY_PATH,
                VMDNS_REG_KEY_ADMIN_PASSWD,
                pszUserDN);
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

static
DWORD
VmDnsRegGetConfig(
    PCSTR               pszSubKey,
    PVMDNS_CONFIG_ENTRY pCfgTable,
    DWORD               dwNumEntries
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMDNS_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        switch (pEntry->Type)
        {
            case VMDNS_CONFIG_VALUE_TYPE_STRING:

                dwError = VmDnsRegConfigGetString(
                            pszSubKey,
                            pEntry->pszName,
                            &pEntry->cfgValue.pszValue);
                if (dwError != 0)
                {   // use default value
                    dwError = VmDnsAllocateStringA(
                                    pEntry->defaultValue.pszDefault,
                                    &pEntry->cfgValue.pszValue);
                    BAIL_ON_VMDNS_ERROR(dwError);
                }

                break;

            case VMDNS_CONFIG_VALUE_TYPE_DWORD:

                dwError = VmDnsRegConfigGetDword(
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
                    VmDnsLog(VMDNS_LOG_LEVEL_INFO,
                            "Config [%s] value (%d) too big, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMax);

                    pEntry->cfgValue.dwValue = pEntry->dwMax;

                }

                if (pEntry->cfgValue.dwValue < pEntry->dwMin)
                {
                    VmDnsLog(
                            VMDNS_LOG_LEVEL_INFO,
                            "Config [%s] value (%d) too small, using (%d).",
                            pEntry->pszName,
                            pEntry->cfgValue.dwValue,
                            pEntry->dwMin);

                    pEntry->cfgValue.dwValue = pEntry->dwMin;
                }

                break;

            case VMDNS_CONFIG_VALUE_TYPE_BOOLEAN:

                dwError = VmDnsRegConfigGetDword(
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

                VmDnsLog(
                        VMDNS_LOG_LEVEL_INFO,
                        "VmDnsRegConfigProcess key [%s] type (%d) not supported.",
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
VmDnsRegConfigGetDword(
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    PDWORD pdwValue
    )
{
    DWORD    dwError =0;
     CHAR    szKey[VM_SIZE_512] = {0};
     CHAR    szValue[VM_SIZE_128] = {0};
     size_t  dwszValueSize = sizeof(szValue);

     dwError = VmDirStringPrintFA(
             &szKey[0], VM_SIZE_512,
             "%s\\%s", pszSubKey, pszKeyName);
     BAIL_ON_VMDNS_ERROR(dwError);

     dwError = VmRegConfigGetKeyA(szKey, szValue, &dwszValueSize);
     BAIL_ON_VMDNS_ERROR(dwError);

     *pdwValue = atol(szValue);

 cleanup:

     return dwError;

 error:
     *pdwValue = 0;

     goto cleanup;
}

static
DWORD
VmDnsRegConfigGetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    *ppszValue)
{
    DWORD dwError = 0;
    CHAR  szKey[VM_SIZE_512] = {0};
    char szValue[VMDNS_MAX_CONFIG_VALUE_LENGTH] = {0};
    DWORD dwszValueSize = sizeof(szValue);
    PSTR pszValue = NULL;

    dwError = VmStringPrintFA(
            &szKey[0], VMDIR_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRegConfigGetKeyA(szKey, szValue, &dwszValueSize);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    VMDNS_SAFE_FREE_STRINGA(pszValue);

    goto cleanup;
}

static
DWORD
VmDnsRegConfigGSetString(
    PCSTR   pszSubKey,
    PCSTR   pszKeyName,
    PSTR    pszValue
    )
{
    DWORD   dwError =  0;
    CHAR    szKey[VM_SIZE_512] = {0};

    if (!pszSubKey || !pszKeyName || !pszValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRegConfigSetKeyA(szKey, pszValue, VmDirStringLenA(pszValue));
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
VOID
VmDnsRegConfigTableFreeContents(
    PVMDNS_CONFIG_ENTRY pCfgTable,
    DWORD dwNumEntries
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwNumEntries; iEntry++)
    {
        PVMDNS_CONFIG_ENTRY pEntry = &pCfgTable[iEntry];

        if (pEntry->Type == VMDNS_CONFIG_VALUE_TYPE_STRING)
        {
            VMDNS_SAFE_FREE_STRINGA(pEntry->cfgValue.pszValue);
        }
    }
}

#else /* #ifndef _WIN32 */
//
// Save pszUserDN and pszPassword into registry
//

//*************************************************************
//
//  VmDnsConfigSetAdminCredentials()
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
VmDnsConfigSetAdminCredentials(
    PCSTR pszUserDN,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PCTSTR pszParamsKeyPath      = VMDNS_CONFIG_PARAMETER_KEY_PATH;
    PCTSTR pszValueAdminDN       = VMDNS_REG_KEY_ADMIN_DN;
    PCTSTR pszCredsKeyPath       = VMDNS_CONFIG_CREDS_KEY_PATH;
    PCTSTR pszValueAdminPassword = VMDNS_REG_KEY_ADMIN_PASSWD;

    HKEY hKey = NULL;
    SIZE_T lengthCheck;
#define MSG_BUFFER_SIZE 2048
    _TCHAR msgBuffer[MSG_BUFFER_SIZE];

    // UserDN and Password can't be NULL or empty string
    if ( IsNullOrEmptyString(pszUserDN)
        || IsNullOrEmptyString(pszPassword)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
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

    BAIL_ON_VMDNS_ERROR(dwError);

    lengthCheck = VmDnsStringLenA(pszUserDN)+1;

    dwError = RegSetValueEx(
                hKey,                   // HKEY hKey,
                pszValueAdminDN,        // LPCTSTR lpValueName,
                0,                      // DWORD Reserved,
                REG_SZ,                 // DWORD dwType,
                (BYTE*)pszUserDN,       // const BYTE *lpData,
                (DWORD)lengthCheck      // DWORD cbData, the size in bytes, including the terminating character.
                );

    BAIL_ON_VMDNS_ERROR(dwError);

    RegCloseKey(hKey); // We don't care if this call fails.
    hKey = NULL;

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

    lengthCheck = VmDnsStringLenA(pszUserDN)+1 ;
    if (lengthCheck > UINT32_MAX)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
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

    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return dwError;
error:
    goto cleanup;
}

#endif
