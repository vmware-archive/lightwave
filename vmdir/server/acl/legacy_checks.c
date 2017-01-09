/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

//
// This module contains code that handles access checks for legacy data
// (upgrades). In 7.0 we changed the ACL model considerably so this code acts
// as a shim to handle old data that isn't properly ACL'ed. It works by
// detecting the old default security descriptor (which, realistically, no one
// ever mucked with) and doing the old access checks (e.g., manually preventing
// people from deleting the admin account) manually.
//


static
DWORD
_VmDirBuildLegacyDacl(
    PCSTR pszAdminUserSid,
    PCSTR pszAdminGroupSid,
    PACL *ppDacl
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwSizeDacl = 0;
    DWORD dwSidCount = 0;
    PACL pDacl = NULL;
    PSID pOwnerSid = NULL;
    PSID pBuiltInAdmins = NULL;
    PSID pSelfSid = NULL;
    ACCESS_MASK amAccess = VMDIR_LEGACY_ACE_ACCESS;

    assert(pszAdminUserSid != NULL);
    assert(pszAdminGroupSid != NULL);
    assert(ppDacl != NULL);

    dwError = VmDirAllocateSidFromCString(pszAdminUserSid, &pOwnerSid);
    dwSidCount++;

    dwError = VmDirAllocateSidFromCString(pszAdminGroupSid, &pBuiltInAdmins);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwSidCount++;

    dwError = VmDirAllocateSidFromCString(VMDIR_SELF_SID, &pSelfSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwSidCount++;

    dwSizeDacl = ACL_HEADER_SIZE +
        dwSidCount * sizeof(ACCESS_ALLOWED_ACE) +
        VmDirLengthSid(pOwnerSid) +
        VmDirLengthSid(pBuiltInAdmins) +
        VmDirLengthSid(pSelfSid) -
        dwSidCount * sizeof(ULONG);

    dwError = VmDirAllocateMemory(dwSizeDacl, (PVOID*)&pDacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAcl(pDacl, dwSizeDacl, ACL_REVISION);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAddAccessAllowedAceEx(pDacl,
                                         ACL_REVISION,
                                         0,
                                         amAccess,
                                         pOwnerSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAddAccessAllowedAceEx(pDacl,
                                         ACL_REVISION,
                                         0,
                                         amAccess,
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
    VMDIR_SAFE_FREE_MEMORY(pOwnerSid);
    VMDIR_SAFE_FREE_MEMORY(pBuiltInAdmins);
    VMDIR_SAFE_FREE_MEMORY(pSelfSid);

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pDacl);

    goto cleanup;
}

DWORD
VmDirSrvCreateLegacySecurityDescriptor(
    PCSTR pszDomainDn,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
    )
{
    DWORD dwError = 0;
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecDescAbs = NULL;
    PACL pDacl = NULL;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel = NULL;
    ULONG ulSecDescLen = 1024;
    SECURITY_INFORMATION SecInfo = 0;
    PSTR pszAdminGroupSid = NULL;
    PSTR pszAdminUserSid = NULL;

    dwError = VmDirGenerateWellknownSid(pszDomainDn,
                                        VMDIR_DOMAIN_USER_RID_ADMIN,
                                        &pszAdminUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDn,
                                        VMDIR_DOMAIN_ALIAS_RID_ADMINS,
                                        &pszAdminGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateSidFromCString(pszAdminGroupSid, &pGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateSidFromCString(pszAdminUserSid, &pOwnerSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE,
                (PVOID*)&pSecDescAbs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateSecurityDescriptorAbsolute(
                pSecDescAbs,
                SECURITY_DESCRIPTOR_REVISION);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetOwnerSecurityDescriptor(
                 pSecDescAbs,
                 pOwnerSid,
                 FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);
    pOwnerSid = NULL; // pSecDescAbs now owns this pointer.

    SecInfo |= OWNER_SECURITY_INFORMATION;

    // BUILD-IN Group Administrators
    dwError = VmDirAllocateSidFromCString(pszAdminGroupSid, &pGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetGroupSecurityDescriptor(
                 pSecDescAbs,
                 pGroupSid,
                 FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);
    pGroupSid = NULL; // pSecDescAbs now owns this pointer.

    SecInfo |= GROUP_SECURITY_INFORMATION;

    // DACL
    dwError = _VmDirBuildLegacyDacl(
                pszAdminUserSid,
                pszAdminGroupSid,
                &pDacl);
    BAIL_ON_VMDIR_ERROR(dwError);

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
        if (ERROR_INSUFFICIENT_BUFFER == dwError)
        {
            ulSecDescLen *= 2;
        }
        else
        {
            BAIL_ON_VMDIR_ERROR(dwError);
        }

    } while (dwError != ERROR_SUCCESS &&
             ulSecDescLen <= SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE);

    if (ulSecDescLen > SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE)
    {
        dwError = ERROR_INVALID_SECURITY_DESCR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pSecDesc->pSecDesc = pSecDescRel;
    pSecDesc->ulSecDesc = ulSecDescLen;
    pSecDesc->SecInfo = SecInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pDacl);
    VMDIR_SAFE_FREE_MEMORY(pOwnerSid);
    VMDIR_SAFE_FREE_MEMORY(pGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszAdminGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszAdminUserSid);

    VmDirFreeAbsoluteSecurityDescriptor(&pSecDescAbs);
    return dwError;
error:
    VMDIR_SAFE_FREE_MEMORY(pSecDescRel);
    goto cleanup;
}

static
BOOLEAN
_VmDirIsProtectedEntry(
    PVDIR_ENTRY pEntry
    )
{
    BOOLEAN bResult = FALSE;
    PCSTR pszDomainDn = NULL;
    PCSTR pszEntryDn = NULL;
    size_t domainDnLen = 0;
    size_t entryDnLen = 0;

    const CHAR szAdministrators[] = "cn=Administrators,cn=Builtin";
    const CHAR szCertGroup[] =      "cn=CAAdmins,cn=Builtin";
    const CHAR szDCAdminsGroup[] =  "cn=DCAdmins,cn=Builtin";
    const CHAR szUsersGroup[] =     "cn=Users,cn=Builtin";
    const CHAR szAdministrator[] =  "cn=Administrator,cn=Users";
    const CHAR szDCClientsGroup[] = "cn=DCClients,cn=Builtin";

    pszDomainDn = gVmdirServerGlobals.systemDomainDN.lberbv.bv_val;
    if (pszDomainDn == NULL)
    {
        goto error;
    }

    pszEntryDn = pEntry->dn.lberbv.bv_val;
    if (pszEntryDn == NULL)
    {
        goto error;
    }

    entryDnLen = strlen(pszEntryDn);
    domainDnLen = strlen(pszDomainDn);

    if (entryDnLen <= domainDnLen)
    {
        goto error;
    }

    if (pszEntryDn[(entryDnLen - domainDnLen) - 1] != ',')
    {
        goto error;
    }

    // Make sure system DN matches
    if (VmDirStringCompareA(&pszEntryDn[entryDnLen - domainDnLen], pszDomainDn, FALSE))
    {
        goto error;
    }

    if (!VmDirStringNCompareA(pszEntryDn, szAdministrators, sizeof(szAdministrators) - 1, FALSE) ||
        !VmDirStringNCompareA(pszEntryDn, szCertGroup, sizeof(szCertGroup) - 1, FALSE) ||
        !VmDirStringNCompareA(pszEntryDn, szDCAdminsGroup, sizeof(szDCAdminsGroup) - 1, FALSE) ||
        !VmDirStringNCompareA(pszEntryDn, szUsersGroup, sizeof(szUsersGroup) - 1, FALSE) ||
        !VmDirStringNCompareA(pszEntryDn, szDCClientsGroup, sizeof(szDCClientsGroup) - 1, FALSE) ||
        !VmDirStringNCompareA(pszEntryDn, szAdministrator, sizeof(szAdministrator) - 1, FALSE))
    {
        bResult = TRUE;
    }

cleanup:
    return bResult;
error:
    goto cleanup;
}

static
BOOLEAN
_VmDirIsInternalEntry(
    PVDIR_ENTRY pEntry
    )
{
    return pEntry->eId < ENTRY_ID_SEQ_INITIAL_VALUE;
}

/*
 * management node computer account
 * 1. allow read to domain tree
 * 2. allow write under service containers cn=services,SYSTEM_DOMAIN
 */
static
BOOLEAN
_VmDirDCClientGroupAccessCheck(
    PVDIR_OPERATION pOperation,
    ACCESS_MASK accessDesired
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsAllowAccess = FALSE;
    PVDIR_BERVALUE pBervDN = NULL;

    // grant read only request
    if (accessDesired & VMDIR_RIGHT_DS_READ_PROP)
    {
        bIsAllowAccess = TRUE;
        goto cleanup;
    }

    if (pOperation->reqCode == LDAP_REQ_ADD)
    {
        pBervDN = &(pOperation->request.addReq.pEntry->dn);
    }
    else if (pOperation->reqCode == LDAP_REQ_DELETE)
    {
        pBervDN = &(pOperation->request.deleteReq.dn);
    }
    else if (pOperation->reqCode == LDAP_REQ_MODIFY ||
             pOperation->reqCode == LDAP_REQ_MODRDN)
    {
        pBervDN = &(pOperation->request.modifyReq.dn);
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // for all other access request, target DN must be under service container
    dwError = VmDirIsAncestorDN(&(gVmdirServerGlobals.bvServicesRootDN), pBervDN, &bIsAllowAccess);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return bIsAllowAccess;

error:

    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "%s failed Access (%u), error (%d)",
                       __FUNCTION__, accessDesired, dwError);

    goto cleanup;
}

PCSTR
_VmDirGetEntryDnFromOperation(
    PVDIR_OPERATION pOperation
    )
{
    PVDIR_BERVALUE pBervDN = NULL;

    if (pOperation->reqCode == LDAP_REQ_ADD)
    {
        pBervDN = &(pOperation->request.addReq.pEntry->dn);
    }
    else if (pOperation->reqCode == LDAP_REQ_DELETE)
    {
        pBervDN = &(pOperation->request.deleteReq.dn);
    }
    else if (pOperation->reqCode == LDAP_REQ_MODIFY ||
             pOperation->reqCode == LDAP_REQ_MODRDN)
    {
        pBervDN = &(pOperation->request.modifyReq.dn);
    }
    else
    {
        return NULL;
    }

    return pBervDN->bvnorm_val;
}

VOID
_VmDirLogLegacyAccessFailure(
    PVDIR_OPERATION pOperation,
    PVDIR_ACCESS_INFO pAccessInfo,
    ACCESS_MASK accessDesired
    )
{
    VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "Failing legacy access check (user = %s) for entry %s (accessDesired = %d)\n", pAccessInfo->pszBindedDn, _VmDirGetEntryDnFromOperation(pOperation), accessDesired);
}


static
BOOLEAN
_VmDirAllowOperationBasedOnGroupMembership(
    PVDIR_OPERATION pOperation,
    PVDIR_ACCESS_INFO pAccessInfo,
    ACCESS_MASK accessDesired
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsMember = FALSE;

    dwError = VmDirIsBindDnMemberOfSystemDomainAdmins(pOperation->pBECtx, pAccessInfo, &bIsMember);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bIsMember)
    {
        goto cleanup; // Access Allowed
    }

    if (gVmdirServerGlobals.bvDCGroupDN.lberbv_val)
    {
        dwError = VmDirIsDirectMemberOf(pAccessInfo->pszBindedDn,
                                        VDIR_ACCESS_DCGROUP_MEMBER_INFO,
                                        &pAccessInfo->accessRoleBitmap,
                                        &bIsMember);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bIsMember)
        {
            goto cleanup; // Access Allowed
        }
    }

    if (gVmdirServerGlobals.bvDCClientGroupDN.lberbv_val)
    {
        dwError = VmDirIsDirectMemberOf(pAccessInfo->pszBindedDn,
                                        VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_INFO,
                                        &pAccessInfo->accessRoleBitmap,
                                        &bIsMember);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bIsMember && !_VmDirDCClientGroupAccessCheck(pOperation, accessDesired))
        {
            bIsMember = FALSE;
        }
    }

cleanup:
    return bIsMember;
error:
    _VmDirLogLegacyAccessFailure(pOperation, pAccessInfo, accessDesired);
    bIsMember = FALSE;
    goto cleanup;
}


BOOLEAN
_VmDirIsSchemaEntry(
    PVDIR_ENTRY pEntry
    )
{
    PCSTR pszDN = BERVAL_NORM_VAL(pEntry->dn);

    return VmDirStringEndsWith(pszDN, SCHEMA_NAMING_CONTEXT_DN, FALSE);
}

//
// This routine mimics the old security checks from 6.5 and prior releases:
// (1) Domain admins should have full access to the tree.
// (2) Domain clients should have RW access to the full tree and FULL access
//     to anything under "cn=services,dc=<domain>"
// (3) Anything under cn=schemacontext shouldn't be deletable.
// (4) Built-in/internal objects shouldn't be deletable.
DWORD
VmDirLegacyAccessCheck(
    PVDIR_OPERATION pOperation,
    PVDIR_ACCESS_INFO pAccessInfo,
    PVDIR_ENTRY pEntry,
    ACCESS_MASK accessDesired
    )
{
    DWORD dwError = 0;

    if (_VmDirAllowOperationBasedOnGroupMembership(pOperation, pAccessInfo, accessDesired))
    {
        goto cleanup;
    }

    if (accessDesired == VMDIR_RIGHT_DS_DELETE_CHILD)
    {
        if (_VmDirIsInternalEntry(pEntry) ||
            _VmDirIsProtectedEntry(pEntry) ||
            _VmDirIsSchemaEntry(pEntry))
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
_VmDirLogLegacySDCheck(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    ULONG ulSecDescLength,
    BOOLEAN bIsLegacySD
    )
{
    PSTR pszAclString = NULL;
    DWORD dwError = 0;

    dwError = LwNtStatusToWin32Error(RtlAllocateSddlCStringFromSecurityDescriptor(
                                        &pszAclString,
                                        pSecurityDescriptor,
                                        SDDL_REVISION_1,
                                        OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION));
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(
        VMDIR_LOG_MASK_ALL,
        "The SD %s (length %d) was determined to be a %s descriptor",
        pszAclString,
        ulSecDescLength,
        bIsLegacySD ? "legacy" : "non-legacy");

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszAclString);
    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Legacy logging failed with error %d", dwError);
    goto cleanup;
}

BOOLEAN
VmDirIsLegacySecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    ULONG ulSecDescLength
    )
{
    BOOLEAN bIsLegacySD = FALSE;

    //
    // New entries will all have a security descriptor but old entries might
    // not.
    //
    if (pSecurityDescriptor == NULL)
    {
        return TRUE;
    }

    if (ulSecDescLength == gVmdirServerGlobals.vsdLegacyDescriptor.ulSecDesc)
    {
        bIsLegacySD = (memcmp(
                                pSecurityDescriptor,
                                gVmdirServerGlobals.vsdLegacyDescriptor.pSecDesc,
                                ulSecDescLength) == 0);
    }

    _VmDirLogLegacySDCheck(pSecurityDescriptor, ulSecDescLength, bIsLegacySD);

    return bIsLegacySD;
}
