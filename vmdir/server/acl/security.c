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

// Grab SD information from back-end
static
DWORD
VmDirReadSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescBuffer,
    PULONG pulsecDescLength
    );

static
DWORD
VmDirInternalUpdateObjectSD(
    PVDIR_ENTRY pEntry,
    BOOLEAN bIsReplace,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel,
    ULONG ulSecDescRel
    );

DWORD
VmDirGetSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE* ppSecDesc,
    PULONG pulSecDescLength
    )
{
    DWORD dwError = ERROR_SUCCESS;
    BYTE secDescBufferStatic[VMDIR_DEFAULT_SD_RELATIVE_SIZE] = { 0 };
    PBYTE pSecDescBuffer = secDescBufferStatic;
    ULONG ulsecDescLength = VMDIR_DEFAULT_SD_RELATIVE_SIZE;
    ULONG ulOldSecDescLength = ulsecDescLength;
    SECURITY_INFORMATION SecInfoAll = (OWNER_SECURITY_INFORMATION |
                                       GROUP_SECURITY_INFORMATION |
                                       DACL_SECURITY_INFORMATION |
                                       SACL_SECURITY_INFORMATION);
    PBYTE pOutputSecDescBuffer = NULL;
    ULONG ulOutputSecDescBuffer = 0;

    BAIL_ON_VMDIR_INVALID_POINTER(pEntry, dwError);

    if (SecurityInformation == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    do
    {
        if (dwError == ERROR_INSUFFICIENT_BUFFER)
        {
            if (pSecDescBuffer == secDescBufferStatic)
            {
                // Have to move to allocating the buffer since
                // static buffer was not large enough)
                pSecDescBuffer = NULL;
            }

            ulOldSecDescLength = ulsecDescLength;
            ulsecDescLength = VMDIR_MIN(ulsecDescLength*2, SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE);

            dwError = VmDirReallocateMemoryWithInit((PVOID)pSecDescBuffer,
                                                    (PVOID *)(&pSecDescBuffer),
                                                    ulsecDescLength,
                                                    ulOldSecDescLength);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirReadSecurityDescriptorForEntry(pEntry,
                                                      (PSECURITY_DESCRIPTOR_RELATIVE)pSecDescBuffer,
                                                      &ulsecDescLength);
    } while ((dwError == ERROR_INSUFFICIENT_BUFFER) &&
             (ulsecDescLength < SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE));
    BAIL_ON_VMDIR_ERROR(dwError);

    /* If the caller wants the complete Security Descriptor, just copy
       the buffer */

    dwError = VmDirAllocateMemory(ulsecDescLength, (PVOID*)&pOutputSecDescBuffer);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (SecurityInformation == SecInfoAll)
    {
        dwError = VmDirCopyMemory(
            pOutputSecDescBuffer, ulsecDescLength,
            pSecDescBuffer, ulsecDescLength );
        BAIL_ON_VMDIR_ERROR(dwError);

        ulOutputSecDescBuffer = ulsecDescLength;
    }
    else
    {
        dwError = VmDirQuerySecurityDescriptorInfo(SecurityInformation,
                                                   (PSECURITY_DESCRIPTOR_RELATIVE)pSecDescBuffer,
                                                   (PSECURITY_DESCRIPTOR_RELATIVE)pOutputSecDescBuffer,
                                                   &ulOutputSecDescBuffer);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    if (pSecDescBuffer != secDescBufferStatic)
    {
        VMDIR_SAFE_FREE_MEMORY(pSecDescBuffer);
    }

    if (dwError != ERROR_SUCCESS)
    {
        VMDIR_SAFE_FREE_MEMORY(pOutputSecDescBuffer);
        pOutputSecDescBuffer = NULL;
        ulOutputSecDescBuffer = 0;
    }

    *ppSecDesc = (PSECURITY_DESCRIPTOR_RELATIVE)pOutputSecDescBuffer;
    *pulSecDescLength = ulOutputSecDescBuffer;

    return dwError;
}

DWORD
VmDirSetSecurityDescriptorForDn(
    PCSTR pszObjectDn,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVDIR_ENTRY pEntry = NULL;

    dwError = VmDirSimpleDNToEntry(pszObjectDn, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSecurityDescriptorForEntry(pEntry,
                                                 pSecDesc->SecInfo,
                                                 pSecDesc->pSecDesc,
                                                 pSecDesc->ulSecDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntry(pEntry);

    return dwError;

error:
    goto cleanup;
}

//
// Sets the security descriptor for object <pszObjectDn> and all objects
// below it (if any).
//
DWORD
VmDirSetRecursiveSecurityDescriptorForDn(
    PCSTR pszObjectDn,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
    )
{
    DWORD dwError = 0;
    VDIR_ENTRY_ARRAY entryArray = {0};
    int iCnt = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    dwError = VmDirFilterInternalSearch(pszObjectDn,
                                        LDAP_SCOPE_SUBTREE,
                                        "objectClass=*",
                                        0,
                                        NULL,
                                        &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendSelect(NULL);
    dwError = pBE->pfnBEConfigureFsync(FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < entryArray.iSize; iCnt++)
    {
        dwError = VmDirSetSecurityDescriptorForEntry(
                    &entryArray.pEntry[iCnt],
                    pSecDesc->SecInfo,
                    pSecDesc->pSecDesc,
                    pSecDesc->ulSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    dwError = pBE->pfnBEConfigureFsync(TRUE);
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;
error:
    goto cleanup;
}

// This function is only used internally to add SD for a given entry during
// instance bootstrap
// normal SD should be set up during object 'ADD' or modified during object 'MOD'
// with correct permissions granted
DWORD
VmDirSetSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel,
    ULONG ulSecDescRel
    )
{
    DWORD dwError = VMDIR_ERROR_INSUFFICIENT_ACCESS;
    PSECURITY_DESCRIPTOR_RELATIVE pNewSecDescRel = NULL;
    ULONG ulNewSecDescLen = 0;
    // Do not free
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRelToSet = NULL;
    ULONG ulSecDescToSetLen = 0;
    PVDIR_ATTRIBUTE pObjectSdExist = NULL;


    /* Sanity checks */
    if (SecurityInformation == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!VmDirValidRelativeSecurityDescriptor(pSecDescRel, ulSecDescRel, SecurityInformation))
    {
        dwError = ERROR_INVALID_SECURITY_DESCR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Entry's SD is not cached yet
    pObjectSdExist = VmDirEntryFindAttribute(
                                     ATTR_OBJECT_SECURITY_DESCRIPTOR,
                                     pEntry);

    if (pObjectSdExist)
    {
        if (
             ( pObjectSdExist->vals[0].lberbv.bv_len < 0 )
             ||
             (pObjectSdExist->vals[0].lberbv.bv_len > ULONG_MAX)
           )
        {
            dwError = ERROR_INVALID_SECURITY_DESCR;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        ulNewSecDescLen = (ULONG)pObjectSdExist->vals[0].lberbv.bv_len + ulSecDescRel;

        dwError = VmDirAllocateMemory(ulNewSecDescLen+1, (PVOID*)&pNewSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSetSecurityDescriptorInfo(
                      SecurityInformation,
                      pSecDescRel,
                      (PSECURITY_DESCRIPTOR_RELATIVE)pObjectSdExist->vals[0].lberbv.bv_val,
                      pNewSecDescRel,
                      &ulNewSecDescLen,
                      &gVmDirEntryGenericMapping);
        BAIL_ON_VMDIR_ERROR(dwError);

        pSecDescRelToSet = pNewSecDescRel;
        ulSecDescToSetLen = ulNewSecDescLen;
    }
    else
    {
        pSecDescRelToSet = pSecDescRel;
        ulSecDescToSetLen = ulSecDescRel;
    }

    // Modify entry's SD
    dwError = VmDirInternalUpdateObjectSD(pEntry,
                                          pObjectSdExist?TRUE:FALSE,
                                          pSecDescRelToSet,
                                          ulSecDescToSetLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Update pEntry SD cache
    dwError = VmDirEntryCacheSecurityDescriptor(pEntry, pSecDescRelToSet, ulSecDescToSetLen);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    VMDIR_SAFE_FREE_MEMORY(pNewSecDescRel);

    return dwError;
}

DWORD
VmDirSecurityAclSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppAbsolute,
    PSECURITY_DESCRIPTOR_RELATIVE pRelative
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR_ABSOLUTE pAbsolute = NULL;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    ULONG ulSecDescAbsSize = 0;
    ULONG ulOwnerSize = 0;
    ULONG ulGroupSize = 0;
    ULONG ulDaclSize = 0;
    ULONG ulSaclSize = 0;

    /* Get the necessary sizes */

    dwError = VmDirSelfRelativeToAbsoluteSD(
                 pRelative,
                 pAbsolute,
                 &ulSecDescAbsSize,
                 pDacl,
                 &ulDaclSize,
                 pSacl,
                 &ulSaclSize,
                 pOwnerSid,
                 &ulOwnerSize,
                 pGroupSid,
                 &ulGroupSize);
    if (dwError != ERROR_INSUFFICIENT_BUFFER)
    {
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCreateSecurityDescriptorAbsolute(&pAbsolute);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ulDaclSize)
    {
        dwError = VmDirAllocateMemory(ulDaclSize, (PVOID*)&pDacl);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ulSaclSize)
    {
        dwError = VmDirAllocateMemory(ulSaclSize, (PVOID*)&pSacl);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ulOwnerSize)
    {
        dwError = VmDirAllocateMemory(ulOwnerSize, (PVOID*)&pOwnerSid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ulGroupSize)
    {
        dwError = VmDirAllocateMemory(ulGroupSize, (PVOID*)&pGroupSid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSelfRelativeToAbsoluteSD(
                 pRelative,
                 pAbsolute,
                 &ulSecDescAbsSize,
                 pDacl,
                 &ulDaclSize,
                 pSacl,
                 &ulSaclSize,
                 pOwnerSid,
                 &ulOwnerSize,
                 pGroupSid,
                 &ulGroupSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAbsolute = pAbsolute;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pOwnerSid);
    VMDIR_SAFE_FREE_MEMORY(pGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pDacl);
    VMDIR_SAFE_FREE_MEMORY(pSacl);
    VMDIR_SAFE_FREE_MEMORY(pAbsolute);

    goto cleanup;
}

// Grab SD information from back-end
static
DWORD
VmDirReadSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescBuffer,
    PULONG pulsecDescLength
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVDIR_ATTRIBUTE pObjectSdAttr = NULL;
    ULONG ulsecDescLength = *pulsecDescLength;

    pObjectSdAttr = VmDirEntryFindAttribute(
                             ATTR_OBJECT_SECURITY_DESCRIPTOR,
                             pEntry);
    if (!pObjectSdAttr)
    {
        dwError = ERROR_NO_SECURITY_DESCRIPTOR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (
        ( pObjectSdAttr->vals[0].lberbv.bv_len < 0 )
        ||
        (pObjectSdAttr->vals[0].lberbv.bv_len > ULONG_MAX)
    )
    {
        pObjectSdAttr = NULL;
        dwError = ERROR_INVALID_SECURITY_DESCR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ulsecDescLength < pObjectSdAttr->vals[0].lberbv.bv_len)
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCopyMemory(
        pSecDescBuffer, ulsecDescLength,
        pObjectSdAttr->vals[0].lberbv.bv_val, pObjectSdAttr->vals[0].lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR(dwError);

    ulsecDescLength = (ULONG)pObjectSdAttr->vals[0].lberbv.bv_len;

error:
    *pulsecDescLength = pObjectSdAttr ? (ULONG)pObjectSdAttr->vals[0].lberbv.bv_len : 0;

    return dwError;
}

static
DWORD
VmDirInternalUpdateObjectSD(
    PVDIR_ENTRY pEntry,
    BOOLEAN bIsReplace,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel,
    ULONG ulSecDescRel
    )
{
    DWORD dwError = 0;
    VDIR_OPERATION ObjectSdOp = {0};
    // Do not free reference to mods in ObjectSdOp
    VDIR_MODIFICATION* pMod = NULL;
    // Do not free
    PSTR pszDn = BERVAL_NORM_VAL(pEntry->dn);;

    dwError = VmDirInitStackOperation( &ObjectSdOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_MODIFY,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    ObjectSdOp.pBEIF = VmDirBackendSelect(ObjectSdOp.reqDn.lberbv.bv_val);
    assert(ObjectSdOp.pBEIF);

    dwError = VmDirAllocateMemory(sizeof(*pMod)*1,
                                 (PVOID*)&ObjectSdOp.request.modifyReq.mods);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMod = ObjectSdOp.request.modifyReq.mods;

    // Prepare mod
    dwError = VmDirAttributeInitialize(ATTR_OBJECT_SECURITY_DESCRIPTOR,
                                  1,
                                  ObjectSdOp.pSchemaCtx,
                                  &pMod->attr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(ulSecDescRel+1, (PVOID*)&pMod->attr.vals[0].lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(
        (PVOID)pMod->attr.vals[0].lberbv.bv_val, ulSecDescRel+1,
        (PVOID)pSecDescRel, ulSecDescRel );
    BAIL_ON_VMDIR_ERROR(dwError);

    pMod->attr.vals[0].bOwnBvVal = TRUE;
    pMod->attr.vals[0].lberbv.bv_len = ulSecDescRel;
    pMod->next = NULL;
    pMod->operation = bIsReplace? MOD_OP_REPLACE : MOD_OP_ADD;

    ObjectSdOp.reqDn.lberbv.bv_val = pszDn;
    ObjectSdOp.reqDn.lberbv.bv_len = VmDirStringLenA(pszDn);
    // Prepare pOrgConfigOp->request.modifyReq
    ObjectSdOp.request.modifyReq.dn.lberbv.bv_val = pszDn;
    ObjectSdOp.request.modifyReq.dn.lberbv.bv_len = VmDirStringLenA(pszDn);
    ObjectSdOp.request.modifyReq.numMods = 1;

    dwError = VmDirInternalModifyEntry(&ObjectSdOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeOperationContent(&ObjectSdOp);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirEntryCacheSecurityDescriptor(
    PVDIR_ENTRY pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRelToSet,
    ULONG ulSecDescToSetLen
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (!pEntry->pAclCtx)
    {
        dwError = VmDirAllocateMemory(sizeof(*pEntry->pAclCtx), (PVOID*)&pEntry->pAclCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        VmDirAclCtxContentFree(pEntry->pAclCtx);
    }

    dwError = VmDirAllocateMemory(ulSecDescToSetLen, (PVOID*)&pEntry->pAclCtx->pSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(
                pEntry->pAclCtx->pSecurityDescriptor,
                ulSecDescToSetLen,
                pSecDescRelToSet,
                ulSecDescToSetLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry->pAclCtx->ulSecDescLength = ulSecDescToSetLen;

cleanup:
    return dwError;
error:
    VmDirAclCtxContentFree(pEntry->pAclCtx);
    VMDIR_SAFE_FREE_MEMORY(pEntry->pAclCtx);
    goto cleanup;
}
