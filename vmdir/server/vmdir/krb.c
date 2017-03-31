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

#ifdef VMDIR_ENABLE_PAC
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
    VMDIR_GROUP_MEMBERSHIP *pGroupIds,
    DWORD dwGroupIds,
    KERB_SID_AND_ATTRIBUTES *pOtherSids,
    DWORD dwOtherSids,
    VMDIR_AUTHZ_INFO** ppInfo
    )
{
    DWORD dwError = 0;
    VMDIR_AUTHZ_INFO* pInfo = NULL;
    SIZE_T iUpnNameLength = 0;
    PWSTR pwszUpnName = NULL;
    NTSTATUS ntStatus = 0;

    dwError = VmDirAllocateMemory(
                      sizeof(VMDIR_AUTHZ_INFO),
                      (PVOID*)&pInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                      pszUpnName,
                      &pwszUpnName);
    BAIL_ON_VMDIR_ERROR(dwError);

    iUpnNameLength = VmDirStringLenA(pszUpnName);

    pInfo->AccountName.Length = iUpnNameLength;
    pInfo->AccountName.MaximumLength = iUpnNameLength;
    pInfo->AccountName.Buffer = pwszUpnName;

    /* UserSid */

    ntStatus = RtlDuplicateSid(
                       (PSID *)&pInfo->UserSid,
                       pUserSid);
    dwError = LwNtStatusToWin32Error(ntStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* GroupIdCount */

   pInfo->GroupIdCount = dwGroupIds;

    /* GroupIds */

    pInfo->GroupIds = pGroupIds;

    /* DomainSid */

    ntStatus = RtlDuplicateSid(
                       (PSID *)&pInfo->DomainSid,
                       pDomainSid);
    dwError = LwNtStatusToWin32Error(ntStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* OtherSidCount */

    pInfo->OtherSidCount = dwOtherSids;

    /* OtherSids */

    pInfo->OtherSids = pOtherSids;

    *ppInfo = pInfo;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pwszUpnName);
    if (pInfo)
    {
        if (pInfo->DomainSid)
        {
            RtlMemoryFree(pInfo->DomainSid);
        }
        VMDIR_SAFE_FREE_MEMORY(pInfo);
    }
    goto cleanup;
}

DWORD
VmDirKrbGetAuthzInfo(
    PCSTR pszUpnName,
    VMDIR_AUTHZ_INFO** ppInfo
    )
{
    DWORD                 dwError = 0;
    PVMDIR_AUTHZ_INFO     pInfo = NULL;
    VDIR_ENTRY_ARRAY      entryArray = {0};
    PVDIR_ATTRIBUTE       pMemberOfAttr = NULL;
    unsigned int          i = 0;
    PVDIR_ENTRY           pEntry = NULL;
    PVDIR_ENTRY           pGroupEntry = NULL;
    PSID                  pUserSid = NULL;
    PSID                  pGroupSid = NULL;
    NTSTATUS              ntStatus = 0;
    PSID                  pDomainSid = NULL;
    PVMDIR_GROUP_MEMBERSHIP pGroupIds = NULL;
    DWORD                 dwGroupIds = 0;
    KERB_SID_AND_ATTRIBUTES *pOtherSids = NULL;
    DWORD                 dwOtherSids = 0;

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

            if (RtlIsPrefixSid(pDomainSid, pGroupSid) &&
                (pGroupSid->SubAuthorityCount - pDomainSid->SubAuthorityCount) <= 2)
            {
                DWORD dwSubAuthorityCount = 0;

                dwSubAuthorityCount = pGroupSid->SubAuthorityCount
                                    - pDomainSid->SubAuthorityCount;

                dwGroupIds++;

                dwError = VmDirReallocateMemory(
                                  pGroupIds,
                                  (PVOID*)&pGroupIds,
                                  dwGroupIds * sizeof(VMDIR_GROUP_MEMBERSHIP));
                BAIL_ON_VMDIR_ERROR(dwError);

                if (dwSubAuthorityCount == 1)
                {
                    pGroupIds[dwGroupIds-1].Identifier[0] = 0;
                }
                else
                {
                    pGroupIds[dwGroupIds-1].Identifier[0] = pGroupSid->SubAuthority[pGroupSid->SubAuthorityCount-2];
                }
                pGroupIds[dwGroupIds-1].Identifier[1] = pGroupSid->SubAuthority[pGroupSid->SubAuthorityCount-1];
                pGroupIds[dwGroupIds-1].Attributes = SE_GROUP_ENABLED;
            }
            else
            {
                dwOtherSids++;

                dwError = VmDirReallocateMemory(
                                  pOtherSids,
                                  (PVOID*)&pOtherSids,
                                  dwOtherSids * sizeof(KERB_SID_AND_ATTRIBUTES));
                BAIL_ON_VMDIR_ERROR(dwError);

                ntStatus = RtlDuplicateSid(
                                   (PSID *)&pOtherSids[dwOtherSids-1].Sid,
                                   pGroupSid);
                dwError = LwNtStatusToWin32Error(ntStatus);
                BAIL_ON_VMDIR_ERROR(dwError);

                pOtherSids[dwOtherSids-1].Attributes = SE_GROUP_ENABLED;
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

    /* Create the authorization info structure */

    dwError = VmDirKrbCreateAuthzInfo(
                  pszUpnName,
                  pDomainSid,
                  pUserSid,
                  pGroupIds,
                  dwGroupIds,
                  pOtherSids,
                  dwOtherSids,
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
        VmDirKrbFreeAuthzInfo(pInfo);
    }
    if (pGroupIds)
    {
        VmDirFreeMemory(pGroupIds);
    }
    if (pOtherSids)
    {
        VmDirFreeMemory(pOtherSids);
    }
    goto cleanup;
}

VOID
VmDirKrbFreeAuthzInfo(
    VMDIR_AUTHZ_INFO* pInfo
    )
{
    if (pInfo)
    {
        if (pInfo->AccountName.Buffer)
        {
            VmDirFreeMemory(pInfo->AccountName.Buffer);
        }
        if (pInfo->GroupIds)
        {
            VmDirFreeMemory(pInfo->GroupIds);
        }
        if (pInfo->OtherSids)
        {
            VmDirFreeMemory(pInfo->OtherSids);
        }
        if (pInfo->DomainSid)
        {
            RtlMemoryFree(pInfo->DomainSid);
        }
        VmDirFreeMemory(pInfo);
    }
}
#endif // VMDIR_ENABLE_PAC
