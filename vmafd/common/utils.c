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

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

#include "includes.h"

DWORD
VmAfdFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound)
{
    DWORD dwError = 0;
    BOOLEAN bFound = FALSE;
    struct stat statBuf = {0};
    int iRetVal = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pszFileName, dwError);

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
VmAfdGetFileSize(
    PCSTR   pszFilePath,
    size_t* pFileSize
    )
{
    DWORD dwError = 0;
    struct stat statBuf = {0};

    BAIL_ON_VMAFD_INVALID_POINTER(pszFilePath, dwError);

    if (stat(pszFilePath, &statBuf) < 0)
    {
#ifndef _WIN32
        dwError = VmAfdGetWin32ErrorCode(errno);
#else
        dwError = GetLastError();
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pFileSize = statBuf.st_size;

cleanup:

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
    char hostBuf[HOST_NAME_MAX];
    DWORD dwBufLen = sizeof(hostBuf) - 1;
    PSTR pszHostName = NULL;

    if (gethostname(hostBuf, dwBufLen) < 0)
    {
        dwError = VmAfdGetWin32ErrorCode(errno);
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
    struct timespec ts = {0};

    ts.tv_sec = dwMilliseconds / 1000;
    ts.tv_nsec = (dwMilliseconds % 1000) * 1000000;

    nanosleep(&ts, NULL);
}

VOID
VmAfdReadString(
    PCSTR szPrompt,
    PSTR szString,
    int len,
    BOOLEAN bHideString
    )
{
    sigset_t sig, osig;
    struct termios ts, ots;
    PSTR pszNl = NULL;

    if (bHideString)
    {
        sigemptyset(&sig);
        sigaddset(&sig, SIGINT);
        sigaddset(&sig, SIGTSTP);
        sigprocmask(SIG_BLOCK, &sig, &osig);

        tcgetattr(fileno(stdin), &ts);
        ots = ts;
        ts.c_lflag &= ~(ECHO);
        tcsetattr(fileno(stdin), TCSANOW, &ts);
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

        tcsetattr(fileno(stdin), TCSANOW, &ots);
        sigprocmask(SIG_SETMASK, &osig, NULL);
    }
    fflush(stderr);
}

DWORD
VmAfdGetTickCount()
{
#ifndef __MACH__
    struct timespec ts = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "Error while getting the clock time - %d",
             errno);
    }
    return ts.tv_sec;
#else
    return (DWORD)time(NULL);
#endif
}

DWORD
VmAfdGetProcessName(
    DWORD pid,
    PSTR *ppszName)
{
    DWORD dwError = 0;
    PSTR pszName = NULL;
    char buf[1024];
    FILE *fp = NULL;
    size_t size = 0;

    if (!ppszName)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    sprintf(buf, "/proc/%d/cmdline", pid);
    fp = fopen(buf, "r");
    if (!fp)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    size = fread(buf, sizeof(char), sizeof(buf), fp);
    if (size > 0)
    {
        if (buf[size-1] == '\n')
        {
            buf[size-1]='\0';
        }
    }

    dwError = VmAfdAllocateStringA(
                      buf,
                      (PSTR*)&pszName);
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszName = pszName;

cleanup:
    if (fp)
    {
        fclose(fp);
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
