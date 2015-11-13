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

/*
 * Module Name:
 *
 *        auth.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Local O/S Identity Store
 *
 *        User and Group lookup
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *          Sung Ruo (sruo@vmware.com)
 *
 */

#include "includes.h"

static
DWORD
VmDirAuthCreateMembership(
    PGROUPINFO        pGroupInfo,
    PMEMBERSHIP_INFO* ppMembership
    );

static
DWORD
VmDirAuthFindUserInGroup(
    PGROUPINFO pGroupInfo,
    PCSTR      pszUsername,
    PBOOLEAN   pbIsMember
    );

static
DWORD
VmDirAuthCloneGroupInfoContents(
    PGROUPINFO pSrcInfo,
    PGROUPINFO pDestInfo
    );

static
DWORD
VmDirAuthFillUserInfo(
    struct passwd* pUser,      /* IN     */
    PUSERINFO      pUserInfo   /* IN OUT */
    );

static
VOID
VmDirAuthFreeMembershipList(
    PMEMBERSHIP_INFO pMemberships
    );

static
VOID
VmDirAuthFreeUserInfoContents(
    PUSERINFO pUserInfo
    );

static
VOID
VmDirAuthFreeGroupInfoContents(
    PGROUPINFO pGroupInfo
    );

static
DWORD
VmDirAuthMapError(
    DWORD dwError
    );

DWORD
VmDirAuthFindUser(
    PCSTR      pszUsername,   /* IN     */
    PUSERINFO* ppUserInfo     /*    OUT */
    )
{
    DWORD dwError = 0;
    size_t size = 4096;
    CHAR   szBuf[size];
    struct passwd user = {0};
    struct passwd* pUser = NULL;
    PUSERINFO pUserInfo = NULL;

    memset(szBuf, 0, sizeof(szBuf));

    if (IsNullOrEmptyString(pszUsername) || !ppUserInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = VmDirAuthMapError(
                    getpwnam_r(pszUsername, &user, szBuf, size, &pUser));
    BAIL_ON_IDM_ERROR(dwError);

    if (!pUser)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = IDMAllocateMemory(sizeof(*pUserInfo), (PVOID*)&pUserInfo);
    BAIL_ON_IDM_ERROR(dwError);

    dwError = VmDirAuthFillUserInfo(pUser, pUserInfo);
    BAIL_ON_IDM_ERROR(dwError);

    *ppUserInfo = pUserInfo;

cleanup:

    return dwError;

error:

    if (ppUserInfo)
    {
        *ppUserInfo = NULL;
    }

    if (pUserInfo)
    {
        VmDirAuthFreeUserInfo(pUserInfo);
    }

    goto cleanup;
}

DWORD
VmDirAuthBeginEnumUsers(
    PAUTHCONTEXT* ppContext   /*    OUT */
    )
{
    DWORD dwError = 0;
    PAUTHCONTEXT pContext = NULL;

    dwError = IDMAllocateMemory(sizeof(*pContext), (PVOID*)&pContext);
    BAIL_ON_IDM_ERROR(dwError);

    setpwent();

    pContext->enumType = ENUMERATION_TYPE_USER;

    *ppContext = pContext;

cleanup:

    return dwError;

error:

    *ppContext = NULL;

    if (pContext)
    {
        IDMFreeMemory(pContext);
    }

    goto cleanup;
}

DWORD
VmDirAuthGetNextUser(
    PAUTHCONTEXT pContext,    /* IN     */
    PUSERINFO*   ppUserInfo   /*    OUT */
    )
{
    DWORD dwError = 0;
    size_t size = 4096;
    CHAR   szBuf[size];
    struct passwd user = {0};
    struct passwd* pUser = NULL;
    PUSERINFO pUserInfo = NULL;

    memset(szBuf, 0, sizeof(szBuf));

    if (!pContext || !ppUserInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = VmDirAuthMapError(
                    getpwent_r(&user, szBuf, size, &pUser));
    BAIL_ON_IDM_ERROR(dwError);

    dwError = IDMAllocateMemory(sizeof(*pUserInfo), (PVOID*)&pUserInfo);
    BAIL_ON_IDM_ERROR(dwError);

    dwError = VmDirAuthFillUserInfo(pUser, pUserInfo);
    BAIL_ON_IDM_ERROR(dwError);

    *ppUserInfo = pUserInfo;

cleanup:

    return dwError;

error:

    if (ppUserInfo)
    {
        *ppUserInfo = NULL;
    }

    if (pUserInfo)
    {
        VmDirAuthFreeUserInfo(pUserInfo);
    }

    goto cleanup;
}

VOID
VmDirAuthEndEnumUsers(
    PAUTHCONTEXT pContext     /* IN OUT */
    )
{
    if (pContext)
    {
        endpwent();

        IDMFreeMemory(pContext);
    }
}

DWORD
VmDirAuthFindGroup(
    PCSTR       pszGroupname, /* IN     */
    PGROUPINFO* ppGroupInfo   /*    OUT */
    )
{
    DWORD dwError = 0;
    size_t size = 4096;
    CHAR   szBuf[size];
    struct group grp = {0};
    struct group* pGroup = NULL;
    PGROUPINFO pGroupInfo = NULL;

    memset(szBuf, 0, sizeof(szBuf));

    if (IsNullOrEmptyString(pszGroupname) || !ppGroupInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = VmDirAuthMapError(
                  getgrnam_r(pszGroupname, &grp, szBuf, size, &pGroup));
    BAIL_ON_IDM_ERROR(dwError);

    if (!pGroup)
    {
        dwError = ERROR_NO_SUCH_GROUP;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = IDMAllocateMemory(sizeof(*pGroupInfo), (PVOID*)&pGroupInfo);
    BAIL_ON_IDM_ERROR(dwError);

    pGroupInfo->gid = pGroup->gr_gid;

    dwError = IDMAllocateStringA(pGroup->gr_name, &pGroupInfo->pszName);
    BAIL_ON_IDM_ERROR(dwError);

    *ppGroupInfo = pGroupInfo;

cleanup:

    return dwError;

error:

    if (ppGroupInfo)
    {
        *ppGroupInfo = NULL;
    }

    if (pGroupInfo)
    {
        VmDirAuthFreeGroupInfo(pGroupInfo);
    }

    goto cleanup;
}

DWORD
VmDirAuthFindGroupById(
    gid_t       groupId,      /* IN     */
    PGROUPINFO* ppGroupInfo   /*    OUT */
    )
{
    DWORD dwError = 0;
    size_t size = 4096;
    CHAR   szBuf[size];
    struct group grp = {0};
    struct group* pGroup = NULL;
    PGROUPINFO pGroupInfo = NULL;

    memset(szBuf, 0, sizeof(szBuf));

    if (!ppGroupInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = VmDirAuthMapError(
                  getgrgid_r(groupId, &grp, szBuf, size, &pGroup));
    BAIL_ON_IDM_ERROR(dwError);

    if (!pGroup)
    {
        dwError = ERROR_NO_SUCH_GROUP;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = IDMAllocateMemory(sizeof(*pGroupInfo), (PVOID*)&pGroupInfo);
    BAIL_ON_IDM_ERROR(dwError);

    pGroupInfo->gid = pGroup->gr_gid;

    dwError = IDMAllocateStringA(pGroup->gr_name, &pGroupInfo->pszName);
    BAIL_ON_IDM_ERROR(dwError);

    *ppGroupInfo = pGroupInfo;

cleanup:

    return dwError;

error:

    if (ppGroupInfo)
    {
        *ppGroupInfo = NULL;
    }

    if (pGroupInfo)
    {
        VmDirAuthFreeGroupInfo(pGroupInfo);
    }

    goto cleanup;
}

DWORD
VmDirAuthBeginEnumGroups(
    PAUTHCONTEXT* ppContext   /*    OUT */
    )
{
    DWORD dwError = 0;
    PAUTHCONTEXT pContext = NULL;

    dwError = IDMAllocateMemory(sizeof(*pContext), (PVOID*)&pContext);
    BAIL_ON_IDM_ERROR(dwError);

    setgrent();

    pContext->enumType = ENUMERATION_TYPE_GROUP;

    *ppContext = pContext;

cleanup:

    return dwError;

error:

    *ppContext = NULL;

    if (pContext)
    {
        IDMFreeMemory(pContext);
    }

    goto cleanup;
}

DWORD
VmDirAuthGetNextGroup(
    PAUTHCONTEXT pContext,    /* IN     */
    PGROUPINFO*  ppGroupInfo  /*    OUT */
    )
{
    DWORD dwError = 0;
    size_t size = 4096;
    CHAR   szBuf[size];
    struct group grp = {0};
    struct group* pGroup = NULL;
    PGROUPINFO pGroupInfo = NULL;

    memset(szBuf, 0, sizeof(szBuf));

    if (!pContext || !ppGroupInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError = VmDirAuthMapError(
                    getgrent_r(&grp, szBuf, size, &pGroup));
    BAIL_ON_IDM_ERROR(dwError);

    dwError = IDMAllocateMemory(sizeof(*pGroupInfo), (PVOID*)&pGroupInfo);
    BAIL_ON_IDM_ERROR(dwError);

    pGroupInfo->gid = pGroup->gr_gid;

    dwError = IDMAllocateStringA(pGroup->gr_name, &pGroupInfo->pszName);
    BAIL_ON_IDM_ERROR(dwError);

    *ppGroupInfo = pGroupInfo;

cleanup:

    return dwError;

error:

    if (ppGroupInfo)
    {
        *ppGroupInfo = NULL;
    }

    if (pGroupInfo)
    {
        VmDirAuthFreeGroupInfo(pGroupInfo);
    }

    goto cleanup;
}

VOID
VmDirAuthEndEnumGroups(
    PAUTHCONTEXT pContext     /* IN OUT */
    )
{
    if (pContext)
    {
        endgrent();

        IDMFreeMemory(pContext);
    }
}

DWORD
VmDirAuthFindUsersInGroup(
    PCSTR       pszGroupname, /* IN     */
    PUSERINFO*  ppUsers,      /*    OUT */
    PDWORD      pdwNumUsers   /* IN OUT */
    )
{
    DWORD dwError = 0;
    PUSERINFO pUserInfoArray = NULL;
    DWORD dwNumUsers = 0;
    size_t size = 4096;
    CHAR   szGroupBuf[size];
    struct group grp = {0};
    struct group* pGroup = NULL;

    if (IsNullOrEmptyString(pszGroupname) || !ppUsers || !pdwNumUsers)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    memset(szGroupBuf, 0, sizeof(szGroupBuf));

    dwError = VmDirAuthMapError(
                    getgrnam_r(pszGroupname, &grp, szGroupBuf, size, &pGroup));
    BAIL_ON_IDM_ERROR(dwError);

    if (!pGroup)
    {
        dwError = ERROR_NO_SUCH_GROUP;
        BAIL_ON_IDM_ERROR(dwError);
    }

    if (pGroup->gr_mem)
    {
        PSTR* ppszMembers = pGroup->gr_mem;
        DWORD iMember = 0;

        while (ppszMembers && *ppszMembers)
        {
            ppszMembers++;
            dwNumUsers++;
        }

        if (dwNumUsers > 0)
        {
            dwError = IDMAllocateMemory(
                            sizeof(USERINFO) * dwNumUsers,
                            (PVOID*)&pUserInfoArray);
            BAIL_ON_IDM_ERROR(dwError);

            ppszMembers = pGroup->gr_mem;
            while (ppszMembers && *ppszMembers)
            {
                size_t size   = 4096;
                CHAR   szBuf[size];
                struct passwd  user  = {0};
                struct passwd* pUser = NULL;

                memset(szBuf, 0, sizeof(szBuf));

                dwError = VmDirAuthMapError(
                                getpwnam_r(
                                       *ppszMembers,
                                       &user,
                                       szBuf,
                                       size,
                                       &pUser));
                BAIL_ON_IDM_ERROR(dwError);

                if (pUser)
                {
                    dwError = VmDirAuthFillUserInfo(
                                    pUser,
                                    &pUserInfoArray[iMember++]);
                    BAIL_ON_IDM_ERROR(dwError);
                }

                ppszMembers++;
            }
			
			dwNumUsers = iMember; // some users might have been skipped
        }
    }

    if (!dwNumUsers && pUserInfoArray)
    {
        VmDirAuthFreeUserInfoArray(pUserInfoArray, dwNumUsers);
        pUserInfoArray = NULL;
    }

    *ppUsers = pUserInfoArray;
    *pdwNumUsers = dwNumUsers;

cleanup:

    return dwError;

error:

    if (ppUsers)
    {
        *ppUsers = NULL;
    }
    if (pdwNumUsers)
    {
        *pdwNumUsers = 0;
    }

    if (pUserInfoArray)
    {
        VmDirAuthFreeUserInfoArray(pUserInfoArray, dwNumUsers);
    }

    goto cleanup;
}

DWORD
VmDirAuthFindGroupsForUser(
    PCSTR       pszUsername,  /* IN     */
    PGROUPINFO* ppGroups,     /*    OUT */
    PDWORD      pdwNumGroups  /* IN OUT */
    )
{
    DWORD dwError = 0;
    PMEMBERSHIP_INFO pMemberships = NULL;
    PGROUPINFO pGroupInfo = NULL;
    PGROUPINFO pGroupArray = NULL;
    DWORD      dwNumGroups = 0;
    PAUTHCONTEXT pContext = NULL;
    PUSERINFO  pUserInfo  = NULL;
    PMEMBERSHIP_INFO pMembership = NULL; // Do not free

    dwError = VmDirAuthFindUser(pszUsername, &pUserInfo);
    BAIL_ON_IDM_ERROR(dwError);

    dwError = VmDirAuthFindGroupById(pUserInfo->gid, &pGroupInfo);
    BAIL_ON_IDM_ERROR(dwError);

    dwError = VmDirAuthCreateMembership(pGroupInfo, &pMembership);
    BAIL_ON_IDM_ERROR(dwError);

    pGroupInfo = NULL;
    pMembership->pNext = pMemberships;
    pMemberships = pMembership;
    pMembership = NULL;
    dwNumGroups++;

    dwError = VmDirAuthBeginEnumGroups(&pContext);
    BAIL_ON_IDM_ERROR(dwError);

    while (1)
    {
        BOOLEAN bIsMember = FALSE;

        if (pGroupInfo)
        {
            VmDirAuthFreeGroupInfo(pGroupInfo);
            pGroupInfo = NULL;
        }

        dwError =  VmDirAuthGetNextGroup(pContext, &pGroupInfo);
        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            dwError = 0;
            break;
        }
        BAIL_ON_IDM_ERROR(dwError);

        dwError = VmDirAuthFindUserInGroup(
                        pGroupInfo,
                        pszUsername,
                        &bIsMember);
        if (dwError == ERROR_NO_SUCH_GROUP)
        {
            dwError = 0;
        }
        BAIL_ON_IDM_ERROR(dwError);
    
        if (bIsMember)
        {
            dwError = VmDirAuthCreateMembership(pGroupInfo, &pMembership);
            BAIL_ON_IDM_ERROR(dwError);

            pGroupInfo = NULL;
            pMembership->pNext = pMemberships;
            pMemberships = pMembership;
            pMembership = NULL;

            dwNumGroups++;
        }
    }

    if (pMemberships)
    {
        PMEMBERSHIP_INFO pMember = pMemberships;
        DWORD iMember = 0;

        dwError = IDMAllocateMemory(
                        sizeof(GROUPINFO) * dwNumGroups,
                        (PVOID*)&pGroupArray);
        BAIL_ON_IDM_ERROR(dwError);

        pMember = pMemberships;
        iMember = 0;
        while (pMember)
        {
            PGROUPINFO pGroup = &pGroupArray[iMember++];

            dwError = VmDirAuthCloneGroupInfoContents(
                           pMember->pGroupInfo,
                           pGroup);
            BAIL_ON_IDM_ERROR(dwError);

            pMember = pMember->pNext;
        }
    }

    *ppGroups = pGroupArray;
    *pdwNumGroups = dwNumGroups;

cleanup:

    if (pContext)
    {
        VmDirAuthEndEnumGroups(pContext);
    }

    if (pUserInfo)
    {
        VmDirAuthFreeUserInfo(pUserInfo);
    }

    if (pGroupInfo)
    {
        VmDirAuthFreeGroupInfo(pGroupInfo);
    }

    if (pMemberships)
    {
        VmDirAuthFreeMembershipList(pMemberships);
    }

    return dwError;

error:

    if (ppGroups)
    {
        *ppGroups = NULL;
    }
    if (pdwNumGroups)
    {
        *pdwNumGroups = 0;
    }

    if (pGroupArray)
    {
        VmDirAuthFreeGroupInfoArray(pGroupArray, dwNumGroups);
    }

    goto cleanup;
}

DWORD
VmDirAuthCredentials(
    PCSTR pszUsername,        /* IN     */
    PCSTR pszPassword         /* IN     */
    )
{
    DWORD dwError = 0;
    struct passwd user = {0};
    struct passwd* pUser = NULL;
    struct spwd  spuser  = {0};
    struct spwd* pSpUser = NULL;
    char   szBuf[4096];
    char   szBuf2[4096];
    struct crypt_data data;
    char   *pszEncPasswd = NULL;

    if (IsNullOrEmptyString(pszUsername) || IsNullOrEmptyString(pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    memset(szBuf, 0, sizeof(szBuf));

    dwError = VmDirAuthMapError(
                    getpwnam_r(
                       pszUsername,
                       &user,
                       szBuf,
                       sizeof(szBuf),
                       &pUser));
    BAIL_ON_IDM_ERROR(dwError);

    if (!pUser)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    memset(szBuf2, 0, sizeof(szBuf2));

    dwError = VmDirAuthMapError(
                    getspnam_r(
                       pUser->pw_name,
                       &spuser,
                       szBuf2,
                       sizeof(szBuf2),
                       &pSpUser));
    BAIL_ON_IDM_ERROR(dwError);

    if (!pSpUser)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    memset(&data, 0, sizeof(data));

    pszEncPasswd = crypt_r(pszPassword, pSpUser->sp_pwdp, &data);
    if (!pszEncPasswd || strcmp(pszEncPasswd, pSpUser->sp_pwdp))
    {
        dwError = ERROR_LOGON_FAILURE;
        BAIL_ON_IDM_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

VOID
VmDirAuthFreeUserInfoArray(
    PUSERINFO pUserInfoArray,   /* IN OUT */
    DWORD     dwNumUsers        /* IN     */
    )
{
    if (pUserInfoArray)
    {
        DWORD iUser = 0;

        for (; iUser < dwNumUsers; iUser++)
        {
            PUSERINFO pUserInfo = &pUserInfoArray[iUser];

            if (pUserInfo)
            {
                VmDirAuthFreeUserInfoContents(pUserInfo);
            }
        }

        IDMFreeMemory(pUserInfoArray);
    }
}

VOID
VmDirAuthFreeUserInfo(
    PUSERINFO pUserInfo         /* IN OUT */
    )
{
    if (pUserInfo)
    {
        VmDirAuthFreeUserInfoContents(pUserInfo);
        IDMFreeMemory(pUserInfo);
    }
}

VOID
VmDirAuthFreeGroupInfoArray(
    PGROUPINFO pGroupInfoArray, /* IN OUT */
    DWORD      dwNumGroups      /* IN     */
    )
{
    if (pGroupInfoArray)
    {
        DWORD iGroup = 0;

        for (; iGroup < dwNumGroups; iGroup++)
        {
            PGROUPINFO pGroupInfo = &pGroupInfoArray[iGroup];

            if (pGroupInfo)
            {
                VmDirAuthFreeGroupInfoContents(pGroupInfo);
            }
        }

        IDMFreeMemory(pGroupInfoArray);
    }
}

VOID
VmDirAuthFreeGroupInfo(
    PGROUPINFO pGroupInfo       /* IN OUT */
    )
{
    if (pGroupInfo)
    {
        VmDirAuthFreeGroupInfoContents(pGroupInfo);
        IDMFreeMemory(pGroupInfo);
    }
}

static
DWORD
VmDirAuthCreateMembership(
    PGROUPINFO        pGroupInfo,
    PMEMBERSHIP_INFO* ppMembership
    )
{
    DWORD dwError = 0;
    PMEMBERSHIP_INFO pMembership = NULL;

    dwError = IDMAllocateMemory(
                    sizeof(MEMBERSHIP_INFO),
                    (PVOID*)&pMembership);
    BAIL_ON_IDM_ERROR(dwError);

    pMembership->pGroupInfo = pGroupInfo;

    *ppMembership = pMembership;

cleanup:

    return dwError;

error:

    *ppMembership = NULL;

    goto cleanup;
}

static
DWORD
VmDirAuthFindUserInGroup(
    PGROUPINFO pGroupInfo,
    PCSTR      pszUsername,
    PBOOLEAN   pbIsMember
    )
{
    DWORD dwError = 0;
    PUSERINFO pUserInfoArray = NULL;
    DWORD dwNumUsers = 0;
    BOOLEAN bIsMember = FALSE;

    dwError = VmDirAuthFindUsersInGroup(
                        pGroupInfo->pszName,
                        &pUserInfoArray,
                        &dwNumUsers);
    BAIL_ON_IDM_ERROR(dwError);

    if (dwNumUsers > 0)
    {
        DWORD iUser = 0;

        for (; iUser < dwNumUsers; iUser++)
        {
            PUSERINFO pUserInfo = &pUserInfoArray[iUser];

            if (!strcmp(pUserInfo->pszName, pszUsername))
            {
                bIsMember = TRUE;

                break;
            }
        }
    }

    *pbIsMember = bIsMember;

cleanup:

    if (pUserInfoArray)
    {
        VmDirAuthFreeUserInfoArray(pUserInfoArray, dwNumUsers);
    }

    return dwError;

error:

    *pbIsMember = FALSE;

    goto cleanup;
}

static
DWORD
VmDirAuthCloneGroupInfoContents(
    PGROUPINFO pSrcInfo,
    PGROUPINFO pDestInfo
    )
{
    DWORD dwError = 0;

    if (pSrcInfo->pszName)
    {
        dwError = IDMAllocateStringA(
                        pSrcInfo->pszName,
                        &pDestInfo->pszName);
        BAIL_ON_IDM_ERROR(dwError);
    }

    if (pSrcInfo->pszComment)
    {
        dwError = IDMAllocateStringA(
                        pSrcInfo->pszComment,
                        &pDestInfo->pszComment);
        BAIL_ON_IDM_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
VmDirAuthFillUserInfo(
    struct passwd* pUser,      /* IN     */
    PUSERINFO      pUserInfo   /* IN OUT */
    )
{
    DWORD dwError = 0;

    pUserInfo->uid = pUser->pw_uid;
    pUserInfo->gid = pUser->pw_gid;

    dwError = IDMAllocateStringA(pUser->pw_name, &pUserInfo->pszName);
    BAIL_ON_IDM_ERROR(dwError);

    if (pUser->pw_gecos)
    {
        dwError = IDMAllocateStringA(
                            pUser->pw_gecos,
                            &pUserInfo->pszFullName);
        BAIL_ON_IDM_ERROR(dwError);
    }

error:

    return dwError;
}

static
VOID
VmDirAuthFreeMembershipList(
    PMEMBERSHIP_INFO pMemberships
    )
{
    while (pMemberships)
    {
        PMEMBERSHIP_INFO pMember = pMemberships;

        pMemberships = pMemberships->pNext;

        if (pMember->pGroupInfo)
        {
            VmDirAuthFreeGroupInfo(pMember->pGroupInfo);
        }

        IDMFreeMemory(pMember);
    }
}

static
VOID
VmDirAuthFreeUserInfoContents(
    PUSERINFO pUserInfo
    )
{
    IDM_SAFE_FREE_MEMORY(pUserInfo->pszName);
    IDM_SAFE_FREE_MEMORY(pUserInfo->pszFullName);
    IDM_SAFE_FREE_MEMORY(pUserInfo->pszComment);
}

static
VOID
VmDirAuthFreeGroupInfoContents(
    PGROUPINFO pGroupInfo
    )
{
    IDM_SAFE_FREE_MEMORY(pGroupInfo->pszName);
    IDM_SAFE_FREE_MEMORY(pGroupInfo->pszComment);
}

static
DWORD
VmDirAuthMapError(
    DWORD dwError
    )
{
    DWORD dwMappedError = 0;

    switch (dwError)
    {
        case 0:

            break;

        case ENOENT:

            dwMappedError = ERROR_NO_MORE_ITEMS;

            break;

        default:

            dwMappedError = LwErrnoToWin32Error(dwError);

            break;
    }

    return dwMappedError;
}

