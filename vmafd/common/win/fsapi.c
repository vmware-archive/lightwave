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
/*
 * Disable some RPC includes to avoid type re-definitions
 * on handle_t and BOOL.
 */
#define __wtypes_h__
#define __RPCASYNC_H__
#define __RPCNDR_H__
#define __RPCNSI_H__
#define __RPCDCE_H__
#include <Aclapi.h>

static
DWORD
VmAfdGetFilesCount(
    PCSTR   pcszDirPath,
    PDWORD  pdwCount
    );

static
DWORD
VmAfdEnumFiles(
    PCSTR  pcszDirPath,
    PSTR** pppszFiles,
    DWORD* pdwCount
    );

DWORD
VmAfdFindFileIndex(
    PCSTR  pszDirPath,
    PCSTR  pszFile,
    BOOLEAN bCrl,
    PLONG  pIndex
    )
{
    DWORD dwError = 0;
    PSTR  pszSearchFilter = NULL;
    HANDLE hFindFile = NULL;
    WIN32_FIND_DATAA findData = {0};
    size_t len = 0;
    LONG   maxIndex = -1;

    dwError = VmAfdAllocateStringPrintf(
                    &pszSearchFilter,
                    bCrl ? "%s\\%s.r*" : "%s\\%s.*",
                    pszDirPath,
                    pszFile);
    BAIL_ON_VMAFD_ERROR(dwError);

    hFindFile = FindFirstFileA(pszSearchFilter, &findData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

    len = strlen(pszFile);

    do
    {
        // Note: Certs and CRLs are written with FILE_ATTRIBUTE_ARCHIVE
        // and FILE_ATTRIBUTE_NOT_CONTENT_INDEXED.
        //if ((findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) == FILE_ATTRIBUTE_NORMAL)
        {
            PSTR pszCursor = findData.cFileName + len;

            if (!IsNullOrEmptyString(pszCursor) && *pszCursor == '.')
            {
                pszCursor++;
            }

            if (!IsNullOrEmptyString(pszCursor)
                && (bCrl ^ (*pszCursor != 'r')))
            {
                long index = -1;
                PSTR pszEnd = NULL;

                index = strtol(pszCursor + (bCrl ? 1 : 0), &pszEnd, 10);
                if (pszEnd && (*pszEnd == '\0') && (index > maxIndex))
                {
                    maxIndex = index;
                }
            }
        }

        if (!FindNextFileA(hFindFile, &findData))
        {
            dwError = GetLastError();
            if (dwError == ERROR_NO_MORE_FILES || dwError == ERROR_FILE_NOT_FOUND)
            {
                dwError = 0;
                break;
            }
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }
    while(TRUE);

    if (maxIndex < 0)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pIndex = maxIndex;

cleanup:

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindFile);
    }

    VMAFD_SAFE_FREE_MEMORY(pszSearchFilter);

    VmAfdLog(VMAFD_DEBUG_DEBUG,
        "VmAfdFindFileIndex: maxIndex: %d.",
        maxIndex);

    return dwError;

error:

    if (pIndex)
    {
        *pIndex = -1;
    }

    goto cleanup;
}

DWORD
VmAfdListFilesInDir(
    PCSTR   pszDirPath,
    DWORD*  pdwCount,
    PSTR**  pppszFiles
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;
    DWORD   dwSize = 0;
    PSTR*   ppszFiles = NULL;

    if (!pdwCount || !pppszFiles || IsNullOrEmptyString(pszDirPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdEnumFiles(pszDirPath, &ppszFiles, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwCount = dwCount;
    *pppszFiles = ppszFiles;

cleanup:

    return dwError;

error:

    if (pppszFiles)
    {
        *pppszFiles = NULL;
    }
    if (ppszFiles)
    {
        VmAfdFreeStringArrayCountA(ppszFiles, dwCount);
    }

    goto cleanup;
}

DWORD
VmAfdCopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    )
{
    DWORD dwError = 0;

    errno = 0;
    if (!CopyFileA(pszSrc, pszDest, TRUE))
    {
        dwError = GetLastError();
    }

    return dwError;
}

DWORD
VmAfdDeleteFile(
    PCSTR pszFile
    )
{
    DWORD dwError = 0;

    if (0 != _unlink(pszFile))
    {
        dwError = GetLastError();
    }

    return dwError;
}

DWORD
VmAfdRenameFile(
    PCSTR pszOldFile,
    PCSTR pszNewFile
    )
{
    DWORD dwError = 0;

    if (0 != rename(pszOldFile, pszNewFile))
    {
        dwError = GetLastError();
    }

    return dwError;
}

static
DWORD
VmAfdGetFilesCount(
    PCSTR   pcszDirPath,
    PDWORD  pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA findData = {0};

    if (IsNullOrEmptyString(pcszDirPath) || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    hFindFile = FindFirstFileA(pcszDirPath, &findData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    while (TRUE)
    {
        if (FindNextFileA(hFindFile, &findData))
        {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) == FILE_ATTRIBUTE_NORMAL)
            {
                dwCount++;
            }
        }
        else
        {
            dwError = GetLastError();
            if (dwError == ERROR_NO_MORE_FILES)
            {
                dwError = 0;
                break;
            }
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    *pdwCount = dwCount;

cleanup:

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindFile);
    }

    return dwError;
error:

    if (pdwCount)
    {
        *pdwCount = 0;
    }
    goto cleanup;
}


static
DWORD
VmAfdEnumFiles(
    PCSTR  pcszDirPath,
    PSTR** pppszFiles,
    PDWORD pdwCount
    )
{
    DWORD  dwError = 0;
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA findData = {0};
    DWORD  dwCount = 0;
    DWORD  dwSize  = 0;
    PSTR   ppszFiles = NULL;

    dwError = VmAfdGetFilesCount(pcszDirPath, &dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwSize)
    {
        dwError = VmAfdAllocateMemory(
                                sizeof(PSTR) * dwSize,
                                (PVOID*)&ppszFiles
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        hFindFile = FindFirstFileA(pcszDirPath, &findData);
        if (hFindFile == INVALID_HANDLE_VALUE)
        {
            dwError = GetLastError();
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        while (TRUE)
        {
            if (FindNextFileA(hFindFile, &findData))
            {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) == FILE_ATTRIBUTE_NORMAL)
                {
                    if (dwCount >= dwSize)
                    {
                        dwError = ERROR_MORE_DATA;
                        BAIL_ON_VMAFD_ERROR(dwError);
                    }

                    dwError = VmAfdAllocateStringA(findData.cFileName,
                        &ppszFiles[dwCount]);
                    BAIL_ON_VMAFD_ERROR(dwError);

                    dwCount++;
                }
            }
            else
            {
                dwError = GetLastError();
                if (dwError == ERROR_NO_MORE_FILES)
                {
                    dwError = 0;
                    break;
                }
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

    *pdwCount = dwCount;
    *pppszFiles = ppszFiles;

cleanup:

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFindFile);
    }

    return dwError;

error:

    if (pppszFiles)
    {
        *pppszFiles = NULL;
    }
    if (ppszFiles)
    {
        VmAfdFreeStringArrayCountA(ppszFiles, dwCount);
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }

    goto cleanup;
}

DWORD
VmAfdRestrictFilePermissionToSelf(
    PCSTR   pszFileName
    )
{
    DWORD dwError = ERROR_SUCCESS;
    SE_OBJECT_TYPE objectType = SE_FILE_OBJECT;
    PACL pNewDacl = NULL;
    EXPLICIT_ACCESS ea[2] = {0};
    SID_IDENTIFIER_AUTHORITY authOwner = SECURITY_CREATOR_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PSID pOwnerSid = NULL;
    PSID pAdminSid = NULL;
    PWSTR pwszFileName = NULL;

    if (pszFileName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!AllocateAndInitializeSid(&authOwner, 1, 0, 0, 0, 0, 0, 0, 0, 0, &pOwnerSid))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfAccessMode = GRANT_ACCESS;
    ea[0].grfInheritance= NO_PROPAGATE_INHERIT_ACE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.ptstrName = (LPSTR)pOwnerSid;

    if(! AllocateAndInitializeSid(
                    &authNT, 2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAdminSid))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ea[1].grfAccessPermissions = FILE_ALL_ACCESS;
    ea[1].grfAccessMode = GRANT_ACCESS;
    ea[1].grfInheritance= NO_PROPAGATE_INHERIT_ACE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPSTR)pAdminSid;

    dwError = SetEntriesInAclA(2, &ea, NULL, &pNewDacl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszFileName, &pwszFileName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = SetNamedSecurityInfoW(pwszFileName, objectType,
        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
        pAdminSid, NULL, pNewDacl, NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_STRINGW(pwszFileName);
    if(pNewDacl != NULL)
    {
        LocalFree((HLOCAL) pNewDacl);
        pNewDacl = NULL;
    }
    if (pAdminSid)
    {
        FreeSid(pAdminSid);
        pAdminSid = NULL;
    }
    if (pOwnerSid)
    {
        FreeSid(pOwnerSid);
        pOwnerSid = NULL;
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdOpenFilePath(
    PCSTR   pszFileName,
    PCSTR   pszOpenMode,
    FILE**  fp,
    int mode
)
{
    DWORD dwError = ERROR_SUCCESS;
    FILE* temp = NULL;
    PWSTR pwszFileName = NULL;
    PWSTR pwszOpenMode = NULL;

    if (IsNullOrEmptyString(pszFileName) ||
        IsNullOrEmptyString(pszOpenMode) ||
        fp == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszFileName, &pwszFileName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszOpenMode, &pwszOpenMode);
    BAIL_ON_VMAFD_ERROR(dwError);

    temp = _wfopen(pwszFileName, pwszOpenMode);
    if (temp == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *fp = temp;

cleanup:
    VMAFD_SAFE_FREE_STRINGW(pwszFileName);
    VMAFD_SAFE_FREE_STRINGW(pwszOpenMode);

    return dwError;
error :
    goto cleanup;
}


