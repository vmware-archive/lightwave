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

#include "stdafx.h"

BOOL APIENTRY
DllMain(
	HMODULE hModule,
    DWORD   ul_reason_for_call,
    LPVOID  lpReserved
    )
{
	DWORD dwError = 0;
	BOOL bResult = TRUE;

	switch (ul_reason_for_call)
	{
        case DLL_PROCESS_ATTACH:

			dwError = EventLogInitialize();

			break;

        case DLL_THREAD_ATTACH:

			break;

        case DLL_THREAD_DETACH:

			break;

        case DLL_PROCESS_DETACH:

			//EventLogRpcShutdown();

		    break;
	}

	BAIL_ON_VMEVENT_ERROR(dwError);

cleanup:

	return bResult;

error:

	bResult = FALSE;

	goto cleanup;
}
