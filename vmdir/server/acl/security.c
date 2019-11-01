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
VmDirGetSecurityDescriptorForDN(
    PCSTR                           pszDN,
    SECURITY_INFORMATION            SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE*  ppSecDesc,
    PULONG                          pulSecDescLength
    )
{
    DWORD	dwError = 0;
    PVDIR_ENTRY pEntry = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pCurSecDesc = NULL;
    ULONG   ulLength = 0;

    if (!pszDN || !ppSecDesc || !pulSecDescLength)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirSimpleDNToEntry(pszDN, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSecurityDescriptorForEntry(
            pEntry,
            SecurityInformation,
            &pCurSecDesc,
            &ulLength);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSecDesc = pCurSecDesc;
    *pulSecDescLength = ulLength;

cleanup:
    VmDirFreeEntry(pEntry);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pCurSecDesc);
    goto cleanup;
}

DWORD
VmDirGetSecurityDescriptorForEntry(
    PVDIR_ENTRY                     pEntry,
    SECURITY_INFORMATION            SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE*  ppSecDesc,
    PULONG                          pulSecDescLength
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
    PCSTR                       pszObjectDn,
    PVMDIR_SECURITY_DESCRIPTOR  pSecDesc
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVDIR_ENTRY pEntry = NULL;

    dwError = VmDirSimpleDNToEntry(pszObjectDn, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSecurityDescriptorForEntry(
            pEntry,
            pSecDesc->SecInfo,
            pSecDesc->pSecDesc,
            pSecDesc->ulSecDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntry(pEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

//
// Sets the security descriptor for object <pszObjectDn> and all objects
// below it (if any).
//
DWORD
VmDirSetRecursiveSecurityDescriptorForDn(
    PCSTR                       pszObjectDn,
    PVMDIR_SECURITY_DESCRIPTOR  pSecDesc
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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

// This function is only used internally to reset SD for a given entry
// during instance bootstrap
//
// normal SD should be set up during object 'ADD' or modified during
// object 'MOD' with correct permissions granted
DWORD
VmDirSetSecurityDescriptorForEntry(
    PVDIR_ENTRY                     pEntry,
    SECURITY_INFORMATION            securityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE   pSecDescRel,
    ULONG                           ulSecDescRel
    )
{
    DWORD   dwError = 0;
    ULONG   ulCurSecDescRel = 0;
    ULONG   ulTmpSecDescRel = 0;
    ULONG   ulNewSecDescRel = 0;
    PVDIR_ATTRIBUTE pCurSDAttr = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pCurSecDescRel = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pTmpSecDescRel = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pNewSecDescRel = NULL;

    if (!pEntry || !securityInformation)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (!VmDirValidRelativeSecurityDescriptor(
            pSecDescRel, ulSecDescRel, securityInformation))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_SECURITY_DESCR);
    }

    // Entry's SD is not cached yet
    pCurSDAttr = VmDirEntryFindAttribute(
            ATTR_OBJECT_SECURITY_DESCRIPTOR, pEntry);

    if (pCurSDAttr)
    {
        pCurSecDescRel = (PSECURITY_DESCRIPTOR_RELATIVE)pCurSDAttr->vals[0].lberbv.bv_val;
        ulCurSecDescRel = (ULONG)pCurSDAttr->vals[0].lberbv.bv_len;

        ulTmpSecDescRel = ulCurSecDescRel + ulSecDescRel;

        dwError = VmDirAllocateMemory(
                ulTmpSecDescRel + 1, (PVOID*)&pTmpSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);

        // note: this is going to replace existing SD
        dwError = VmDirSetSecurityDescriptorInfo(
                securityInformation,
                pSecDescRel,
                pCurSecDescRel,
                pTmpSecDescRel,
                &ulTmpSecDescRel,
                &gVmDirEntryGenericMapping);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNewSecDescRel = pTmpSecDescRel;
        ulNewSecDescRel = ulTmpSecDescRel;
    }
    else
    {
        pNewSecDescRel = pSecDescRel;
        ulNewSecDescRel = ulSecDescRel;
    }

    // Modify entry's SD
    dwError = VmDirInternalUpdateObjectSD(
            pEntry,
            pCurSDAttr ? TRUE : FALSE,
            pNewSecDescRel,
            ulNewSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Update pEntry SD cache
    dwError = VmDirEntryCacheSecurityDescriptor(
            pEntry, pNewSecDescRel, ulNewSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pTmpSecDescRel);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirAppendSecurityDescriptorForDn(
    PCSTR                       pszObjectDn,
    PVMDIR_SECURITY_DESCRIPTOR  pSecDesc,
    BOOLEAN                     bReplaceOwnerAndGroup
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVDIR_ENTRY pEntry = NULL;

    dwError = VmDirSimpleDNToEntry(pszObjectDn, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAppendSecurityDescriptorForEntry(
            pEntry,
            pSecDesc->SecInfo,
            pSecDesc->pSecDesc,
            pSecDesc->ulSecDesc,
            bReplaceOwnerAndGroup);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntry(pEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

// This function is only used internally to extend SD for a given entry
// during instance bootstrap
DWORD
VmDirAppendSecurityDescriptorForEntry(
    PVDIR_ENTRY                     pEntry,
    SECURITY_INFORMATION            securityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE   pSecDescRel,
    ULONG                           ulSecDescRel,
    BOOLEAN                         bReplaceOwnerAndGroup
    )
{
    DWORD   dwError = 0;
    BOOLEAN bTmp = FALSE;
    ULONG   ulTmpSecDescRel = 0;
    PACL    pInDacl = NULL;
    PACL    pCurDacl = NULL;
    PSID    pOwnerSid = NULL;
    PSID    pGroupSid = NULL;
    PACL    pNewDacl = NULL;
    PSID    pNewOwnerSid = NULL;
    PSID    pNewGroupSid = NULL;
    ULONG   ulNewSecDescRel = 0;
    PVDIR_ATTRIBUTE pCurSDAttr = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pInSecDescAbs = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pCurSecDescRel = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pCurSecDescAbs = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pTmpSecDescRel = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pNewSecDescRel = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pNewSecDescAbs = NULL;

    if (!pEntry || !securityInformation)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (!VmDirValidRelativeSecurityDescriptor(
            pSecDescRel, ulSecDescRel, securityInformation))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_SECURITY_DESCR);
    }

    // Entry's SD is not cached yet
    pCurSDAttr = VmDirEntryFindAttribute(
            ATTR_OBJECT_SECURITY_DESCRIPTOR, pEntry);

    if (pCurSDAttr)
    {
        pCurSecDescRel = (PSECURITY_DESCRIPTOR_RELATIVE)pCurSDAttr->vals[0].lberbv.bv_val;

        // convert input relative SD to absolute SD
        dwError = VmDirSecurityAclSelfRelativeToAbsoluteSD(
                &pInSecDescAbs, pSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetDaclSecurityDescriptor(
                pInSecDescAbs, &bTmp, &pInDacl, &bTmp);
        BAIL_ON_VMDIR_ERROR(dwError);

        // convert current relative SD to absolute SD
        dwError = VmDirSecurityAclSelfRelativeToAbsoluteSD(
                &pCurSecDescAbs, pCurSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetDaclSecurityDescriptor(
                pCurSecDescAbs, &bTmp, &pCurDacl, &bTmp);
        BAIL_ON_VMDIR_ERROR(dwError);

        // construct new absolute SD
        dwError = VmDirCreateSecurityDescriptorAbsolute(&pNewSecDescAbs);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bReplaceOwnerAndGroup)
        {
            dwError = VmDirGetOwnerSecurityDescriptor(
                    pInSecDescAbs, &pOwnerSid, &bTmp);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirGetGroupSecurityDescriptor(
                    pInSecDescAbs, &pGroupSid, &bTmp);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirGetOwnerSecurityDescriptor(
                    pCurSecDescAbs, &pOwnerSid, &bTmp);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirGetGroupSecurityDescriptor(
                    pCurSecDescAbs, &pGroupSid, &bTmp);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = RtlDuplicateSid(&pNewOwnerSid, pOwnerSid);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = RtlDuplicateSid(&pNewGroupSid, pGroupSid);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirMergeAces(pCurDacl, pInDacl, &pNewDacl);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSetOwnerSecurityDescriptor(
                pNewSecDescAbs, pNewOwnerSid, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);
        pNewOwnerSid = NULL;

        dwError = VmDirSetGroupSecurityDescriptor(
                pNewSecDescAbs, pNewGroupSid, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);
        pNewGroupSid = NULL;

        dwError = VmDirSetDaclSecurityDescriptor(
                pNewSecDescAbs, TRUE, pNewDacl, FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);
        pNewDacl = NULL;

        // convert new absolute SD to relative SD
        dwError = VmDirAbsoluteToSelfRelativeSD(
                pNewSecDescAbs, NULL, &ulTmpSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError != ERROR_INSUFFICIENT_BUFFER);

        dwError = VmDirAllocateMemory(
                ulTmpSecDescRel, (PVOID*)&pTmpSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAbsoluteToSelfRelativeSD(
                pNewSecDescAbs, pTmpSecDescRel, &ulTmpSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNewSecDescRel = pTmpSecDescRel;
        ulNewSecDescRel = ulTmpSecDescRel;
    }
    else
    {
        pNewSecDescRel = pSecDescRel;
        ulNewSecDescRel = ulSecDescRel;
    }

    // Modify entry's SD
    dwError = VmDirInternalUpdateObjectSD(
            pEntry,
            pCurSDAttr ? TRUE : FALSE,
            pNewSecDescRel,
            ulNewSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Update pEntry SD cache
    dwError = VmDirEntryCacheSecurityDescriptor(
            pEntry, pNewSecDescRel, ulNewSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeAbsoluteSecurityDescriptor(&pNewSecDescAbs);
    VmDirFreeAbsoluteSecurityDescriptor(&pCurSecDescAbs);
    VmDirFreeAbsoluteSecurityDescriptor(&pInSecDescAbs);
    VMDIR_SAFE_FREE_MEMORY(pTmpSecDescRel);
    VMDIR_SAFE_FREE_MEMORY(pNewOwnerSid);
    VMDIR_SAFE_FREE_MEMORY(pNewGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pNewDacl);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

// This function is only used internally to extend SD for a given entry
// during instance bootstrap
DWORD
VmDirAppendAllowAceForEntryEx(
    PVDIR_ENTRY pEntry,
    PCSTR       pszTrusteeDN,
    ULONG       aceFlags,
    ACCESS_MASK accessMask
    )
{
    DWORD   dwError = 0;
    BOOLEAN bTmp = FALSE;
    ULONG   ulTmpLen = 0;
    ULONG   ulNewDaclLen = 0;
    ULONG   ulNewSecDescLen = 0;
    PSID    pTrusteeSid = NULL;
    PACL    pCurDacl = NULL;
    PACL    pNewDacl = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pCurSecDescRel = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pNewSecDescAbs = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE   pNewSecDescRel = NULL;

    SECURITY_INFORMATION    SecInfoAll =
            OWNER_SECURITY_INFORMATION |
            GROUP_SECURITY_INFORMATION |
            DACL_SECURITY_INFORMATION |
            SACL_SECURITY_INFORMATION;

    if (!pEntry || IsNullOrEmptyString(pszTrusteeDN))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // get trustee SID
    dwError = VmDirGetObjectSidFromDn(pszTrusteeDN, &pTrusteeSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get current SD and DACL
    dwError = VmDirGetSecurityDescriptorForEntry(
            pEntry, SecInfoAll, &pCurSecDescRel, &ulTmpLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSecurityAclSelfRelativeToAbsoluteSD(
            &pNewSecDescAbs, pCurSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDaclSecurityDescriptor(
            pNewSecDescAbs, &bTmp, &pCurDacl, &bTmp);
    BAIL_ON_VMDIR_ERROR(dwError);

    // build new DACL and replace old DACL
    ulNewDaclLen = RtlGetAclSize(pCurDacl) +
            sizeof(ACCESS_ALLOWED_ACE) + VmDirLengthSid(pTrusteeSid);

    dwError = VmDirAllocateMemory(ulNewDaclLen, (PVOID*)&pNewDacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateAcl(pNewDacl, ulNewDaclLen, ACL_REVISION);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyAces(pCurDacl, pNewDacl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAddAccessAllowedAceEx(
            pNewDacl, ACL_REVISION, aceFlags, accessMask, pTrusteeSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetDaclSecurityDescriptor(
            pNewSecDescAbs, TRUE, pNewDacl, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_SAFE_FREE_MEMORY(pCurDacl);
    pNewDacl = NULL;

    // convert new absolute SD to relative SD
    dwError = VmDirAbsoluteToSelfRelativeSD(
            pNewSecDescAbs, NULL, &ulNewSecDescLen);
    BAIL_ON_VMDIR_ERROR(dwError != ERROR_INSUFFICIENT_BUFFER);

    dwError = VmDirAllocateMemory(
            ulNewSecDescLen, (PVOID*)&pNewSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAbsoluteToSelfRelativeSD(
            pNewSecDescAbs, pNewSecDescRel, &ulNewSecDescLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    // modify entry's SD
    dwError = VmDirInternalUpdateObjectSD(
            pEntry, TRUE, pNewSecDescRel, ulNewSecDescLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    // update pEntry SD cache
    dwError = VmDirEntryCacheSecurityDescriptor(
            pEntry, pNewSecDescRel, ulNewSecDescLen);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeAbsoluteSecurityDescriptor(&pNewSecDescAbs);
    VMDIR_SAFE_FREE_MEMORY(pNewSecDescRel);
    VMDIR_SAFE_FREE_MEMORY(pCurSecDescRel);
    VMDIR_SAFE_FREE_MEMORY(pTrusteeSid);
    VMDIR_SAFE_FREE_MEMORY(pNewDacl);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

// This function is only used internally to extend SD for a given entry
// during instance bootstrap
DWORD
VmDirAppendAllowAceForEntry(
    PVDIR_ENTRY pEntry,
    PCSTR       pszTrusteeDN,
    ACCESS_MASK accessMask
    )
{
    return VmDirAppendAllowAceForEntryEx(pEntry, pszTrusteeDN, 0, accessMask);
}

DWORD
VmDirAppendAllowAceForDn(
    PCSTR       pszObjectDn,
    PCSTR       pszTrusteeDN,
    ACCESS_MASK accessMask
    )
{
    return VmDirAppendAllowAceForDnEx(pszObjectDn, pszTrusteeDN, 0, accessMask);
}

DWORD
VmDirAppendAllowAceForDnEx(
    PCSTR       pszObjectDn,
    PCSTR       pszTrusteeDN,
    ULONG       aceFlags,
    ACCESS_MASK accessMask
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVDIR_ENTRY pEntry = NULL;

    dwError = VmDirSimpleDNToEntry(pszObjectDn, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAppendAllowAceForEntryEx(pEntry, pszTrusteeDN, aceFlags, accessMask);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntry(pEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirSetDefaultSecurityDescriptorForClass(
    PCSTR   pszClassName,
    PCSTR   pszDacl
    )
{
    DWORD   dwError = 0;
    PSTR    pszClassDN = NULL;
    VDIR_BERVALUE   berval = VDIR_BERVALUE_INIT;
    VDIR_OPERATION  ldapOp = {0};

    if (IsNullOrEmptyString(pszClassName) || IsNullOrEmptyString(pszDacl))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(
            &pszClassDN, "cn=%s,cn=schemacontext", pszClassName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation(
            &ldapOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    ldapOp.reqDn.lberbv_val = pszClassDN;
    ldapOp.reqDn.lberbv_len = VmDirStringLenA(pszClassDN);
    ldapOp.request.modifyReq.dn.lberbv_val = ldapOp.reqDn.lberbv_val;
    ldapOp.request.modifyReq.dn.lberbv_len = ldapOp.reqDn.lberbv_len;

    dwError = VmDirStringToBervalContent(pszDacl, &berval);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirOperationAddModReq(
            &ldapOp, MOD_OP_ADD, ATTR_DEFAULT_SECURITY_DESCRIPTOR, &berval, 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalModifyEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszClassDN);
    VmDirFreeOperationContent(&ldapOp);
    VmDirFreeBervalContent(&berval);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed for (%s), error code (%d)",
            __FUNCTION__,
            VDIR_SAFE_STRING(pszClassName),
            dwError);

    goto cleanup;
}

DWORD
VmDirSecurityAclSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE*  ppAbsolute,
    PSECURITY_DESCRIPTOR_RELATIVE   pRelative
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    ULONG   ulSecDescAbsSize = 0;
    ULONG   ulOwnerSize = 0;
    ULONG   ulGroupSize = 0;
    ULONG   ulDaclSize = 0;
    ULONG   ulSaclSize = 0;
    PSID    pOwnerSid = NULL;
    PSID    pGroupSid = NULL;
    PACL    pDacl = NULL;
    PACL    pSacl = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE   pAbsolute = NULL;

    /* Get the necessary sizes */
    dwError = VmDirSelfRelativeToAbsoluteSD(
            pRelative,
            pAbsolute, &ulSecDescAbsSize,
            pDacl, &ulDaclSize,
            pSacl, &ulSaclSize,
            pOwnerSid, &ulOwnerSize,
            pGroupSid, &ulGroupSize);
    BAIL_ON_VMDIR_ERROR(dwError != ERROR_INSUFFICIENT_BUFFER);

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
            pAbsolute, &ulSecDescAbsSize,
            pDacl, &ulDaclSize,
            pSacl, &ulSaclSize,
            pOwnerSid, &ulOwnerSize,
            pGroupSid, &ulGroupSize);
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
