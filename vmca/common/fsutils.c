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



/*
 * Module Name: ThinAppVMCAService
 *
 * Filename: fsutils.c
 *
 * Abstract:
 *
 * File System Utilities
 *
 */

#include "includes.h"

#ifdef _WIN32

#include <sys/stat.h>
#include <direct.h>

#ifndef S_ISREG
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#endif

#define COPY_BUFFER_SIZE 4*1024

DWORD
VMCAOpenFilePath(
    PCSTR   pszFileName,
    PCSTR   pszOpenMode,
    FILE**  fp
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

#ifdef WIN32
    dwError = VMCAAllocateStringWFromA(pszFileName, &pwszFileName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(pszOpenMode, &pwszOpenMode);
    BAIL_ON_VMCA_ERROR(dwError);

    temp = _wfopen(pwszFileName, pwszOpenMode);
    if (temp == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }
#else
    temp = fopen(pszFileName, pszOpenMode);
    if (temp == NULL) 
    {
        dwError = VMCAGetWin32ErrorCode(errno);
        BAIL_ON_VMCA_ERROR(dwError);
    }
#endif
    *fp = temp;

cleanup:
    VMCA_SAFE_FREE_STRINGW(pwszFileName);
    VMCA_SAFE_FREE_STRINGW(pwszOpenMode);

    return dwError;
error :
    goto cleanup;
}

#ifndef _WIN32

DWORD
VMCACopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    )
{
    DWORD dwError = 0;
    FILE* pfSrc = NULL;
    FILE* pfDest = NULL;
    size_t cbRead = 0;
    BYTE  buf[COPY_BUFFER_SIZE];

    dwError = VMCAOpenFilePath(pszSrc, "r", &pfSrc);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAOpenFilePath(pszDest, "w", &pfDest);
    BAIL_ON_VMCA_ERROR(dwError);

    while ((cbRead = fread(buf, 1, COPY_BUFFER_SIZE, pfSrc)) > 0)
    {
        size_t bytesWritten = 0;

        if ((bytesWritten = fwrite(buf, 1, cbRead, pfDest)) == 0)
        {
            dwError = VMCAGetWin32ErrorCode(errno);
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    if (pfDest)
    {
        fclose(pfDest);
        pfDest = NULL;
    }

    if(chmod(pszDest,S_IRUSR | S_IWUSR) != 0)
    {
        dwError = VMCAGetWin32ErrorCode(errno);
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:

    if (pfSrc)
    {
        fclose(pfSrc);
        pfSrc = NULL;
    }
    return dwError;

error:

    if (pfDest)
    {
        fclose(pfDest);
        pfDest = NULL;
    }

    goto cleanup;
}

#endif

#ifdef _WIN32

DWORD
VMCACopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    )
{
    DWORD dwError = 0;

    errno = 0;
    if (!CopyFileA(pszSrc, pszDest, FALSE))
    {
        dwError = GetLastError();
    }

    return dwError;
}

DWORD
VMCARestrictDirectoryAccess(
    PCSTR szDirectoryPath
    )
{
    PSID pLocalSystemSid = NULL, pAdminSID = NULL;
    PACL pACL = NULL;
    EXPLICIT_ACCESS ea[2] = {0};
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    DWORD dwError = ERROR_SUCCESS;

    VMCA_LOG_INFO("Restricting access to directory %s", szDirectoryPath);

    if (!AllocateAndInitializeSid(
                    &SIDAuthNT,1,
                    SECURITY_LOCAL_SYSTEM_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pLocalSystemSid)) 
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea[0].Trustee.ptstrName  = (LPSTR) pLocalSystemSid;

    if(! AllocateAndInitializeSid(
                    &SIDAuthNT, 2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAdminSID))
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    ea[1].grfAccessPermissions = FILE_ALL_ACCESS;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPSTR) pAdminSID;

    dwError = SetEntriesInAclA(2, ea, NULL, &pACL);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = SetNamedSecurityInfoA(
                    (LPSTR)szDirectoryPath,
                    SE_FILE_OBJECT,
                    DACL_SECURITY_INFORMATION|PROTECTED_DACL_SECURITY_INFORMATION,
                    pAdminSID,
                    NULL,
                    pACL,
                    NULL);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:

    if (pLocalSystemSid)
    {
        FreeSid(pLocalSystemSid);
    }

    if (pAdminSID)
    {
        FreeSid(pAdminSID);
    }

    if (pACL)
    {
        LocalFree(pACL);
    }

    return dwError;

error:
    goto cleanup;
}

#endif
