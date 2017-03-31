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

DWORD
EventLogFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound)
{
    DWORD dwError = 0;
    BOOLEAN bFound = FALSE;
    WIN32_FIND_DATAA FindFileData = {0};
    HANDLE handle = NULL;

    BAIL_ON_VMEVENT_INVALID_POINTER(pszFileName, dwError);

    handle = FindFirstFileA(pszFileName, &FindFileData) ;
    if (handle != INVALID_HANDLE_VALUE)
    {
        bFound = TRUE;
        FindClose(handle);
    }

    *pbFound = bFound;

error:

    return dwError;
}

DWORD
EventLogGetHostName(
    PSTR* ppszHostName
)
{
    DWORD dwError = ERROR_SUCCESS;
    char hostBuf[NI_MAXHOST+1];
    DWORD dwBufLen = sizeof(hostBuf) - 1;
    PSTR pszHostName = NULL;

    /*
     * MSDN:
     * If no error occurs, gethostname returns zero.
     * Otherwise, it returns SOCKET_ERROR and a specific error code
     * can be retrieved by calling WSAGetLastError.
     */
    if (gethostname(hostBuf, dwBufLen) != 0)
    {
        dwError = WSAGetLastError();
    }

    dwError = EventLogAllocateStringA(hostBuf, &pszHostName);
    BAIL_ON_VMEVENT_ERROR(dwError);

    *ppszHostName = pszHostName;

error:

    return dwError;
}
