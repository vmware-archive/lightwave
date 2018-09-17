/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#define COPY_BUFFER_SIZE 4*1024

DWORD
LwCAOpenFilePath(
    PCSTR   pszFileName,
    PCSTR   pszOpenMode,
    FILE**  fp
    )
{
    DWORD dwError = LWCA_SUCCESS;
    FILE* temp = NULL;
    PWSTR pwszOpenMode = NULL;

    if (IsNullOrEmptyString(pszFileName) ||
        IsNullOrEmptyString(pszOpenMode) ||
        fp == NULL)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    temp = fopen(pszFileName, pszOpenMode);
    if (temp == NULL)
    {
        dwError = LWCA_ERRNO_TO_LWCAERROR(errno);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    *fp = temp;

cleanup:
    LWCA_SAFE_FREE_STRINGW(pwszOpenMode);

    return dwError;
error :
    goto cleanup;
}

DWORD
LwCACopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    )
{
    DWORD dwError = 0;
    FILE* pfSrc = NULL;
    FILE* pfDest = NULL;
    size_t cbRead = 0;
    BYTE  buf[COPY_BUFFER_SIZE];

    dwError = LwCAOpenFilePath(pszSrc, "r", &pfSrc);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAOpenFilePath(pszDest, "w", &pfDest);
    BAIL_ON_LWCA_ERROR(dwError);

    while ((cbRead = fread(buf, 1, COPY_BUFFER_SIZE, pfSrc)) > 0)
    {
        size_t bytesWritten = 0;

        if ((bytesWritten = fwrite(buf, 1, cbRead, pfDest)) == 0)
        {
            dwError = LWCA_ERRNO_TO_LWCAERROR(errno);
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

    if (pfDest)
    {
        fclose(pfDest);
        pfDest = NULL;
    }

    if(chmod(pszDest,S_IRUSR | S_IWUSR) != 0)
    {
        dwError = LWCA_ERRNO_TO_LWCAERROR(errno);
        BAIL_ON_LWCA_ERROR(dwError);
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
