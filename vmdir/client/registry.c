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
VmDirConfigSetDefaultSiteandLduGuid(
    PCSTR pszDefaultSiteGuid,
    PCSTR pszDefaultLduGuid
    )
#ifndef _WIN32
{
    DWORD   dwError = 0;

    if (IsNullOrEmptyString(pszDefaultSiteGuid) || IsNullOrEmptyString(pszDefaultLduGuid))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = RegUtilSetValue(
                NULL,
                HKEY_THIS_MACHINE,
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                NULL,
                VMDIR_REG_KEY_SITE_GUID,
                REG_SZ,
                (PVOID)pszDefaultSiteGuid,
                VmDirStringLenA(pszDefaultSiteGuid)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegUtilSetValue(
                NULL,
                HKEY_THIS_MACHINE,
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                NULL,
                VMDIR_REG_KEY_LDU_GUID,
                REG_SZ,
                (PVOID)pszDefaultLduGuid,
                VmDirStringLenA(pszDefaultLduGuid)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirConfigSetDefaultSiteandLduGuid failed with error (%u)", dwError);
    goto cleanup;
}
#else
{
    DWORD   dwError = 0;
    HKEY    hKey = NULL;

    if (IsNullOrEmptyString(pszDefaultSiteGuid) || IsNullOrEmptyString(pszDefaultLduGuid))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = RegCreateKeyExA(
                        HKEY_LOCAL_MACHINE,
                        VMDIR_CONFIG_PARAMETER_KEY_PATH,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hKey,
                        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        VMDIR_REG_KEY_SITE_GUID,
                        0,
                        REG_SZ,
                        (BYTE*)pszDefaultSiteGuid,
                        (DWORD)VmDirStringLenA(pszDefaultSiteGuid)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        VMDIR_REG_KEY_LDU_GUID,
                        0,
                        REG_SZ,
                        (BYTE*)pszDefaultLduGuid,
                        (DWORD)VmDirStringLenA(pszDefaultLduGuid)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirConfigSetDefaultSiteandLduGuid failed with error (%u)", dwError);
    goto cleanup;
}
#endif

DWORD
VmDirConfigSetDCAccountInfo(
    PCSTR pszDCAccount,
    PCSTR pszDCAccountDN,
    PCSTR pszDCAccountPassword,
    DWORD dwPasswordSize,
    PCSTR pszMachineGUID
    )
#ifndef _WIN32
{
    DWORD   dwError = 0;
    PSTR    pszPasswordBuf = NULL;

    if (IsNullOrEmptyString(pszDCAccountDN) ||
        IsNullOrEmptyString(pszDCAccount) ||
        IsNullOrEmptyString(pszDCAccountPassword) ||
        IsNullOrEmptyString(pszMachineGUID) ||
        dwPasswordSize == 0)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                dwPasswordSize+1,
                (PVOID*)&pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNCpyA(
                pszPasswordBuf,
                dwPasswordSize+1,
                pszDCAccountPassword,
                dwPasswordSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegUtilSetValue(
                NULL,
                HKEY_THIS_MACHINE,
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                NULL,
                VMDIR_REG_KEY_DC_ACCOUNT_DN,
                REG_SZ,
                (PVOID)pszDCAccountDN,
                VmDirStringLenA(pszDCAccountDN)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegUtilSetValue(
                NULL,
                HKEY_THIS_MACHINE,
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                NULL,
                VMDIR_REG_KEY_DC_ACCOUNT,
                REG_SZ,
                (PVOID)pszDCAccount,
                VmDirStringLenA(pszDCAccount)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegUtilSetValue(
                NULL,
                HKEY_THIS_MACHINE,
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                NULL,
                VMDIR_REG_KEY_DC_ACCOUNT_PWD,
                REG_SZ,
                (PVOID)pszPasswordBuf,
                dwPasswordSize+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Ignore error
    RegUtilDeleteValue(
        NULL,
        HKEY_THIS_MACHINE,
        VMDIR_CONFIG_PARAMETER_KEY_PATH,
        NULL,
        VMDIR_REG_KEY_DC_ACCOUNT_OLD_PWD);

    dwError = RegUtilSetValue(
                NULL,
                HKEY_THIS_MACHINE,
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                NULL,
                VMDIR_REG_KEY_MACHINE_GUID,
                REG_SZ,
                (PVOID)pszMachineGUID,
                VmDirStringLenA(pszMachineGUID)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPasswordBuf);
    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirConfigSetDCAccountInfo failed with error (%u)", dwError);
    goto cleanup;
}
#else
{
    DWORD   dwError = 0;
    HKEY    hKey = NULL;
    PSTR    pszPasswordBuf = NULL;

    if (IsNullOrEmptyString(pszDCAccountDN) ||
        IsNullOrEmptyString(pszDCAccount) ||
        IsNullOrEmptyString(pszDCAccountPassword) ||
        IsNullOrEmptyString(pszMachineGUID) ||
        dwPasswordSize == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                dwPasswordSize+1,
                (PVOID*)&pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNCpyA(
                pszPasswordBuf,
                dwPasswordSize+1,
                pszDCAccountPassword,
                dwPasswordSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegCreateKeyExA(
                        HKEY_LOCAL_MACHINE,
                        VMDIR_CONFIG_PARAMETER_KEY_PATH,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hKey,
                        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        VMDIR_REG_KEY_DC_ACCOUNT_DN,
                        0,
                        REG_SZ,
                        (BYTE*)pszDCAccountDN,
                        (DWORD)VmDirStringLenA(pszDCAccountDN)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        VMDIR_REG_KEY_DC_ACCOUNT,
                        0,
                        REG_SZ,
                        (BYTE*)pszDCAccount,
                        (DWORD)VmDirStringLenA(pszDCAccount)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        VMDIR_REG_KEY_DC_ACCOUNT_PWD,
                        0,
                        REG_SZ,
                        (BYTE*)pszPasswordBuf,
                        dwPasswordSize+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        VMDIR_REG_KEY_MACHINE_GUID,
                        0,
                        REG_SZ,
                        (BYTE*)pszMachineGUID,
                        (DWORD)VmDirStringLenA(pszMachineGUID)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPasswordBuf);
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirConfigSetDCAccountInfo failed with error (%u)", dwError);
    goto cleanup;
}
#endif

DWORD
VmDirConfigSetDCAccountPassword(
    PCSTR pszDCAccountPassword,
    DWORD dwPasswordSize
    )
{
    DWORD   dwError = 0;
    PSTR    pszOldPassword = NULL;

    if (IsNullOrEmptyString(pszDCAccountPassword) || dwPasswordSize == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /*
     * There might not be a current password so ignore any errors.
     */
    (VOID)VmDirReadDCAccountPassword(&pszOldPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWriteDCAccountPassword(
                pszDCAccountPassword,
                dwPasswordSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszOldPassword != NULL)
    {
        dwError = VmDirWriteDCAccountOldPassword(
                    pszOldPassword,
                    (DWORD)strlen(pszOldPassword));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszOldPassword);
    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirConfigSetDCAccountPassword failed with error (%u)", dwError);
    goto cleanup;
}

DWORD
VmDirConfigSetSZKey(
    PCSTR pszKeyPath,
    PCSTR pszKeyName,
    PCSTR pszKeyValue
    )
#ifndef _WIN32
{
    DWORD   dwError = 0;

    if (IsNullOrEmptyString(pszKeyValue))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = RegUtilSetValue(
                NULL,
                HKEY_THIS_MACHINE,
                pszKeyPath,
                NULL,
                pszKeyName,
                REG_SZ,
                (PVOID)pszKeyValue,
                VmDirStringLenA(pszKeyValue)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ANY, "%s failed (%s)(%s)(%s) with error (%u)", __FUNCTION__,
             VDIR_SAFE_STRING(pszKeyPath), VDIR_SAFE_STRING(pszKeyName), VDIR_SAFE_STRING(pszKeyValue), dwError);
    goto cleanup;
}
#else
{
    DWORD   dwError = 0;
    HKEY    hKey = NULL;

    if (IsNullOrEmptyString(pszKeyValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = RegCreateKeyExA(
                        HKEY_LOCAL_MACHINE,
                        pszKeyPath,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hKey,
                        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        pszKeyName,
                        0,
                        REG_SZ,
                        (BYTE*)pszKeyValue,
                        (DWORD) VmDirStringLenA(pszKeyValue) +1 );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ANY, "%s failed (%s)(%s)(%s) with error (%u)", __FUNCTION__,
             VDIR_SAFE_STRING(pszKeyPath), VDIR_SAFE_STRING(pszKeyName), VDIR_SAFE_STRING(pszKeyValue), dwError);
    goto cleanup;
}
#endif

