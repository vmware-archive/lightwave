/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

//TODO, move to gVmkdcGlobals?
int  ldap_syslog_level = 0;
int  ldap_syslog = 0;
int  slap_debug = 0;

static
VOID
WINAPI
VmKdcServiceMain(
    DWORD argc,
    _TCHAR *argv[]
);

static
DWORD
VmKdcUpdateSCMStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    DWORD dwCurrentState,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    );

static
DWORD
WINAPI
VmKdcServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    );

static
PVOID
VmKdcEventService(
    PVOID pInfo
    );

static
DWORD
VmKdcInitEventThread(
    PVMKDC_GLOBALS pGlobals,
    HANDLE hEvent
    );

int _kdctmain(int argc, _TCHAR* targv[])
{
    DWORD dwError = ERROR_SUCCESS;
    SERVICE_TABLE_ENTRY VmKdcServiceTable[] =
    {
        {VMKDC_NT_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)VmKdcServiceMain},
        {NULL, NULL}
    };
    BOOLEAN bEnableConsole = FALSE;
#ifdef _DEBUG
    int i = 1;
#endif

#ifdef _DEBUG
    /* Preparse the arguments to check for just the console option */
    while ( i < argc )
    {
        if ( VmKdcStringCompareA(VMKDC_OPTION_ENABLE_CONSOLE, targv[i], TRUE ) == 0 )
        {
            bEnableConsole = TRUE;
            break;
        }
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
        if( !StartServiceCtrlDispatcher(VmKdcServiceTable))
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR( dwError );
        }
    } 
    else
    {
        VmKdcServiceMain(argc, targv);
    }

error:

    return dwError;
}

static
VOID
WINAPI
VmKdcServiceMain(
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
    VMKDC_NTSERVICE_DATA vmKdcNtServiceData =
    {
        NULL,
        NULL
    };
    int     logLevel = 0;
    PCSTR   pszStateDir = /*VMKDC_DB_DIR*/ VMKDC_PATH_SEPARATOR_STR;
    BOOLEAN bEnableSysLog = FALSE;
    BOOLEAN bEnableConsole = FALSE;
    PSTR*   allocArgv = NULL;
    PSTR*   argv = NULL;

#ifdef UNICODE
    dwError = VmKdcAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMKDC_ERROR(dwError);
    argv = allocArgv;
#else
    argv = targv; // non-unicode => targv is char
#endif

    dwError = VmKdcSrvUpdateConfig(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcParseArgs(
        argc,
        argv,
        &logLevel,
        &bEnableSysLog,
        &bEnableConsole
    );
    if(dwError != ERROR_SUCCESS)
    {
//        ShowUsage( argv[0] );
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    ldap_syslog_level = slap_debug = logLevel; // Used by lber too
    if( bEnableSysLog != FALSE )
    {
        ldap_syslog = 1;
    }

    VmKdcDeallocateArgsA(argc, allocArgv);
    allocArgv = NULL;

    if (!bEnableConsole)
    {
        vmKdcNtServiceData.hServiceStatus = RegisterServiceCtrlHandlerEx(
            VMKDC_NT_SERVICE_NAME,
            (LPHANDLER_FUNCTION_EX)VmKdcServiceCtrlHandlerEx,
            &vmKdcNtServiceData
        );
        if (!vmKdcNtServiceData.hServiceStatus)
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        dwError = VmKdcUpdateSCMStatus(
            vmKdcNtServiceData.hServiceStatus, SERVICE_START_PENDING,
            0, 1, 8000
        );
        BAIL_ON_VMKDC_ERROR(dwError);

        dwError = VmKdcUpdateSCMStatus(
        vmKdcNtServiceData.hServiceStatus, SERVICE_START_PENDING,
            0, 2, 5000);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    vmKdcNtServiceData.stopServiceEvent =
        CreateEvent(NULL, TRUE, FALSE, NULL);
    if (vmKdcNtServiceData.stopServiceEvent == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (!bEnableConsole)
    {
        dwError = VmKdcUpdateSCMStatus(
            vmKdcNtServiceData.hServiceStatus, SERVICE_START_PENDING,
            0, 3, 1000
        );
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMKDC_ERROR(dwError);
    bWsaStartup = TRUE;

    // TODO: once we have backend ported etc. and can test etc.
    // we should switch VmKdcRpcServerInit() -> VmKdcInit()

    dwError = VmKdcInit();
    if (dwError)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ERROR: vmkdc VmKdcInit failed (%d)\n",
                 dwError);
    }
    BAIL_ON_VMKDC_ERROR(dwError);

    //dwError = VmKdcRpcServerInit();
    //BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Notify the service manager that vmkdc is running.
     */
    if (!bEnableConsole)
    {
        dwError = VmKdcUpdateSCMStatus(
            vmKdcNtServiceData.hServiceStatus, SERVICE_RUNNING, 0, 0, 0
        );
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Create a dedicated thread to handle the stop service event.
     */
    dwError = VmKdcInitEventThread(&gVmkdcGlobals,
                                   vmKdcNtServiceData.stopServiceEvent);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "vmkdcd: started!");

    /*
     * Start the init loop which initializes the directory and
     * then waits until signaled to reinitialize.  It returns
     * when shutting down.
     */
    dwError = VmKdcInitLoop(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

error:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Vmkdcd: stop");

    // TODO: once we have backend ported and can test etc.
    // and have the VmKdcInit() above,
    // we should switch     VmKdcRpcServerShutdown() -> VmKdcShutdown()
    //VmKdcShutdown();
    VmKdcRpcServerShutdown();

    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }

    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMKDC_CLOSE_HANDLE( vmKdcNtServiceData.stopServiceEvent );

    if( !bEnableConsole && vmKdcNtServiceData.hServiceStatus )
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
            vmKdcNtServiceData.hServiceStatus, &stoppedServiceStatus
        );
    }

    VmKdcDeallocateArgsA(argc, allocArgv);
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
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ERROR: vmkdc VmKdcInit failed (%d)\n",
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
    VmKdcdStateSet(VMKDC_STOPPING);
    VmKdcShutdown();

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Vmkdcd: stop");

    return;
}

static
PVOID
VmKdcEventService(
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
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    VmKdcdStateSet(VMKDC_STOPPING);

    return NULL;
}

static
DWORD
VmKdcInitEventThread(
    PVMKDC_GLOBALS pGlobals,
    HANDLE hEvent)
{
    DWORD dwError = 0;
    int   sts = 0;

    sts = pthread_create(
            &pGlobals->thread,
            NULL,
            VmKdcEventService,
            hEvent);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
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
        VmKdcUpdateSCMStatus(
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
VmKdcServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    )
{
    DWORD dwError = 0;
    PVMKDC_NTSERVICE_DATA serviceData = NULL;

    BAIL_ON_VMKDC_INVALID_POINTER( lpContext, dwError );
    serviceData = (PVMKDC_NTSERVICE_DATA)lpContext;

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
            VmKdcUpdateSCMStatus(
                serviceData->hServiceStatus, SERVICE_RUNNING, 0, 0, 0
            );
            break;
    }

error:

    return dwError;
}

static
DWORD
VmKdcUpdateSCMStatus(
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
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:

    return dwError;
}
