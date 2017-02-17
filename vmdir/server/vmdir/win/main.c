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

PCSTR _gVmDirLogFileName   = NULL;
ULONG _giLocalLogMask = 0;
INT64 _gVmDirLogMaxSizeBytes = 0;
DWORD _gVmDirLogMaxOldLogs = 0;

__declspec(dllexport)
DWORD
VmDirSRPGetIdentityData(
    PCSTR       pszUPN,
    PBYTE*      ppByteSecret,
    DWORD*      pdwSecretLen
    );

static
DWORD
WINAPI
VmDirConsoleMain(
    DWORD   argc,
    _TCHAR *targv[]
);

static
VOID
WINAPI
VmDirServiceMain(
    DWORD argc,
    _TCHAR *argv[]
);

static
DWORD
VmDirUpdateSCMStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    DWORD dwCurrentState,
    DWORD dwServiceSpecificExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    );

static
DWORD
WINAPI
VmDirServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    );

int _tmain(int argc, _TCHAR* targv[])
{
    DWORD   dwError = ERROR_SUCCESS;
    SERVICE_TABLE_ENTRY VmDirServiceTable[] =
    {
        {VMDIR_NT_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)VmDirServiceMain},
        {NULL, NULL}
    };
    PCSTR   logFileName = NULL;
    PCSTR   pszBootstrapSchemaFile = NULL;
    PCSTR   pszStateDir = /*VMDIR_DB_DIR*/ VMDIR_PATH_SEPARATOR_STR;
    BOOLEAN bEnableSysLog = FALSE;
    BOOLEAN bConsoleMode = FALSE;
    BOOLEAN bPatchSchema = FALSE;
    DWORD   dwOldLogs = 0;
    INT64   i64MaxLogSize = 0;
    PSTR*   allocArgv = NULL;
    PSTR*   argv = NULL;

    _TCHAR gLogFileName[MAX_PATH];
    _TCHAR gBootStrapSchemaFile[MAX_PATH];
    dwError = VmDirGetLogFilePath(gLogFileName);
    BAIL_ON_VMDIR_ERROR ( dwError );

    VmDirGetLogMaximumOldLogs(&dwOldLogs);
    VmDirGetLogMaximumLogSize(&i64MaxLogSize);

    dwError = VmDirGetBootStrapSchemaFilePath(gBootStrapSchemaFile);
    BAIL_ON_VMDIR_ERROR ( dwError );

#ifdef UNICODE
    dwError = VmDirAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMDIR_ERROR(dwError);
    argv = allocArgv;
#else
    argv = targv; // non-unicode => targv is char
#endif

    dwError = VmDirSrvUpdateConfig();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirParseArgs(
        argc,
        argv,
        &pszBootstrapSchemaFile,
        &_giLocalLogMask,
        &logFileName,
        &bEnableSysLog,
        &bConsoleMode,
        &bPatchSchema
    );
    if(dwError != ERROR_SUCCESS)
    {
        ShowUsage( argv[0] );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // TODO: set the parsed args to the globals structure ...

    _gVmDirLogMaxSizeBytes = i64MaxLogSize;
    _gVmDirLogMaxOldLogs = dwOldLogs;
    if (logFileName)
    {
        _gVmDirLogFileName   = logFileName;
    }
    else
    {
        _gVmDirLogFileName   = gLogFileName;
    }

    if (pszBootstrapSchemaFile)
    {
        dwError = VmDirAllocateStringA(
                pszBootstrapSchemaFile,
                &gVmdirGlobals.pszBootStrapSchemaFile);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(
                gBootStrapSchemaFile,
                &gVmdirGlobals.pszBootStrapSchemaFile);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    gVmdirGlobals.bPatchSchema = bPatchSchema;

    dwError = VmDirAllocateStringA(pszStateDir, &gVmdirGlobals.pszBDBHome);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirDeallocateArgsA(argc, allocArgv);
    allocArgv = NULL;

    if ( gVmdirGlobals.bPatchSchema && !bConsoleMode )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Can patch schema only in console mode" );
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
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
        dwError = VmDirConsoleMain(argc,allocArgv);
    }
    else
    {
        if( !StartServiceCtrlDispatcher(VmDirServiceTable))
        {
            dwError = GetLastError();
            BAIL_ON_VMDIR_ERROR( dwError );
        }
    }

error:

    VmDirSrvFreeConfig();

    VmDirDeallocateArgsA(argc, allocArgv);

    return dwError;
}

static
DWORD
WINAPI
VmDirConsoleMain(
    DWORD argc,
    _TCHAR *targv[]
)
{
    DWORD       dwThreadId = 0;
    DWORD       dwError = ERROR_SUCCESS;
    WSADATA     wsaData = {0};
    BOOLEAN     bWsaStartup = FALSE;
    BOOLEAN     bVmDirInit = FALSE;
    BOOLEAN     bShutdownKDCService = FALSE;
    BOOLEAN     bWaitTimeOut = FALSE;

    VMDIR_NTSERVICE_DATA vmDirNtServiceData =
    {
        NULL,
        NULL
    };

    dwError = VmDirLogInternalInitialize(_gVmDirLogFileName, FALSE, NULL, VMDIR_LOG_INFO, _giLocalLogMask, _gVmDirLogMaxOldLogs, _gVmDirLogMaxSizeBytes);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirdStateSet(VMDIRD_STATE_STARTUP);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: starting..." );

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMDIR_ERROR(dwError);
    bWsaStartup = TRUE;

    dwError = VmDirInit();
    BAIL_ON_VMDIR_ERROR(dwError);
    bVmDirInit = TRUE;

    if ( ! gVmdirGlobals.bPatchSchema )
    {
        dwError = VmKdcServiceStartup();
        BAIL_ON_VMDIR_ERROR(dwError);
        bShutdownKDCService = TRUE;
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmkdcd: running....");

    VmDirdStateSet(VmDirdGetTargetState());
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "Lotus Vmdird: running... state (%d)",
                    VmDirdState());

    if ( ! gVmdirGlobals.bPatchSchema && VmDirdState() != VMDIRD_STATE_RESTORE )
    {
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
        gVmdirGlobals.hStopServiceEvent = vmDirNtServiceData.stopServiceEvent;

        if ( WaitForSingleObject(vmDirNtServiceData.stopServiceEvent, INFINITE) == WAIT_FAILED )
        {
            dwError = GetLastError();
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: exiting..." );

cleanup:

    if ( bShutdownKDCService )
    {
        VmKdcServiceShutdown();
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus vmkdc service: stop" );
    }

    if ( bVmDirInit )
    {
        VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);
        VmDirShutdown(&bWaitTimeOut);
        if (bWaitTimeOut)
        {
           VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: stop" );
           goto done;
        }

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: stop" );
    }

    VmDirLogTerminate();

    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }

done:
    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMDIR_CLOSE_HANDLE( vmDirNtServiceData.stopServiceEvent );
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Lotus Vmdird startup failed (%d)", dwError );

    goto cleanup;
}

static
VOID
WINAPI
VmDirServiceMain(
    DWORD argc,
    _TCHAR *targv[]
)
{
    DWORD dwThreadId = 0;
    DWORD dwError = ERROR_SUCCESS;
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
    BOOLEAN bVmDirInit = FALSE;
    BOOLEAN bShutdownKDCService = FALSE;
    BOOLEAN bWaitTimeOut = FALSE;

    SERVICE_STATUS stoppedServiceStatus =
    {
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_STOPPED
    };
    VMDIR_NTSERVICE_DATA vmDirNtServiceData =
    {
        NULL,
        NULL
    };

    vmDirNtServiceData.hServiceStatus = RegisterServiceCtrlHandlerEx(
        VMDIR_NT_SERVICE_NAME,
        (LPHANDLER_FUNCTION_EX)VmDirServiceCtrlHandlerEx,
        &vmDirNtServiceData
    );
    if (!vmDirNtServiceData.hServiceStatus)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirUpdateSCMStatus(
        vmDirNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 1, 8000
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInternalInitialize(_gVmDirLogFileName, FALSE, NULL, VMDIR_LOG_INFO, _giLocalLogMask, _gVmDirLogMaxOldLogs, _gVmDirLogMaxSizeBytes);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirdStateSet(VMDIRD_STATE_STARTUP);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: starting..." );

    dwError = VmDirUpdateSCMStatus(
        vmDirNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 2, 5000
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    vmDirNtServiceData.stopServiceEvent =
        CreateEvent(NULL, TRUE, FALSE, NULL);

    if (vmDirNtServiceData.stopServiceEvent == NULL)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirUpdateSCMStatus(
        vmDirNtServiceData.hServiceStatus, SERVICE_START_PENDING,
        0, 3, 1000
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMDIR_ERROR(dwError);
    bWsaStartup = TRUE;

    dwError = VmDirInit();
    BAIL_ON_VMDIR_ERROR(dwError);
    bVmDirInit = TRUE;

    if ( ! gVmdirGlobals.bPatchSchema )
    {
        dwError = VmKdcServiceStartup();
        BAIL_ON_VMDIR_ERROR(dwError);
        bShutdownKDCService = TRUE;
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus vmkdc service: running....");

    dwError = VmDirUpdateSCMStatus(
        vmDirNtServiceData.hServiceStatus, SERVICE_RUNNING, 0, 0, 0
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirdStateSet(VmDirdGetTargetState());

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "Lotus Vmdird: running... state (%d)",
                    VmDirdState());

    if ( ! gVmdirGlobals.bPatchSchema && VmDirdState() != VMDIRD_STATE_RESTORE )
    {
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
                 vmDirNtServiceData.stopServiceEvent, INFINITE
                 ) == WAIT_FAILED )
        {
            dwError = GetLastError();
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: exiting..." );

cleanup:

    if ( bShutdownKDCService )
    {
        VmKdcServiceShutdown();
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus vmkdc service: stop" );
    }

    if ( bVmDirInit )
    {
        VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);
        VmDirShutdown(&bWaitTimeOut);
        if (bWaitTimeOut)
        {
           VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: stop" );
           goto done;
        }

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus Vmdird: stop" );
    }

    VmDirLogTerminate();

    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }

done:
    /* MSDN:
         The service status handle does not have to be closed.
    */
    VMDIR_CLOSE_HANDLE( vmDirNtServiceData.stopServiceEvent );

    if( vmDirNtServiceData.hServiceStatus )
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
            vmDirNtServiceData.hServiceStatus, &stoppedServiceStatus
        );
        vmDirNtServiceData.hServiceStatus = NULL;
    }
    return;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Lotus Vmdird startup failed (%d)", dwError );

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
        VmDirUpdateSCMStatus(
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
VmDirServiceCtrlHandlerEx (
    DWORD dwControlCode,
    DWORD dwEventType,
    LPVOID lpEventData,
    LPVOID lpContext
    )
{
    DWORD dwError = 0;
    PVMDIR_NTSERVICE_DATA serviceData = NULL;

    BAIL_ON_VMDIR_INVALID_POINTER( lpContext, dwError );
    serviceData = (PVMDIR_NTSERVICE_DATA)lpContext;

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
            VmDirUpdateSCMStatus(
                serviceData->hServiceStatus, SERVICE_RUNNING, 0, 0, 0
            );
            break;
    }

error:

    return dwError;
}

static
DWORD
VmDirUpdateSCMStatus(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:

    return dwError;
}
