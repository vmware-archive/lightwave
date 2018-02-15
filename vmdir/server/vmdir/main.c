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

#ifndef _WIN32

static
DWORD
VmDirNotifyLikewiseServiceManager(
    VOID
);

static
DWORD
VmDirSetEnvironment(
    VOID
);

int
main(
   int     argc,
   char  * argv[])
{
    DWORD        dwError = 0;
    const char * logFileName = NULL;
    const char * pszBootstrapSchemaFile = NULL;
    const char * pszStateDir = VMDIR_DB_DIR VMDIR_PATH_SEPARATOR_STR;
    BOOLEAN      bEnableSysLog = FALSE;
    BOOLEAN      bConsoleMode = FALSE;
    BOOLEAN      bPatchSchema = FALSE;
    int          iLocalLogMask = 0;
    BOOLEAN      bVmDirInit = FALSE;
    BOOLEAN      bShutdownKDCService = FALSE;
    BOOLEAN      bWaitTimeOut = FALSE;

    dwError = VmDirSrvUpdateConfig();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirParseArgs(
                    argc,
                    argv,
                    &pszBootstrapSchemaFile,
                    &iLocalLogMask,
                    &logFileName,
                    &bEnableSysLog,
                    &bConsoleMode,
                    &bPatchSchema);
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(
            pszBootstrapSchemaFile,
            &gVmdirGlobals.pszBootStrapSchemaFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVmdirGlobals.bPatchSchema = bPatchSchema;

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


    if ( ! bPatchSchema && VmDirdGetTargetState() != VMDIRD_STATE_RESTORE )
    {   // Normal server startup route

        dwError = VmKdcServiceStartup();
        BAIL_ON_VMDIR_ERROR(dwError);
        bShutdownKDCService = TRUE;

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmkdcd: running...");

        dwError = VmDirNotifyLikewiseServiceManager();
        BAIL_ON_VMDIR_ERROR(dwError);

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

done:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDirNotifyLikewiseServiceManager()
{
    DWORD dwError = ERROR_SUCCESS;
    PCSTR   pszSmNotify = NULL;
    int  ret = 0;
    int  notifyFd = -1;
    char notifyCode = 0;

    // interact with likewise service manager (start/stop control)
    if ((pszSmNotify = getenv("LIKEWISE_SM_NOTIFY")) != NULL)
    {
        notifyFd = atoi(pszSmNotify);

        do
        {
            ret = write(notifyFd, &notifyCode, sizeof(notifyCode));

        } while (ret != sizeof(notifyCode) && errno == EINTR);

        if (ret < 0)
        {
#define BUFFER_SIZE 1024
            char buffer[BUFFER_SIZE]= {0};
            int errorNumber = errno;

            VmDirStringErrorA( buffer, BUFFER_SIZE, errorNumber );
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                      "Could not notify service manager: %s (%i)",
                      buffer,
                      errorNumber);

            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMDIR_ERROR(dwError);
#undef BUFFER_SIZE
        }

    }

error:
    if(notifyFd != -1)
    {
        close(notifyFd);
    }

    return dwError;
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

#endif
