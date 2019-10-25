/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

#include "vmincludes.h"

static int      _gPipeFd[2] = {0};

static uid_t    _gLightwaveUid = 0;
static gid_t    _gLightwaveGid = 0;

/*
 * fork child and make it a daemon process
 */
DWORD
VmDaemon(
    VOID
    )
{
    DWORD   dwError = 0;
    pid_t   cpid = -1;
    CHAR    pipeData = 0;
    int     pipeRet = 0;
    int     devNullFd = -1;
    int     i = 0;


    if (pipe2(_gPipeFd, 0) != 0)
    {
        dwError = errno;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    cpid = fork();

    if (cpid < 0)
    {   // fork fail, return error to parent.
        dwError = errno;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else if (cpid > 0)
    {   // parent process
        close(_gPipeFd[1]);  // close write pipe

        // wait for child daemon reach ready state
        do
        {
            pipeRet = read(_gPipeFd[0], &pipeData, sizeof(pipeData));
        } while (pipeRet < 0 && errno == EINTR);

        exit(0);
    }
    else
    {   // child process, daemonize itself

        close(_gPipeFd[0]);  // close read pipe

        if (setsid() == -1)  // become its own process group
        {
            BAIL_WITH_VM_COMMON_ERROR(dwError, errno);
        }

        if (chdir("/") == -1) // set home dir
        {
            BAIL_WITH_VM_COMMON_ERROR(dwError, errno);
        }

        if ((devNullFd = open("/dev/null", O_RDWR)) == -1)
        {
            BAIL_WITH_VM_COMMON_ERROR(dwError, errno);
        }

        // point standard i/o to /dev/null
        // STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO
        for (i = 0; i <= 2; i++)
        {
            if (dup2(devNullFd, i) < 0)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, errno);
            }
        }

        close(devNullFd);

        umask(0022);
    }

error:

    return dwError;
}

/*
 * Daemon process call this to notify forking parent it is ready for business
 */
DWORD
VmDaemonReady(
    VOID
    )
{
    DWORD   dwError = 0;
    CHAR    pipeData = 1;
    int     pipeRet = 0;

    do
    {
        pipeRet = write(_gPipeFd[1], &pipeData, sizeof(pipeData));

    } while (pipeRet != sizeof(pipeData) && errno == EINTR);

    if (pipeRet != sizeof(pipeData))
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, errno);
    }

    close(_gPipeFd[1]);

error:
    return dwError;
}

DWORD
VmGetLWUserGroupId(
    VOID
    )
{
    DWORD   dwError = 0;
    int     iStatus = 0;
    struct  passwd pwd = { 0 };
    struct  passwd *pResult = NULL;
    size_t  ibufsize = 0;
    CHAR*   pBuffer = NULL;

    // LW services runs a Lightwave user/group
    // Root user can run tools but need to keep the file r/w for Lightwave user
    if (getuid() == 0 && _gLightwaveUid == 0)
    {
        ibufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (ibufsize == -1)
        {
            ibufsize = VM_MAX_GWTPWR_BUF_LENGTH;
        }

        dwError = VmAllocateMemory(
            ibufsize,
            (PVOID*)&pBuffer);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        iStatus = getpwnam_r(
            VM_LIGHTWAVE_USER,
            &pwd,
            pBuffer,
            ibufsize,
            &pResult);
        if (iStatus || !pResult)
        {
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_NO_SUCH_USER);
        }

        _gLightwaveUid = pwd.pw_uid;
        _gLightwaveGid = pwd.pw_gid;
    }

error:
    if (pBuffer)
    {
        VM_COMMON_SAFE_FREE_MEMORY(pBuffer);
    }
    return dwError;
}

/*
 * as root, change file ownership to lightwave user and group.
 */
DWORD
VmSetLWOwnership(
    PCSTR       pszName
    )
{
    DWORD       dwError = 0;

    if (getuid() == 0)
    {
        if (_gLightwaveUid == 0)
        {
            dwError = VmGetLWUserGroupId();
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }

        if (chown(pszName, _gLightwaveUid, _gLightwaveGid) != 0)
        {
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_CHANGE_OWNER);
        }
    }

error:
    return dwError;
}
