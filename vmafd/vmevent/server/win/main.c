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

void VMEventLog(
                int level,
                const char *fmt,
                ...)
{
}

static
VOID
WINAPI
VMEventConsoleMain(
    DWORD   argc,
    _TCHAR *targv[]
);

static
VOID
WINAPI
VMEventServiceMain(
    DWORD argc,
    _TCHAR *argv[]
);

static
DWORD
VMEventUpdateSCMStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    DWORD dwCurrentState,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    );

static
DWORD
WINAPI
VMEventServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    );

int 
_tmain(int argc, _TCHAR* targv[])
{
    DWORD   dwError = ERROR_SUCCESS;
    SERVICE_TABLE_ENTRY VMEventServiceTable[] =
    {
        {VMEVENT_NT_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)VMEventServiceMain},
        {NULL, NULL}
    };
    BOOLEAN bConsoleMode = FALSE;
    PSTR*   allocArgv = NULL;
    PSTR*   argv = NULL;

    dwError = EventLogAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMEVENT_ERROR(dwError);
    argv = allocArgv;

    dwError = EventLogParseArgs(
        argc,
        argv,
        &bConsoleMode
    );
    if(dwError != ERROR_SUCCESS)
    {
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

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
    if (bConsoleMode)
    {
        VMEventConsoleMain(argc, targv);
    }
    else
    {
        if( !StartServiceCtrlDispatcher(VMEventServiceTable))
        {
            dwError = GetLastError();
            printf("%d\n", dwError);

            BAIL_ON_VMEVENT_ERROR( dwError );
        }
    }

error:
    EventLogDeallocateArgsA(argc, allocArgv);

    return dwError;
}

static
VOID
WINAPI
VMEventConsoleMain(
    DWORD argc,
    _TCHAR *targv[]
)
{
    DWORD       dwThreadId = 0;
    DWORD       dwError = ERROR_SUCCESS;
    WSADATA     wsaData = {0};
    BOOLEAN     bWsaStartup = FALSE;

    VMEVENT_NTSERVICE_DATA vmDirNtServiceData =
    {
        NULL,
        NULL
    };

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMEVENT_ERROR(dwError);
    bWsaStartup = TRUE;

    dwError = EventLogInitialize();
    BAIL_ON_VMEVENT_ERROR(dwError);

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
    vmDirNtServiceData.stopServiceEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if ( WaitForSingleObject(vmDirNtServiceData.stopServiceEvent, INFINITE) == WAIT_FAILED )
    {
        dwError = GetLastError();
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

cleanup:

    EventLogShutdown();

    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }

    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMEVENT_CLOSE_HANDLE( vmDirNtServiceData.stopServiceEvent );

error:

    goto cleanup;
}

static
VOID
WINAPI
VMEventServiceMain(
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
    VMEVENT_NTSERVICE_DATA vmEventNtServiceData =
    {
        NULL,
        NULL
    };

    vmEventNtServiceData.hServiceStatus = RegisterServiceCtrlHandlerEx(
        VMEVENT_NT_SERVICE_NAME,
        (LPHANDLER_FUNCTION_EX)VMEventServiceCtrlHandlerEx,
        &vmEventNtServiceData
    );
    if (!vmEventNtServiceData.hServiceStatus)
    {
        dwError = GetLastError();
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    dwError = VMEventUpdateSCMStatus(
        vmEventNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 1, 8000
    );
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = VMEventUpdateSCMStatus(
        vmEventNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 2, 5000
    );
    BAIL_ON_VMEVENT_ERROR(dwError);

    vmEventNtServiceData.stopServiceEvent =
        CreateEvent(NULL, TRUE, FALSE, NULL);

    if (vmEventNtServiceData.stopServiceEvent == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    dwError = VMEventUpdateSCMStatus(
        vmEventNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 3, 1000
    );
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMEVENT_ERROR(dwError);
    bWsaStartup = TRUE;

    dwError = EventLogInitialize();
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = VMEventUpdateSCMStatus(
        vmEventNtServiceData.hServiceStatus, SERVICE_RUNNING, 0, 0, 0
    );
    BAIL_ON_VMEVENT_ERROR(dwError);


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
    if ( WaitForSingleObject(
            vmEventNtServiceData.stopServiceEvent, INFINITE
         ) == WAIT_FAILED )
    {
        dwError = GetLastError();
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

cleanup:

    EventLogShutdown();

    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }

    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMEVENT_CLOSE_HANDLE( vmEventNtServiceData.stopServiceEvent );

    if( vmEventNtServiceData.hServiceStatus )
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
            vmEventNtServiceData.hServiceStatus, &stoppedServiceStatus
        );
    }

error:

    goto cleanup;
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
        VMEventUpdateSCMStatus(
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
VMEventServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    )
{
    DWORD dwError = 0;
    PVMEVENT_NTSERVICE_DATA serviceData = NULL;

    BAIL_ON_VMEVENT_INVALID_POINTER( lpContext, dwError );
    serviceData = (PVMEVENT_NTSERVICE_DATA)lpContext;

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
            VMEventUpdateSCMStatus(
                serviceData->hServiceStatus, SERVICE_RUNNING, 0, 0, 0
            );
            break;
    }

error:

    return dwError;
}

static
DWORD
VMEventUpdateSCMStatus(
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
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

error:

    return dwError;
}
