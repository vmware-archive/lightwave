/*
 * Copyright ©2018 VMware, Inc.  All Rights Reserved.
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

typedef struct _VMDIR_GRPC_THREAD_CONTEXT {
    DWORD argc;
    PSTR *argv;
} VMDIR_GRPC_THREAD_CONTEXT, *PVMDIR_GRPC_THREAD_CONTEXT;

static
DWORD
VmDirGrpcServerStartRoutine(
    PVOID pThreadArgs
    )
{
    PVMDIR_GRPC_THREAD_CONTEXT pArgs = (PVMDIR_GRPC_THREAD_CONTEXT) pThreadArgs;

    RunServer_CXX(pArgs->argc, pArgs->argv);
    return 0;
}

DWORD
VmDirGrpcServerInitialize(void)
{
    DWORD dwError = 0;
    DWORD i = 0;
    DWORD argc = 2;
    VMDIR_THREAD thread = {0};
    PVMDIR_GRPC_THREAD_CONTEXT pGrpcContext = NULL;
    PSTR *ppArgv = NULL;
    PSTR pArgv0 = NULL;
    PSTR pArgv1 = NULL;

    dwError = VmDirAllocateMemory(
                sizeof(VMDIR_GRPC_THREAD_CONTEXT),
                (PVOID*)&pGrpcContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                sizeof(PSTR) * argc,
                (PVOID*)&ppArgv);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                  "postd",
                  &pArgv0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                  "localhost:50053",
                  &pArgv1);
    BAIL_ON_VMDIR_ERROR(dwError);


    ppArgv[0] = pArgv0;
    ppArgv[1] = pArgv1;

    pGrpcContext->argc = argc;
    pGrpcContext->argv = ppArgv;

    dwError = VmDirCreateThread(
                  &thread,
                  FALSE, /* Detached thread */
                  VmDirGrpcServerStartRoutine,
                  pGrpcContext);
    BAIL_ON_VMDIR_ERROR(dwError);
printf("VmDirGrpcServerInitialize: after VmDirCreateThread %d\n", dwError);

cleanup:
    return dwError;

error:
    if (ppArgv)
    {
        for (i=0; i<argc; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppArgv[i]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppArgv);
    }
    VMDIR_SAFE_FREE_MEMORY(pArgv0);
    VMDIR_SAFE_FREE_MEMORY(pArgv1);
    VMDIR_SAFE_FREE_MEMORY(pGrpcContext);
    goto cleanup;
}
