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

static
VOID
VmDirHandleError(
    UINT32 apiType,
    DWORD dwInputError,
    VMW_TYPE_SPEC *output_spec,
    DWORD noOfArgsOut,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

static
DWORD
VmDirMarshalResponse (
    UINT32 apiType,
    VMW_TYPE_SPEC *output_spec,
    DWORD noOfArgsOut,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

DWORD
VmDirIpcInitializeHost(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMDIR_IPC_INITIALIZE_HOST;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszSiteName = NULL;
    PWSTR pwszReplURI = NULL;
    DWORD dwFirstReplCycleMode = 0;
    VMW_TYPE_SPEC input_spec[] = INITIALIZE_HOST_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Entering VmDirIpcInitializeHost");

    if (!VmDirIsRootSecurityContext(pSecurityContext))
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                         "%s: Access Denied",
                         __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmDirUnMarshal (
                    apiType,
                    VER1_INPUT,
                    noOfArgsIn,
                    pRequest,
                    dwRequestSize,
                    input_spec);
    BAIL_ON_VMDIR_ERROR (dwError);

    pwszDomainName = input_spec[0].data.pWString;
    pwszUserName = input_spec[1].data.pWString;
    pwszPassword = input_spec[2].data.pWString;
    pwszSiteName = input_spec[3].data.pWString;
    pwszReplURI = input_spec[4].data.pWString;
    dwFirstReplCycleMode = *input_spec[5].data.pUint32;

    uResult = VmDirSrvInitializeHost(
                    pwszDomainName,
                    pwszUserName,
                    pwszPassword,
                    pwszSiteName,
                    pwszReplURI,
                    dwFirstReplCycleMode);
    output_spec[0].data.pUint32 = &uResult;

    dwError = VmDirMarshalResponse (
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR (dwError);

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcInitializeHost");

cleanup:

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmDirFreeTypeSpecContent (input_spec, noOfArgsIn);
    return dwError;

error:
    VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirIpcInitializeHost failed (%u)",
                     dwError);

    dwError = 0;
    goto cleanup;
}

DWORD
VmDirIpcInitializeTenant(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMDIR_IPC_INITIALIZE_TENANT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    VMW_TYPE_SPEC input_spec[] = INITIALIZE_TENANT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Entering VmDirIpcInitializeTenant");

    if (!VmDirIsRootSecurityContext(pSecurityContext))
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                         "%s: Access Denied",
                         __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmDirUnMarshal (
                    apiType,
                    VER1_INPUT,
                    noOfArgsIn,
                    pRequest,
                    dwRequestSize,
                    input_spec);
    BAIL_ON_VMDIR_ERROR (dwError);

    pwszDomainName = input_spec[0].data.pWString;
    pwszUserName = input_spec[1].data.pWString;
    pwszPassword = input_spec[2].data.pWString;

    uResult = VmDirSrvInitializeTenant(
                    pwszDomainName,
                    pwszUserName,
                    pwszPassword);
    output_spec[0].data.pUint32 = &uResult;

    dwError = VmDirMarshalResponse (
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR (dwError);

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcInitializeTenant");

cleanup:

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmDirFreeTypeSpecContent (input_spec, noOfArgsIn);
    return dwError;

error:
    VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirIpcInitializeTenant failed (%u)",
                     dwError);

    dwError = 0;
    goto cleanup;
}

DWORD
VmDirIpcForceResetPassword(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMDIR_IPC_FORCE_RESET_PASSWORD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszTargetUPN = NULL;
    VMDIR_DATA_CONTAINER  dataContainer = {0};
    DWORD dwContainerLength = 0;
    PBYTE pContainerBlob = NULL;
    VMW_TYPE_SPEC input_spec[] = FORCE_RESET_PASSWORD_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = FORCE_RESET_PASSWORD_OUTPUT_PARAMS;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Entering VmDirIpcForceResetPassword");

    if (!VmDirIsRootSecurityContext(pSecurityContext))
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                         "%s: Access Denied",
                         __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmDirUnMarshal (
                    apiType,
                    VER1_INPUT,
                    noOfArgsIn,
                    pRequest,
                    dwRequestSize,
                    input_spec);
    BAIL_ON_VMDIR_ERROR (dwError);

    pwszTargetUPN = input_spec[0].data.pWString;

    uResult = VmDirSrvForceResetPassword(
                    pwszTargetUPN,
                    &dataContainer);

    dwError = VmDirMarshalContainerLength(
                              (PVMDIR_IPC_DATA_CONTAINER)&dataContainer,
                              &dwContainerLength);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory(
                             dwContainerLength,
                             (PVOID*)&pContainerBlob);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirMarshalContainer(
                               (PVMDIR_IPC_DATA_CONTAINER)&dataContainer,
                               dwContainerLength,
                               pContainerBlob);
    BAIL_ON_VMDIR_ERROR (dwError);

    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = &dwContainerLength;
    output_spec[2].data.pByte = (PBYTE) pContainerBlob;

    dwError = VmDirMarshalResponse (
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR (dwError);

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcForceResetPassword");

cleanup:

    VMDIR_SAFE_FREE_MEMORY(dataContainer.data);
    VMDIR_SAFE_FREE_MEMORY(pContainerBlob);

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmDirFreeTypeSpecContent (input_spec, noOfArgsIn);
    return dwError;

error:
    VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirIpcForceResetPassword failed (%u)",
                     dwError);

    dwError = 0;
    goto cleanup;
}

DWORD
VmDirIpcGeneratePassword(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMDIR_IPC_GENERATE_PASSWORD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMDIR_DATA_CONTAINER  dataContainer = {0};
    DWORD dwContainerLength = 0;
    PBYTE pContainerBlob = NULL;
    VMW_TYPE_SPEC output_spec[] = GENERATE_PASSWORD_OUTPUT_PARAMS;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Entering VmDirIpcGeneratePassword");

    if (!VmDirIsRootSecurityContext(pSecurityContext))
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                         "%s: Access Denied",
                         __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmDirUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirGenerateRandomPasswordByDefaultPolicy((PSTR*)&dataContainer.data );
    if ( dwError == ERROR_NOT_JOINED )
    {
        dwError = VmKdcGenerateRandomPassword(
                              VMDIR_KDC_RANDOM_PWD_LEN,
                              (PSTR*)&dataContainer.data );
    }
    BAIL_ON_VMDIR_ERROR( dwError );

    dataContainer.dwCount = (int)VmDirStringLenA((PSTR)dataContainer.data);

    dwError = VmDirMarshalContainerLength(
                              (PVMDIR_IPC_DATA_CONTAINER)&dataContainer,
                              &dwContainerLength);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory(
                             dwContainerLength,
                             (PVOID*)&pContainerBlob);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirMarshalContainer(
                               (PVMDIR_IPC_DATA_CONTAINER)&dataContainer,
                               dwContainerLength,
                               pContainerBlob);
    BAIL_ON_VMDIR_ERROR (dwError);

    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = &dwContainerLength;
    output_spec[2].data.pByte = (PBYTE) pContainerBlob;

    dwError = VmDirMarshalResponse (
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR (dwError);

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcGeneratePassword");

cleanup:

    VMDIR_SAFE_FREE_MEMORY(dataContainer.data);
    VMDIR_SAFE_FREE_MEMORY(pContainerBlob);

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    return dwError;

error:
    VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirIpcGeneratePassword failed (%u)",
                     dwError);

    dwError = 0;
    goto cleanup;
}

DWORD
VmDirIpcSetSRPSecret(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMDIR_IPC_SET_SRP_SECRET;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszUPN = NULL;
    PWSTR pwszSecret = NULL;
    VMW_TYPE_SPEC input_spec[] = SET_SRP_SECRET_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Entering VmDirIpcSetSRPSecret");

    if (!VmDirIsRootSecurityContext(pSecurityContext))
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                         "%s: Access Denied",
                         __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmDirUnMarshal (
                    apiType,
                    VER1_INPUT,
                    noOfArgsIn,
                    pRequest,
                    dwRequestSize,
                    input_spec);
    BAIL_ON_VMDIR_ERROR (dwError);

    pwszUPN = input_spec[0].data.pWString;
    pwszSecret = input_spec[1].data.pWString;

    uResult = VmDirSrvSetSRPSecret(
                    pwszUPN,
                    pwszSecret);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VmDirMarshalResponse (
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR (dwError);

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcSetSRPSecret");

cleanup:

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmDirFreeTypeSpecContent (input_spec, noOfArgsIn);
    return dwError;

error:
    VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirIpcSetSRPSecret failed (%u)",
                     dwError);

    dwError = 0;
    goto cleanup;
}

DWORD
VmDirIpcGetServerState(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMDIR_IPC_GET_SERVER_STATE;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    UINT32 uServerState = 0;
    VMW_TYPE_SPEC output_spec[] = GET_SERVER_STATE_OUTPUT_PARAMS;

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Entering VmDirIpcGetServerState ");

    if (!VmDirIsRootSecurityContext(pSecurityContext))
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                         "%s: Access Denied",
                         __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    dwError = VmDirUnMarshal (
                    apiType,
                    VER1_INPUT,
                    noOfArgsIn,
                    pRequest,
                    dwRequestSize,
                    NULL);
    BAIL_ON_VMDIR_ERROR (dwError);

    uResult = VmDirSrvGetServerState(&uServerState);

    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = &uServerState;

    dwError = VmDirMarshalResponse (
                    apiType,
                    output_spec,
                    noOfArgsOut,
                    &pResponse,
                    &dwResponseSize);
    BAIL_ON_VMDIR_ERROR (dwError);

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Exiting VmDirIpcGetServerState");

cleanup:

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    return dwError;

error:
    VmDirHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirIpcGetServerState failed (%u)",
                     dwError);

    dwError = 0;
    goto cleanup;
}

static
VOID
VmDirHandleError(
    UINT32 apiType,
    DWORD dwInputError,
    VMW_TYPE_SPEC *output_spec,
    DWORD noOfArgsOut,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    DWORD dwResponseSize = 0;
    PBYTE pResponse = NULL;

    output_spec[0].data.pUint32 = &dwInputError;

    dwError = VmDirGetMarshalLength(
                        output_spec,
                        noOfArgsOut,
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
                        noOfArgsOut,
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
VmDirMarshalResponse (
    UINT32 apiType,
    VMW_TYPE_SPEC *output_spec,
    DWORD noOfArgsOut,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    DWORD dwResponseSize = 0;
    PBYTE pResponse = NULL;

    dwError = VmDirGetMarshalLength (
                  output_spec,
                  noOfArgsOut,
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
                  noOfArgsOut,
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
