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
 * Module Name:  main.c
 *
 * Abstract: VMware Domain Name Service.
 *
 */

#include "includes.h"

//TODO, move to gVmdnsGlobals?
int  vmdns_syslog_level = VMDNS_LOG_LEVEL_INFO;
int  vmdns_syslog = 0;
int  vmdns_debug = 0;

static
VOID
WINAPI
VmDnsServiceMain(
    DWORD argc,
    _TCHAR *argv[]
    );

static
VOID
VmDnsConsoleMain(
    DWORD argc,
    _TCHAR *argv[]
    );

static
DWORD
VmDnsUpdateSCMStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    DWORD dwCurrentState,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    );

static
DWORD
WINAPI
VmDnsServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    );

static
DWORD
VmDnsInitLog();

int _tmain(int argc, _TCHAR* targv[])
{
    DWORD   dwError = ERROR_SUCCESS;
    SERVICE_TABLE_ENTRY VmDnsServiceTable[] =
    {
        {VMDNS_NT_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)VmDnsServiceMain},
        {NULL, NULL}
    };
    int     portNumber = VMDNS_PORT;
    int     logLevel = VMDNS_LOG_LEVEL_INFO;
    PCSTR   logFileName = NULL;
    PCSTR   pszStateDir = /*VMDNS_DB_DIR*/ VMDNS_PATH_SEPARATOR_STR;
    PSTR*   allocArgv = NULL;
    PSTR*   argv = NULL;
    PSTR    pszLogDir = NULL;
    BOOLEAN bEnableSysLog = TRUE;
    BOOLEAN bConsoleMode = FALSE;

#ifdef UNICODE
    dwError = VmDnsAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMDNS_ERROR(dwError);
    argv = allocArgv;
#else
    argv = targv; // non-unicode => targv is char
#endif

    // TODO: uncomment once registy change is ported to windows ...
    //dwError = VmDnsSrvUpdateConfig();
    //BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsParseArgs(
                    argc,
                    argv,
                    &logLevel,
                    &logFileName,
                    &portNumber,
                    &bEnableSysLog,
                    &bConsoleMode);
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

    if (!logFileName)
    {
        if (!VmDnsConfigGetStringA(
                VMDNS_SOFTWARE_CONFIG_KEY_PATH,
                VMDNS_KEY_VALUE_LOG_PATH,
                &pszLogDir))
        {
            VmDnsAllocateStringPrintfA(
                &logFileName,
                "%s%s",
                pszLogDir,
                "vmdns.log");
        }
    }

    dwError = VmDnsAllocateStringA(
            logFileName,
            &gVmdnsGlobals.pszLogFile);
    BAIL_ON_VMDNS_ERROR(dwError);

    gVmdnsGlobals.iListenPort = portNumber;

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
        VmDnsConsoleMain(argc, argv);
    }
    else
    {
        if( !StartServiceCtrlDispatcher(VmDnsServiceTable))
        {
            dwError = GetLastError();
            BAIL_ON_VMDNS_ERROR( dwError );
        }
    }

cleanup:
    VMDNS_SAFE_FREE_STRINGA(logFileName);
    VMDNS_SAFE_FREE_STRINGA(pszLogDir);
    VmDnsDeallocateArgsA(argc, allocArgv);
    return dwError;

error:
    goto cleanup;
}

static
VOID
WINAPI
VmDnsServiceMain(
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
    VMDNS_NTSERVICE_DATA vmDnsNtServiceData =
    {
        NULL,
        NULL
    };

    vmDnsNtServiceData.hServiceStatus = RegisterServiceCtrlHandlerEx(
        VMDNS_NT_SERVICE_NAME,
        (LPHANDLER_FUNCTION_EX)VmDnsServiceCtrlHandlerEx,
        &vmDnsNtServiceData
    );
    if (!vmDnsNtServiceData.hServiceStatus)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsUpdateSCMStatus(
        vmDnsNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 1, 8000
    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsInitLog();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsUpdateSCMStatus(
        vmDnsNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 2, 5000
    );
    BAIL_ON_VMDNS_ERROR(dwError);

    vmDnsNtServiceData.stopServiceEvent =
        CreateEvent(NULL, TRUE, FALSE, NULL);

    if (vmDnsNtServiceData.stopServiceEvent == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsUpdateSCMStatus(
        vmDnsNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 3, 1000
    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMDNS_ERROR(dwError);
    bWsaStartup = TRUE;

    // TODO: once we have backend ported etc. and can test etc.
    // we should switch VmDnsRpcServerInit() -> VmDnsInit()
    dwError = VmDnsInit();
    BAIL_ON_VMDNS_ERROR(dwError);
    //dwError = VmDnsRpcServerInit();
    //BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsUpdateSCMStatus(
        vmDnsNtServiceData.hServiceStatus, SERVICE_RUNNING, 0, 0, 0
    );
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsdStateSet(VMDNSD_RUNNING);

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
            vmDnsNtServiceData.stopServiceEvent, INFINITE
         ) == WAIT_FAILED )
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:

    VmDnsdStateSet(VMDNS_SHUTDOWN);
    VmDnsShutdown();
    VmDnsLogTerminate();

    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }

    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMDNS_CLOSE_HANDLE( vmDnsNtServiceData.stopServiceEvent );

    if( vmDnsNtServiceData.hServiceStatus )
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
            vmDnsNtServiceData.hServiceStatus, &stoppedServiceStatus
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
        VmDnsUpdateSCMStatus(
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
VmDnsServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    )
{
    DWORD dwError = 0;
    PVMDNS_NTSERVICE_DATA serviceData = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER( lpContext, dwError );
    serviceData = (PVMDNS_NTSERVICE_DATA)lpContext;

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
            VmDnsUpdateSCMStatus(
                serviceData->hServiceStatus, SERVICE_RUNNING, 0, 0, 0
            );
            break;
    }

error:

    return dwError;
}

static
DWORD
VmDnsUpdateSCMStatus(
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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:

    return dwError;
}

static HANDLE ghConsoleExitEvent = NULL;


BOOL CtrlHandler(DWORD fdwCtrlType)
{
    BOOL bStopService = FALSE;
    BOOL bHandled = TRUE;

    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        {

            bStopService = TRUE;
            break;
        }

    default:
        break;
    }

    if (bStopService && ghConsoleExitEvent != NULL)
    {
        bHandled = SetEvent(ghConsoleExitEvent);
    }

    return bHandled;
}

static
VOID
VmDnsConsoleMain(
    DWORD argc,
    _TCHAR *argv[]
    )
{
    DWORD dwThreadId = 0;
    DWORD dwError = ERROR_SUCCESS;
    WSADATA wsaData = { 0 };
    BOOLEAN bWsaStartup = FALSE;

    ghConsoleExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ghConsoleExitEvent == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsInitLog();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMDNS_ERROR(dwError);
    bWsaStartup = TRUE;

    // TODO: once we have backend ported etc. and can test etc.
    // we should switch VmDnsRpcServerInit() -> VmDnsInit()
    dwError = VmDnsInit();
    BAIL_ON_VMDNS_ERROR(dwError);

    if (WaitForSingleObject(ghConsoleExitEvent, INFINITE) == WAIT_FAILED)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    VmDnsShutdown();
    VmDnsLogTerminate();

    if (bWsaStartup != FALSE)
    {
        WSACleanup();
    }

    VMDNS_CLOSE_HANDLE(ghConsoleExitEvent);

    return ;

error:
    goto cleanup;
}

static
DWORD
VmDnsInitLog()
{
    DWORD dwError = ERROR_SUCCESS;
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

cleanup:

    return dwError;

error:
    goto cleanup;
}
