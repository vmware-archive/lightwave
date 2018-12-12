/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
DWORD
_VmDirBuildTokenGroups(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszBuiltinUsersGroupSid,
    PTOKEN_GROUPS*  ppTokenGroups
    );

static
VOID
_VmDirFreeTokenGroups(
    PTOKEN_GROUPS   pGroups
    );

DWORD
VmDirSrvCreateAccessTokenWithEntry(
    PVDIR_ENTRY     pEntry,
    PACCESS_TOKEN*  ppToken,
    PSTR*           ppszObjectSid
    )
{
    DWORD               dwError = ERROR_SUCCESS;
    PACCESS_TOKEN       pToken = NULL;
    TOKEN_USER          user = {{0}};
    TOKEN_OWNER         owner = {0};
    PTOKEN_GROUPS       pGroups = {0};
    TOKEN_PRIVILEGES    privileges = {0};
    TOKEN_PRIMARY_GROUP primaryGroup = {0};
    TOKEN_DEFAULT_DACL  dacl = {0};
    PSTR                pszObjectSid = NULL;
    PSTR                pszBuiltinUsersGroupSid = NULL;
    PCSTR               pszDomainDn = NULL;

    if (!pEntry || !ppToken || !ppszObjectSid)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pToken = *ppToken;

    if (pToken)
    {
        dwError = ERROR_TOKEN_IN_USE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetObjectSidFromEntry(
            pEntry, &pszObjectSid, &user.User.Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    owner.Owner = user.User.Sid;

    pszDomainDn = VmDirSearchDomainDN(BERVAL_NORM_VAL(pEntry->dn));
    if (!pszDomainDn)
    {
        dwError = VMDIR_ERROR_DOMAIN_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGenerateWellknownSid(
            pszDomainDn,
            VMDIR_DOMAIN_ALIAS_RID_USERS,
            &pszBuiltinUsersGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // The primary group should be built-in\Users for all users.
    dwError = VmDirAllocateSidFromCString(
            pszBuiltinUsersGroupSid, &primaryGroup.PrimaryGroup);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirBuildTokenGroups(
            pEntry, pszBuiltinUsersGroupSid, &pGroups);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAccessToken(
            &pToken,
            &user,
            pGroups,
            &privileges,
            &owner,
            &primaryGroup,
            &dacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszObjectSid = pszObjectSid;
    *ppToken = pToken;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(user.User.Sid);
    VMDIR_SAFE_FREE_MEMORY(primaryGroup.PrimaryGroup);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltinUsersGroupSid);
    _VmDirFreeTokenGroups(pGroups);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszObjectSid);
    VmDirReleaseAccessToken(&pToken);
    goto cleanup;
}

DWORD
VmDirSrvCreateAccessTokenForWellKnowObject(
    PACCESS_TOKEN*  ppToken,
    PCSTR           pszWellknownObjectSid
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
VmDirSrvCreateAccessTokenForAdmin(
    PACCESS_TOKEN*  ppToken
    )
{
    DWORD   dwError = 0;
    PSTR    pszAdministratorSid = NULL;
    PSTR    pszBuiltInAdminsGroupSid = NULL;
    PSTR    pszBuiltInUsersGroupSid = NULL;
    TOKEN_USER      user = {{0}};
    TOKEN_OWNER     owner = {0};
    PTOKEN_GROUPS   pGroups = NULL;
    TOKEN_PRIVILEGES    privileges = {0};
    TOKEN_PRIMARY_GROUP primaryGroup = {0};
    TOKEN_DEFAULT_DACL  dacl = {0};
    PACCESS_TOKEN   pToken = NULL;

    // build user token
    dwError = VmDirGenerateWellknownSid(
            gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
            VMDIR_DOMAIN_USER_RID_ADMIN,
            &pszAdministratorSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateSidFromCString(
            pszAdministratorSid, &user.User.Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    owner.Owner = user.User.Sid;

    // build group token
    dwError = VmDirAllocateMemory(
            sizeof(TOKEN_GROUPS) + sizeof(SID_AND_ATTRIBUTES),
            (PVOID*)&pGroups);
    BAIL_ON_VMDIR_ERROR(dwError);

    pGroups->GroupCount = 1;

    dwError = VmDirGenerateWellknownSid(
            gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
            VMDIR_DOMAIN_ALIAS_RID_ADMINS,
            &pszBuiltInAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateSidFromCString(
            pszBuiltInAdminsGroupSid, &pGroups->Groups[0].Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    pGroups->Groups[0].Attributes = SE_GROUP_ENABLED; // TODO should be set on the basis of status of the group?

    // build primary group token (built-in users)
    dwError = VmDirGenerateWellknownSid(
            gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
            VMDIR_DOMAIN_ALIAS_RID_USERS,
            &pszBuiltInUsersGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateSidFromCString(
            pszBuiltInUsersGroupSid, &primaryGroup.PrimaryGroup);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAccessToken(
            &pToken, &user, pGroups, &privileges, &owner, &primaryGroup, &dacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppToken = pToken;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAdministratorSid);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInAdminsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInUsersGroupSid);
    VMDIR_SAFE_FREE_MEMORY(user.User.Sid);
    VMDIR_SAFE_FREE_MEMORY(pGroups->Groups[0].Sid);
    VMDIR_SAFE_FREE_MEMORY(pGroups);
    VMDIR_SAFE_FREE_MEMORY(primaryGroup.PrimaryGroup);
    return dwError;

error:
    if (pToken)
    {
        VmDirReleaseAccessToken(&pToken);
    }
    goto cleanup;
}

DWORD
VmDirCreateAccessToken(
    PACCESS_TOKEN*          AccessToken,
    PTOKEN_USER             User,
    PTOKEN_GROUPS           Groups,
    PTOKEN_PRIVILEGES       Privileges,
    PTOKEN_OWNER            Owner,
    PTOKEN_PRIMARY_GROUP    PrimaryGroup,
    PTOKEN_DEFAULT_DACL     DefaultDacl
    )
{
    return LwNtStatusToWin32Error(
            RtlCreateAccessToken(
                    AccessToken,
                    User,
                    Groups,
                    Privileges,
                    Owner,
                    PrimaryGroup,
                    DefaultDacl,
                    NULL));
}

//
// Builds up a list of all the groups this user is a member of. All users
// (anyone who doesn't login anonymously) automatically are members of their
// domain's "Users" group and the global "Authenticated Users" group. All other
// memberships are explicit (dictated by the "memberOf" attribute).
//
static
DWORD
_VmDirBuildTokenGroups(
    PVDIR_ENTRY     pEntry,
    PCSTR           pszBuiltinUsersGroupSid,
    PTOKEN_GROUPS*  ppTokenGroups
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwDefaultGroupCount = 0;
    DWORD   dwEntryGroupCount = 0;
    DWORD   dwTotalGroupCount = 0;
    DWORD   i = 0, j = 0;
    BOOLEAN bHasTxn = FALSE;
    VDIR_OPERATION  searchOp = {0};
    PVDIR_ATTRIBUTE pMemberOfAttr = NULL;
    PVDIR_ENTRY     pGroupEntry = NULL;
    PTOKEN_GROUPS   pTokenGroups = NULL;
    PCSTR   ppszDefaultGroups[] = {pszBuiltinUsersGroupSid, VMDIR_AUTHENTICATED_USER_SID};

    if (!pEntry || !ppTokenGroups || IsNullOrEmptyString(pszBuiltinUsersGroupSid))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation(
            &searchOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_SEARCH, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOp.pBEIF = VmDirBackendSelect(ALIAS_MAIN);

    // begin txn
    dwError = searchOp.pBEIF->pfnBETxnBegin(searchOp.pBECtx, VDIR_BACKEND_TXN_READ, &bHasTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBuildMemberOfAttribute(&searchOp, pEntry, &pMemberOfAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    // commit txn
    if (bHasTxn)
    {
        dwError = searchOp.pBEIF->pfnBETxnCommit(searchOp.pBECtx);
        bHasTxn = FALSE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwDefaultGroupCount = VMDIR_ARRAY_SIZE(ppszDefaultGroups);
    dwEntryGroupCount = pMemberOfAttr ? pMemberOfAttr->numVals : 0;
    dwTotalGroupCount = dwDefaultGroupCount + dwEntryGroupCount;

    dwError = VmDirAllocateMemory(
            sizeof(TOKEN_GROUPS) +
            (sizeof(SID_AND_ATTRIBUTES) * dwTotalGroupCount),
            (PVOID*)&pTokenGroups);
    BAIL_ON_VMDIR_ERROR(dwError);
    pTokenGroups->GroupCount = dwTotalGroupCount;

    for (i = 0; i < dwDefaultGroupCount; i++)
    {
        dwError = VmDirAllocateSidFromCString(
                ppszDefaultGroups[i],
                &pTokenGroups->Groups[i].Sid);
        BAIL_ON_VMDIR_ERROR(dwError);
        pTokenGroups->Groups[i].Attributes = SE_GROUP_ENABLED;
    }

    for (i = 0, j = dwDefaultGroupCount; i < dwEntryGroupCount; i++, j++)
    {
        dwError = VmDirSimpleDNToEntry(
                pMemberOfAttr->vals[i].lberbv.bv_val, &pGroupEntry);
        if (dwError)
        {
            // may be deleted in the meanwhile
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                    "_VmDirBuildTokenGroups() memmberOf entry (%s) not found, error code (%d)",
                    pMemberOfAttr->vals[i].lberbv.bv_val, dwError );
            continue;
        }

        dwError = VmDirGetObjectSidFromEntry(
                pGroupEntry, NULL, &pTokenGroups->Groups[j].Sid);
        BAIL_ON_VMDIR_ERROR(dwError);
        pTokenGroups->Groups[j].Attributes = SE_GROUP_ENABLED;

        VmDirFreeEntry(pGroupEntry);
        pGroupEntry = NULL;
    }

    *ppTokenGroups = pTokenGroups;

cleanup:
    VmDirFreeAttribute(pMemberOfAttr);
    VmDirFreeEntry(pGroupEntry);
    VmDirFreeOperationContent(&searchOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "_VmDirBuildTokenGroups() failed, entry DN (%s), error code (%d)",
            pEntry ? pEntry->dn.lberbv.bv_val : "NULL", dwError);

    if (bHasTxn)
    {
        searchOp.pBEIF->pfnBETxnAbort(searchOp.pBECtx);
    }
    _VmDirFreeTokenGroups(pTokenGroups);
    goto cleanup;
}

static
VOID
_VmDirFreeTokenGroups(
    PTOKEN_GROUPS   pGroups
    )
{
    DWORD i = 0;

    if (pGroups)
    {
        for (i = 0; i < pGroups->GroupCount; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(pGroups->Groups[i].Sid);
        }
        VMDIR_SAFE_FREE_MEMORY(pGroups);
    }
}
