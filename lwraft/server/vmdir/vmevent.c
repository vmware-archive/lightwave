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


static
DWORD
_VmDirGetEventLogLibPath(
        PSTR*  ppszEventLogLibPath
        )
{
    DWORD   dwError = 0;
    CHAR    pszRegLibPath[VMDIR_MAX_PATH_LEN] = {0};
    PSTR    pszVmafdName = NULL;
    PSTR    pszEventLogLibPath = NULL;

#ifndef _WIN32
    dwError = VmDirGetRegKeyValue( VMAFD_CONFIG_KEY_ROOT,
                                   VMAFD_REG_KEY_PATH,
                                   pszRegLibPath,
                                   sizeof(pszRegLibPath)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszVmafdName = strstr(pszRegLibPath, VMAFD_NAME);
    if (pszVmafdName == NULL)
    {
        dwError = VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pszVmafdName[strlen(VMAFD_NAME)] = '\0';
    }
#else
    if (!GetSystemDirectory(pszRegLibPath, VMDIR_MAX_PATH_LEN))
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }
#endif

    dwError = VmDirAllocateStringPrintf( &pszEventLogLibPath, "%s%s",pszRegLibPath, VMEVENT_CLIENT_LIBRARY);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszEventLogLibPath = pszEventLogLibPath;
    pszEventLogLibPath = NULL;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszEventLogLibPath);
    goto cleanup;
}

DWORD
VmDirLoadEventLogLibrary(
        PFEVENTLOG_ADD *ppfEventLogAdd
        )
{
    DWORD dwError = 0;
    PSTR pszEventLogLibPath = NULL;
    VMDIR_LIB_HANDLE pEventLogLibHandle = NULL;
    PFEVENTLOG_ADD pfEventLogAdd = NULL;

    dwError = _VmDirGetEventLogLibPath(&pszEventLogLibPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLoadLibrary(pszEventLogLibPath, &pEventLogLibHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    pfEventLogAdd = (PFEVENTLOG_ADD)VmDirGetLibSym(pEventLogLibHandle, EVENTLOG_ADD);

    if (pfEventLogAdd == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppfEventLogAdd = pfEventLogAdd;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszEventLogLibPath);
    goto cleanup;
}
