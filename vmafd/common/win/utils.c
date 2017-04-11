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
VmAfdFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound)
{
    DWORD dwError = 0;
    BOOLEAN bFound = FALSE;
    WIN32_FIND_DATAA FindFileData = {0};
    HANDLE handle = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszFileName, dwError);

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
VmAfdGetFileSize(
    PCSTR   pszFilePath,
    size_t* pFileSize
    )
{
    DWORD dwError = 0;
    HANDLE hFile = NULL;
    LARGE_INTEGER fileSize = {0};

    hFile = CreateFileA(
                pszFilePath,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_READONLY,
                NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!GetFileSizeEx(hFile, &fileSize))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pFileSize = fileSize.QuadPart;

cleanup:

    if (hFile)
    {
        CloseHandle(hFile);
    }

    return dwError;

error:

    *pFileSize = 0;

    goto cleanup;
}

DWORD
VmAfdGetHostName(
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

    dwError = VmAfdAllocateStringA(hostBuf, &pszHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHostName = pszHostName;

error:

    return dwError;
}

VOID
VmAfdSleep(
    DWORD dwMilliseconds
)
{
    Sleep(dwMilliseconds);
}

VOID
VmAfdReadString(
    PCSTR szPrompt,
    PSTR szString,
    int len,
    BOOLEAN bHideString
    )
{
    DWORD oldMode = 0;
    HANDLE hConIn = INVALID_HANDLE_VALUE;
    PSTR pszNl = NULL;

    if (bHideString)
    {
        hConIn = GetStdHandle(STD_INPUT_HANDLE);
        if (hConIn != INVALID_HANDLE_VALUE)
        {
            if (GetConsoleMode(hConIn, &oldMode))
            {
                SetConsoleMode(hConIn, oldMode & ~ENABLE_ECHO_INPUT);
            }
        }
    }

    if (szPrompt)
    {
        fputs(szPrompt, stderr);
        fflush(stderr);
    }

    if (fgets(szString, len, stdin) == NULL)
    {
        szString[0] = '\0';
    }
    else
    {
        pszNl = VmAfdStringChrA(szString, '\n');
        if (pszNl)
        {
            *pszNl = '\0';
        }
    }

    if (bHideString)
    {
        fputs("\n", stderr);
        SetConsoleMode(hConIn, oldMode);
    }
    fflush(stderr);
}

DWORD
VmAfdGetTickCount()
{
    return GetTickCount();
}

DWORD
VmAfdGetProcessName(
    DWORD pid,
    PSTR *ppszName)
{
    DWORD dwError = 0;
    PSTR pszName = NULL;
    char buf[1024] = { 0 };
    HANDLE hProcess = NULL;

    if (!ppszName)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    hProcess = OpenProcess(
                        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                        FALSE,
                        pid);
    if (!hProcess)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if(!GetProcessImageFileNameA(
                        hProcess,
                        buf,
                        sizeof(buf)));
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringA(
                      buf,
                      (PSTR*)&pszName);
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszName = pszName;

cleanup:
    if (hProcess)
    {
        CloseHandle(hProcess);
    }
    return dwError;

error:
    if (ppszName)
    {
        *ppszName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszName);
    goto cleanup;
}
