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

int
VmDirGenerateAttrMetaData(
    PVDIR_ENTRY    pEntry,
    PSTR           pszAttributeName
    );


static
DWORD
_VmDirGenerateWellKnownBinarySid(
    DWORD dwWellKnownRid,
    PSID *ppSid
    )
{
    PSTR pszSid = NULL;
    DWORD dwError = 0;
    PSID pSid = NULL;

    dwError = VmDirGenerateWellknownSid(gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
                                        dwWellKnownRid,
                                        &pszSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateSidFromCString(pszSid, &pSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSid = pSid;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSid);
    return dwError;
error:
    goto cleanup;
}

static
DWORD
_VmDirSrvCreateAccessTokenForAdmin(
    PACCESS_TOKEN * ppToken
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PACCESS_TOKEN pToken = NULL;
    TOKEN_USER user = {{0}};
    TOKEN_OWNER owner = {0};
    PTOKEN_GROUPS pGroups = NULL;
    TOKEN_PRIVILEGES privileges = {0};
    TOKEN_PRIMARY_GROUP primaryGroup = {0};
    TOKEN_DEFAULT_DACL dacl = {0};
    PSTR pszBuiltinUsersGroupSid = NULL;

    dwError = _VmDirGenerateWellKnownBinarySid(
            VMDIR_DOMAIN_USER_RID_ADMIN,
            &user.User.Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    owner.Owner = user.User.Sid;

    dwError = VmDirAllocateMemory(
            sizeof(TOKEN_GROUPS) + sizeof(SID_AND_ATTRIBUTES),
            (PVOID*)&pGroups);
    BAIL_ON_VMDIR_ERROR(dwError);

    pGroups->GroupCount = 1;

    dwError = _VmDirGenerateWellKnownBinarySid(
            VMDIR_DOMAIN_ALIAS_RID_ADMINS,
            &pGroups->Groups[0].Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // SJ-TBD: should be set on the basis of status of the group??
    pGroups->Groups[0].Attributes = SE_GROUP_ENABLED;

    dwError = VmDirGenerateWellknownSid(
            gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
            VMDIR_DOMAIN_ALIAS_RID_USERS,
            &pszBuiltinUsersGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Primary groups should be built-in\Users not admins
    dwError = VmDirAllocateSidFromCString(
            pszBuiltinUsersGroupSid,
            &primaryGroup.PrimaryGroup);
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

    *ppToken = pToken;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pGroups->Groups[0].Sid);
    VMDIR_SAFE_FREE_MEMORY(pGroups);
    VMDIR_SAFE_FREE_MEMORY(user.User.Sid);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltinUsersGroupSid);
    VMDIR_SAFE_FREE_MEMORY(primaryGroup.PrimaryGroup);

    return dwError;

error:
    if (pToken)
    {
        VmDirReleaseAccessToken(&pToken);
    }

    goto cleanup;
}


//
// Takes an entry with a ATTR_ACL_STRING (which is an SDDL/text-based
// security descriptor) and convert that string into a binary security
// descriptor and then removes the ATTR_ACL_STRING attribute.
//
static
DWORD
_VmDirConvertAndRemoveSDDLAttribute(
    PVDIR_ENTRY pEntry,
    PVDIR_ATTRIBUTE pAclStringAttr,
    PSECURITY_DESCRIPTOR_RELATIVE *ppSecDesc,
    PULONG pulLength
    )
{
    DWORD dwError = 0;
    PSECURITY_DESCRIPTOR_RELATIVE pSecDesc = NULL;
    ULONG ulLength = 0;

    dwError = LwNtStatusToWin32Error(
                RtlAllocateSecurityDescriptorFromSddlCString(
                    &pSecDesc,
                    &ulLength,
                    pAclStringAttr->vals[0].lberbv.bv_val,
                    SDDL_REVISION_1));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryRemoveAttribute(pEntry, ATTR_ACL_STRING);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSecDesc = pSecDesc;
    *pulLength = ulLength;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
_VmDirGetSecurityDescriptorAttribute(
    PVDIR_ENTRY pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE *ppSecDesc,
    PULONG pulLength
    )
{
    PSECURITY_DESCRIPTOR_RELATIVE pSecDesc = NULL;
    ULONG ulLength = 0;
    PVDIR_ATTRIBUTE pSecDescAttr = NULL;
    PVDIR_ATTRIBUTE pAclStringAttr = NULL;
    DWORD dwError = 0;

    pAclStringAttr = VmDirEntryFindAttribute(ATTR_ACL_STRING, pEntry);

    //
    // If there's an ATTR_OBJECT_SECURITY_DESCRIPTOR in the request use it.
    // However, if there's also a ATTR_ACL_STRING return an error as both
    // shouldn't be specified.
    //
    pSecDescAttr = VmDirEntryFindAttribute(ATTR_OBJECT_SECURITY_DESCRIPTOR, pEntry);
    if (pSecDescAttr)
    {
        if (pAclStringAttr != NULL)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION);
        }

        pSecDesc = (PSECURITY_DESCRIPTOR_RELATIVE)pSecDescAttr->vals[0].lberbv.bv_val;
        ulLength = (ULONG)pSecDescAttr->vals[0].lberbv.bv_len;

        dwError = VmDirAllocateAndCopyMemory(pSecDesc, ulLength, (PVOID*)&pSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pAclStringAttr)
    {
        //
        // If there's an ATTR_ACL_STRING in the request convert it to a
        // security descriptor and use it.
        //
        dwError = _VmDirConvertAndRemoveSDDLAttribute(
                    pEntry,
                    pAclStringAttr,
                    &pSecDesc,
                    &ulLength);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSecDesc = pSecDesc;
    *pulLength = ulLength;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
_VmDirGetSchemaDefaultSecurityDescriptor(
    PVDIR_ENTRY                     pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE   pParentSecDesc,
    PSECURITY_DESCRIPTOR_RELATIVE*  ppSecDesc,
    PULONG                          pulLength
    )
{
    DWORD   dwError = 0;
    BOOLEAN bTmp = FALSE;
    ULONG   ulLength = 0;
    PCSTR   pszDomainSid = NULL;
    PSTR    pszClassDn = NULL;
    PSTR    pszDacl = NULL;
    PSTR    pszOwnerSid = NULL;
    PSTR    pszGroupSid = NULL;
    PSTR    pszSecDesc = NULL;
    PSID    pParentOwnerSid = NULL;
    PSID    pParentGroupSid = NULL;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;
    PVDIR_ENTRY     pOCEntry = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pParentSecDescAbs = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pSecDesc = NULL;

    dwError = VmDirSchemaGetEntryStructureOCDesc(pEntry, &pOCDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszClassDn,
            "cn=%s,cn=schemacontext",
            pOCDesc->pszName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleDNToEntry(pszClassDn, &pOCEntry);
    if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        //
        // This is being called for the class object itself, so there's
        // no backend object yet.
        //
        dwError = 0;
        goto cleanup;
    }

    pAttr = VmDirFindAttrByName(pOCEntry, ATTR_DEFAULT_SECURITY_DESCRIPTOR);
    if (pAttr)
    {
        pszDomainSid = VmDirFindDomainSid(pEntry->dn.lberbv.bv_val);
        if (IsNullOrEmptyString(pszDomainSid))
        {
            // special object such as schema, config, etc
            // no need to fill in domain SID template
            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv.bv_val,
                    &pszDacl);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirStringReplaceAll(
                    pAttr->vals[0].lberbv.bv_val,
                    VMDIR_NULL_SID_TEMPLATE,
                    pszDomainSid,
                    &pszDacl);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pParentSecDesc)
        {
            // take parent's owner and group
            dwError = VmDirSecurityAclSelfRelativeToAbsoluteSD(
                    &pParentSecDescAbs, pParentSecDesc);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirGetOwnerSecurityDescriptor(
                    pParentSecDescAbs, &pParentOwnerSid, &bTmp);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirGetGroupSecurityDescriptor(
                    pParentSecDescAbs, &pParentGroupSid, &bTmp);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateCStringFromSid(
                    &pszOwnerSid, pParentOwnerSid);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateCStringFromSid(
                    &pszGroupSid, pParentGroupSid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            // use system domain administrator as default owner
            // and system domain builtin admins group as default group
            dwError = VmDirGenerateWellknownSid(
                    gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
                    VMDIR_DOMAIN_USER_RID_ADMIN,
                    &pszOwnerSid);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirGenerateWellknownSid(
                    gVmdirServerGlobals.systemDomainDN.lberbv.bv_val,
                    VMDIR_DOMAIN_ALIAS_RID_ADMINS,
                    &pszGroupSid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateStringPrintf(
                &pszSecDesc,
                "O:%sG:%s%s",
                pszOwnerSid,
                pszGroupSid,
                pszDacl);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwNtStatusToWin32Error(
                RtlAllocateSecurityDescriptorFromSddlCString(
                        &pSecDesc,
                        &ulLength,
                        pszSecDesc,
                        SDDL_REVISION_1));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSecDesc = pSecDesc;

cleanup:
    VmDirFreeAbsoluteSecurityDescriptor(&pParentSecDescAbs);
    VMDIR_SAFE_FREE_STRINGA(pszClassDn);
    VMDIR_SAFE_FREE_STRINGA(pszDacl);
    VMDIR_SAFE_FREE_STRINGA(pszOwnerSid);
    VMDIR_SAFE_FREE_STRINGA(pszGroupSid);
    VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
    VmDirFreeEntry(pOCEntry);
    return dwError;

error:
    goto cleanup;
}

BOOLEAN _VmDirIsContainer(
    PVDIR_ENTRY pEntry
    )
{
    DWORD i = 0;
    BOOLEAN bContainer = FALSE;

    PVDIR_ATTRIBUTE pAttr = NULL;

    pAttr = VmDirFindAttrByName(pEntry, ATTR_OBJECT_CLASS);
    if (pAttr != NULL)
    {
        for (i = 0; i < pAttr->numVals; i++)
        {
            if (VmDirStringCompareA(pAttr->vals[i].lberbv_val, OC_CONTAINER, FALSE) == 0)
            {
                bContainer = TRUE;
                break;
            }
        }
    }

    return bContainer;
}

VOID
_VmDirLogSecurityDescriptor(
    PVDIR_ENTRY pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDesc
    )
{
    PSTR pszAclString = NULL;
    DWORD dwError = 0;

    dwError = LwNtStatusToWin32Error(
            RtlAllocateSddlCStringFromSecurityDescriptor(
                    &pszAclString,
                    pSecDesc,
                    SDDL_REVISION_1,
                    OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION));
    if (dwError == 0)
    {
        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "Calculated SD %s for entry %s",
                pszAclString,
                pEntry->dn.lberbv.bv_val);

        VMDIR_SAFE_FREE_STRINGA(pszAclString);
    }
}

/*
 * If an explicit security descriptor is specified (either via the ATTR_ACL_STRING
 * or ATTR_OBJECT_SECURITY_DESCRIPTOR attribute) then we'll use that. If one
 * isn't specified, then we'll use the defaultSecurityDescriptor from the
 * object's class's schema. If that doesn't exist then we'd normally use the SD
 * from the creator's access token (this is what AD does).
 *
 * Whatever DACL we get from the step above we then combine with any
 * inheritable ACEs from the parent.
 */
DWORD
VmDirComputeObjectSecurityDescriptor(
    PVDIR_ACCESS_INFO   pAccessInfo,
    PVDIR_ENTRY         pEntry,
    PVDIR_ENTRY         pParentEntry
    )
{
    DWORD   dwError = 0;
    ULONG   ulLength = 0;
    BOOLEAN bInternalEntry = FALSE;
    PACCESS_TOKEN   pAccessToken = NULL;
    PACCESS_TOKEN   pAdminAccessToken = NULL;
    PVDIR_ATTRIBUTE pObjectSdAttr = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pSecDesc = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pParentSecDesc = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pComputedSecDesc = NULL;

    SECURITY_INFORMATION    SecInfoAll =
            OWNER_SECURITY_INFORMATION |
            GROUP_SECURITY_INFORMATION |
            DACL_SECURITY_INFORMATION |
            SACL_SECURITY_INFORMATION;

    // get access token
    pAccessToken = pAccessInfo ? pAccessInfo->pAccessToken : NULL;

    // if not provided, this entry is being internally created
    bInternalEntry = pAccessToken ? FALSE : TRUE;

    // get user provided SD
    dwError = _VmDirGetSecurityDescriptorAttribute(
            pEntry, &pSecDesc, &ulLength);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get parent entry SD if parent exists
    if (pParentEntry)
    {
        dwError = VmDirGetSecurityDescriptorForEntry(
                pParentEntry, SecInfoAll, &pParentSecDesc, &ulLength);

        // parent entry may not have SD if it's also internally created
        if (bInternalEntry && dwError == VMDIR_ERROR_NO_SECURITY_DESCRIPTOR)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // get class default SD if user didn't provide SD
    if (!pSecDesc)
    {
        dwError = _VmDirGetSchemaDefaultSecurityDescriptor(
                pEntry, pParentSecDesc, &pSecDesc, &ulLength);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // check if it doens't have SD to set
    if (!pParentSecDesc && !pSecDesc)
    {
        // if internal entry, log warning and exit gracefully
        if (bInternalEntry)
        {
            VMDIR_LOG_WARNING(
                    LDAP_DEBUG_ACL,
                    "%s: entry (%s) has no SD to set ",
                    __FUNCTION__,
                    VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val));

            goto cleanup;
        }

        // otherwise, return appropriate error code
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_SECURITY_DESCRIPTOR);
    }

    // use admin access token for internal entry
    if (bInternalEntry)
    {
        dwError = _VmDirSrvCreateAccessTokenForAdmin(&pAdminAccessToken);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAccessToken = pAdminAccessToken;
    }

    // handle legacy SD
    if (VmDirIsLegacySecurityDescriptor())
    {
        if (pParentSecDesc && !pSecDesc)
        {
            dwError = VmDirAllocateMemory(ulLength, (PVOID*)&pComputedSecDesc);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirCopyMemory(
                    pComputedSecDesc, ulLength, pParentSecDesc, ulLength);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            pComputedSecDesc = pSecDesc;
            pSecDesc = NULL;
        }
    }
    // handle regular SD
    else
    {
        dwError = LwNtStatusToWin32Error(
                RtlCreatePrivateObjectSecurityEx(
                        pParentSecDesc,
                        pSecDesc,
                        &pComputedSecDesc,
                        &ulLength,
                        NULL,
                        _VmDirIsContainer(pEntry),
                        SEF_DACL_AUTO_INHERIT | SEF_DEFAULT_OWNER_FROM_PARENT | SEF_DEFAULT_GROUP_FROM_PARENT,
                        pAccessToken,
                        &gVmDirEntryGenericMapping));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // log final SD
    _VmDirLogSecurityDescriptor(pEntry, pComputedSecDesc);

    // add the final SD to the entry
    dwError = VmDirAttributeAllocate(
            ATTR_OBJECT_SECURITY_DESCRIPTOR,
            1,
            pEntry->pSchemaCtx,
            &pObjectSdAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    pObjectSdAttr->vals[0].lberbv.bv_val = (PSTR)pComputedSecDesc;
    pObjectSdAttr->vals[0].lberbv.bv_len = ulLength;
    pObjectSdAttr->vals[0].bOwnBvVal = TRUE;
    pComputedSecDesc = NULL;

    // add a terminating NULL as some code assumes that these values are
    // NULL-terminated, even though this value isn't a string
    dwError = VmDirReallocateMemoryWithInit(
            (PVOID)pObjectSdAttr->vals[0].lberbv.bv_val,
            (PVOID *)(&pObjectSdAttr->vals[0].lberbv.bv_val),
            pObjectSdAttr->vals[0].lberbv.bv_len+1,
            pObjectSdAttr->vals[0].lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddAttribute(pEntry, pObjectSdAttr);
    BAIL_ON_VMDIR_ERROR(dwError);
    pObjectSdAttr = NULL;

    dwError = VmDirGenerateAttrMetaData(
            pEntry, ATTR_OBJECT_SECURITY_DESCRIPTOR);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirReleaseAccessToken(&pAdminAccessToken);
    VMDIR_SAFE_FREE_MEMORY(pComputedSecDesc);
    VMDIR_SAFE_FREE_MEMORY(pParentSecDesc);
    VMDIR_SAFE_FREE_MEMORY(pSecDesc);
    VmDirFreeAttribute(pObjectSdAttr);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed for (%s), error code (%d)",
            __FUNCTION__,
            VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val),
            dwError);

    goto cleanup;
}
