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
// detecting if this system has been upgraded and, if so, doing the old access
// checks manually (e.g., preventing people from deleting the admin account).
//

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

BOOLEAN
VmDirIsLegacySecurityDescriptor(
    VOID
    )
{
    return bLegacySecurityDescriptorsNeeded;
}
