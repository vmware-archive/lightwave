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

static
DWORD
VmDirSetEnvironment(
    VOID
);

static
DWORD
_VmDirInitVmRegConfig(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmRegConfigInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMDIR_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMAFD_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

int
main(
   int     argc,
   char  * argv[])
{
    DWORD        dwError = 0;
    const char * logFileName = NULL;
    const char * pszBootstrapSchemaFile = VMDIR_CONFIG_PATH "/vmdirschema.ldif";
    const char * pszStateDir = VMDIR_DB_DIR VMDIR_PATH_SEP;
    BOOLEAN      bEnableSysLog = FALSE;
    BOOLEAN      bConsoleMode = FALSE;
    BOOLEAN      bDaemonMode = FALSE;
    int          iLocalLogMask = 0;
    BOOLEAN      bVmDirInit = FALSE;
    BOOLEAN      bShutdownKDCService = FALSE;
    BOOLEAN      bWaitTimeOut = FALSE;


    dwError = VmDirParseArgs(
                    argc,
                    argv,
                    &bDaemonMode,
                    &iLocalLogMask,
                    &logFileName,
                    &bEnableSysLog,
                    &bConsoleMode);
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (bDaemonMode)
    {
        dwError = VmDaemon();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirInitVmRegConfig();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvUpdateConfig();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            pszBootstrapSchemaFile,
            &gVmdirGlobals.pszBootStrapSchemaFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszStateDir, &gVmdirGlobals.pszBDBHome);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInitialize( logFileName, bEnableSysLog, "vmdird", VMDIR_LOG_INFO, iLocalLogMask);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirdStateSet(VMDIRD_STATE_STARTUP);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: starting...");

    VmDirBlockSelectedSignals();

    dwError = VmDirSetEnvironment();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInit();
    BAIL_ON_VMDIR_ERROR(dwError);
    bVmDirInit = TRUE;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmkdcd: starting...");


    if (VmDirdGetTargetState() != VMDIRD_STATE_RESTORE)
    {   // Normal server startup route

        dwError = VmKdcServiceStartup();
        BAIL_ON_VMDIR_ERROR(dwError);
        bShutdownKDCService = TRUE;

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmkdcd: running...");

        if (bDaemonMode)
        {
            dwError = VmDaemonReady();
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        VmDirdStateSet( VmDirdGetTargetState() );
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                        "Lotus Vmdird: running... state (%d)",
                        VmDirdState());

        // main thread waits on signals
        dwError = VmDirHandleSignals();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: exiting..." );

cleanup:

    if ( bShutdownKDCService )
    {
        VmKdcServiceShutdown();
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmkdcd: stop" );
    }

    if ( bVmDirInit )
    {
        VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);
        VmDirShutdown(&bWaitTimeOut);
        if (bWaitTimeOut)
        {
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: stop" );
            goto done;
        }

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: stop" );
    }

    VmDirLogTerminate();

    VmDirSrvFreeConfig();

    VmRegConfigFree();
done:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDirSetEnvironment(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszKrb5Conf = NULL;

    dwError = VmDirRegReadKrb5Conf(&pszKrb5Conf);
    if (dwError) {
        dwError = VmDirAllocateStringA(VMDIR_DEFAULT_KRB5_CONF,
                                       &pszKrb5Conf);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (-1 == setenv("KRB5_CONFIG", pszKrb5Conf, 1))
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszKrb5Conf);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetEnvironment failed (%u)", dwError);

    goto cleanup;
}
