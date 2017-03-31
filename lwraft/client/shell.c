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



/*
 * Module Name: common
 *
 * Filename: shell.c
 *
 * Abstract:
 *
 * shell commands
 *
 */

#include "includes.h"

DWORD
VmDirStartService(
    VOID
    )
{
    return VmDirRun(VMDIR_START_SERVICE);
}

DWORD
VmDirStopService(
    VOID
    )
{
    return VmDirRun(VMDIR_STOP_SERVICE);
}

DWORD
VmDirCleanupData(
    VOID
    )
{
#ifndef _WIN32

    return VmDirRun(VMDIR_CLEANUP_DATA);

#else

    DWORD dwError = 0;
    PSTR  pszPath[MAX_PATH+1] = {0};
    PSTR  pszCmd = NULL;

    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_SOFTWARE_KEY_PATH,
                                        VMDIR_REG_KEY_DATA_PATH,
                                        (PSTR)pszPath,
                                        MAX_PATH );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirAllocateStringAVsnprintf( &pszCmd, "del /q \"%s\"", pszPath );
    BAIL_ON_VMDIR_ERROR(dwError)

    dwError = VmDirRun( pszCmd );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszCmd);

    return dwError;

error:
    goto cleanup;

#endif
}

DWORD
VmDirResetVmdir(
    VOID)
{
    DWORD dwError = 0;

    dwError = VmDirStopService();
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VMWare Directory Service stopped." );

    //Test for bug#1034595. On Windows, vmdird may fail to stop. Wait 2 seconds here.
    //Remove it if it's not useful
    VmDirSleep(2000);

    dwError = VmDirCleanupData();
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VMWare Directory Service database removed." );

    dwError = VmDirStartService();
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VMWare Directory Service ready for re-promotion." );

error:
    return dwError;
}

DWORD
VmKdcStartService(
    VOID
    )
{
    return VmDirRun(VMKDC_START_SERVICE);
}

DWORD
VmKdcStopService(
    VOID
    )
{
    return VmDirRun(VMKDC_STOP_SERVICE);
}

DWORD
VmDirGetVmDirLogPath(
    PSTR  pszPath,
    PCSTR pszLogFile)
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmDirStringCpyA(pszPath, MAX_PATH, LWRAFT_LOG_DIR);
    BAIL_ON_VMDIR_ERROR(dwError);
#else
    _TCHAR* programDataPath           = NULL;

    if ((dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_SOFTWARE_KEY_PATH, VMDIR_REG_KEY_LOG_PATH, pszPath,
                                        MAX_PATH )) != 0)
    {
       dwError = VmDirGetProgramDataEnvVar((_TCHAR *)"PROGRAMDATA", &programDataPath);
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringPrintFA(pszPath, MAX_PATH, "%s%s", programDataPath, "\\vmware\\cis\\logs\\lwraftd\\");
       BAIL_ON_VMDIR_ERROR(dwError);
    }
#endif

    dwError = VmDirStringCatA(pszPath, MAX_PATH, pszLogFile);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
#ifdef _WIN32
    VMDIR_SAFE_FREE_MEMORY(programDataPath);
#endif
    return dwError;
error:
    VmDirLog(LDAP_DEBUG_ERROR, "VmDirGetVmDirLogPath failed with error (%u)\n", dwError);
    goto cleanup;
}
