/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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
 * Module Name: post-manager
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Post manager prototypes
 *
 */

//signal.c

VOID
VmDirBlockSelectedSignals(
    VOID
    );

DWORD
VmDirHandleSignals(
    VOID
    );

//process.c

DWORD
VmDirStartProcess(
    DWORD   dwGroupId
    );

DWORD
VmDirStopProcess(
    DWORD   dwGroupId
    );

DWORD
VmDirStopAllProcesses(
    VOID
    );

//processtable.c

DWORD
VmDirProcessTableInit(
    VOID
    );

DWORD
VmDirProcessTableRead(
    DWORD           dwGroupId,
    PVMDIR_PROCESS  *ppProcessOut
    );

DWORD
VmDirProcessTableUpdate(
    DWORD           dwGroupId,
    PVMDIR_PROCESS  pProcess
    );

DWORD
VmDirProcessTableGetList(
    PVMDIR_PROCESS_LIST  *ppProcessList,
    PDWORD               pdwProcessCount
    );

//ipcserver.c

DWORD
VmDirIpcServerInit(
    VOID
    );

VOID
VmDirIpcServerShutDown(
    VOID
    );

//ipcapihandler.c

DWORD
VmDirLocalAPIHandler(
    PVM_DIR_SECURITY_CONTEXT    pSecurityContext,
    PBYTE                       pRequest,
    DWORD                       dwRequestSize,
    PBYTE                       *ppResponse,
    DWORD                       *pdwResponseSize
    );

//ipclocalapi.c

DWORD
VmDirIpcGetProcessList(
    PBYTE   pRequest,
    DWORD   dwRequestSize,
    PBYTE   *ppResponse,
    PDWORD  pdwResponseSize
    );

DWORD
VmDirIpcServerStart(
    PBYTE   pRequest,
    DWORD   dwRequestSize,
    PBYTE   *ppResponse,
    PDWORD  pdwResponseSize
    );

DWORD
VmDirIpcServerStop(
    PBYTE   pRequest,
    DWORD   dwRequestSize,
    PBYTE   *ppResponse,
    PDWORD  pdwResponseSize
    );
