/*
 * Copyright © 208 VMware, Inc.  All Rights Reserved.
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
VmDirSrvCreateDomainTrust(
    PCSTR   pszTrustName,
    PCSTR   pszDomainName,
    PCSTR   pszTrustPasswdIn,
    PCSTR   pszTrustPasswdOut,
    PCSTR   pszEntryDN
    )
{
    DWORD   dwError = 0;
    PSTR    pszUPN1 = NULL;
    PSTR    pszUPN2 = NULL;

    PSTR    ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,      (PSTR)OC_TRUSTED_DOMAIN,
            ATTR_CN,                (PSTR)pszTrustName,
            NULL
    };

    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    if (IsNullOrEmptyString(pszTrustName) ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszTrustPasswdIn) ||
        IsNullOrEmptyString(pszTrustPasswdOut) ||
        IsNullOrEmptyString(pszEntryDN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirSimpleEntryCreate(
                    pSchemaCtx,
                    ppszAttributes,
                    (PSTR)pszEntryDN,
                    0);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSchemaCtxRelease(pSchemaCtx);
    pSchemaCtx = NULL;

    dwError = VmDirAllocateStringPrintf(
                  &pszUPN1,
                  "krbtgt/%s@%s",
                  pszTrustName,
                  pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                  &pszUPN2,
                  "krbtgt/%s@%s",
                  pszDomainName,
                  pszTrustName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirKrbSetTrustAuthInfo(
                    pszUPN1,
                    pszEntryDN,
                    VMDIR_TRUST_DIRECTION_INCOMING,
                    pszTrustPasswdIn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirKrbSetTrustAuthInfo(
                    pszUPN2,
                    pszEntryDN,
                    VMDIR_TRUST_DIRECTION_OUTGOING,
                    pszTrustPasswdOut);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirSchemaCtxRelease(pSchemaCtx);
    VMDIR_SAFE_FREE_MEMORY(pszUPN1);
    VMDIR_SAFE_FREE_MEMORY(pszUPN2);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s DN(%s) failed, (%u)",
                     __FUNCTION__,
                     VDIR_SAFE_STRING(pszEntryDN),
                     dwError);

    goto cleanup;
}

DWORD
VmDirKrbSetTrustAuthInfo(
    PCSTR pszUPN,
    PCSTR pszDN,
    DWORD dwTrustDirection,
    PCSTR pszPasswd
    )
{
    DWORD            dwError = 0;
    VDIR_OPERATION   op = {0};
    PSTR             pszLocalErrMsg = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PVDIR_ENTRY      pEntry = NULL;
    PVDIR_ATTRIBUTE  pAttrAuthInfo = NULL;
    VDIR_BERVALUE    bvUPN = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE    bvDN = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE    bvPasswd = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE    bvAuthInfoBlob = VDIR_BERVALUE_INIT;

    if ( IsNullOrEmptyString(pszDN)    ||
         IsNullOrEmptyString(pszPasswd)
        )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    bvUPN.lberbv_val = (PSTR)pszUPN;
    bvUPN.lberbv_len = VmDirStringLenA(pszUPN);

    bvDN.lberbv_val = (PSTR)pszDN;
    bvDN.lberbv_len = VmDirStringLenA(pszDN);

    bvPasswd.lberbv_val = (PSTR)pszPasswd;
    bvPasswd.lberbv_len = VmDirStringLenA(pszPasswd);

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    "",
                    LDAP_SCOPE_SUBTREE,
                    ATTR_DN,
                    pszDN,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 1)
    {
        if (dwTrustDirection == VMDIR_TRUST_DIRECTION_INCOMING)
        {
            pAttrAuthInfo = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_TRUST_AUTH_INCOMING);
        }
        else
        {
            pAttrAuthInfo = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_TRUST_AUTH_OUTGOING);
        }
        if (pAttrAuthInfo)
        {
            dwError = VMDIR_ERROR_ENTRY_ALREADY_EXIST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = &(entryArray.pEntry[0]);

    dwError = VmDirKrbCreateKeyBlob(
                  &bvUPN,
                  &bvPasswd,
                  1,
                  &bvAuthInfoBlob);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pEntry->allocType == ENTRY_STORAGE_FORMAT_PACK)
    {
        dwError = VmDirEntryUnpack( pEntry );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
        dwError,
        pszLocalErrMsg,
        "%s: VmDirInitStackOperation failed: %u",
        __FUNCTION__,
        dwError);

    op.pBEIF = VmDirBackendSelect(NULL);
    assert(op.pBEIF);

    op.reqDn.lberbv.bv_val = pEntry->dn.lberbv.bv_val;
    op.reqDn.lberbv.bv_len = pEntry->dn.lberbv.bv_len;
    op.request.modifyReq.dn.lberbv = op.reqDn.lberbv;

    if (dwTrustDirection == VMDIR_TRUST_DIRECTION_INCOMING)
    {
        dwError = VmDirAppendAMod( &op,
                                   MOD_OP_ADD,
                                   ATTR_TRUST_AUTH_INCOMING,
                                   ATTR_TRUST_AUTH_INCOMING_LEN,
                                   bvAuthInfoBlob.lberbv_val,
                                   bvAuthInfoBlob.lberbv_len);
    }
    else
    {
        dwError = VmDirAppendAMod( &op,
                                   MOD_OP_ADD,
                                   ATTR_TRUST_AUTH_OUTGOING,
                                   ATTR_TRUST_AUTH_OUTGOING_LEN,
                                   bvAuthInfoBlob.lberbv_val,
                                   bvAuthInfoBlob.lberbv_len);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalModifyEntry(&op);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirFreeBervalContent(&bvAuthInfoBlob);
    VmDirFreeEntryArrayContent(&entryArray);
    VmDirFreeOperationContent(&op);
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s (%s) failed, (%u)",
                     __FUNCTION__,
                     VDIR_SAFE_STRING(pszDN),
                     dwError);
    goto cleanup;
}

DWORD
VmDirKrbGetTrustAuthInfo(
    PCSTR  pszDN,
    DWORD  dwTrustDirection,
    PBYTE* ppByteAuthInfo,
    DWORD* pdwAuthInfoLen
    )
{
    DWORD            dwError = 0;
    PVDIR_ATTRIBUTE  pAttrAuthInfo = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PBYTE            pLocalAuthInfo = NULL;

    if ( IsNullOrEmptyString(pszDN) ||
         ppByteAuthInfo == NULL ||
         pdwAuthInfoLen == NULL
        )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    "",
                    LDAP_SCOPE_SUBTREE,
                    ATTR_DN,
                    pszDN,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 1)
    {
        if (dwTrustDirection == VMDIR_TRUST_DIRECTION_INCOMING)
        {
            pAttrAuthInfo = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_TRUST_AUTH_INCOMING);
        }
        else
        {
            pAttrAuthInfo = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_TRUST_AUTH_OUTGOING);
        }
        if (!pAttrAuthInfo)
        {
            dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateAndCopyMemory(
                        pAttrAuthInfo->vals[0].lberbv_val,
                        pAttrAuthInfo->vals[0].lberbv_len,
                        (PVOID*)&pLocalAuthInfo);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (entryArray.iSize == 0)
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppByteAuthInfo = pLocalAuthInfo;
    *pdwAuthInfoLen = (DWORD) (pAttrAuthInfo->vals[0].lberbv_len);

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY( pLocalAuthInfo );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s (%s) failed, (%u)",
                     __FUNCTION__,
                     VDIR_SAFE_STRING(pszDN),
                     dwError);
    goto cleanup;
}
