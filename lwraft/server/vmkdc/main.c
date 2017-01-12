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

//TODO, move to gVmkdcGlobals?
int  ldap_syslog_level = 0;
int  ldap_syslog = 0;
int  slap_debug = 0;

static
DWORD
VmKdcNotifyLikewiseServiceManager();

int
main(
   int     argc,
   char  * argv[])
{
    DWORD        dwError = 0;
    int          logLevel = 0;
    BOOLEAN      bEnableSysLog = FALSE;
    BOOLEAN      bEnableConsole = FALSE;

    /*
     * Block selected signals.  This must be done prior to creating
     * any threads since the signal mask is inherited.
     */
    VmKdcBlockSelectedSignals();

    /*
     * Load the server configuration from the registry.
     * Note that this may create a new thread.
     */
    dwError = VmKdcSrvUpdateConfig(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcParseArgs(
                    argc,
                    argv,
                    &logLevel,
                    &bEnableSysLog,
                    &bEnableConsole);
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    ldap_syslog_level = slap_debug = logLevel; // Used by lber too
    if( bEnableSysLog != FALSE )
    {
        ldap_syslog = 1;
    }

    dwError = VmKdcInit();
    if (dwError)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ERROR: vmkdc VmKdcInit failed (%d)",
                 dwError);
    }
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcNotifyLikewiseServiceManager();
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Create a dedicated thread to handle signals synchronously.
     */
    dwError = VmKdcInitSignalThread(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "vmkdcd: started!");

    /*
     * Start the init loop which initializes the directory and
     * then waits until signaled to reinitialize.  It returns
     * when shutting down.
     */
    dwError = VmKdcInitLoop(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

cleanup:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Vmkdcd: stop");
    VmKdcShutdown();

    return dwError;

error:
    goto cleanup;
}

DWORD
VmKdcInitKdcServiceThread(
    PVMKDC_GLOBALS pGlobals)
{
    DWORD dwError = 0;
    int   sts = 0;
    void*(*pThrFn)(void*) = (void*(*)(void*))VmKdcInitLoop;

    sts = pthread_create(
            &pGlobals->thread,
            NULL,
            //((PVOID)(*)(PVOID))VmKdcInitLoop,
            pThrFn,
            pGlobals);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
VmKdcServiceStartup(
    VOID
    )
{
    DWORD    dwError = 0;

    /*
     * Load the server configuration from the registry.
     * Note that this may create a new thread.
     */
    dwError = VmKdcSrvUpdateConfig(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcInit();
    if (dwError)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ERROR: vmkdc VmKdcInit failed (%d)",
                 dwError);
    }
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcInitKdcServiceThread(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcSrvInit");

cleanup:

    return dwError;

error:
    goto cleanup;
}

VOID
VmKdcServiceShutdown(
    VOID
    )
{
    VmKdcdStateSet(VMKDC_SHUTDOWN);
    VmKdcShutdown();

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Vmkdcd: stop");

    return;
}

static
DWORD
VmKdcNotifyLikewiseServiceManager()
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

            VmKdcStringErrorA( buffer, BUFFER_SIZE, errorNumber );
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                      "Could not notify service manager: %s (%i)",
                      buffer,
                      errorNumber);

            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMKDC_ERROR(dwError);
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

#endif
