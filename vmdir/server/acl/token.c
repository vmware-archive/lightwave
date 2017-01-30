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
VmDirCreateAccessToken(
    PACCESS_TOKEN*          AccessToken,
    PTOKEN_USER             User,
    PTOKEN_GROUPS           Groups,
    PTOKEN_PRIVILEGES       Privileges,
    PTOKEN_OWNER            Owner,
    PTOKEN_PRIMARY_GROUP    PrimaryGroup,
    PTOKEN_DEFAULT_DACL     DefaultDacl
    );

static
DWORD
_VmDirBuildTokenGroups(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszBuiltinUsersGroupSid,
    PTOKEN_GROUPS * ppTokenGroups
    );

// Create access token for the bind
DWORD
VmDirSrvCreateAccessTokenWithDn(
    PCSTR           pszObjectDn,
    PACCESS_TOKEN*  ppToken
    )
{
    DWORD           dwError = ERROR_SUCCESS;
    PVDIR_ENTRY     pEntry = NULL;
    PACCESS_TOKEN   pToken = NULL;

    dwError = VmDirSimpleDNToEntry(pszObjectDn, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateAccessTokenWithEntry(pEntry, &pToken, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppToken = pToken;

cleanup:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    return dwError;

error:
    if (pToken)
    {
        VmDirReleaseAccessToken(&pToken);
    }

    goto cleanup;
}

DWORD
VmDirSrvCreateAccessTokenWithEntry(
    PVDIR_ENTRY     pEntry,
    PACCESS_TOKEN*  ppToken,
    PSTR *          ppszObjectSid /* Optional */
    )
{
    DWORD               dwError = ERROR_SUCCESS;
    PACCESS_TOKEN       pToken = *ppToken;
    TOKEN_USER          user = {{0}};
    TOKEN_OWNER         owner = {0};
    PTOKEN_GROUPS       pGroups = {0};
    TOKEN_PRIVILEGES    privileges = {0};
    TOKEN_PRIMARY_GROUP primaryGroup = {0};
    TOKEN_DEFAULT_DACL  dacl = {0};
    PSTR                pszObjectSid = NULL;
    PSTR                pszBuildinUsersGroupSid = NULL;
    PCSTR               pszDomainDn = NULL;
    unsigned int        i = 0;

    if (pToken)
    {
        dwError = ERROR_TOKEN_IN_USE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetObjectSidFromEntry(pEntry, &pszObjectSid, &user.User.Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    owner.Owner = user.User.Sid;

    pszDomainDn = VmDirSearchDomainDN(BERVAL_NORM_VAL(pEntry->dn));
    if (!pszDomainDn)
    {
        dwError = VMDIR_ERROR_DOMAIN_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGenerateWellknownSid(pszDomainDn, VMDIR_DOMAIN_ALIAS_RID_USERS, &pszBuildinUsersGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Primary groups should be built-in\Users not admins
    dwError = VmDirAllocateSidFromCString(pszBuildinUsersGroupSid, &primaryGroup.PrimaryGroup);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirBuildTokenGroups(pEntry, pszBuildinUsersGroupSid, &pGroups);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAccessToken(&pToken,
                                     &user,
                                     pGroups,
                                     &privileges,
                                     &owner,
                                     &primaryGroup,
                                     &dacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppszObjectSid)
    {
        *ppszObjectSid = pszObjectSid;
        pszObjectSid = NULL;
    }
    *ppToken = pToken;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszObjectSid);
    VMDIR_SAFE_FREE_MEMORY(user.User.Sid);
    VMDIR_SAFE_FREE_MEMORY(primaryGroup.PrimaryGroup);
    VMDIR_SAFE_FREE_MEMORY(pszBuildinUsersGroupSid);
    if (pGroups)
    {
        for (i = 0; i < pGroups->GroupCount; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(pGroups->Groups[i].Sid);
        }
        VmDirFreeMemory(pGroups);
    }

    return dwError;

error:
    *ppToken = NULL; // TODO

    VmDirReleaseAccessToken(&pToken);
    goto cleanup;
}


//
// Builds up a list of all the groups this user is a member of. Note that all
// known/authenticated users belong to the "Users" group (and the rest are
// dictated by the "memberOf" attribute).
//
static
DWORD
_VmDirBuildTokenGroups(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszBuiltinUsersGroupSid,
    PTOKEN_GROUPS * ppTokenGroups)
{
    DWORD               dwError = ERROR_SUCCESS;
    PVDIR_ATTRIBUTE     pMemberOfAttr = NULL;
    unsigned int        i = 0;
    PVDIR_ENTRY         pGroupEntry = NULL;
    VDIR_OPERATION      searchOp = {0};
    BOOLEAN             bHasTxn = FALSE;
    PTOKEN_GROUPS       pLocalTokenGroups = NULL;
    DWORD               dwGroupCount = 0;

    if ( pEntry == NULL || ppTokenGroups == NULL )
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &searchOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_SEARCH, NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOp.pBEIF = VmDirBackendSelect(NULL);

    // start txn
    dwError = searchOp.pBEIF->pfnBETxnBegin( searchOp.pBECtx, VDIR_BACKEND_TXN_READ );
    BAIL_ON_VMDIR_ERROR(dwError);

    bHasTxn = TRUE;

    dwError = VmDirBuildMemberOfAttribute( &searchOp, pEntry, &pMemberOfAttr );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pMemberOfAttr != NULL)
    {
        dwGroupCount = pMemberOfAttr->numVals + 1;
    }
    else
    {
        dwGroupCount = 1;
    }

    // SJ-TBD: Do we need to align the address??
    dwError = VmDirAllocateMemory( sizeof(TOKEN_GROUPS) +
                                   (sizeof(SID_AND_ATTRIBUTES) * dwGroupCount),
                                   (PVOID*)&pLocalTokenGroups );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateSidFromCString(
                pszBuiltinUsersGroupSid,
                &pLocalTokenGroups->Groups[0].Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // SJ-TBD: should be set on the basis of status of the group??
    pLocalTokenGroups->Groups[0].Attributes = SE_GROUP_ENABLED;

    pLocalTokenGroups->GroupCount = dwGroupCount;

    if (pMemberOfAttr)
    {
        for (i = 0; i < pMemberOfAttr->numVals; i++)
        {
            if ((dwError = VmDirSimpleDNToEntry(pMemberOfAttr->vals[i].lberbv.bv_val, &pGroupEntry)) != 0)
            {
                // may be deleted in the meanwhile

                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                                   "_VmDirBuildTokenGroups() memmberOf entry (%s) not found, error code (%d)",
                                   pMemberOfAttr->vals[i].lberbv.bv_val, dwError );
                continue;
            }

            dwError = VmDirGetObjectSidFromEntry(pGroupEntry, NULL, &pLocalTokenGroups->Groups[i + 1].Sid);
            BAIL_ON_VMDIR_ERROR(dwError);

            // SJ-TBD: should be set on the basis of status of the group??
            pLocalTokenGroups->Groups[i + 1].Attributes = SE_GROUP_ENABLED;

            VmDirFreeEntry(pGroupEntry);
            pGroupEntry = NULL;
        }
    }

    *ppTokenGroups = pLocalTokenGroups;

cleanup:
    VmDirFreeAttribute(pMemberOfAttr);
    if (pGroupEntry)
    {
        VmDirFreeEntry(pGroupEntry);
    }
    if (bHasTxn)
    {
        searchOp.pBEIF->pfnBETxnCommit( searchOp.pBECtx);
    }
    VmDirFreeOperationContent(&searchOp);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirBuildTokenGroups() failed, entry DN (%s), error code (%d)",
                     pEntry ? pEntry->dn.lberbv.bv_val : "NULL", dwError );

    VMDIR_SAFE_FREE_MEMORY(pLocalTokenGroups);

    goto cleanup;
}

DWORD
VmDirSrvCreateAccessTokenForWellKnowObject(
    PACCESS_TOKEN * ppToken,
    PCSTR            pszWellknownObjectSid
    )
{
    DWORD               dwError = ERROR_SUCCESS;
    PACCESS_TOKEN       pToken = NULL;
    TOKEN_USER          user = {{0}};
    TOKEN_OWNER         owner = {0};
    TOKEN_GROUPS        groups = {0};
    TOKEN_PRIVILEGES    privileges = {0};
    TOKEN_PRIMARY_GROUP primaryGroup = {0};
    TOKEN_DEFAULT_DACL  dacl = {0};

    dwError = VmDirAllocateSidFromCString(pszWellknownObjectSid, &user.User.Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAccessToken(&pToken,
                                     &user,
                                     &groups,
                                     &privileges,
                                     &owner,
                                     &primaryGroup,
                                     &dacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppToken = pToken;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(user.User.Sid);

    return dwError;

error:
    *ppToken = NULL;

    if (pToken)
    {
        VmDirReleaseAccessToken(&pToken);
    }

    goto cleanup;
}

DWORD
VmDirCreateAccessToken(
    PACCESS_TOKEN *         AccessToken,
    PTOKEN_USER             User,
    PTOKEN_GROUPS           Groups,
    PTOKEN_PRIVILEGES       Privileges,
    PTOKEN_OWNER            Owner,
    PTOKEN_PRIMARY_GROUP    PrimaryGroup,
    PTOKEN_DEFAULT_DACL     DefaultDacl
    )
{
    return LwNtStatusToWin32Error(RtlCreateAccessToken(
                                   AccessToken,
                                   User,
                                   Groups,
                                   Privileges,
                                   Owner,
                                   PrimaryGroup,
                                   DefaultDacl,
                                   NULL));
}
