/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
int  vmafd_debug = 0;

static
VOID
WINAPI
VmAfdServiceMain(
    DWORD argc,
    _TCHAR *argv[]
);

static
DWORD
VmAfdUpdateSCMStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    DWORD dwCurrentState,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    );

static
DWORD
WINAPI
VmAfdServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    );

static
PVOID
VmAfdEventService(
    PVOID pInfo
    );

static
DWORD
VmAfdInitEventThread(
    PVMAFD_GLOBALS pGlobals,
    HANDLE hEvent
    );

int
_tmain(int argc, _TCHAR* targv[])
{
    DWORD   dwError = ERROR_SUCCESS;
    SERVICE_TABLE_ENTRY VmAfdServiceTable[] =
    {
        {VMAFD_NT_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)VmAfdServiceMain},
        {NULL, NULL}
    };
    int     logLevel = 0;
    BOOLEAN bEnableConsole = FALSE;
#ifdef _DEBUG
    int     i = 0;
    PSTR    pszArg = NULL;
#endif

#ifdef _DEBUG
    /* Preparse the arguments to check for just the console option */
    for (i=1; i < argc; i++)
    {
        dwError = VmAfdAllocateStringAFromW(targv[i], &pszArg);
        BAIL_ON_VMAFD_ERROR(dwError);

        if ( VmAfdStringCompareA(VMAFD_OPTION_ENABLE_CONSOLE, pszArg, TRUE ) == 0 ||
             VmAfdStringCompareA(VMAFD_OPTION_ENABLE_CONSOLE_LONG, pszArg, TRUE ) == 0 )
        {
            bEnableConsole = TRUE;
            break;
        }
        VMAFD_SAFE_FREE_STRINGA(pszArg);
    }
#endif

    if (!bEnableConsole)
    {

        /*
        MSDN:
            When the service control manager starts a service process,
            it waits for the process to call the
            StartServiceCtrlDispatcher function.
            The main thread of a service process should make this call
            as soon as possible after it starts up (within 30 seconds).
            If StartServiceCtrlDispatcher succeeds, it connects the calling
            thread to the service control manager and does not return until
            all running services in the process have entered
            the SERVICE_STOPPED state.
        */
        if( !StartServiceCtrlDispatcher(VmAfdServiceTable))
        {
            dwError = GetLastError();
            BAIL_ON_VMAFD_ERROR( dwError );
        }
    }
    else
    {
        VmAfdServiceMain(argc, targv);
    }

error:

#ifdef _DEBUG
    VMAFD_SAFE_FREE_STRINGA(pszArg);
#endif

    return dwError;
}

static
VOID
WINAPI
VmAfdServiceMain(
    DWORD argc,
    _TCHAR *targv[]
)
{
    DWORD dwThreadId = 0;
    DWORD dwError = ERROR_SUCCESS;
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
    SERVICE_STATUS stoppedServiceStatus =
    {
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_STOPPED
    };
    VMAFD_NTSERVICE_DATA vmAfdNtServiceData =
    {
        NULL,
        NULL
    };
    int     logLevel = 0;
    BOOLEAN bEnableSysLog = FALSE;
    BOOLEAN bEnableConsole = FALSE;
    PSTR*   allocArgv = NULL;
    PSTR*   argv = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

#ifdef UNICODE
    dwError = VmAfdAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMAFD_ERROR(dwError);
    argv = allocArgv;
#else
    argv = targv; // non-unicode => targv is char
#endif

    dwError = VmAfCfgInit();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSrvUpdateConfig(&gVmafdGlobals);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdParseArgs(
        argc,
        argv,
        &logLevel,
        &bEnableSysLog,
        &bEnableConsole
    );
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    vmafd_syslog_level = vmafd_debug = logLevel;
    if( bEnableSysLog != FALSE )
    {
        vmafd_syslog = 1;
    }

    VmAfdDeallocateArgsA(argc, allocArgv);
    allocArgv = NULL;

    if (!bEnableConsole)
    {
        vmAfdNtServiceData.hServiceStatus = RegisterServiceCtrlHandlerEx(
            VMAFD_NT_SERVICE_NAME,
            (LPHANDLER_FUNCTION_EX)VmAfdServiceCtrlHandlerEx,
            &vmAfdNtServiceData
        );
        if (!vmAfdNtServiceData.hServiceStatus)
        {
            dwError = GetLastError();
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdUpdateSCMStatus(
            vmAfdNtServiceData.hServiceStatus, SERVICE_START_PENDING,
            0, 1, 8000
        );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdUpdateSCMStatus(
            vmAfdNtServiceData.hServiceStatus, SERVICE_START_PENDING,
            0, 2, 5000
        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    vmAfdNtServiceData.stopServiceEvent =
        CreateEvent(NULL, TRUE, FALSE, NULL);
    if (vmAfdNtServiceData.stopServiceEvent == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!bEnableConsole)
    {
        dwError = VmAfdUpdateSCMStatus(
            vmAfdNtServiceData.hServiceStatus, SERVICE_START_PENDING,
            0, 3, 1000
        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMAFD_ERROR(dwError);
    bWsaStartup = TRUE;

    // TODO: once we have backend ported etc. and can test etc.
    // we should switch VmAfdRpcServerInit() -> VmAfdInit()

    dwError = VmAfdInit();
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "ERROR: VmAfdInit failed (%d)\n",
                 dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    //dwError = VmAfdRpcServerInit();
    //BAIL_ON_VMAFD_ERROR(dwError);

    /*
     * Notify the service manager that vmafd is running.
     */
    if (!bEnableConsole)
    {
        dwError = VmAfdUpdateSCMStatus(
            vmAfdNtServiceData.hServiceStatus, SERVICE_RUNNING, 0, 0, 0
        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /*
     * Create a dedicated thread to handle the stop service event.
     */
    dwError = VmAfdInitEventThread(&gVmafdGlobals,
                                   vmAfdNtServiceData.stopServiceEvent);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdInitCertificateThread(&gVmafdGlobals.pCertUpdateThr);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdInitPassRefreshThread(&gVmafdGlobals.pPassRefreshThr);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcInitCdcService(&gVmafdGlobals.pCdcContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "vmafdd: started!");

    /*
     * Start the init loop which initializes configuration and
     * then waits until signaled to reinitialize.  It returns
     * when shutting down.
     */
    dwError = VmAfdInitLoop(&gVmafdGlobals);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "vmafdd: stop");

    VmAfdSrvSetStatus(VMAFD_STATUS_STOPPED);

    // TODO: once we have backend ported and can test etc.
    // and have the VmAfdInit() above,
    // we should switch     VmAfdRpcServerShutdown() -> VmAfdShutdown()
    //VmAfdShutdown();
    VmAfdRpcServerShutdown();

    if (!bEnableConsole)
    {
        VmAfdLogTerminate();
    }

    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }

    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMAFD_CLOSE_HANDLE( vmAfdNtServiceData.stopServiceEvent );


    if ( !bEnableConsole && vmAfdNtServiceData.hServiceStatus )
    {
        /*
        MSDN:
            If the status is SERVICE_STOPPED, perform all necessary cleanup
            and call SetServiceStatus one time only.
            This function makes an LRPC call to the SCM. The first call to
            the function in the SERVICE_STOPPED state closes
            the RPC context handle and any subsequent calls can cause
            the process to crash.

            Do not attempt to perform *any* additional work after calling
            SetServiceStatus with SERVICE_STOPPED,
            because the service process can be terminated at any time.
        */

        // calling the SetServiceStatus directly intentionally
        SetServiceStatus(
            vmAfdNtServiceData.hServiceStatus, &stoppedServiceStatus
        );
    }

    VmAfdDeallocateArgsA(argc, allocArgv);
}

static
PVOID
VmAfdEventService(
    PVOID pInfo)
{
    DWORD dwError = 0;
    HANDLE hEvent = (HANDLE)pInfo;

    /*
    MSDN:
        should perform clean-up tasks, including closing the global event,
        and call SetServiceStatus with SERVICE_STOPPED.
        After the service has stopped, you should not execute any additional
        service code because you can introduce a race condition if the service
        receives a start control and ServiceMain is called again.
        Note that this problem is more likely to occur when
        multiple services share a process.
    */
    if ( WaitForSingleObject(hEvent, INFINITE) == WAIT_FAILED )
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:
    VmAfdSrvSetStatus(VMAFD_STATUS_STOPPING);

    return NULL;
}

static
DWORD
VmAfdInitEventThread(
    PVMAFD_GLOBALS pGlobals,
    HANDLE hEvent)
{
    DWORD dwError = 0;
    int   sts = 0;

    sts = pthread_create(
            &pGlobals->thread,
            NULL,
            VmAfdEventService,
            hEvent);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}


static
VOID
StopService(
    SERVICE_STATUS_HANDLE hServiceStatus,
    HANDLE stopServiceEvent
    )
{
    DWORD dwError = ERROR_SUCCESS;

    /*
    MSDN:
        When the service is stopping, the service control handler should
        call SetServiceStatus with SERVICE_STOP_PENDING and signal this event.

        When the service control manager sends a control code to a service,
        it waits for the handler function to return before sending
        additional control codes to other services.
        The control handler should return as quickly as possible;
    */

    // check if the event is signalled or not
    // if it is signalled, we are a no-op
    if( WaitForSingleObject( stopServiceEvent, 0 ) != WAIT_OBJECT_0 )
    {
        VmAfdUpdateSCMStatus(
            hServiceStatus, SERVICE_STOP_PENDING, 0, 1, 5000
        );

        if ( !SetEvent(stopServiceEvent) )
        {
            dwError = GetLastError();
        }
    }
}

static
DWORD
WINAPI
VmAfdServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    )
{
    DWORD dwError = 0;
    PVMAFD_NTSERVICE_DATA serviceData = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER( lpContext, dwError );
    serviceData = (PVMAFD_NTSERVICE_DATA)lpContext;

    switch (dwControlCode)
    {
        /*
        MSDN:
            The SERVICE_CONTROL_SHUTDOWN control code should
            only be processed by services that must absolutely clean up
            during shutdown, because there is a limited time
            (about 20 seconds) available for service shutdown.
        */
        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:
            StopService(
                serviceData->hServiceStatus,
                serviceData->stopServiceEvent
            );
            break;
        case SERVICE_CONTROL_INTERROGATE:
        default:
            VmAfdUpdateSCMStatus(
                serviceData->hServiceStatus, SERVICE_RUNNING, 0, 0, 0
            );
            break;
    }

error:

    return dwError;
}

static
DWORD
VmAfdUpdateSCMStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    DWORD dwCurrentState,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    )
{
    DWORD dwError = NO_ERROR;
    SERVICE_STATUS currentServiceStatus = {0};

    currentServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    currentServiceStatus.dwCurrentState = dwCurrentState;

    if ( (dwCurrentState == SERVICE_START_PENDING)
         ||
         (dwCurrentState == SERVICE_STOP_PENDING))
    {
        currentServiceStatus.dwControlsAccepted = 0;
    }
    else
    {
        currentServiceStatus.dwControlsAccepted =
            SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN ;
    }

    currentServiceStatus.dwWin32ExitCode = NO_ERROR;
    currentServiceStatus.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
    currentServiceStatus.dwCheckPoint = dwCheckPoint;
    currentServiceStatus.dwWaitHint = dwWaitHint;

    if( !SetServiceStatus( hServiceStatus, &currentServiceStatus ) )
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}
