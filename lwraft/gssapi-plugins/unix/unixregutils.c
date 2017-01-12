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
VMCISLIBAccountDnToUpn(
    PSTR dn,
    PSTR *retUpn)
{
/*
 * Convert:  cn=adam-sles11.ssolabs2.com,ou=Domain Controllers,dc=VSPHERE,dc=LOCAL
 *      to:  adam-sles11.ssolabs2.com@VSPHERELOCAL
 */
    DWORD dwError = 0;

    PSTR ptr = NULL;
    PSTR end = NULL;
    PSTR upn = NULL;
    PSTR fmtupn = NULL;
    PSTR sep = ".";
    DWORD len = (DWORD) strlen(dn);

    upn = calloc(len+2, sizeof(CHAR));
    if (!upn)
    {
        dwError = ERROR_NO_MEMORY;
        goto error;
    }
    fmtupn = upn;

    /*
     * TBD: Note: this code assumes DN is all lower case.
     * Handle "cn=" portion of UPN
     */
    ptr = strstr(dn, "cn=");
    if (ptr)
    {
        ptr += 3; /* Skip over cn= */
        end = strstr(ptr, ",ou=");
        if (!end)
        {
            end = strstr(ptr, ",dc=");
        }
        if (end)
        {
            fmtupn += snprintf(fmtupn, len, "%.*s@", (int) (end-ptr), ptr);
        }
    }

    ptr = strstr(ptr, "dc=");
    while (ptr)
    {
        ptr += 3;
        if (*ptr)
        {
            end = strstr(ptr, ",dc=");
            if (!end)
            {
                end = ptr + strlen(ptr);
                sep = "";
            }
            fmtupn += snprintf(fmtupn, len, "%.*s%s", (int) (end-ptr), ptr, sep);
        }
        ptr = strstr(ptr, "dc=");
    }
    *retUpn = upn;
    upn = NULL;

error:
    if (dwError)
    {
        if (upn)
        {
            free(upn);
        }
    }
    return dwError;
}

DWORD
VmDirRegConfigHandleOpen(
    PVMDIR_CONFIG_CONNECTION_HANDLE *ppCfgHandle)
{
    DWORD dwError = 0;
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle = NULL;

    /* substitute for VmDirAllocateMemory() */
    pCfgHandle = calloc(1, sizeof(VMDIR_CONFIG_CONNECTION_HANDLE));
    if (!pCfgHandle)
    {
        dwError = ERROR_NO_MEMORY;
    }
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
            RegCloseKey(
                pCfgHandle->hConnection,
                pCfgHandle->hKey);
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

DWORD
VmDirRegConfigGetValue(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    DWORD  valueType,
    PBYTE  pRetValue,
    PDWORD pRetValueLen
    )
{
    DWORD dwError = 0;

    dwError = RegGetValueA(
#ifndef _WIN32
                pCfgHandle->hConnection,
#endif
                pCfgHandle->hKey,
                pszSubKey,
                pszKeyName,
                valueType,
                NULL,
                (PVOID) pRetValue,
                pRetValueLen);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    if (dwError)
    {
        *pRetValueLen = 0;
    }

    return dwError;
}
