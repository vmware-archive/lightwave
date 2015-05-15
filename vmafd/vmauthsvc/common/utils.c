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
VmAuthsvcFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound)
{
    DWORD dwError = 0;
    BOOLEAN bFound = FALSE;
    struct stat statBuf = {0};
    int iRetVal = 0;

    BAIL_ON_VMAUTHSVC_INVALID_POINTER(pszFileName, dwError);

    iRetVal = stat(pszFileName, &statBuf);
    if (iRetVal == 0 && S_ISREG(statBuf.st_mode))
    {
        bFound = TRUE;
    }

    *pbFound = bFound;

error:

    return dwError;
}

DWORD
VmAuthsvcGetHostName(
    PSTR* ppszHostName
)
{
    DWORD dwError = ERROR_SUCCESS;
    char hostBuf[HOST_NAME_MAX];
    DWORD dwBufLen = sizeof(hostBuf) - 1;
    PSTR pszHostName = NULL;

    if (gethostname(hostBuf, dwBufLen) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
    }

    dwError = VmAuthsvcAllocateStringA(hostBuf, &pszHostName);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    *ppszHostName = pszHostName;

error:

    return dwError;
}

VOID
VmAuthsvcSleep(
    DWORD dwMilliseconds
)
{
    struct timespec ts = {0};

    ts.tv_sec = dwMilliseconds / 1000;
    ts.tv_nsec = (dwMilliseconds % 1000) * 1000000;

    nanosleep(&ts, NULL);
}
