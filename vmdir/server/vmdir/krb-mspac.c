/*
 * Copyright �2012-2016 VMware, Inc.  All Rights Reserved.
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

static
DWORD
VmDirGetDomainSidFromUpn(
    PCSTR pszUpnName,
    PSID *ppDomainSid)
{
    DWORD dwError = 0;
    PSID pDomainSid = NULL;
    PSTR pszName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszDomainDN = NULL;
    PVDIR_ENTRY pEntry = NULL;

    dwError = VmDirUPNToNameAndDomain(
                      pszUpnName,
                      &pszName,
                      &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFQDNToDN(
                      pszDomainName,
                      &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleDNToEntry(
                      pszDomainDN,
                      &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetObjectSidFromEntry(
                      pEntry,
                      NULL,
                      &pDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppDomainSid = pDomainSid;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDirKrbCreateAuthzInfo(
    PCSTR pszUpnName,
    PSID pDomainSid,
    PSID pUserSid,
    GROUP_MEMBERSHIP *pGroupIds,
    DWORD dwGroupIds,
    KERB_SID_AND_ATTRIBUTES *pExtraSids,
    DWORD dwExtraSids,
    KERB_VALIDATION_INFO** ppInfo
    )
{
    DWORD dwError = 0;
    KERB_VALIDATION_INFO* pInfo = NULL;
    SIZE_T iUpnNameLength = 0;
    PWSTR pwszUpnName = NULL;
    NTSTATUS ntStatus = 0;
    ULONG ulUserRid = 0;

    dwError = VmDirAllocateMemory(
                      sizeof(KERB_VALIDATION_INFO),
                      (PVOID*)&pInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* EffectiveName */

    dwError = VmDirAllocateStringWFromA(
                      pszUpnName,
                      &pwszUpnName);
    BAIL_ON_VMDIR_ERROR(dwError);

    iUpnNameLength = VmDirStringLenA(pszUpnName);

    pInfo->EffectiveName.Length = iUpnNameLength;
    pInfo->EffectiveName.MaximumLength = iUpnNameLength;
    pInfo->EffectiveName.Buffer = pwszUpnName;

    /* UserId */

#if 1
    ntStatus = RtlGetRidSid(&ulUserRid, pUserSid);
    BAIL_ON_VMDIR_ERROR(ntStatus);
    pInfo->UserId = ulUserRid;
#else
    ntStatus = RtlDuplicateSid(
                       (PSID *)&pInfo->UserSid,
                       pUserSid);
#endif 
    dwError = LwNtStatusToWin32Error(ntStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

#if 1
    /* PrimaryGroupId */

   pInfo->PrimaryGroupId = 0;
#endif

    /* GroupCount */

   pInfo->GroupCount = dwGroupIds;

    /* GroupIds */

    pInfo->GroupIds = pGroupIds;

    /* LogonDomainId */

    ntStatus = RtlDuplicateSid(
                       (PSID *)&pInfo->LogonDomainId,
                       pDomainSid);
    dwError = LwNtStatusToWin32Error(ntStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

#if 1
    /* LogonDomainName */

    pInfo->LogonDomainName.Length = 0;

    /* Reserved1 */

    pInfo->Reserved1[0] = 0;
    pInfo->Reserved1[1] = 0;

    /* UserAccountControl */

    pInfo->UserAccountControl = 0;

    /* SubAuthStatus */

    pInfo->SubAuthStatus = 0x00000000;

    /* LastSuccessfulILogon */

    pInfo->LastSuccessfulILogon.dwLowDateTime = 0xFFFFFFFF;
    pInfo->LastSuccessfulILogon.dwHighDateTime = 0x7FFFFFFF;

    /* LastFailedILogon */

    pInfo->LastFailedILogon.dwLowDateTime = 0xFFFFFFFF;
    pInfo->LastFailedILogon.dwHighDateTime = 0x7FFFFFFF;

    /* FailedILogonCount */

    pInfo->FailedILogonCount = 0;

    /* Reserved3 */

    pInfo->Reserved3 = 0;
#endif

    /* SidCount */

    pInfo->SidCount = dwExtraSids;

    /* ExtraSids */

    pInfo->ExtraSids = pExtraSids;

#if 1
    /* ResourceGroupDomainSid */

    pInfo->ResourceGroupDomainSid = NULL;

    /* ResourceGroupCount */

    pInfo->ResourceGroupCount = 0;

    /* ResourceGroupIds */

    pInfo->ResourceGroupIds = NULL;
#endif

    *ppInfo = pInfo;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pwszUpnName);
    if (pInfo)
    {
        if (pInfo->LogonDomainId)
        {
            RtlMemoryFree(pInfo->LogonDomainId);
        }
        VMDIR_SAFE_FREE_MEMORY(pInfo);
    }
    goto cleanup;
}

/* 
 * VmDirKrbGetAuthzInfo() -> VmDirKrbGetKerbValidationInfo()
 *    vmdir pac                 ms pac
 */
DWORD
VmDirKrbGetKerbValidationInfo(
    PCSTR pszUpnName,
    KERB_VALIDATION_INFO** ppInfo
    )
{
    DWORD                 dwError = 0;
    KERB_VALIDATION_INFO* pInfo = NULL;
    VDIR_ENTRY_ARRAY      entryArray = {0};
    PVDIR_ATTRIBUTE       pMemberOfAttr = NULL;
    unsigned int          i = 0;
    PVDIR_ENTRY           pEntry = NULL;
    PVDIR_ENTRY           pGroupEntry = NULL;
    PSID                  pUserSid = NULL;
    PSID                  pGroupSid = NULL;
    NTSTATUS              ntStatus = 0;
#if 0
    ULONG                 ulUserRid = 0;
#endif
    ULONG                 ulGroupRid = 0;
    PSID                  pDomainSid = NULL;
    GROUP_MEMBERSHIP      *pGroupIds = NULL;
    DWORD                 dwGroupIds = 0;
    KERB_SID_AND_ATTRIBUTES *pExtraSids = NULL;
    DWORD                 dwExtraSids = 0;

    if (IsNullOrEmptyString(pszUpnName) ||
        ppInfo == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(
                      "",
                      LDAP_SCOPE_SUBTREE,
                      ATTR_KRB_UPN,
                      pszUpnName,
                      &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 0)
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (entryArray.iSize > 1)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pEntry = &entryArray.pEntry[0];

    dwError = VmDirGetDomainSidFromUpn(
                      pszUpnName,
                      &pDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirFindMemberOfAttribute(
                      pEntry,
                      &pMemberOfAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pMemberOfAttr && pMemberOfAttr->numVals > 0)
    {
        for (i = 0; i < pMemberOfAttr->numVals; i++)
        {
            dwError = VmDirSimpleDNToEntry(
                              pMemberOfAttr->vals[i].lberbv.bv_val,
                              &pGroupEntry);
            if (dwError != 0)
            {
                // may be deleted in the meanwhile

                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                                   "VmDirKrbGetAuthzInfo() memberOf entry (%s) not found, error code (%d)",
                                   pMemberOfAttr->vals[i].lberbv.bv_val, dwError );
                continue;
            }

            dwError = VmDirGetObjectSidFromEntry(
                              pGroupEntry,
                              NULL,
                              &pGroupSid);
            BAIL_ON_VMDIR_ERROR(dwError);

#if 1 /* TBD: Adam-Debug */

{
PSTR sidstr = NULL;
ntStatus = RtlAllocateCStringFromSid(&sidstr, pGroupSid);
if (ntStatus == 0)
{
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "DEBUG: VmDirKrbGetKerbValidationInfo: pGroupSid=%s", sidstr);
    RTL_FREE(&sidstr);

}
}
#endif /* if 1 */

            if (RtlIsPrefixSid(pDomainSid, pGroupSid))
            {
                dwGroupIds++;

                dwError = VmDirReallocateMemory(
                                  pGroupIds,
                                  (PVOID*)&pGroupIds,
                                  dwGroupIds * sizeof(GROUP_MEMBERSHIP));
                BAIL_ON_VMDIR_ERROR(dwError);

                ntStatus = RtlGetRidSid(&ulGroupRid, pGroupSid);
                dwError = LwNtStatusToWin32Error(ntStatus);
                BAIL_ON_VMDIR_ERROR(dwError);

                pGroupIds[dwGroupIds-1].RelativeId = ulGroupRid;
                pGroupIds[dwGroupIds-1].Attributes = SE_GROUP_ENABLED;
            }
            else
            {
                dwExtraSids++;

                dwError = VmDirReallocateMemory(
                                  pExtraSids,
                                  (PVOID*)&pExtraSids,
                                  dwExtraSids * sizeof(KERB_SID_AND_ATTRIBUTES));
                BAIL_ON_VMDIR_ERROR(dwError);

                ntStatus = RtlDuplicateSid(
                                   (PSID *)&pExtraSids[dwExtraSids-1].Sid,
                                   pGroupSid);
                dwError = LwNtStatusToWin32Error(ntStatus);
                BAIL_ON_VMDIR_ERROR(dwError);

                pExtraSids[dwExtraSids-1].Attributes = SE_GROUP_ENABLED;
            }

            VmDirFreeEntry(pGroupEntry);
            pGroupEntry = NULL;

            VMDIR_SAFE_FREE_MEMORY(pGroupSid);
            pGroupSid = NULL;
        }
    }

    /* Get the UserId */

    dwError = VmDirGetObjectSidFromEntry(
                      pEntry,
                      NULL,
                      &pUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

#if 0
    ntStatus = RtlGetRidSid(&ulUserRid, pUserSid);
    dwError = LwNtStatusToWin32Error(ntStatus);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    /* Create the logon info structure */

    dwError = VmDirKrbCreateAuthzInfo(
                  pszUpnName,
                  pDomainSid,
                  pUserSid,
                  pGroupIds,
                  dwGroupIds,
                  pExtraSids,
                  dwExtraSids,
                  &pInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppInfo = pInfo;

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);
    VmDirFreeAttribute(pMemberOfAttr);

    if (pGroupEntry)
    {
        VmDirFreeEntry(pGroupEntry);
    }

    VMDIR_SAFE_FREE_MEMORY(pUserSid);
    VMDIR_SAFE_FREE_MEMORY(pGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pDomainSid);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirKrbGetAuthzInfo() failed, entry UPN (%s), error code (%d)",
                     VDIR_SAFE_STRING(pszUpnName), dwError );
    if (pInfo)
    {
        VmDirKrbFreeKerbValidationInfo(pInfo);
    }
    if (pGroupIds)
    {
        VmDirFreeMemory(pGroupIds);
    }
    if (pExtraSids)
    {
        VmDirFreeMemory(pExtraSids);
    }
    goto cleanup;
}

VOID
VmDirKrbFreeKerbValidationInfo(
    KERB_VALIDATION_INFO* pInfo
    )
{
    if (pInfo)
    {
        if (pInfo->EffectiveName.Buffer)
        {
            VmDirFreeMemory(pInfo->EffectiveName.Buffer);
        }
        if (pInfo->GroupIds)
        {
            VmDirFreeMemory(pInfo->GroupIds);
        }
        if (pInfo->ExtraSids)
        {
            VmDirFreeMemory(pInfo->ExtraSids);
        }
        if (pInfo->LogonDomainId)
        {
            RtlMemoryFree(pInfo->LogonDomainId);
        }
        VmDirFreeMemory(pInfo);
    }
}
