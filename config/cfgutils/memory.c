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

DWORD
VmwDeployAllocateMemory(
    size_t size,
    PVOID* ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !size)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pMemory = calloc(1, size);
    if (!pMemory)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    *ppMemory = pMemory;

cleanup:

    return dwError;

error:

	if (ppMemory)
	{
		*ppMemory = NULL;
	}

	if (pMemory)
	{
		VmwDeployFreeMemory(pMemory);
	}

	goto cleanup;
}

VOID
VmwDeployFreeMemory(
    PVOID pMemory
    )
{
	if (pMemory)
	{
		free(pMemory);
	}
}

