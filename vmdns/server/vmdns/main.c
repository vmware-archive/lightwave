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
 *
 * Module Name:  main.c
 *
 * Abstract: VMware Domain Name Service.
 */

#include "includes.h"

//TODO, move to gVmdnsGlobals?
int  vmdns_syslog_level = VMDNS_LOG_LEVEL_INFO;
int  vmdns_syslog = 0;
int  vmdns_debug = 0;

static
DWORD
VMDNSInitVmRegConfig(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmRegConfigInit();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMDIR_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMAFD_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMDNS_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

error:
    return dwError;
}

int
main(
   int     argc,
   char  * argv[])
{
    DWORD       dwError = 0;
    int         logLevel = VMDNS_LOG_LEVEL_INFO;
    PCSTR       pszLogFile = NULL;
    BOOLEAN     bEnableSysLog = FALSE;
    BOOLEAN     bEnableDaemon = FALSE;

    dwError = VmDnsParseArgs(
                    argc,
                    argv,
                    &logLevel,
                    &pszLogFile,
                    &gVmdnsGlobals.iListenPort,
                    &bEnableSysLog,
                    &bEnableDaemon,
                    NULL);
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (bEnableDaemon)
    {
        dwError = VmDaemon();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    vmdns_syslog_level = logLevel;
    if( bEnableSysLog != FALSE )
    {
        vmdns_syslog = 1;
    }

    if (pszLogFile)
    {
        dwError = VmDnsAllocateStringA(
                pszLogFile,
                &gVmdnsGlobals.pszLogFile);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VMDNSInitVmRegConfig();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigGetDword(VMDNS_KEY_VALUE_LOG_CAP, &gVmdnsGlobals.dwMaximumOldFiles);
    if (dwError)
    {
        gVmdnsGlobals.dwMaximumOldFiles = VMDNS_DEFAULT_LOG_CAP;
    }

    dwError = VmDnsConfigGetDword(VMDNS_KEY_VALUE_LOG_SIZE, &gVmdnsGlobals.dwMaxLogSizeBytes);
    if (dwError)
    {
        gVmdnsGlobals.dwMaxLogSizeBytes = VMDNS_DEFAULT_LOG_SIZE;
    }

    dwError = VmDnsLogInitialize(gVmdnsGlobals.pszLogFile,
                                gVmdnsGlobals.dwMaximumOldFiles,
                                gVmdnsGlobals.dwMaxLogSizeBytes);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsLog(VMDNS_LOG_LEVEL_INFO, "Vmdnsd: start" );

    VmDnsBlockSelectedSignals();

    dwError = VmDnsInit();
    BAIL_ON_VMDNS_ERROR(dwError);

    if (bEnableDaemon)
    {
        dwError = VmDaemonReady();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsdStateSet(VMDNSD_RUNNING);

    // main thread waits on signals
    dwError = VmDnsHandleSignals();
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsLog( VMDNS_LOG_LEVEL_INFO, "Vmdnsd exiting..." );

cleanup:

   VmDnsdStateSet(VMDNS_SHUTDOWN);

   VmDnsShutdown();

   VmDnsLog( VMDNS_LOG_LEVEL_INFO, "Vmdnsd: stop" );

   VmDnsLogTerminate();

   VmRegConfigFree();

   return dwError;

error:

    goto cleanup;
}
