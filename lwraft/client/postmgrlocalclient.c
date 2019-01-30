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

DWORD
VmDirLocalPostMgrIPCRequest(
    UINT32 apiType,
    DWORD noOfArgsIn,
    DWORD noOfArgsOut,
    VMW_TYPE_SPEC *input_spec,
    VMW_TYPE_SPEC *output_spec
    )
{
    DWORD dwError = 0;

    DWORD dwRequestSize = 0;
    PBYTE pRequest = NULL;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PVM_DIR_CONNECTION pConnection = NULL;

    dwError = VmDirGetMarshalLength (
                            input_spec,
                            noOfArgsIn,
                            &dwRequestSize
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory (
                            dwRequestSize,
                            (PVOID *) &pRequest
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirMarshal (
                            apiType,
                            VER1_INPUT,
                            noOfArgsIn,
                            input_spec,
                            pRequest,
                            dwRequestSize
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirOpenClientConnection(&pConnection, POST_MGR_SOCKET_FILE_PATH);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirMakeServerRequest (
                            pConnection,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirUnMarshal (
                        apiType,
                        VER1_OUTPUT,
                        noOfArgsOut,
                        pResponse,
                        dwResponseSize,
                        output_spec
                        );
    BAIL_ON_VMDIR_ERROR (dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY (pResponse);
    VMDIR_SAFE_FREE_MEMORY (pRequest);

    if (pConnection)
    {

        VmDirFreeClientConnection (pConnection);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirLocalStopPostProcess(
    DWORD   dwGroupId
    )
{
    DWORD           dwError = 0;
    UINT32          apiType = VMDIR_IPC_PROCESS_STOP;
    DWORD           noOfArgsIn = 0;
    DWORD           noOfArgsOut = 0;
    VMW_TYPE_SPEC   input_spec[] = PROCESS_START_STOP_INPUT_PARAMS;
    VMW_TYPE_SPEC   output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof(input_spec) / sizeof(input_spec[0]);
    noOfArgsOut = sizeof(output_spec) / sizeof(output_spec[0]);

    input_spec[0].data.pUint32 = &dwGroupId;

    dwError = VmDirLocalPostMgrIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalStopPostProcess failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirLocalStartPostProcess(
    DWORD   dwGroupId
    )
{
    DWORD           dwError = 0;
    UINT32          apiType = VMDIR_IPC_PROCESS_START;
    DWORD           noOfArgsIn = 0;
    DWORD           noOfArgsOut = 0;
    VMW_TYPE_SPEC   input_spec[] = PROCESS_START_STOP_INPUT_PARAMS;
    VMW_TYPE_SPEC   output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof(input_spec) / sizeof(input_spec[0]);
    noOfArgsOut = sizeof(output_spec) / sizeof(output_spec[0]);

    input_spec[0].data.pUint32 = &dwGroupId;

    dwError = VmDirLocalPostMgrIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalStartPostProcess failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirLocalListPostProcesses(
    PVMDIR_DATA_CONTAINER  pContainer
    )
{
    DWORD           dwError = 0;
    UINT32          apiType = VMDIR_IPC_PROCESS_LIST;
    DWORD           noOfArgsIn = 0;
    DWORD           noOfArgsOut = 0;
    VMW_TYPE_SPEC   output_spec[] = PROCESS_LIST_OUTPUT_PARAMS;
    PVMDIR_IPC_DATA_CONTAINER pIpcContainer = NULL;
    DWORD           dwBlobSize = 0;

    if (!pContainer)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }
    noOfArgsOut = sizeof(output_spec) / sizeof(output_spec[0]);

    dwError = VmDirLocalPostMgrIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwBlobSize = *(output_spec[1].data.pUint32);

    dwError = VmDirUnMarshalContainer(
                             dwBlobSize,
                             output_spec[2].data.pByte,
                             &pIpcContainer);
    BAIL_ON_VMDIR_ERROR (dwError);

    pContainer->dwCount = pIpcContainer->dwCount;
    pContainer->data = pIpcContainer->data;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pIpcContainer);
    VmDirFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalStartPostProcess failed. Error(%u)", dwError);
    goto cleanup;
}
