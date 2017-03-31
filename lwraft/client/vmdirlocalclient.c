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
VmDirLocalInitializeHost(
    PWSTR   pwszNamingContext,
    PWSTR   pwszUserName,
    PWSTR   pwszPassword,
    PWSTR   pwszSiteName,
    PWSTR   pwszReplURI,
    UINT32  firstReplCycleMode
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMDIR_IPC_INITIALIZE_HOST;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = INITIALIZE_HOST_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    if (IsNullOrEmptyString(pwszNamingContext) ||
        IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    input_spec[0].data.pWString = (PWSTR) pwszNamingContext;
    input_spec[1].data.pWString = (PWSTR) pwszUserName;
    input_spec[2].data.pWString = (PWSTR) pwszPassword;
    input_spec[3].data.pWString = (PWSTR) pwszSiteName;
    input_spec[4].data.pWString = (PWSTR) pwszReplURI;
    input_spec[5].data.pUint32  = &firstReplCycleMode;

    dwError = VmDirLocalIPCRequest(
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

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalInitializeHost failed (%u)",
                     dwError );

    goto cleanup;
}

DWORD
VmDirLocalGetServerState(
    UINT32  *pServerState
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMDIR_IPC_GET_SERVER_STATE;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC output_spec[] = GET_SERVER_STATE_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    if ( !pServerState )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pServerState = *(output_spec[1].data.pUint32);

cleanup:

    VmDirFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalGetServerState failed (%u)",
                     dwError );

    goto cleanup;
}

DWORD
VmDirLocalInitializeTenant(
    PWSTR   pwszNamingContext,
    PWSTR   pwszUserName,
    PWSTR   pwszPassword
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMDIR_IPC_INITIALIZE_TENANT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = INITIALIZE_TENANT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    if (IsNullOrEmptyString(pwszNamingContext) ||
        IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    input_spec[0].data.pWString = (PWSTR) pwszNamingContext;
    input_spec[1].data.pWString = (PWSTR) pwszUserName;
    input_spec[2].data.pWString = (PWSTR) pwszPassword;

    dwError = VmDirLocalIPCRequest(
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

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalInitializeTenant failed (%u)",
                     dwError );

    goto cleanup;
}

DWORD
VmDirLocalForceResetPassword(
    PWSTR        pwszTargetUPN,
    VMDIR_DATA_CONTAINER* pPasswdContainer
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMDIR_IPC_FORCE_RESET_PASSWORD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    DWORD dwBlobSize = 0;
    VMDIR_IPC_DATA_CONTAINER *pContainer = NULL;
    VMW_TYPE_SPEC input_spec[] = FORCE_RESET_PASSWORD_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = FORCE_RESET_PASSWORD_OUTPUT_PARAMS;

    if (IsNullOrEmptyString(pwszTargetUPN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszTargetUPN;

    dwError = VmDirLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwBlobSize = *(output_spec[1].data.pUint32);

    dwError = VmDirUnMarshalContainer(
                             dwBlobSize,
                             output_spec[2].data.pByte,
                             &pContainer);
    BAIL_ON_VMDIR_ERROR (dwError);

    pPasswdContainer->dwCount = pContainer->dwCount;
    pPasswdContainer->data = pContainer->data;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pContainer);

    VmDirFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalForceResetPassword failed (%u)",
                     dwError );

    goto cleanup;
}

DWORD
VmDirLocalGeneratePassword(
    VMDIR_DATA_CONTAINER* pPasswdContainer
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMDIR_IPC_GENERATE_PASSWORD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    DWORD dwBlobSize = 0;
    VMDIR_IPC_DATA_CONTAINER *pContainer = NULL;
    VMW_TYPE_SPEC output_spec[] = FORCE_RESET_PASSWORD_OUTPUT_PARAMS;

    if (!pPasswdContainer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmDirLocalIPCRequest(
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
                             &pContainer);
    BAIL_ON_VMDIR_ERROR (dwError);

    pPasswdContainer->dwCount = pContainer->dwCount;
    pPasswdContainer->data = pContainer->data;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pContainer);

    VmDirFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalGeneratePassword failed (%u)",
                     dwError );

    goto cleanup;
}

DWORD
VmDirLocalSetSRPSecret(
    PCWSTR      pwszUPN,
    PCWSTR      pwszSecret
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VMDIR_IPC_SET_SRP_SECRET;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_SRP_SECRET_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    if (IsNullOrEmptyString(pwszUPN) ||
        IsNullOrEmptyString(pwszSecret))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    input_spec[0].data.pWString = (PWSTR) pwszUPN;
    input_spec[1].data.pWString = (PWSTR) pwszSecret;

    dwError = VmDirLocalIPCRequest(
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

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLocalSetSRPSecret failed (%u)",
                     dwError );

    goto cleanup;
}

DWORD
VmDirLocalIPCRequest(
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

    dwError = VmDirOpenClientConnection ( &pConnection);
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
