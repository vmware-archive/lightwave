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
VmDirKrbRealmNameNormalize(
    PCSTR       pszName,
    PSTR*       ppszNormalizeName
    )
{
    DWORD       dwError = 0;
    PSTR        pszRealmName = NULL;

    if ( !pszName  ||  !ppszNormalizeName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszName, &pszRealmName);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        size_t iCnt=0;
        size_t iLen = VmDirStringLenA(pszRealmName);

        for (iCnt = 0; iCnt < iLen; iCnt++)
        {
            VMDIR_ASCII_LOWER_TO_UPPER(pszRealmName[iCnt]);
        }
    }

    *ppszNormalizeName = pszRealmName;

cleanup:

    return dwError;

error:

    VmDirFreeMemory(pszRealmName);
    goto cleanup;
}

DWORD
VmDirGetKrbMasterKey(
    PSTR        pszFQDN, // [in] FQDN
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
)
{
    DWORD       dwError = 0;
    PBYTE       pRetMasterKey = NULL;

    if (IsNullOrEmptyString(pszFQDN)
        || !ppKeyBlob
        || !pSize
       )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Currently, we only support single krb realm.
    // Global cache gVmdirKrbGlobals is initialized during startup stage.

    if (VmDirStringCompareA( pszFQDN, VDIR_SAFE_STRING(gVmdirKrbGlobals.pszRealm), FALSE) != 0)
    {
        dwError = VMDIR_ERROR_INVALID_REALM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                    gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len,
                    (PVOID*)&pRetMasterKey
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory (
                    pRetMasterKey,
                    gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len,
                    gVmdirKrbGlobals.bervMasterKey.lberbv.bv_val,
                    gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppKeyBlob = pRetMasterKey;
    *pSize     = (DWORD) gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len;
    pRetMasterKey = NULL;

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_RPC, "VmDirGetKrbMasterKey failed. (%u)(%s)",
                                     dwError, VDIR_SAFE_STRING(pszFQDN));
    VMDIR_SAFE_FREE_MEMORY(pRetMasterKey);

    goto cleanup;

}

DWORD
VmDirGetKrbUPNKey(
    PSTR        pszUpnName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
)
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pKrbUPNKey = NULL;
    PBYTE               pRetUPNKey = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};

    if (IsNullOrEmptyString(pszUpnName)
        || !ppKeyBlob
        || !pSize
       )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    "",
                    LDAP_SCOPE_SUBTREE,
                    ATTR_KRB_UPN,
                    pszUpnName,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 1)
    {
        pKrbUPNKey = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_KRB_PRINCIPAL_KEY);

        if (!pKrbUPNKey)
        {
            dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateMemory(
                            pKrbUPNKey->vals[0].lberbv.bv_len,
                            (PVOID*)&pRetUPNKey
                            );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirCopyMemory(
                        pRetUPNKey,
                        pKrbUPNKey->vals[0].lberbv.bv_len,
                        pKrbUPNKey->vals[0].lberbv.bv_val,
                        pKrbUPNKey->vals[0].lberbv.bv_len
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppKeyBlob = pRetUPNKey;
        *pSize     = (DWORD) pKrbUPNKey->vals[0].lberbv.bv_len;
        pRetUPNKey = NULL;
    }
    else
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_RPC, "VmDirGetKrbUPNKey failed. (%u)(%s)",
                                     dwError, VDIR_SAFE_STRING(pszUpnName));
    VMDIR_SAFE_FREE_MEMORY(pRetUPNKey);

    goto cleanup;

}

DWORD
VmDirGetKeyTabRecBlob(
    PSTR      pszUpnName,
    PBYTE*    ppBlob,
    DWORD*    pdwBlobLen
)
{
    DWORD                  dwError = 0;
    PBYTE                  pBlob = NULL;
    DWORD                  dwBlobLen = 0;
    PVMDIR_KEYTAB_HANDLE   pKeyTabHandle = NULL;
    PBYTE                  pUPNKeyByte = NULL;
    DWORD                  dwUPNKeySize = 0;

    if ( !pszUpnName || !ppBlob || !pdwBlobLen )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_KEYTAB_HANDLE),
                                 (PVOID*)&pKeyTabHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetKrbUPNKey(
                    pszUpnName,
                    &pUPNKeyByte,
                    &dwUPNKeySize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirKeyTabWriteKeysBlob(
                      pKeyTabHandle,
                      pszUpnName,
                      pUPNKeyByte,
                      dwUPNKeySize,
                      gVmdirKrbGlobals.bervMasterKey.lberbv.bv_val,
                      (DWORD)gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len,
                      &pBlob,
                      &dwBlobLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppBlob     = pBlob;
    *pdwBlobLen = dwBlobLen;
    pBlob       = NULL;

cleanup:

    if (pKeyTabHandle)
    {
        VmDirKeyTabClose(pKeyTabHandle);
    }

    VMDIR_SAFE_FREE_MEMORY( pUPNKeyByte );

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_RPC, "VmDirGetKeyTabRecBlob failed, (%u)(%s)",
                                     dwError, VDIR_SAFE_STRING(pszUpnName));
    VMDIR_SAFE_FREE_MEMORY( pBlob );

    goto cleanup;
}
