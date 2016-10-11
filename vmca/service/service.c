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
//
// VMCAService.cpp: Defines the entry point for the console application.
//

#include "includes.h"

static
DWORD
InitializeRPCServer(
    VOID
    );

static
BOOLEAN
VMCACheckRPCServerIsActive(
    VOID
    );

static
VOID
VMCAShutdownRPCServer(
    VOID
    );

DWORD
VMCARPCInit(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = InitializeRPCServer();
    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}

VOID
VMCARPCShutdown(
    VOID
    )
{
    VMCAShutdownRPCServer();
}


DWORD
VMCAServiceInit(
    VOID
    )
{
    DWORD dwError = 0;


    return dwError;
}



VOID
VMCAServiceShutdown(
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

    dwError  = VMCAStartRpcServer();
    BAIL_ON_VMCA_ERROR(dwError);

	dwError = dcethread_create(
                &gVMCAServerGlobals.pRPCServerThread,
                NULL,
                VMCAListenRpcServer,
                NULL);
	dwError = VMCAMapDCEErrorCode(dwError);
	BAIL_ON_VMCA_ERROR(dwError);


    while (!VMCACheckRPCServerIsActive())
    {
        // Wait for RPC Server to come up.
        VMCASleep(1);
    }

error:

    return dwError;
}

BOOLEAN
VMCACheckRPCServerIsActive(
    VOID
    )
{
    DWORD dwError = 0;
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
            dwError = VMCAMapDCEErrorCode(dwError);
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMCA_ERROR(dwError);

cleanup:

    return bIsActive;

error:

    bIsActive = FALSE;

    goto cleanup;
}


static
VOID
VMCAShutdownRPCServer(
    VOID
    )
{
    DWORD dwError = 0;

    // stop rpc listening
    dwError = VMCAStopRpcServer();
    BAIL_ON_VMCA_ERROR(dwError);

    if (gVMCAServerGlobals.pRPCServerThread)
    {
        dwError = dcethread_interrupt(gVMCAServerGlobals.pRPCServerThread);
        dwError = VMCAMapDCEErrorCode(dwError);
        BAIL_ON_VMCA_ERROR(dwError);
#if defined(_WIN32)
        // BUGBUG BUGBUG PR http://bugzilla.eng.vmware.com/show_bug.cgi?id=1219191
        // This is most likely a pthread issue due to signal lost.
        // We should update pthread to see if we can resolve this.
        // http://bugzilla.eng.vmware.com/show_bug.cgi?id=1224401 tracks this effort.

        VMCASleep(1);  // sleep 1s
#else
        dwError = dcethread_join(gVMCAServerGlobals.pRPCServerThread, NULL);
        dwError = VMCAMapDCEErrorCode(dwError);
        BAIL_ON_VMCA_ERROR(dwError);
#endif
        gVMCAServerGlobals.pRPCServerThread = NULL;
    }

error:
    return;
}
