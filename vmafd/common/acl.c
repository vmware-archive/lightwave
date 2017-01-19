/*
 * Copyright © 016 VMware, Inc.  All Rights Reserved.
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
VmAfdAllocateSidFromUid(
    uid_t uid,
    PSID *ppSid);

static
DWORD
VmAfdAllocateSidFromGid(
    gid_t gid,
    PSID *ppSid);

static
VOID
VmAfdFreeTokenGroups(
    PTOKEN_GROUPS pTokenGroups);

static
DWORD
VmAfdBuildTokenGroups(
    gid_t *pGidList,
    DWORD dwGidListLen,
    PTOKEN_GROUPS *ppTokenGroups);

static
DWORD
VmAfdAllocateSidFromCString(
    PSID *pSid,
    PSTR pszSid);

static
DWORD
VmAfdCreateAccessToken(
    PACCESS_TOKEN *         AccessToken,
    PTOKEN_USER             User,
    PTOKEN_GROUPS           Groups,
    PTOKEN_PRIVILEGES       Privileges,
    PTOKEN_OWNER            Owner,
    PTOKEN_PRIMARY_GROUP    PrimaryGroup,
    PTOKEN_DEFAULT_DACL     DefaultDacl);

static
DWORD
VmAfdGetGroupList(
    uid_t userId,
    gid_t *pPrimaryGid,
    gid_t **ppGidList,
    DWORD *pdwGidListLen);

DWORD
VmAfdCreateAccessTokenFromSecurityContext(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PACCESS_TOKEN *ppAccessToken)
{
    DWORD dwError = 0;
    PACCESS_TOKEN pAccessToken = NULL;
    gid_t gid = 0;
    gid_t *pGidList = NULL;
    int iGidListLen = 0;
    TOKEN_USER user = {{0}};
    TOKEN_OWNER owner = {0};
    PTOKEN_GROUPS pGroups = NULL;
    TOKEN_PRIVILEGES privileges = {0};
    TOKEN_PRIMARY_GROUP primaryGroup = {0};
    TOKEN_DEFAULT_DACL dacl = {0};

    dwError = VmAfdAllocateSidFromUid(
                  pSecurityContext->uid,
                  &user.User.Sid);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetGroupList(
                  pSecurityContext->uid,
                  &gid,
                  &pGidList,
                  &iGidListLen);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateSidFromGid(
                  gid,
                  &primaryGroup.PrimaryGroup);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdBuildTokenGroups(
                   pGidList,
                   iGidListLen,
                   &pGroups);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCreateAccessToken(
                   &pAccessToken,
                   &user,
                   pGroups,
                   &privileges,
                   &owner,
                   &primaryGroup,
                   &dacl);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppAccessToken = pAccessToken;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pGidList);
    VMAFD_SAFE_FREE_MEMORY(user.User.Sid);
    VMAFD_SAFE_FREE_MEMORY(primaryGroup.PrimaryGroup);
    VmAfdFreeTokenGroups(pGroups);
    return dwError;

error:
    if (pAccessToken)
    {
        VmAfdReleaseAccessToken(&pAccessToken);
    }
    goto cleanup;
}

DWORD
VmAfdSecurityAclSelfRelativeToAbsoluteSD(
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

    dwError = VmAfdSelfRelativeToAbsoluteSD(
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                  SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE,
                  (PVOID*)&pAbsolute);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCreateSecurityDescriptorAbsolute(
                  pAbsolute,
                  SECURITY_DESCRIPTOR_REVISION);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ulDaclSize)
    {
        dwError = VmAfdAllocateMemory(ulDaclSize, (PVOID*)&pDacl);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (ulSaclSize)
    {
        dwError = VmAfdAllocateMemory(ulSaclSize, (PVOID*)&pSacl);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (ulOwnerSize)
    {
        dwError = VmAfdAllocateMemory(ulOwnerSize, (PVOID*)&pOwnerSid);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (ulGroupSize)
    {
        dwError = VmAfdAllocateMemory(ulGroupSize, (PVOID*)&pGroupSid);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSelfRelativeToAbsoluteSD(
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
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppAbsolute = pAbsolute;

cleanup:
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pOwnerSid);
    VMAFD_SAFE_FREE_MEMORY(pGroupSid);
    VMAFD_SAFE_FREE_MEMORY(pDacl);
    VMAFD_SAFE_FREE_MEMORY(pSacl);
    VMAFD_SAFE_FREE_MEMORY(pAbsolute);

    goto cleanup;
}

DWORD
VmAfdAllocateSecurityDescriptorFromSddlCString(
    PSECURITY_DESCRIPTOR_RELATIVE *ppSDRel,
    DWORD *pdwSDRelSize,
    PCSTR pszSddlAcl)
{
    DWORD dwError = 0;
    NTSTATUS ntStatus = 0;

    ntStatus = RtlAllocateSecurityDescriptorFromSddlCString(
                  ppSDRel,
                  pdwSDRelSize,
                  pszSddlAcl,
                  SDDL_REVISION_1);
    dwError = LwNtStatusToWin32Error(ntStatus);

    return dwError;
}

VOID
VmAfdFreeAbsoluteSecurityDescriptor(
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

    VmAfdGetOwnerSecurityDescriptor(pSecDesc, &pOwner, &bDefaulted);
    VmAfdGetGroupSecurityDescriptor(pSecDesc, &pGroup, &bDefaulted);
    VmAfdGetDaclSecurityDescriptor(pSecDesc, &bPresent, &pDacl, &bDefaulted);
    VmAfdGetSaclSecurityDescriptor(pSecDesc, &bPresent, &pSacl, &bDefaulted);

    VMAFD_SAFE_FREE_MEMORY(pSecDesc);
    VMAFD_SAFE_FREE_MEMORY(pOwner);
    VMAFD_SAFE_FREE_MEMORY(pGroup);
    VMAFD_SAFE_FREE_MEMORY(pDacl);
    VMAFD_SAFE_FREE_MEMORY(pSacl);

    *ppSecDesc = NULL;
}


static
DWORD
VmAfdAllocateSidFromUid(
    uid_t uid,
    PSID *ppSid)
{
    DWORD dwError = 0;
    NTSTATUS ntStatus = 0;
    PSID pSid = NULL;
    PSTR pszSid = NULL;

    dwError = VmAfdAllocateStringPrintf(
                   &pszSid,
                   "S-1-22-1-%d", /* Unix uid SID string */
                   uid);
    BAIL_ON_VMAFD_ERROR(dwError);

    ntStatus = RtlAllocateSidFromCString(
                   &pSid,
                   pszSid);
    dwError = LwNtStatusToWin32Error(ntStatus);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppSid = pSid;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszSid);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmAfdAllocateSidFromGid(
    gid_t gid,
    PSID *ppSid)
{
    DWORD dwError = 0;
    PSID pSid = NULL;
    PSTR pszSid = NULL;

    dwError = VmAfdAllocateStringPrintf(
                   &pszSid,
                   "S-1-22-2-%d", /* Unix gid SID string */
                   gid);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateSidFromCString(
                   &pSid,
                   pszSid);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppSid = pSid;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszSid);
    return dwError;

error:
    goto cleanup;
}

static
VOID
VmAfdFreeTokenGroups(
    PTOKEN_GROUPS pTokenGroups)
{
    unsigned int i = 0;

    if (pTokenGroups)
    {
       for (i = 0; i < pTokenGroups->GroupCount; i++)
        {
            VMAFD_SAFE_FREE_MEMORY(pTokenGroups->Groups[i].Sid);
        }
        VmAfdFreeMemory(pTokenGroups);
    }
}

static
DWORD
VmAfdBuildTokenGroups(
    gid_t *pGidList,
    DWORD dwGidListLen,
    PTOKEN_GROUPS *ppTokenGroups)
{
    DWORD dwError = 0;
    PTOKEN_GROUPS pTokenGroups = NULL;
    unsigned int i = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pGidList, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppTokenGroups, dwError);

    dwError = VmAfdAllocateMemory(
                  sizeof(TOKEN_GROUPS) + (sizeof(SID_AND_ATTRIBUTES) * dwGidListLen),
                  (PVOID*)&pTokenGroups);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (i = 0; i < dwGidListLen; i++)
    {
        dwError = VmAfdAllocateSidFromGid(
                      pGidList[i],
                      &pTokenGroups->Groups[i].Sid);
        BAIL_ON_VMAFD_ERROR(dwError);

        pTokenGroups->Groups[i].Attributes = SE_GROUP_ENABLED;
    }
    pTokenGroups->GroupCount = dwGidListLen;

    *ppTokenGroups = pTokenGroups;

cleanup:
    return dwError;

error:
    VmAfdFreeTokenGroups(pTokenGroups);
    goto cleanup;
}

static
DWORD
VmAfdCreateAccessToken(
    PACCESS_TOKEN *         AccessToken,
    PTOKEN_USER             User,
    PTOKEN_GROUPS           Groups,
    PTOKEN_PRIVILEGES       Privileges,
    PTOKEN_OWNER            Owner,
    PTOKEN_PRIMARY_GROUP    PrimaryGroup,
    PTOKEN_DEFAULT_DACL     DefaultDacl)
{
    DWORD dwError = 0;
    NTSTATUS ntStatus = 0;

    ntStatus = RtlCreateAccessToken(
                   AccessToken,
                   User,
                   Groups,
                   Privileges,
                   Owner,
                   PrimaryGroup,
                   DefaultDacl,
                   NULL);
    dwError = LwNtStatusToWin32Error(ntStatus);

    return dwError;
}

static
DWORD
VmAfdAllocateSidFromCString(
    PSID *ppSid,
    PSTR pszSid)
{
    DWORD dwError = 0;
    NTSTATUS ntStatus = 0;

    ntStatus = RtlAllocateSidFromCString(
                   ppSid,
                   pszSid);
    dwError = LwNtStatusToWin32Error(ntStatus);

    return dwError;
}

static
DWORD
VmAfdGetGroupList(
    uid_t userId,
    gid_t *pPrimaryGid,
    gid_t **ppGidList,
    DWORD *pdwGidListLen)
{
    DWORD dwError = 0;
    int iStatus = 0;
    gid_t *pGidList = NULL;
    int iGidListLen = 0;
    struct passwd pwd = { 0 };
    struct passwd *pResult = NULL;
    size_t bufsize = 0;
    char* pBuffer = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppGidList, dwError);

    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
    {
        bufsize = MAX_GWTPWR_BUF_LENGTH;
    }

    dwError = VmAfdAllocateMemory(
                    bufsize,
                    (PVOID*)&pBuffer);
    BAIL_ON_VMAFD_ERROR(dwError);

    iStatus = getpwuid_r(userId, &pwd, pBuffer, bufsize, &pResult);
    if (!pResult)
    {
        dwError = LwErrnoToWin32Error(iStatus);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iGidListLen = 0;
    iStatus = getgrouplist(
                  pwd.pw_name,
                  pwd.pw_gid,
                  NULL,
                  &iGidListLen);
    if (iStatus == -1 && iGidListLen > 0)
    {
        dwError = VmAfdAllocateMemory(
                      iGidListLen * sizeof(gid_t),
                      (PVOID *)&pGidList);
        BAIL_ON_VMAFD_ERROR(dwError);

        iStatus = getgrouplist(
                      pwd.pw_name,
                      pwd.pw_gid,
                      pGidList,
                      &iGidListLen);
        if (iStatus == -1)
        {
            dwError = LwErrnoToWin32Error(iStatus);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    *pPrimaryGid = pwd.pw_gid;
    *ppGidList = pGidList;
    *pdwGidListLen = iGidListLen;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pBuffer);
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pGidList);
    goto cleanup;
}
