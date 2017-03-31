/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
 * Module Name:  main.c
 *
 * Abstract: VMware Certificate Service.
 *
 */

#include "..\includes.h"

VOID
WINAPI
VMCAServiceMain(
    DWORD argc,
    _TCHAR *argv[]
);

static
DWORD
VMCAUpdateSCMStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    DWORD dwCurrentState,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    );

static
DWORD
WINAPI
VMCAServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    );

static
VOID
WINAPI
VMCAConsoleMain(
    DWORD argc,
    _TCHAR *targv[]
)
{
    DWORD       dwError = ERROR_SUCCESS;
    HANDLE      hStopEvent = NULL;

    dwError = VMCAInitialize(FALSE, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!hStopEvent)
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if ( WaitForSingleObject(hStopEvent, INFINITE) == WAIT_FAILED )
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:

    VMCAShutdown();

    if (hStopEvent)
    {
        CloseHandle( hStopEvent );
    }

    return;

error:

    goto cleanup;
}



int _tmain(int argc, _TCHAR* targv[])
{
    DWORD dwError = 0;
    char* pszSmNotify = NULL;
    int notifyFd = -1;
    int notifyCode = 0;
    int ret = -1;
//    setlocale(LC_ALL, "");
    if ( argc > 2 ){

        VMCAConsoleMain(argc,targv);
        return 0;
    }


    SERVICE_TABLE_ENTRY VMCAServiceTable[] =
    {
        {(LPTSTR)VMCA_NT_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)VMCAServiceMain},
        {NULL, NULL}
    };

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
    if( !StartServiceCtrlDispatcher(VMCAServiceTable))
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR( dwError );
    }

cleanup:

    VMCAShutdown();

    return (dwError);

error:

    VMCA_LOG_ERROR("VM Certificate exiting due to error [code:%d]", dwError);

    goto cleanup;
}

static
VOID
WINAPI
VMCAServiceMain(
    DWORD argc,
    _TCHAR *targv[]
)
{
    DWORD dwThreadId = 0;
    DWORD dwError = ERROR_SUCCESS;

	SERVICE_STATUS stoppedServiceStatus =
    {
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_STOPPED
    };
    VMCA_NTSERVICE_DATA VMCANtServiceData =
    {
        NULL,
        NULL
    };

    VMCANtServiceData.hServiceStatus = RegisterServiceCtrlHandlerExA(
        (LPSTR)VMCA_NT_SERVICE_NAME,
        (LPHANDLER_FUNCTION_EX)VMCAServiceCtrlHandlerEx,
        &VMCANtServiceData
    );
    if (!VMCANtServiceData.hServiceStatus)
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAUpdateSCMStatus(
        VMCANtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 1, 8000
    );
    BAIL_ON_VMCA_ERROR(dwError);

    // TODO: init the log
    //dwError = VMCALogInitialize(gVMCAServerGlobals.pszLogFile);

    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAUpdateSCMStatus(
        VMCANtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 2, 5000
    );
    BAIL_ON_VMCA_ERROR(dwError);

    VMCANtServiceData.stopServiceEvent =
        CreateEvent(NULL, TRUE, FALSE, NULL);

    if (VMCANtServiceData.stopServiceEvent == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAUpdateSCMStatus(
        VMCANtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 3, 1000
    );
    BAIL_ON_VMCA_ERROR(dwError);

   
    // TODO: once we have backend ported etc. and can test etc.
    // we should switch VMCARpcServerInit() -> VMCAInit()

    dwError = VMCAInitialize(FALSE, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    //dwError = VMCARpcServerInit();
    //BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAUpdateSCMStatus(
        VMCANtServiceData.hServiceStatus, SERVICE_RUNNING, 0, 0, 0
    );
    BAIL_ON_VMCA_ERROR(dwError);

    VMCASrvSetState(VMCAD_RUNNING);

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
            VMCANtServiceData.stopServiceEvent, INFINITE
         ) == WAIT_FAILED )
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

error:

    VMCASrvSetState(VMCA_SHUTDOWN);

    // TODO: once we have backend ported and can test etc.
    // and have the VMCAInit() above,
    // we should switch     VMCARpcServerShutdown() -> VMCAShutdown()
    //VMCAShutdown();
    VMCARPCShutdown();

    // TODO:
    // add loging termination

    //VMCALogTerminate();
    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMCA_CLOSE_HANDLE( VMCANtServiceData.stopServiceEvent );

    if( VMCANtServiceData.hServiceStatus )
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
            VMCANtServiceData.hServiceStatus, &stoppedServiceStatus
        );
    }
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
        VMCAUpdateSCMStatus(
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
VMCAServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    )
{
    DWORD dwError = 0;
    PVMCA_NTSERVICE_DATA serviceData = NULL;

    BAIL_ON_VMCA_INVALID_POINTER( lpContext, dwError );
    serviceData = (PVMCA_NTSERVICE_DATA)lpContext;

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
            VMCAUpdateSCMStatus(
                serviceData->hServiceStatus, SERVICE_RUNNING, 0, 0, 0
            );
            break;
    }

error:

    return dwError;
}

static
DWORD
VMCAUpdateSCMStatus(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

error:

    return dwError;
}

