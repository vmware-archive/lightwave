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

#define COPY_BUFFER_SIZE 256

static
DWORD
VmAfdGetFilesCount(
    DIR*    pDir,
    PDWORD  pdwCount
    );

static
DWORD
VmAfdEnumFiles(
    DIR*    pDir,
    PSTR**  pppszFiles,
    DWORD*  pdwCount
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
    DIR* pDir = NULL;
    struct dirent* pDirCursor = NULL;
    size_t len = 0;
    long  maxIndex = -1;

    if (!(pDir = opendir(pszDirPath)))
    {
        dwError = VmAfdGetWin32ErrorCode(errno);
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

    len = strlen(pszFile);

    do
    {
        union
        {
          struct dirent d;
          char b[offsetof (struct dirent, d_name) + NAME_MAX + 1];
        } u;

        errno = 0;

        if (readdir_r(pDir, &u.d, &pDirCursor) < 0)
        {
            dwError = VmAfdGetWin32ErrorCode(errno);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pDirCursor &&
            (pDirCursor->d_type == DT_REG || pDirCursor->d_type == DT_LNK) &&
            !strncmp(pDirCursor->d_name, pszFile, len))
        {
            PSTR pszCursor = pDirCursor->d_name + len;

            if (!IsNullOrEmptyString(pszCursor) && *pszCursor == '.')
            {
                pszCursor++;
            }

            if (!IsNullOrEmptyString(pszCursor))
            {
                long index = -1;
                PSTR pszEnd = NULL;

                if ((*pszCursor == 'r') ^ bCrl)
                {
                    continue;
                }

                index = strtol(pszCursor + (bCrl ? 1 : 0), &pszEnd, 10);
                if (pszEnd && (*pszEnd == '\0') && (index > maxIndex))
                {
                    maxIndex = index;
                }
            }
        }

    } while (pDirCursor);

    if (maxIndex < 0)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

    *pIndex = maxIndex;

cleanup:

    if (pDir)
    {
        closedir(pDir);
    }

    return dwError;

error:

    *pIndex = -1;

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
    DIR*    pDir = NULL;
    DWORD   dwCount = 0;
    PSTR*   ppszFiles = NULL;

    if (!pdwCount || !pppszFiles || IsNullOrEmptyString(pszDirPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!(pDir = opendir(pszDirPath)))
    {
        dwError = VmAfdGetWin32ErrorCode(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdEnumFiles(pDir, &ppszFiles, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwCount = dwCount;
    *pppszFiles = ppszFiles;

cleanup:

    if (pDir)
    {
        closedir(pDir);
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

    goto cleanup;
}

DWORD
VmAfdDeleteFile(
    PCSTR pszFile
    )
{
    DWORD dwError = 0;

    errno = 0;
    unlink(pszFile);
    dwError = VmAfdGetWin32ErrorCode(errno);

    return dwError;
}

DWORD
VmAfdCopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    )
{
    DWORD dwError = 0;
    FILE* pfSrc = NULL;
    FILE* pfDest = NULL;
    size_t cbRead = 0;
    BYTE  buf[COPY_BUFFER_SIZE];

    dwError = VmAfdOpenFilePath(pszSrc, "r", &pfSrc, 0);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdOpenFilePath(pszDest, "w", &pfDest, 0);
    BAIL_ON_VMAFD_ERROR(dwError);

    while ((cbRead = fread(buf, 1, COPY_BUFFER_SIZE, pfSrc)) > 0)
    {
        size_t bytesWritten = 0;

        if ((bytesWritten = fwrite(buf, 1, cbRead, pfDest)) == 0)
        {
#ifndef _WIN32
            dwError = VmAfdGetWin32ErrorCode(errno);
#else
            dwError = GetLastError();
#endif
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    if (pfDest)
    {
        fclose(pfDest);
        pfDest = NULL;
    }

#ifndef _WIN32
    if(chmod(pszDest,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
    {
        dwError = VmAfdGetWin32ErrorCode(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
#endif

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

DWORD
VmAfdRenameFile(
    PCSTR pszOldFile,
    PCSTR pszNewFile
    )
{
    DWORD dwError = 0;

    errno = 0;

    rename(pszOldFile, pszNewFile);
    dwError = VmAfdGetWin32ErrorCode(errno);

    return dwError;
}

static
DWORD
VmAfdGetFilesCount(
    DIR*   pDir,
    PDWORD pdwCount
)
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    struct dirent* pDirCursor = NULL;

    union
    {
      struct dirent d;
      char b[offsetof (struct dirent, d_name) + NAME_MAX + 1];
    } u;

    if (!pDir || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    rewinddir(pDir);

    do
    {
        errno = 0;

        if (readdir_r(pDir, &u.d, &pDirCursor) < 0)
        {
            dwError = VmAfdGetWin32ErrorCode(errno);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pDirCursor &&
            (pDirCursor->d_type == DT_REG))
        {

            dwCount++;
        }
    }
    while (pDirCursor);

cleanup:

    if (pDir)
    {
      rewinddir(pDir);
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
    DIR*    pDir,
    PSTR**  pppszFiles,
    DWORD*  pdwCount
)
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;
    DWORD   dwSize  = 0;
    struct dirent* pDirCursor = NULL;
    PSTR* ppszFiles = NULL;

    if (!pdwCount || !pppszFiles || !pDir)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetFilesCount(pDir, &dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwSize)
    {
        union
        {
          struct dirent d;
          char b[offsetof (struct dirent, d_name) + NAME_MAX + 1];
        } u;

        dwError = VmAfdAllocateMemory(
                            sizeof(PSTR) * dwSize,
                            (PVOID*)&ppszFiles
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        do
        {
            errno = 0;

            if (readdir_r(pDir, &u.d, &pDirCursor) < 0)
            {
                dwError = VmAfdGetWin32ErrorCode(errno);
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            if (pDirCursor &&
                (pDirCursor->d_type == DT_REG))
            {
                if (dwCount >= dwSize)
                {
                    dwError = ERROR_MORE_DATA;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pDirCursor->d_name,
                    &ppszFiles[dwCount]);
                BAIL_ON_VMAFD_ERROR(dwError);
                dwCount++;
            }
        }
        while (pDirCursor);
    }

    *pdwCount = dwCount;
    *pppszFiles = ppszFiles;

cleanup:

    return dwError;

error:

    if (pppszFiles)
    {
        *pppszFiles = 0;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (ppszFiles)
    {
      VmAfdFreeStringArrayCountA(ppszFiles, dwCount);
    }

    goto cleanup;
}

DWORD
VmAfdRestrictFilePermissionToSelf(
    PCSTR   pcszFileName
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if(chmod(pcszFileName, S_IRUSR | S_IWUSR) != 0)
    {
        dwError = VmAfdGetWin32ErrorCode(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    return dwError;
error :
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

    temp = fopen(pszFileName, pszOpenMode);
    if (temp == NULL) 
    {
        dwError = VmAfdGetWin32ErrorCode(errno);
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    } 
    else 
    {
        if (mode != 0)
        {
            if(chmod(pszFileName, mode) != 0)
            {
                dwError = VmAfdGetWin32ErrorCode(errno);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

    *fp = temp;

cleanup:
    VMAFD_SAFE_FREE_STRINGW(pwszFileName);
    VMAFD_SAFE_FREE_STRINGW(pwszOpenMode);

    return dwError;
error :
    goto cleanup;
}


