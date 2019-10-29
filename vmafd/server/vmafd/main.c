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

//TODO, move to gVmafdGlobals?
int  vmafd_syslog_level = 0;
int  vmafd_syslog = 0;
int  vmafd_console_log = 0;

static
DWORD
_VmAfdInitVmRegConfig(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmRegConfigInit();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMDIR_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMAFD_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}


int
main(
   int     argc,
   char  * argv[])
{
    DWORD        dwError = 0;
    int          logLevel = VMAFD_DEBUG_ERROR;
    BOOLEAN      bEnableSysLog = FALSE;
    BOOLEAN      bEnableConsole = FALSE;
    BOOLEAN      bEnableDaemon = FALSE;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    umask(0);

    dwError = VmAfdParseArgs(
                    argc,
                    argv,
                    &logLevel,
                    &bEnableSysLog,
                    &bEnableConsole,
                    &bEnableDaemon);
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (bEnableDaemon)
    {
        dwError = VmDaemon();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /*
     * Block selected signals.  This must be done prior to creating
     * any threads since the signal mask is inherited.
     */
    VmAfdBlockSelectedSignals();

    dwError = _VmAfdInitVmRegConfig();
    BAIL_ON_VMAFD_ERROR(dwError);

    /*
     * Load the server configuration from the registry.
     * Note that this may create a new thread.
     */
    dwError = VmAfdSrvUpdateConfig(&gVmafdGlobals);
    BAIL_ON_VMAFD_ERROR(dwError);

    vmafd_syslog_level = logLevel;
    if( bEnableSysLog != FALSE )
    {
        vmafd_syslog = 1;
    }
    if (bEnableConsole)
    {
        vmafd_console_log = 1;
    }

    dwError = VmAfdInit();
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "ERROR: VmAfdInit failed (%d)\n",
                 dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bEnableDaemon)
    {
        dwError = VmDaemonReady();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /*
     * Create a dedicated thread to handle signals synchronously.
     */
    dwError = VmAfdInitSignalThread(&gVmafdGlobals);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdInitCertificateThread(&gVmafdGlobals.pCertUpdateThr);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdInitPassRefreshThread(&gVmafdGlobals.pPassRefreshThr);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcInitCdcService(&gVmafdGlobals.pCdcContext);
    BAIL_ON_VMAFD_ERROR(dwError);

#if 0
    //TODO: Comment out DDNS client code for now
    dwError = VmDdnsInitThread(&gVmafdGlobals.pDdnsContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdInitSourceIpThread(&gVmafdGlobals.pSourceIpContext);
    BAIL_ON_VMAFD_ERROR(dwError);
#endif
    VmAfdLog(VMAFD_DEBUG_ANY, "vmafdd: started!" );
    /*
     * Start the init loop which initializes configuration and
     * then waits until signaled to reinitialize.  It returns
     * when shutting down.
     */
    dwError = VmAfdInitLoop(&gVmafdGlobals);
    BAIL_ON_VMAFD_ERROR(dwError);


cleanup:

   VmAfdLog(VMAFD_DEBUG_ANY, "vmafdd: stop");
   VmAfdServerShutdown();
   VmAfdLogTerminate();
   VmRegConfigFree();

   return dwError;

error:

    goto cleanup;
}
