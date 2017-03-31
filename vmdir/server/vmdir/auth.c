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
ULONG
LogAccessInfo(
    HANDLE hToken,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    ULONG ulAccessDesired
    );

static
VOID
VmDirSrvFreeAccessToken(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken
    );

DWORD
VmDirSrvCreateAccessToken(
    PCSTR pszUPN,
    PVMDIR_SRV_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pAccessToken), (PVOID*)&pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAccessToken->refCount = 1;

    dwError = VmDirAllocateStringA(pszUPN, &pAccessToken->pszUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAccessToken = pAccessToken;

cleanup:

    return dwError;

error:

    *ppAccessToken = NULL;

    if (pAccessToken)
    {
        VmDirSrvReleaseAccessToken(pAccessToken);
    }

    goto cleanup;
}

PVMDIR_SRV_ACCESS_TOKEN
VmDirSrvAcquireAccessToken(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken
    )
{
    if (pAccessToken)
    {
        InterlockedIncrement(&pAccessToken->refCount);
    }

    return pAccessToken;
}

VOID
VmDirSrvReleaseAccessToken(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken
    )
{
    if (InterlockedDecrement(&pAccessToken->refCount) == 0)
    {
        VmDirSrvFreeAccessToken(pAccessToken);
    }
}

//
// Following function performs AccessCheck, checking if identity in IDL_handle
// is allowed access for a resource whose SD is pSD.
//

BOOL
VmDirIsRpcOperationAllowed(
    handle_t pBinding,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    ULONG    ulAccessDesired
    )
{
#if defined(HAVE_DCERPC_WIN32)
	VMDIR_LOG_VERBOSE(LDAP_DEBUG_ACL, "RPC Access GRANTED!");
	return TRUE;
#else
    ULONG           ulError  = ERROR_SUCCESS;
    PACCESS_TOKEN   hToken         = NULL;
    ACCESS_MASK     accessGranted  = 0;
    BOOLEAN         bAccessGranted = FALSE;
    GENERIC_MAPPING genericMapping = {0};
#if defined(_WIN32) && !defined(HAVE_DCERPC_WIN32)
    BOOLEAN         bImpersonated = FALSE;
#endif

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
    rpc_binding_inq_access_token_caller(pBinding, &hToken, &ulError);
    BAIL_ON_VMDIR_ERROR(ulError);
#else
    ulError = RpcImpersonateClient( pBinding );
    BAIL_ON_VMDIR_ERROR(ulError);
    bImpersonated = TRUE;

    if ( OpenThreadToken(
            GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &hToken) == 0 )
    {
        ulError = GetLastError();
        BAIL_ON_VMDIR_ERROR(ulError);
    }

#endif

    ulError = LogAccessInfo(hToken, pSD, ulAccessDesired);
    BAIL_ON_VMDIR_ERROR(ulError);

    // Initialize generic mapping structure to map all.
    memset(&genericMapping, 0xff, sizeof(GENERIC_MAPPING));

    genericMapping.GenericRead    = GENERIC_READ;
    genericMapping.GenericWrite   = GENERIC_WRITE;
    genericMapping.GenericExecute = 0;
    genericMapping.GenericAll     = GENERIC_READ | GENERIC_WRITE;

    VmDirMapGenericMask(&ulAccessDesired, &genericMapping);

    bAccessGranted = VmDirAccessCheck(
                        pSD,
                        hToken,
                        ulAccessDesired,
                        0,
                        &genericMapping,
                        &accessGranted,
                        &ulError);
    BAIL_ON_VMDIR_ERROR(ulError);

cleanup:

#if defined(_WIN32) && !defined(HAVE_DCERPC_WIN32)
    if( bImpersonated != FALSE )
    {
        DWORD rpcError = RpcRevertToSelfEx(pBinding);

        if( rpcError != RPC_S_OK )
        {
            // real bad, need to exit the process ....
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "RpcRevertToSelfEx failed with %d. Exiting process.",
                rpcError );
            ExitProcess(rpcError);
        }
    }

#endif

    if (hToken)
    {
        VmDirReleaseAccessToken(&hToken);
    }

    if (bAccessGranted)
    {
        VMDIR_LOG_VERBOSE(LDAP_DEBUG_ACL, "RPC Access GRANTED!");
    }
    else
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "RPC Access DENIED!");
    }

    return bAccessGranted;

error:

    bAccessGranted = FALSE;

    goto cleanup;
#endif
}

// Log subject SID, resource SD, and desired access.
static
ULONG
LogAccessInfo(
    HANDLE hToken,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    ULONG ulAccessDesired
    )
{
   ULONG           ulError = ERROR_SUCCESS;
   PTOKEN_USER     pUser  = NULL;
   PTOKEN_GROUPS   pGroupInfo = NULL;
   ULONG           ulBufLen = 0;
   PSTR            pszUserStrSid=NULL;
   PSTR            pszStrSD = NULL;
   PSTR            pszGroupStrSid=NULL;
   PSECURITY_DESCRIPTOR_RELATIVE pRelSD=NULL;
   ULONG           iGroup = 0;

   VMDIR_LOG_VERBOSE(LDAP_DEBUG_ACL, "Access desired = %u.", ulAccessDesired);

   // Get SID from TokenUser

   // Determine required size of buffer for token information.
   if ((ulError = VmDirQueryAccessTokenInformation(
                               hToken,
                               TokenUser,
                               NULL,
                               0,
                               &ulBufLen)) != ERROR_INSUFFICIENT_BUFFER)
   {
      VMDIR_LOG_ERROR(
              VMDIR_LOG_MASK_ALL,
              "Error: expected return code (%u) found (%u)",
              ERROR_INSUFFICIENT_BUFFER,
              ulError);

      ulError = ERROR_GEN_FAILURE;
      BAIL_ON_VMDIR_ERROR(ulError);
   }

   ulError = VmDirAllocateMemory(ulBufLen, (PVOID *) &pUser);
   BAIL_ON_VMDIR_ERROR(ulError);

   ulError = VmDirQueryAccessTokenInformation(
                               hToken,
                               TokenUser,
                               pUser,
                               ulBufLen,
                               &ulBufLen);
   BAIL_ON_VMDIR_ERROR(ulError);

   ulError = VmDirAllocateCStringFromSid(
                               &pszUserStrSid,
                               pUser->User.Sid);
   BAIL_ON_VMDIR_ERROR(ulError);

   VMDIR_LOG_VERBOSE(LDAP_DEBUG_ACL, "Subject sid = %s.", VDIR_SAFE_STRING(pszUserStrSid));

   // Get Group memberships from TokenGroups

   // Determine required size of buffer for token information.
   ulBufLen = 0;
   if ((ulError = VmDirQueryAccessTokenInformation(
                               hToken,
                               TokenGroups,
                               NULL,
                               0,
                               &ulBufLen)) != ERROR_INSUFFICIENT_BUFFER)
   {
       VMDIR_LOG_ERROR(
               VMDIR_LOG_MASK_ALL,
               "Error: expected return code (%u) found (%u)",
               ERROR_INSUFFICIENT_BUFFER,
               ulError);

       ulError = ERROR_GEN_FAILURE;
       BAIL_ON_VMDIR_ERROR(ulError);
   }

   ulError = VmDirAllocateMemory(ulBufLen, (PVOID *) &pGroupInfo);
   BAIL_ON_VMDIR_ERROR(ulError);

   ulError = VmDirQueryAccessTokenInformation(
                               hToken,
                               TokenGroups,
                               pGroupInfo,
                               ulBufLen,
                               &ulBufLen);
   BAIL_ON_VMDIR_ERROR(ulError);

   for (iGroup = 0; iGroup < pGroupInfo->GroupCount; iGroup++)
   {
      ulError = VmDirAllocateCStringFromSid(
                                &pszGroupStrSid,
                                pGroupInfo->Groups[iGroup].Sid);
      BAIL_ON_VMDIR_ERROR(ulError);

      VMDIR_LOG_VERBOSE(
              LDAP_DEBUG_ACL,
              "Member of group sid = %s.",
              VDIR_SAFE_STRING(pszGroupStrSid));

      VMDIR_SAFE_FREE_MEMORY(pszGroupStrSid);
   }

   ulBufLen = 0;
   if ((ulError = VmDirAbsoluteToSelfRelativeSD(
                               pSD,
                               NULL,
                               &ulBufLen)) != ERROR_INSUFFICIENT_BUFFER)
   {
      // Call should have failed due to zero-length buffer.
      VMDIR_LOG_ERROR(
              VMDIR_LOG_MASK_ALL,
              "Error: expected return code (%u) found (%u)",
              ERROR_INSUFFICIENT_BUFFER,
              ulError);

      ulError = ERROR_GEN_FAILURE;
      BAIL_ON_VMDIR_ERROR(ulError);
   }

   ulError = VmDirAllocateMemory(ulBufLen, (PVOID *) &pRelSD);
   BAIL_ON_VMDIR_ERROR(ulError);

   ulError = VmDirAbsoluteToSelfRelativeSD(
                               pSD,
                               pRelSD,
                               &ulBufLen);
   BAIL_ON_VMDIR_ERROR(ulError);

   ulError = VmDirAllocateSddlCStringFromSecurityDescriptor(
        pRelSD, SDDL_REVISION_1, 255, &pszStrSD
   );
   BAIL_ON_VMDIR_ERROR(ulError);

   VMDIR_LOG_VERBOSE(LDAP_DEBUG_ACL, "String SD = %s.", VDIR_SAFE_STRING(pszStrSD));

error:

   VMDIR_SAFE_FREE_MEMORY(pszUserStrSid);
   VMDIR_SAFE_FREE_MEMORY(pszGroupStrSid);
   VMDIR_SAFE_FREE_MEMORY(pszStrSD);

   VMDIR_SAFE_FREE_MEMORY(pUser);
   VMDIR_SAFE_FREE_MEMORY(pGroupInfo);
   VMDIR_SAFE_FREE_MEMORY(pRelSD);

   return ulError;
}

static
VOID
VmDirSrvFreeAccessToken(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken
    )
{
    VMDIR_SAFE_FREE_MEMORY(pAccessToken->pszUPN);
    VmDirFreeMemory(pAccessToken);
}

DWORD
VmDirAdministratorAccessCheck(
    PCSTR pszUpn
    )
{
    DWORD dwError = 0;
    PCSTR pszDomainDn = NULL;
    const CHAR szAdministrators[] = "cn=Administrators,cn=Builtin";
    PSTR pszAdministratorsDn = NULL;
    PSTR *ppszMemberships = NULL;
    DWORD dwMemberships = 0;

    if (IsNullOrEmptyString(pszUpn))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszDomainDn = gVmdirServerGlobals.systemDomainDN.lberbv.bv_val;
    if (pszDomainDn == NULL)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetUPNMemberships(pszUpn, &ppszMemberships, &dwMemberships);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszAdministratorsDn, "%s,%s", szAdministrators, pszDomainDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VmDirIsMemberOf(ppszMemberships, dwMemberships, gVmdirServerGlobals.bvDCGroupDN.lberbv.bv_val) &&
        !VmDirIsMemberOf(ppszMemberships, dwMemberships, pszAdministratorsDn))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszAdministratorsDn);
    VmDirFreeMemberships(ppszMemberships, dwMemberships);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirAdministratorAccessCheck failed (%u)", dwError);
    goto cleanup;

}

BOOLEAN
VmDirIsMemberOf(
    PSTR *ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupDn
    )
{
    BOOLEAN bRetVal = FALSE;
    DWORD i = 0;

    if (ppszMemberships && pszGroupDn)
    {
        for (i = 0; i < dwMemberships; i++)
        {
            if (VmDirStringCompareA(ppszMemberships[i], pszGroupDn, FALSE) == 0)
            {
                bRetVal = TRUE;
                break;
            }
        }
    }
    return bRetVal;
}

DWORD
VmDirGetUPNMemberships(
    PCSTR pszUpnName,
    PSTR **pppszMemberships,
    PDWORD pdwMemberships
    )
{
    DWORD dwError = 0;
    VDIR_ENTRY_ARRAY entryArray = {0};
    VDIR_OPERATION searchOp = {0};
    BOOLEAN bHasTxn = FALSE;
    PVDIR_ATTRIBUTE pMemberOf = NULL;
    PSTR *ppszMemberships = NULL;
    DWORD dwMemberships = 0;
    DWORD i = 0;

    if (IsNullOrEmptyString(pszUpnName) ||
        pppszMemberships == NULL ||
        pdwMemberships == NULL)
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

    dwError = VmDirInitStackOperation(&searchOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_SEARCH, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOp.pBEIF = VmDirBackendSelect(NULL);

    dwError = searchOp.pBEIF->pfnBETxnBegin(searchOp.pBECtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = VmDirBuildMemberOfAttribute(&searchOp, entryArray.pEntry, &pMemberOf);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pMemberOf)
    {
        dwMemberships = pMemberOf->numVals;
    }

    if (dwMemberships)
    {
        dwError = VmDirAllocateMemory(dwMemberships * sizeof(PSTR), (PVOID)&ppszMemberships);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < dwMemberships; i++)
        {
            PCSTR pszMemberOf = pMemberOf->vals[i].lberbv.bv_val;

            dwError = VmDirAllocateStringA(pszMemberOf, &ppszMemberships[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *pppszMemberships = ppszMemberships;
    *pdwMemberships = dwMemberships;

cleanup:

    if (pMemberOf)
    {
        VmDirFreeAttribute(pMemberOf);
    }
    if (bHasTxn)
    {
        searchOp.pBEIF->pfnBETxnCommit(searchOp.pBECtx);
    }
    VmDirFreeOperationContent(&searchOp);
    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:
    VmDirFreeMemberships(ppszMemberships, dwMemberships);

    goto cleanup;
}
