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

// EventLogService.cpp: Defines the entry point for the console application.

#include "includes.h"


#ifdef _WIN32
DWORD LwMapErrnoToLwError(DWORD error){return error;}
#endif

static
DWORD
InitializeRPCServer(
    VOID
    );

static
BOOLEAN
EventLogCheckRPCServerIsActive(
    VOID
    );

static
VOID
EventLogShutdownRPCServer(
    VOID
    );


DWORD
EventLogRPCInit(
	VOID
	)
{
	DWORD dwError = 0;

    dwError = InitializeRPCServer();
    BAIL_ON_VMEVENT_ERROR(dwError);

error:

	return dwError;
}

VOID
EventLogRPCShutdown(
	VOID
	)
{
    EventLogShutdownRPCServer();
}

DWORD
EventLogServiceInit(
    VOID
    )
{
    DWORD dwError = 0;

    return dwError;
}



VOID
EventLogServiceShutdown(
    VOID
    )
{
}


static
DWORD
InitializeRPCServer(
    VOID
    )
{
    DWORD dwError = 0;

    dwError  = EventLogStartRpcServer();
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwMapErrnoToLwError(
                    dcethread_create(
                            &gEventLogServerGlobals.pRPCServerThread,
                            NULL,
                            EventLogListenRpcServer,
                            NULL));
    BAIL_ON_VMEVENT_ERROR(dwError);

    while (!EventLogCheckRPCServerIsActive())
    {
        // Wait for RPC Server to come up.
    }

error:

    return dwError;
}

static
BOOLEAN
EventLogCheckRPCServerIsActive(
    VOID
    )
{
    volatile DWORD dwError = 0;
    BOOLEAN bIsActive = FALSE;

    DCETHREAD_TRY
    {
        bIsActive = rpc_mgmt_is_server_listening(NULL, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus (THIS_CATCH);
        }
        if (!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMEVENT_ERROR(dwError);

cleanup:

    return bIsActive;

error:

    bIsActive = FALSE;

    goto cleanup;
}

static
VOID
EventLogShutdownRPCServer(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = EventLogStopRpcServer();
    BAIL_ON_VMEVENT_ERROR(dwError);

    if (gEventLogServerGlobals.pRPCServerThread)
    {
        dwError = LwMapErrnoToLwError(
                        dcethread_interrupt(
                                gEventLogServerGlobals.pRPCServerThread));
        BAIL_ON_VMEVENT_ERROR(dwError);

        dwError = LwMapErrnoToLwError(
                        dcethread_join(
                                gEventLogServerGlobals.pRPCServerThread,
                                NULL));
        BAIL_ON_VMEVENT_ERROR(dwError);

        gEventLogServerGlobals.pRPCServerThread = NULL;
    }

error:

    return;
}
