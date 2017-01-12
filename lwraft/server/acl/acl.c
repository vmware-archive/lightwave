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
DWORD
VmDirBuildDefaultDaclForEntry(
    PSID    pOwnerSid,
    PSTR    pszAdminsGroupSid,
    PACL *  ppDacl
    );

static
DWORD
VmDirSrvCreateAccessTokenForWellKnowObject(
    PACCESS_TOKEN*  ppToken,
    PSTR            pszWellknownObjectSid
    );

static
DWORD
VmDirSrvAccessCheckSelf(
    PSTR        pszNormBindedDn,
    PVDIR_ENTRY pEntry,
    PSTR        pszWellKnownObjectSid,
    ACCESS_MASK accessDesired,
    ACCESS_MASK *psamGranted
    );

static
DWORD
VmDirSrvAccessCheckEntry(
    PACCESS_TOKEN   pToken,
    PVDIR_ENTRY     pEntry,
    ACCESS_MASK     accessDesired,
    ACCESS_MASK *   psamGranted
    );

static
DWORD
VmDirIsBindDnMemberOfSystemDomainAdmins(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ACCESS_INFO   pAccessInfo,
    PBOOLEAN            pbIsMemberOfAdmins
    );

static
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
    PTOKEN_GROUPS * ppTokenGroups);

static
BOOLEAN
VmDirIsSpecialAllowedSearchEntry(
    PVDIR_ENTRY pSrEntry
    )
{
    // DSE_ROOT_DN and PERSISTED_DSE_ROOT_DN, SCHEMA_NAMING_CONTEXT_DN
    // SUB_SCHEMA_SUB_ENTRY_DN should allow anonymous bind READ
    return (!VmDirStringCompareA(pSrEntry->dn.lberbv.bv_val, DSE_ROOT_DN, FALSE)
            || !VmDirStringCompareA(pSrEntry->dn.lberbv.bv_val, PERSISTED_DSE_ROOT_DN, FALSE)
            || !VmDirStringCompareA(pSrEntry->dn.lberbv.bv_val, SCHEMA_NAMING_CONTEXT_DN, FALSE)
            || !VmDirStringCompareA(pSrEntry->dn.lberbv.bv_val, SUB_SCHEMA_SUB_ENTRY_DN, FALSE));
}


/*
 * management node computer account
 * 1. allow read to domain tree
 * 2. allow write under service containers cn=services,SYSTEM_DOMAIN
 */
static
BOOLEAN
_VmDirDCClientGroupAccessCheck(
    PVDIR_OPERATION     pOperation,
    ACCESS_MASK         accessDesired
    )
{
    DWORD           dwError = 0;
    BOOLEAN         bIsAllowAccess = FALSE;
    PVDIR_BERVALUE  pBervDN = NULL;

    if ( (accessDesired & -1) == VMDIR_RIGHT_DS_READ_PROP )
    {  // grant read only request
        bIsAllowAccess = TRUE;
        goto cleanup;
    }

    if ( pOperation->reqCode == LDAP_REQ_ADD )
    {
        pBervDN = &(pOperation->request.addReq.pEntry->dn);
    }
    else if ( pOperation->reqCode == LDAP_REQ_DELETE )
    {
        pBervDN = &(pOperation->request.deleteReq.dn);
    }
    else if ( pOperation->reqCode == LDAP_REQ_MODIFY )
    {
        pBervDN = &(pOperation->request.modifyReq.dn);
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // for all other access request, target DN must be under service container
    dwError = VmDirIsAncestorDN( &(gVmdirServerGlobals.bvServicesRootDN), pBervDN, &bIsAllowAccess);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return bIsAllowAccess;

error:

    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "%s failed Access (%u), error (%d)",
                       __FUNCTION__, accessDesired, dwError);

    goto cleanup;
}

DWORD
VmDirSrvAccessCheck(
    PVDIR_OPERATION     pOperation,
    PVDIR_ACCESS_INFO   pAccessInfo,
    PVDIR_ENTRY         pEntry,
    ACCESS_MASK         accessDesired
    )
{
    DWORD       dwError = 0;
    ACCESS_MASK samGranted = 0;
    BOOLEAN     bIsAdminRole = FALSE;
    PSTR        pszAdminsGroupSid = NULL;
    BOOLEAN     bIsDCClient = FALSE;

    assert( pOperation );

    if (pOperation->conn == NULL || pOperation->opType != VDIR_OPERATION_TYPE_EXTERNAL)
    {
        goto cleanup; // Access Allowed
    }
    if (accessDesired == VMDIR_RIGHT_DS_READ_PROP && VmDirIsSpecialAllowedSearchEntry( pEntry ))
    {
        goto cleanup; // Access Allowed
    }

    // invalid accessDesired or
    // Anonymous user has only VMDIR_RIGHT_DS_READ_PROP access to "Special Allowed Search Entries" and NOTHING else.
    if ( !accessDesired
         ||
         pOperation->conn->bIsAnonymousBind)
    {
        dwError = VMDIR_ERROR_INSUFFICIENT_ACCESS;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    BAIL_ON_INVALID_ACCESSINFO(pAccessInfo, dwError);

    // Checks for System Admins group membership
    dwError = VmDirIsBindDnMemberOfSystemDomainAdmins(pOperation->pBECtx, pAccessInfo, &bIsAdminRole);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bIsAdminRole)
    {
        goto cleanup; // Access Allowed
    }

    // per PROD2013 requirements, member of domaincontrollergroup gets system admin rights.
    if ( gVmdirServerGlobals.bvDCGroupDN.lberbv_val )
    {
        dwError = VmDirIsDirectMemberOf( pAccessInfo->pszBindedDn,
                                         VDIR_ACCESS_DCGROUP_MEMBER_INFO,
                                         &pAccessInfo->accessRoleBitmap,
                                         &bIsAdminRole);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bIsAdminRole)
        {
            goto cleanup; // Access Allowed
        }
    }

    if ( gVmdirServerGlobals.bvDCClientGroupDN.lberbv_val )
    {
        dwError = VmDirIsDirectMemberOf( pAccessInfo->pszBindedDn,
                                         VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_INFO,
                                         &pAccessInfo->accessRoleBitmap,
                                         &bIsDCClient);
        BAIL_ON_VMDIR_ERROR(dwError);

        if(bIsDCClient && _VmDirDCClientGroupAccessCheck( pOperation, accessDesired ))
        {
            goto cleanup; // Access Allowed
        }
    }

    // Check Access Token in connection
    dwError = VmDirSrvAccessCheckEntry(pAccessInfo->pAccessToken, pEntry, accessDesired, &samGranted);
    if (!dwError)
    {
        if (samGranted != accessDesired)
        {
            dwError = VMDIR_ERROR_INSUFFICIENT_ACCESS;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        goto cleanup; // Access Allowed
    }
    else if (VMDIR_ERROR_INSUFFICIENT_ACCESS == dwError)
    {
        // Continue with more ACL check
        dwError = 0;
        samGranted = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    // Otherwise, continue (1) Check whether granted with SELF access right
    dwError = VmDirSrvAccessCheckSelf(pAccessInfo->pszNormBindedDn, pEntry, VMDIR_SELF_SID, accessDesired, &samGranted);
    if (!dwError)
    {
        if (samGranted != accessDesired)
        {
            dwError = VMDIR_ERROR_INSUFFICIENT_ACCESS;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        goto cleanup; // Access Allowed
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAdminsGroupSid);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDirSrvAccessCheckSelf(
    PSTR            pszNormBindedDn,
    PVDIR_ENTRY     pEntry,
    PSTR            pszWellKnowObjectSid,
    ACCESS_MASK     accessDesired,
    ACCESS_MASK *   psamGranted
    )
{
    DWORD           dwError = ERROR_SUCCESS;
    PACCESS_TOKEN   pWellKnownToken = NULL;

    if (IsNullOrEmptyString(pEntry->dn.bvnorm_val))
    {
        dwError = VmDirNormalizeDN( &(pEntry->dn), pEntry->pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // If not self, do not need recreate self token
    if (VmDirStringCompareA(pszWellKnowObjectSid, VMDIR_SELF_SID, TRUE) == 0 &&
        VmDirStringCompareA(pszNormBindedDn, BERVAL_NORM_VAL(pEntry->dn), TRUE) != 0)
    {
        dwError = VMDIR_ERROR_INSUFFICIENT_ACCESS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateAccessTokenForWellKnowObject(&pWellKnownToken, pszWellKnowObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvAccessCheckEntry(pWellKnownToken,
                                       pEntry,
                                       accessDesired,
                                       psamGranted);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pWellKnownToken)
    {
        VmDirReleaseAccessToken(&pWellKnownToken);
    }

    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDirSrvAccessCheckEntry(
    PACCESS_TOKEN   pToken,
    PVDIR_ENTRY     pEntry,
    ACCESS_MASK     accessDesired,
    ACCESS_MASK *   psamGranted
    )
{
    DWORD                           dwError = ERROR_SUCCESS;
    ACCESS_MASK                     AccessMask = 0;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pSecDescAbs = NULL;
    SECURITY_INFORMATION            SecInfoAll = (OWNER_SECURITY_INFORMATION |
                                                  GROUP_SECURITY_INFORMATION |
                                                  DACL_SECURITY_INFORMATION |
                                                  SACL_SECURITY_INFORMATION);

    BAIL_ON_VMDIR_INVALID_POINTER(pEntry, dwError);
    BAIL_ON_VMDIR_INVALID_POINTER(psamGranted, dwError);

    if (!pToken)
    {
        dwError = ERROR_NO_TOKEN;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pEntry->pAclCtx)
    {
        dwError = VmDirAllocateMemory(sizeof(*pEntry->pAclCtx), (PVOID*)&pEntry->pAclCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pEntry->pAclCtx->pSecurityDescriptor == NULL || pEntry->pAclCtx->ulSecDescLength == 0)
    {
        dwError = VmDirGetSecurityDescriptorForEntry(pEntry,
                                                     SecInfoAll,
                                                     &pEntry->pAclCtx->pSecurityDescriptor,
                                                     &pEntry->pAclCtx->ulSecDescLength);
        // In case of an internally constructed (non-persist) entry, bypass ACL check
        if (dwError == ERROR_NO_SECURITY_DESCRIPTOR)
        {
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirSrvAccessCheckEntr() No SD found for (%s)", pEntry->dn.lberbv.bv_val );

            // if (VmDirIsInternalEntry(pEntry))
            // {
                AccessMask = accessDesired;
                dwError = 0;

                goto cleanup;
            // }
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSecurityAclSelfRelativeToAbsoluteSD(&pSecDescAbs,
                                                       pEntry->pAclCtx->pSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Access check
    if (!VmDirAccessCheck(pSecDescAbs,
                          pToken,
                          accessDesired,
                          0,
                          &gVmDirEntryGenericMapping,
                          &AccessMask,
                          &dwError))
    {
        // VmDirAccessCheck return MS error space.  TODO, need a generic way to handle this.
        if ( dwError == ERROR_ACCESS_DENIED )
        {
            dwError = VMDIR_ERROR_INSUFFICIENT_ACCESS;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    *psamGranted = AccessMask;

    VmDirFreeAbsoluteSecurityDescriptor(&pSecDescAbs);

    return dwError;

error:
    AccessMask = 0;

    goto cleanup;
}

VOID
VmDirFreeAbsoluteSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppSecDesc
    )
{
    PSID                            pOwner = NULL;
    PSID                            pGroup = NULL;
    PACL                            pDacl = NULL;
    PACL                            pSacl = NULL;
    BOOLEAN                         bDefaulted = FALSE;
    BOOLEAN                         bPresent = FALSE;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pSecDesc = NULL;

    if (ppSecDesc == NULL || *ppSecDesc == NULL) {
        return;
    }

    pSecDesc = *ppSecDesc;

    VmDirGetOwnerSecurityDescriptor(pSecDesc, &pOwner, &bDefaulted);
    VmDirGetGroupSecurityDescriptor(pSecDesc, &pGroup, &bDefaulted);
    VmDirGetDaclSecurityDescriptor(pSecDesc, &bPresent, &pDacl, &bDefaulted);
    VmDirGetSaclSecurityDescriptor(pSecDesc, &bPresent, &pSacl, &bDefaulted);

    VMDIR_SAFE_FREE_MEMORY(pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(pOwner);
    VMDIR_SAFE_FREE_MEMORY(pGroup);
    VMDIR_SAFE_FREE_MEMORY(pDacl);
    VMDIR_SAFE_FREE_MEMORY(pSacl);

    *ppSecDesc = NULL;
}


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

    dwError = _VmDirBuildTokenGroups(pEntry, &pGroups);
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
    }
    *ppToken = pToken;

cleanup:
    if (!ppszObjectSid)
    {
        VMDIR_SAFE_FREE_MEMORY(pszObjectSid);
    }
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
    if (ppszObjectSid)
    {
        *ppszObjectSid = NULL;
    }

    *ppToken = NULL;

    if (pToken)
    {
        VmDirReleaseAccessToken(&pToken);
    }
    VMDIR_SAFE_FREE_MEMORY(pszObjectSid);

    goto cleanup;
}

DWORD
VmDirSrvCreateDefaultSecDescRel(
    PSTR                            pszSystemAdministratorDn,
    PSTR                            pszAdminsGroupSid,
    PSECURITY_DESCRIPTOR_RELATIVE*  ppSecDescRel,
    PULONG                          pulSecDescLength,
    PSECURITY_INFORMATION           pSecInfo
    )
{
    DWORD                           dwError = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pSecDescAbs = NULL;
    PACL                            pDacl = NULL;
    PSID                            pOwnerSid = NULL;
    PSID                            pGroupSid = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pSecDescRel = NULL;
    ULONG                           ulSecDescLen = 1024;
    SECURITY_INFORMATION            SecInfo = 0;

    // Owner: Administrators
    // Get administrator's PSID
    dwError = VmDirGetObjectSidFromDn(pszSystemAdministratorDn, &pOwnerSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE,
                                                (PVOID*)&pSecDescAbs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateSecurityDescriptorAbsolute(pSecDescAbs,
                                                SECURITY_DESCRIPTOR_REVISION);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetOwnerSecurityDescriptor(
                 pSecDescAbs,
                 pOwnerSid,
                 FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    SecInfo |= OWNER_SECURITY_INFORMATION;

    // BUILD-IN Group Administrators
    dwError = VmDirAllocateSidFromCString(pszAdminsGroupSid, &pGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetGroupSecurityDescriptor(
                 pSecDescAbs,
                 pGroupSid,
                 FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);
    pGroupSid = NULL;

    SecInfo |= GROUP_SECURITY_INFORMATION;

    // Do not set Sacl currently

    // DACL
    dwError = VmDirBuildDefaultDaclForEntry(pOwnerSid,
                                            pszAdminsGroupSid,
                                            &pDacl);
    BAIL_ON_VMDIR_ERROR(dwError);
    pOwnerSid = NULL;

    dwError = VmDirSetDaclSecurityDescriptor(pSecDescAbs,
                                             TRUE,
                                             pDacl,
                                             FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);
    pDacl = NULL;

    SecInfo |= DACL_SECURITY_INFORMATION;

    if (!VmDirValidSecurityDescriptor(pSecDescAbs))
    {
        dwError = ERROR_INVALID_SECURITY_DESCR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    do
    {
        VMDIR_SAFE_FREE_MEMORY(pSecDescRel);
        dwError = VmDirAllocateMemory(ulSecDescLen,
                                    (PVOID*)&pSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);

        memset(pSecDescRel, 0, ulSecDescLen);

        dwError = VmDirAbsoluteToSelfRelativeSD(pSecDescAbs,
                                               pSecDescRel,
                                               &ulSecDescLen);

        if (ERROR_INSUFFICIENT_BUFFER  == dwError)
        {
            ulSecDescLen *= 2;
        }
        else
        {
            BAIL_ON_VMDIR_ERROR(dwError);
        }

    }
    while((dwError != ERROR_SUCCESS) &&
          (ulSecDescLen <= SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE));

    if (ulSecDescLen > SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE)
    {
        dwError = ERROR_INVALID_SECURITY_DESCR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSecDescRel = pSecDescRel;
    *pulSecDescLength = ulSecDescLen;
    *pSecInfo = SecInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pDacl);
    VMDIR_SAFE_FREE_MEMORY(pOwnerSid);
    VMDIR_SAFE_FREE_MEMORY(pGroupSid);

    VmDirFreeAbsoluteSecurityDescriptor(&pSecDescAbs);

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pSecDescRel);
    ulSecDescLen = 0;
    SecInfo = 0;

    goto cleanup;
}

DWORD
VmDirGetObjectSidFromDn(
    PCSTR   pszObjectDn,
    PSID *  ppSid
    )
{
    DWORD       dwError = 0;
    PVDIR_ENTRY pEntry = NULL;

    dwError = VmDirSimpleDNToEntry(pszObjectDn, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetObjectSidFromEntry(pEntry, NULL, ppSid);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    return dwError;
}

DWORD
VmDirGetObjectSidFromEntry(
    PVDIR_ENTRY pEntry,
    PSTR *      ppszObjectSid, /* Optional */
    PSID *      ppSid /* Optional */
    )
{
    DWORD           dwError = 0;
    PVDIR_ATTRIBUTE pAttrObjectSid = NULL;

    pAttrObjectSid = VmDirEntryFindAttribute( ATTR_OBJECT_SID, pEntry );
    if (!pAttrObjectSid)
    {
        dwError = ERROR_NO_OBJECTSID_ATTR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ppszObjectSid)
    {
        dwError = VmDirAllocateStringA((PCSTR)pAttrObjectSid->vals[0].lberbv.bv_val, ppszObjectSid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ppSid)
    {
        dwError = VmDirAllocateSidFromCString((PCSTR)pAttrObjectSid->vals[0].lberbv.bv_val, ppSid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

VOID
VmDirAclCtxContentFree(
    PVDIR_ACL_CTX pAclCtx
    )
{
    if (pAclCtx)
    {
        VMDIR_SAFE_FREE_MEMORY(pAclCtx->pSecurityDescriptor);
    }
}

static
DWORD
_VmDirBuildTokenGroups(
    PVDIR_ENTRY     pEntry,
    PTOKEN_GROUPS * ppTokenGroups)
{
    DWORD               dwError = ERROR_SUCCESS;
    PVDIR_ATTRIBUTE     pMemberOfAttr = NULL;
    unsigned int        i = 0;
    PVDIR_ENTRY         pGroupEntry = NULL;
    VDIR_OPERATION      searchOp = {0};
    BOOLEAN             bHasTxn = FALSE;
    PTOKEN_GROUPS       pLocalTokenGroups = NULL;

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

    // SJ-TBD: Do we need to align the address??
    dwError = VmDirAllocateMemory( sizeof(TOKEN_GROUPS) +
                                   (sizeof(SID_AND_ATTRIBUTES) * (pMemberOfAttr ? pMemberOfAttr->numVals : 0)),
                                   (PVOID*)&pLocalTokenGroups );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pMemberOfAttr)
    {
        pLocalTokenGroups->GroupCount = pMemberOfAttr->numVals;

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

            dwError = VmDirGetObjectSidFromEntry(pGroupEntry, NULL, &pLocalTokenGroups->Groups[i].Sid);
            BAIL_ON_VMDIR_ERROR(dwError);

            // SJ-TBD: should be set on the basis of status of the group??
            pLocalTokenGroups->Groups[i].Attributes = SE_GROUP_ENABLED;

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

static
DWORD
VmDirBuildDefaultDaclForEntry(
    PSID    pOwnerSid, // system Administrator SID, at least in our context
    PSTR    pszAdminsGroupSid,
    PACL *  ppDacl
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwSizeDacl = 0;
    PSID    pBuiltInAdmins = NULL;
    PSID    pSelfSid = NULL;
    DWORD   dwSidCount = 0;
    PACL    pDacl = NULL;

    assert(pOwnerSid);
    dwSidCount++;

    dwError = VmDirAllocateSidFromCString(VMDIR_SELF_SID, &pSelfSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwSidCount++;

    dwError = VmDirAllocateSidFromCString(pszAdminsGroupSid, &pBuiltInAdmins);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwSidCount++;

    dwSizeDacl = ACL_HEADER_SIZE +
        dwSidCount * sizeof(ACCESS_ALLOWED_ACE) +
        VmDirLengthSid(pOwnerSid) +
        VmDirLengthSid(pSelfSid) +
        VmDirLengthSid(pBuiltInAdmins) -
        dwSidCount * sizeof(ULONG);

    dwError = VmDirAllocateMemory(dwSizeDacl, (PVOID*)&pDacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAcl(pDacl, dwSizeDacl, ACL_REVISION);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Note: This is a useful ACL which is REALLY used in RtlAccessCheck()
    dwError = VmDirAddAccessAllowedAceEx(pDacl,
                                         ACL_REVISION,
                                         0,
                                         VMDIR_ENTRY_ALL_ACCESS,
                                         pOwnerSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Note: This is really NOT a useful ACL because today group memberships are NOT set in the access token
    dwError = VmDirAddAccessAllowedAceEx(pDacl,
                                         ACL_REVISION,
                                         0,
                                         VMDIR_ENTRY_ALL_ACCESS,
                                         pBuiltInAdmins);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAddAccessAllowedAceEx(pDacl,
                                         ACL_REVISION,
                                         0,
                                         VMDIR_RIGHT_DS_READ_PROP |
                                         VMDIR_RIGHT_DS_WRITE_PROP,
                                         pSelfSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppDacl = pDacl;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pSelfSid);
    VMDIR_SAFE_FREE_MEMORY(pBuiltInAdmins);

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pDacl);

    goto cleanup;
}

static
DWORD
VmDirSrvCreateAccessTokenForWellKnowObject(
    PACCESS_TOKEN * ppToken,
    PSTR            pszWellknownObjectSid
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

/* Given a targetDN and the current bindedDN (the credentail represents the current user access)
 * return whether such bindedDN has adminRole to perform on the targetDN
 * pOperation is optional (to provide operation context if needed)
 *
 */
DWORD
VmDirSrvAccessCheckIsAdminRole(
   PVDIR_OPERATION      pOperation, /* Optional */
   PCSTR                pszNormTargetDN, /* Mandatory */
   PVDIR_ACCESS_INFO    pAccessInfo, /* Mandatory */
   PBOOLEAN             pbIsAdminRole
   )
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOLEAN bIsAdminRole = FALSE;

    if (pOperation->opType != VDIR_OPERATION_TYPE_EXTERNAL)
    {
        *pbIsAdminRole = TRUE;
        goto cleanup;
    }

    if ( pOperation->conn->bIsAnonymousBind ) // anonymous bind
    {
       *pbIsAdminRole = FALSE;
       goto cleanup;
    }

    if (IsNullOrEmptyString(pszNormTargetDN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    BAIL_ON_INVALID_ACCESSINFO(pAccessInfo, dwError);

    //Check whether bindedDN is member of build-in administrators group
    dwError = VmDirIsBindDnMemberOfSystemDomainAdmins(pOperation->pBECtx,
                                                pAccessInfo,
                                                &bIsAdminRole);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bIsAdminRole)
    {
        *pbIsAdminRole = TRUE;
        goto cleanup;
    }

    // per PROD2013 requirements, member of domaincontrollergroup gets system admin rights.
    if (gVmdirServerGlobals.bvDCGroupDN.lberbv_val == NULL)
    {
        *pbIsAdminRole = FALSE;
        goto cleanup;
    }

    dwError = VmDirIsDirectMemberOf( pAccessInfo->pszBindedDn,
                                     VDIR_ACCESS_DCGROUP_MEMBER_INFO,
                                     &pAccessInfo->accessRoleBitmap,
                                     &bIsAdminRole);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbIsAdminRole = bIsAdminRole;

cleanup:

    return dwError;

error:
    *pbIsAdminRole = FALSE;

    goto cleanup;
}

/* Check whether it is a valid accessInfo
 * (i.e.: resulted by doing a successful bind in an operation) */
BOOLEAN
VmDirIsFailedAccessInfo(
    PVDIR_ACCESS_INFO pAccessInfo
    )
{

    BOOLEAN     bIsFaliedAccessPermission = TRUE;


    if ( ! pAccessInfo->pAccessToken )
    {   // internal operation has NULL pAccessToken, yet we granted root privilege
        bIsFaliedAccessPermission = FALSE;
    }
    else
    {   // coming from LDAP protocol, we should have BIND information
        if ( ! IsNullOrEmptyString(pAccessInfo->pszBindedObjectSid)
             &&
             ! IsNullOrEmptyString(pAccessInfo->pszNormBindedDn)
             &&
             ! IsNullOrEmptyString(pAccessInfo->pszBindedDn)
           )
        {
            bIsFaliedAccessPermission = FALSE;
        }
    }

    return bIsFaliedAccessPermission;
}

/*
 * This APIs is called under the assumption that it is already checked
 * to make sure that bindedDn and searched Base Dn are in the same tenant scope
 *
 * Do an internalSearch against the domain object of 'pszBindedDn' using 'SUBTREE scope'
 * The filters is constructed as
 * '&(pszAttrFilterName1=pszAttrFilterVal1)(pszAttrFilterName2=pszAttrFilterVal2)'
 * return the found pEntry
 */
static
DWORD
VmDirIsBindDnMemberOfSystemDomainAdmins(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ACCESS_INFO   pAccessInfo,
    PBOOLEAN            pbIsMemberOfAdmins
    )
{
    DWORD               dwError = ERROR_SUCCESS;
    VDIR_OPERATION      SearchOp = {0};
    PVDIR_FILTER        pSearchFilter = NULL;
    BOOLEAN             bIsMemberOfAdmins = FALSE;
    PSTR                pszAdminsGroupSid = NULL;
    PVDIR_BACKEND_CTX   pSearchOpBECtx = NULL;
    PSTR                pszBindedDn = NULL;

    if (pAccessInfo->accessRoleBitmap & VDIR_ACCESS_ADMIN_MEMBER_VALID_INFO)
    {
        *pbIsMemberOfAdmins = (pAccessInfo->accessRoleBitmap & VDIR_ACCESS_IS_ADMIN_MEMBER) != 0;
        goto cleanup;
    }

    pszBindedDn = pAccessInfo->pszBindedDn;
    dwError = VmDirInitStackOperation( &SearchOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_SEARCH,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    // If there is pBECtx passing in, use explicit BE Context
    // Otherwise, establish a new implicit context
    if (pBECtx && pBECtx->pBEPrivate)
    {
        pSearchOpBECtx = SearchOp.pBECtx;
        SearchOp.pBECtx = pBECtx;
    }
    else
    {
        SearchOp.pBEIF = VmDirBackendSelect(SearchOp.reqDn.lberbv.bv_val);
        assert(SearchOp.pBEIF);
    }

    SearchOp.reqDn.lberbv.bv_val = (PSTR)gVmdirServerGlobals.systemDomainDN.lberbv.bv_val;
    SearchOp.reqDn.lberbv.bv_len = VmDirStringLenA(SearchOp.reqDn.lberbv.bv_val);
    SearchOp.request.searchReq.scope = LDAP_SCOPE_SUBTREE;

    dwError = VmDirGenerateWellknownSid(gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
                                        VMDIR_DOMAIN_ALIAS_RID_ADMINS,
                                        &pszAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConcatTwoFilters(SearchOp.pSchemaCtx,
                                    ATTR_OBJECT_SID,
                                    pszAdminsGroupSid,
                                    ATTR_MEMBER,
                                    pszBindedDn,
                                    &pSearchFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    SearchOp.request.searchReq.filter = pSearchFilter;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
          "VmDirIsBindDnMemberOfSystemDomainAdmins: internal search for dn %s", pszBindedDn);

    dwError = VmDirInternalSearch(&SearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (SearchOp.internalSearchEntryArray.iSize > 0)
    {
        bIsMemberOfAdmins = TRUE;
    }

    pAccessInfo->accessRoleBitmap |= VDIR_ACCESS_ADMIN_MEMBER_VALID_INFO;
    if (bIsMemberOfAdmins)
    {
        // Set VDIR_ACCESS_IS_ADMIN_MEMBER bit
        pAccessInfo->accessRoleBitmap |= VDIR_ACCESS_IS_ADMIN_MEMBER;
    }

    *pbIsMemberOfAdmins = bIsMemberOfAdmins;

cleanup:

    if ( pSearchOpBECtx )
    {
        SearchOp.pBECtx = pSearchOpBECtx;
    }

    VmDirFreeOperationContent(&SearchOp);
    VMDIR_SAFE_FREE_MEMORY(pszAdminsGroupSid);

    return dwError;

error:
    *pbIsMemberOfAdmins = FALSE;

    goto cleanup;
}

static
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

