/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
 * Module Name:  main.c
 *
 * Abstract: VMware Domain Name Service.
 *
 * Created on: Sep 18, 2012
 *
 * Author: Sanjay Jain (sanjain@vmware.com)
 */

#include "includes.h"

#ifndef _WIN32

//TODO, move to gVmdnsGlobals?
int  vmdns_syslog_level = 0;
int  vmdns_syslog = 0;
int  vmdns_debug = 0;

static
DWORD
VmDnsNotifyLikewiseServiceManager();

int
main(
   int     argc,
   char  * argv[])
{
    DWORD       dwError = 0;
    int         logLevel = 0;
    PCSTR       pszLogFile = NULL;
    BOOLEAN     bEnableSysLog = FALSE;

    //dwError = VmDnsSrvUpdateConfig();
    //BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsParseArgs(
                    argc,
                    argv,
                    &logLevel,
                    &pszLogFile,
                    &gVmdnsGlobals.iListenPort,
                    &bEnableSysLog,
                    NULL);
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
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

    dwError = VmDnsNotifyLikewiseServiceManager();
    BAIL_ON_VMDNS_ERROR(dwError);

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

   return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsNotifyLikewiseServiceManager()
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

            VmDnsStringErrorA( buffer, BUFFER_SIZE, errorNumber );
            VmDnsLog( VMDNS_LOG_LEVEL_DEBUG,
                      "Could not notify service manager: %s (%i)",
                      buffer,
                      errorNumber);

            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMDNS_ERROR(dwError);
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
