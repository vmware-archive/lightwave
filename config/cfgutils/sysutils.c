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

DWORD
VmwDeployGetHostname(
    PSTR* ppszHostname
    )
{
    DWORD dwError = 0;
    CHAR  szHostname[HOST_NAME_MAX + 1] = "";
    PSTR  pszHostname = NULL;

    if (gethostname(szHostname, sizeof(szHostname)-1) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringA(szHostname, &pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszHostname = pszHostname;

cleanup:

    return dwError;

error:

    *ppszHostname = NULL;

    goto cleanup;
}

BOOLEAN
VmwDeployHaveAdminRights(
    VOID
    )
{
    return (geteuid() == 0);
}

BOOLEAN
VmwDeployIsIPAddress(
    PCSTR pszIPAddr
    )
{
    return (VmwDeployIsIPV4Address(pszIPAddr) ||
            VmwDeployIsIPV6Address(pszIPAddr));
}

BOOLEAN
VmwDeployIsIPV4Address(
    PCSTR pszIPAddr
    )
{
    BOOLEAN bResult = FALSE;

    if (!IsNullOrEmptyString(pszIPAddr))
    {
        unsigned char buf[sizeof(struct in_addr)];

        bResult = (inet_pton(AF_INET, pszIPAddr, &buf[0]) == 1);
    }

    return bResult;
}

BOOLEAN
VmwDeployIsIPV6Address(
    PCSTR pszIPAddr
    )
{
    BOOLEAN bResult = FALSE;

    if (!IsNullOrEmptyString(pszIPAddr))
    {
        unsigned char buf[sizeof(struct in6_addr)];

        bResult = (inet_pton(AF_INET6, pszIPAddr, &buf[0]) == 1);
    }

    return bResult;
}

DWORD
VmwDeployWriteToFile(
    PCSTR pszContent,
    PCSTR pszDirPath,
    PCSTR pszFilename
    )
{
    DWORD dwError = 0;
    PSTR  pszPath = NULL;
    FILE* fp = NULL;
    size_t nWritten = 0;
    size_t nToWrite = 0;

    if (IsNullOrEmptyString(pszContent) ||
        IsNullOrEmptyString(pszDirPath) ||
        IsNullOrEmptyString(pszFilename))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringPrintf(
                    &pszPath,
                    "%s/%s",
                    pszDirPath,
                    pszFilename);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if ((fp = fopen(pszPath, "w")) == NULL)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    nToWrite = strlen(pszContent);

    nWritten = fwrite(pszContent, 1, nToWrite, fp);
    if (nWritten != nToWrite)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

cleanup:

    if (fp)
    {
        fclose(fp);
    }
    if (pszPath)
    {
        VmwDeployFreeMemory(pszPath);
    }

    return dwError;

error:

    goto cleanup;
}
