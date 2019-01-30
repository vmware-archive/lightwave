/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

static
VOID
_VmDirHandleError(
    UINT32          apiType,
    DWORD           dwInputError,
    VMW_TYPE_SPEC   *output_spec,
    DWORD           dwNoOfArgsOut,
    PBYTE           *ppResponse,
    PDWORD          pdwResponseSize
    );

static
DWORD
_VmDirMarshalResponse (
    UINT32          apiType,
    VMW_TYPE_SPEC   *output_spec,
    DWORD           dwNoOfArgsOut,
    PBYTE           *ppResponse,
    PDWORD          pdwResponseSize
    );

DWORD
VmDirIpcServerStop(
    PBYTE   pRequest,
    DWORD   dwRequestSize,
    PBYTE   *ppResponse,
    PDWORD  pdwResponseSize
    )
{
    DWORD           dwError = 0;
    UINT32          uResult = 0;
    UINT32          apiType = VMDIR_IPC_PROCESS_STOP;
    DWORD           noOfArgsIn = 0;
    DWORD           noOfArgsOut = 0;
    PBYTE           pResponse = NULL;
    DWORD           dwResponseSize = 0;
    VMW_TYPE_SPEC   input_spec[] = PROCESS_START_STOP_INPUT_PARAMS;
    VMW_TYPE_SPEC   output_spec[] = RESPONSE_PARAMS;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Entering VmDirIpcServerStop");

    noOfArgsIn = VMDIR_ARRAY_SIZE(input_spec);
    dwError = VmDirUnMarshal(
                    apiType,
                    VER1_INPUT,
                    noOfArgsIn,
                    pRequest,
                    dwRequestSize,
                    input_spec);
    BAIL_ON_VMDIR_ERROR (dwError);

    uResult = VmDirStopProcess(*input_spec[0].data.pUint32);

    output_spec[0].data.pUint32 = &uResult;

    noOfArgsOut = VMDIR_ARRAY_SIZE(output_spec);

    dwError = _VmDirMarshalResponse(
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcServerStop");

cleanup:

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;
    VmDirFreeTypeSpecContent (input_spec, noOfArgsIn);

    return dwError;

error:
    _VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirIpcServerStop failed (%u)",
                    dwError);

    dwError = 0;
    goto cleanup;
}

DWORD
VmDirIpcServerStart(
    PBYTE   pRequest,
    DWORD   dwRequestSize,
    PBYTE   *ppResponse,
    PDWORD  pdwResponseSize
    )
{
    DWORD           dwError = 0;
    UINT32          uResult = 0;
    UINT32          apiType = VMDIR_IPC_PROCESS_START;
    DWORD           noOfArgsIn = 0;
    DWORD           noOfArgsOut = 0;
    PBYTE           pResponse = NULL;
    DWORD           dwResponseSize = 0;
    VMW_TYPE_SPEC   input_spec[] = PROCESS_START_STOP_INPUT_PARAMS;
    VMW_TYPE_SPEC   output_spec[] = RESPONSE_PARAMS;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Entering VmDirIpcServerStart");

    noOfArgsIn = VMDIR_ARRAY_SIZE(input_spec);
    dwError = VmDirUnMarshal(
                    apiType,
                    VER1_INPUT,
                    noOfArgsIn,
                    pRequest,
                    dwRequestSize,
                    input_spec);
    BAIL_ON_VMDIR_ERROR (dwError);

    uResult = VmDirStartProcess(*input_spec[0].data.pUint32);

    output_spec[0].data.pUint32 = &uResult;

    noOfArgsOut = VMDIR_ARRAY_SIZE(output_spec);

    dwError = _VmDirMarshalResponse(
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcServerStart");

cleanup:

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;
    VmDirFreeTypeSpecContent (input_spec, noOfArgsIn);

    return dwError;

error:
    _VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirIpcServerStart failed (%u)",
                    dwError);

    dwError = 0;
    goto cleanup;
}

DWORD
VmDirIpcGetProcessList(
    PBYTE   pRequest,
    DWORD   dwRequestSize,
    PBYTE   *ppResponse,
    PDWORD  pdwResponseSize
    )
{
    DWORD                       dwError = 0;
    UINT32                      uResult = 0;
    UINT32                      apiType = VMDIR_IPC_PROCESS_LIST;
    DWORD                       noOfArgsOut = 0;
    PBYTE                       pResponse = NULL;
    DWORD                       dwResponseSize = 0;
    VMW_TYPE_SPEC               output_spec[] = PROCESS_LIST_OUTPUT_PARAMS;
    PVMDIR_PROCESS_LIST         pProcessList = NULL;
    DWORD                       dwProcessCount = 0;
    PBYTE                       pContainerBlob = NULL;
    DWORD                       dwContainerLength = 0;
    VMDIR_IPC_DATA_CONTAINER    dataContainer = {0};

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Entering VmDirIpcGetProcessList");

    uResult = VmDirProcessTableGetList(&pProcessList, &dwProcessCount);

    output_spec[0].data.pUint32 = &uResult;

    dataContainer.dwCount = dwProcessCount * sizeof(VMDIR_PROCESS_LIST);
    dataContainer.data = (PBYTE) pProcessList;

    dwError = VmDirMarshalContainerLength(&dataContainer, &dwContainerLength);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory(
                             dwContainerLength,
                             (PVOID*)&pContainerBlob);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirMarshalContainer(
                               &dataContainer,
                               dwContainerLength,
                               pContainerBlob);
    BAIL_ON_VMDIR_ERROR (dwError);

    output_spec[1].data.pUint32 = &dwContainerLength;
    output_spec[2].data.pByte = pContainerBlob;

    noOfArgsOut = VMDIR_ARRAY_SIZE(output_spec);

    dwError = _VmDirMarshalResponse(
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcGetProcessList");

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;
    VMDIR_SAFE_FREE_MEMORY(pProcessList);
    VMDIR_SAFE_FREE_MEMORY(pContainerBlob);
    return dwError;

error:
    _VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirIpcGetProcessList failed (%u)",
                    dwError);

    dwError = 0;
    goto cleanup;
}

static
VOID
_VmDirHandleError(
    UINT32          apiType,
    DWORD           dwInputError,
    VMW_TYPE_SPEC   *output_spec,
    DWORD           dwNoOfArgsOut,
    PBYTE           *ppResponse,
    PDWORD          pdwResponseSize
    )
{
    DWORD dwError = 0;
    DWORD dwResponseSize = 0;
    PBYTE pResponse = NULL;

    output_spec[0].data.pUint32 = &dwInputError;

    dwError = VmDirGetMarshalLength(
                        output_spec,
                        dwNoOfArgsOut,
                        &dwResponseSize
                        );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory(
                        dwResponseSize,
                        (PVOID *) &pResponse
                        );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirMarshal(
                        apiType,
                        VER1_OUTPUT,
                        dwNoOfArgsOut,
                        output_spec,
                        pResponse,
                        dwResponseSize
                        );
    BAIL_ON_VMDIR_ERROR (dwError);

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

cleanup:

        return;

error:
    if (ppResponse)
    {
        *ppResponse = NULL;
    }
    if (pdwResponseSize)
    {
        *pdwResponseSize = 0;
    }
    if (pResponse)
    {
        VMDIR_SAFE_FREE_MEMORY (pResponse);
    }

    goto cleanup;
}

static
DWORD
_VmDirMarshalResponse (
    UINT32          apiType,
    VMW_TYPE_SPEC   *output_spec,
    DWORD           dwNoOfArgsOut,
    PBYTE           *ppResponse,
    PDWORD          pdwResponseSize
    )
{
    DWORD dwError = 0;
    DWORD dwResponseSize = 0;
    PBYTE pResponse = NULL;

    dwError = VmDirGetMarshalLength (
                  output_spec,
                  dwNoOfArgsOut,
                  &dwResponseSize
                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory (
                  dwResponseSize,
                  (PVOID *) &pResponse
                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirMarshal (
                  apiType,
                  VER1_OUTPUT,
                  dwNoOfArgsOut,
                  output_spec,
                  pResponse,
                  dwResponseSize
                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

cleanup:
    return dwError;

error:
    if (ppResponse)
    {
        *ppResponse = NULL;
    }
    if (pdwResponseSize)
    {
        *pdwResponseSize = 0;
    }

    VMDIR_SAFE_FREE_MEMORY (pResponse);

    goto cleanup;
}
