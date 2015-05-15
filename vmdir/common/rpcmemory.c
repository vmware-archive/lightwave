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

ULONG
VmDirRpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    )
{
    ULONG           ulError  = 0;
    PVOID           pMemory  = NULL;
    idl_ulong_int   ulStatus = 0;

    if (size <= 0 || !ppMemory)
    {
        ulError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(ulError);
    }

    pMemory = rpc_sm_allocate((idl_size_t) size, &ulStatus);
    if ( ulStatus != rpc_s_ok || !pMemory)
    {
        ulError = VMDIR_ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(ulError);
    }

    memset(pMemory,0, size);

    *ppMemory = pMemory;

cleanup:

    return ulError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirRpcAllocateMemory failed, size (%u), status (%u), error (%u)",
                              size, ulStatus, ulError);
    if (ppMemory)
    {
        *ppMemory = NULL;
    }

    goto cleanup;
}

VOID
VmDirRpcFreeMemory(
    PVOID pMemory
    )
{
    idl_ulong_int ulStatus = 0;

    if (pMemory)
    {
        rpc_sm_free(pMemory, &ulStatus);
        if (ulStatus != rpc_s_ok)
        {
            VmDirLog( LDAP_DEBUG_ANY, "VmDirRpcFreeMemory failed, status (%u)", ulStatus);
        }
    }
}

VOID
VmDirRpcClientFreeMemory(
    PVOID pMemory
    )
{
    idl_ulong_int ulStatus = 0;

    if (pMemory)
    {
        rpc_sm_client_free(pMemory, &ulStatus);
        if (ulStatus != rpc_s_ok)
        {
            VmDirLog( LDAP_DEBUG_ANY, "VmDirRpcClientFreeMemory failed, status (%u)", ulStatus);
        }
    }
}
