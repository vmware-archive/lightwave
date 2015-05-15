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

//TODO, move to gVmauthsvcGlobals?
int  vmauthsvc_syslog_level = 0;
int  vmauthsvc_syslog = 0;
int  vmauthsvc_debug = 0;

static
DWORD
VmAuthsvcNotifyLikewiseServiceManager();

int
main(
   int     argc,
   char  * argv[])
{
    DWORD        dwError = 0;
    int          logLevel = 0;
    const char * logFileName = NULL;
    BOOLEAN      bEnableSysLog = FALSE;

    dwError = VmAuthsvcSrvUpdateConfig();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = VmAuthsvcParseArgs(
                    argc,
                    argv,
                    &logLevel,
                    &logFileName,
                    &gVmauthsvcGlobals.iListenPort,
                    &bEnableSysLog);
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    vmauthsvc_syslog_level = logLevel;
    if( bEnableSysLog != FALSE )
    {
        vmauthsvc_syslog = 1;
    }

    dwError = VmAuthsvcAllocateStringA(
            logFileName,
            &gVmauthsvcGlobals.pszLogFile);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = VmAuthsvcLogInitialize( gVmauthsvcGlobals.pszLogFile );
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    VmAuthsvcLog( VMAUTHSVC_DEBUG_TRACE, "Vmauthsvcd: start" );

    VmAuthsvcBlockSelectedSignals();

    dwError = VmAuthsvcInit();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = VmAuthsvcNotifyLikewiseServiceManager();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    VmAuthsvcdStateSet(VMAUTHSVCD_RUNNING);

    // main thread waits on signals
    dwError = VmAuthsvcHandleSignals();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    VmAuthsvcLog( VMAUTHSVC_DEBUG_TRACE, "Vmauthsvcd exiting..." );

cleanup:

   VmAuthsvcdStateSet(VMAUTHSVC_SHUTDOWN);

   VmAuthsvcShutdown();

   VmAuthsvcLog( VMAUTHSVC_DEBUG_TRACE, "Vmauthsvcd: stop" );

   VmAuthsvcLogTerminate();

   return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAuthsvcNotifyLikewiseServiceManager()
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

            VmAuthsvcStringErrorA( buffer, BUFFER_SIZE, errorNumber );
            VmAuthsvcLog( VMAUTHSVC_DEBUG_TRACE,
                      "Could not notify service manager: %s (%i)",
                      buffer,
                      errorNumber);

            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMAUTHSVC_ERROR(dwError);
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
