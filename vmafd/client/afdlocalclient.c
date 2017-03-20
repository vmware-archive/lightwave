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
VmAfdLocalGetStatus(
    PVMAFD_STATUS pStatus
)
{
    DWORD dwError = 0;
    UINT32 status = 0;
    UINT32 apiType = VMAFD_IPC_GET_STATUS;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC output_spec[] = GET_STATUS_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    status = *(output_spec[1].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pStatus = (VMAFD_STATUS) status;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetStatus failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetDomainName(
    PWSTR* ppwszDomain
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_DOMAIN_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszDomain = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_NAME_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszDomain
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszDomain = pwszDomain;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetDomainName failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetDomainName(
    PCWSTR pwszDomain
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_DOMAIN_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_DOMAIN_NAME_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString(pwszDomain))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszDomain;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetDomainName failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetDomainState(
    VMAFD_DOMAIN_STATE* pDomainState
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_DOMAIN_STATE;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    UINT32 state = 0;
    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_STATE_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    state = *(output_spec[1].data.pUint32);

    *pDomainState = (VMAFD_DOMAIN_STATE) state;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetDomainState failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetLDU(
    PWSTR* ppwszLDU
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_LDU;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszLDU = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_LDU_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszLDU
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszLDU = pwszLDU;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszLDU);
    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetLDU failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetLDU(
    PCWSTR pwszLDU
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_LDU;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_LDU_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString(pwszLDU))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszLDU;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetLDU failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetRHTTPProxyPort(
    PDWORD pdwPort
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_RHTTPPROXY_PORT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC output_spec[] = GET_RHTTPPROXY_PORT_OUTPUT_PARAMS;

    if (!pdwPort)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwPort = *(output_spec[1].data.pUint32);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    if (pdwPort)
    {
        *pdwPort = 0;
    }

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetRHTTPProxyPort failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetRHTTPProxyPort(
    DWORD dwPort
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_RHTTPPROXY_PORT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_RHTTPPROXY_PORT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (dwPort > 65535)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pUint32 = &dwPort;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetRHTTPProxyPort failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetDCPort(
    DWORD dwPort
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_DC_PORT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_DC_PORT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (dwPort > 65535)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pUint32 = &dwPort;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetDCPort failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetCMLocation(
    PWSTR* ppwszCMLocation
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_CM_LOCATION;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszCMLocation = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_CM_LOCATION_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszCMLocation
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszCMLocation = pwszCMLocation;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszCMLocation);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetCMLocation failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetLSLocation(
    PWSTR* ppwszLSLocation
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_LS_LOCATION;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszLSLocation = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_LS_LOCATION_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszLSLocation
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszLSLocation = pwszLSLocation;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszLSLocation);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetLSLocation failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetDCName(
    PWSTR* ppwszDCName
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_DC_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszDCName = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_DC_NAME_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszDCName
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszDCName = pwszDCName;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetDCName failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetDCName(
    PCWSTR pwszDCName
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_DC_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_DC_NAME_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString(pwszDCName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszDCName;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetDCName failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetMachineAccountInfo(
    PWSTR* ppwszAccount,
    PWSTR* ppwszPassword
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_MACHINE_ACCOUNT_INFO;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszAccount= NULL;
    PWSTR pwszPassword = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_MACHINE_ACCOUNT_INFO_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszAccount
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!output_spec[2].data.pWString)
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[2].data.pWString,
                &pwszPassword
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszAccount = pwszAccount;
    *ppwszPassword = pwszPassword;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetMachineAccountInfo failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetSiteGUID(
    PWSTR* ppwszSiteGUID
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_SITE_GUID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszSiteGUID = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_SITE_GUID_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszSiteGUID
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszSiteGUID = pwszSiteGUID;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszSiteGUID);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetSiteGUID failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetSiteName(
    PWSTR* ppwszSiteName
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_SITE_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszSiteName = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_SITE_NAME_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszSiteName
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszSiteName = pwszSiteName;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetSiteName failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetMachineID(
    PWSTR* ppwszMachineID
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_MACHINE_ID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszMachineID = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_MACHINE_ID_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszMachineID
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszMachineID = pwszMachineID;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszMachineID);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetMachineID failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetMachineID(
    PCWSTR pwszMachineID
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_MACHINE_ID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_MACHINE_ID_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString(pwszMachineID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszMachineID;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetMachineID failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalPromoteVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszSiteName,
    PCWSTR pwszPartnerHostName
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_PROMOTE_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = PROMOTE_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszServerName;
    input_spec[1].data.pWString = (PWSTR) pwszDomainName;
    input_spec[2].data.pWString = (PWSTR) pwszUserName;
    input_spec[3].data.pWString = (PWSTR) pwszPassword;
    input_spec[4].data.pWString = (PWSTR) pwszSiteName;
    input_spec[5].data.pWString = (PWSTR) pwszPartnerHostName;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalPromoteVmDir failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalDemoteVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_DEMOTE_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = DEMOTE_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszServerName;
    input_spec[1].data.pWString = (PWSTR) pwszUserName;
    input_spec[2].data.pWString = (PWSTR) pwszPassword;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalDemoteVmDir failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalJoinValidateDomainCredentials(
    PCWSTR pwszDomainName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    )
{
	DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_VALIDATE_CREDENTIALS;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = JOIN_VALIDATE_CREDENTIALS_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszDomainName;
    input_spec[1].data.pWString = (PWSTR) pwszUserName;
    input_spec[2].data.pWString = (PWSTR) pwszPassword;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(
    	VMAFD_DEBUG_ANY,
    	"VmAfdLocalJoinValidateDomainCredentials failed. Error(%u)",
    	dwError);

    goto cleanup;
}

DWORD
VmAfdLocalJoinVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszMachineName,
    PCWSTR pwszDomainName,
    PCWSTR pwszOrgUnit
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = JOIN_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszServerName;
    input_spec[1].data.pWString = (PWSTR) pwszUserName;
    input_spec[2].data.pWString = (PWSTR) pwszPassword;
    input_spec[3].data.pWString = (PWSTR) pwszMachineName;
    input_spec[4].data.pWString = (PWSTR) pwszDomainName;
    input_spec[5].data.pWString = (PWSTR) pwszOrgUnit;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalJoinVmDir failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalJoinVmDir2(
    PCWSTR           pwszDomainName,
    PCWSTR           pwszUserName,
    PCWSTR           pwszPassword,
    PCWSTR           pwszMachineName,
    PCWSTR           pwszOrgUnit,
    VMAFD_JOIN_FLAGS dwFlags
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_VMDIR_2;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    DWORD idx = 0;
    VMW_TYPE_SPEC input_spec[] = JOIN_VMDIR_2_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[idx++].data.pWString = (PWSTR) pwszDomainName;
    input_spec[idx++].data.pWString = (PWSTR) pwszUserName;
    input_spec[idx++].data.pWString = (PWSTR) pwszPassword;
    input_spec[idx++].data.pWString = (PWSTR) pwszMachineName;
    input_spec[idx++].data.pWString = (PWSTR) pwszOrgUnit;
    input_spec[idx++].data.pUint32  = (PDWORD)&dwFlags;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalJoinVmDir2 failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalLeaveVmDir(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    DWORD  dwLeaveFlags
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_LEAVE_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = LEAVE_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszServerName;
    input_spec[1].data.pWString = (PWSTR) pwszUserName;
    input_spec[2].data.pWString = (PWSTR) pwszPassword;
    input_spec[3].data.pUint32  = (PDWORD) &dwLeaveFlags;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalLeaveVmDir failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalCreateComputerAccount(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszMachineName,
    PCWSTR pwszOrgUnit,
    PWSTR* ppwszOutPassword
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_CREATE_COMPUTER_ACCOUNT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    DWORD idx = 0;
    PWSTR pwszOutPassword = NULL;
    SIZE_T dwOutPasswordLength = 0;
    VMW_TYPE_SPEC input_spec[] = CREATE_COMPUTER_ACCOUNT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = CREATE_COMPUTER_ACCOUNT_OUTPUT_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[idx++].data.pWString = (PWSTR) pwszUserName;
    input_spec[idx++].data.pWString = (PWSTR) pwszPassword;
    input_spec[idx++].data.pWString = (PWSTR) pwszMachineName;
    input_spec[idx++].data.pWString = (PWSTR) pwszOrgUnit;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                  output_spec[1].data.pWString,
                  &pwszOutPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetStringLengthW(
                  output_spec[1].data.pWString,
                  &dwOutPasswordLength);
    BAIL_ON_VMAFD_ERROR(dwError);

    memset(output_spec[1].data.pWString, 0,
           dwOutPasswordLength * sizeof(WCHAR));

    if (ppwszOutPassword)
    {
        *ppwszOutPassword = pwszOutPassword;
    }

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);

    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszOutPassword);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalCreateComputerAccount failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalJoinAD(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszDomainName,
    PCWSTR pwszOrgUnit
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_AD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = JOIN_AD_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszUserName;
    input_spec[1].data.pWString = (PWSTR) pwszPassword;
    input_spec[2].data.pWString = (PWSTR) pwszDomainName;
    input_spec[3].data.pWString = (PWSTR) pwszOrgUnit;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalJoinAD failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalLeaveAD(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_LEAVE_AD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = LEAVE_AD_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszUserName;
    input_spec[1].data.pWString = (PWSTR) pwszPassword;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalLeaveAD failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalQueryAD(
    PWSTR *ppwszComputer,
    PWSTR *ppwszDomain,
    PWSTR *ppwszDistinguishedName,
    PWSTR *ppwszNetbiosName
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_QUERY_AD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszComputer = NULL;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDistinguishedName = NULL;
    PWSTR pwszNetbiosName = NULL;
    VMW_TYPE_SPEC output_spec[] = QUERY_AD_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszComputer
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (IsNullOrEmptyString(output_spec[2].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[2].data.pWString,
                &pwszDomain
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (ppwszDistinguishedName && !IsNullOrEmptyString(output_spec[3].data.pWString))
    {
        dwError = VmAfdAllocateStringW(
                    output_spec[3].data.pWString,
                    &pwszDistinguishedName
                    );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (ppwszNetbiosName && !IsNullOrEmptyString(output_spec[4].data.pWString))
    {
        dwError = VmAfdAllocateStringW(
                    output_spec[4].data.pWString,
                    &pwszNetbiosName
                    );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppwszComputer = pwszComputer;
    *ppwszDomain = pwszDomain;

    if (ppwszDistinguishedName)
    {
        *ppwszDistinguishedName = pwszDistinguishedName;
    }

    if (ppwszNetbiosName)
    {
        *ppwszNetbiosName = pwszNetbiosName;
    }

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszComputer);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszDistinguishedName);
    VMAFD_SAFE_FREE_MEMORY(pwszNetbiosName);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalQueryAD failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalForceReplication(
    PCWSTR pwszServerName
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_FORCE_REPLICATION;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = FORCE_REPLICATION_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszServerName;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalForceReplication failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetPNID(
    PWSTR* ppwszPNID
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_PNID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszPNID = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_PNID_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszPNID
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszPNID = pwszPNID;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetPNID failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetPNID(
    PCWSTR pwszPNID
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_PNID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_PNID_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString(pwszPNID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszPNID;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetPNID failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetCAPath(
    PWSTR* ppwszCAPath
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_CA_PATH;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PWSTR pwszCAPath = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_CA_PATH_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(
                output_spec[1].data.pWString,
                &pwszCAPath
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppwszCAPath = pwszCAPath;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetCAPath failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalSetCAPath(
    PCWSTR pwszCAPath
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_SET_CA_PATH;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = SET_CA_PATH_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString(pwszCAPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszCAPath;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalSetCAPath failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetDCList(
    PCSTR pszDomain,
    PDWORD pdwServerCount,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfo
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_DC_LIST;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    DWORD dwServerCount = 0;
    PVMAFD_DC_INFO_W pVmAfdDCInfoList = NULL;

    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_LIST_OUTPUT_PARAMS;
    VMW_TYPE_SPEC input_spec[]  = GET_DOMAIN_NAME_LIST_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pString = (PSTR) pszDomain;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwServerCount = *(output_spec[1].data.pUint32);
    dwError = VmAfdUnMarshalGetDCList(
                              dwServerCount,
                              *(output_spec[3].data.pUint32),
                              output_spec[2].data.pByte,
                              &pVmAfdDCInfoList
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwServerCount = dwServerCount;
    *ppVmAfdDCInfo = pVmAfdDCInfoList;

cleanup:
    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    VmAfdFreeTypeSpecContent(input_spec, noOfArgsIn);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetDCList failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalTriggerRootCertsRefresh(
    VOID
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_TRIGGER_ROOT_CERTS_REFRESH;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalTriggerRootCertsRefresh failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalPostHeartbeat(
    PCWSTR pwszServiceName,
    DWORD  dwPort
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_POST_HEARTBEAT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = POST_HEARTBEAT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    if (IsNullOrEmptyString(pwszServiceName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszServiceName;
    input_spec[1].data.pUint32 = &dwPort;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalPostHeartbeat failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLocalGetHeartbeatStatus(
    PVMAFD_HB_STATUS_W* ppHeartbeatStatus
)
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_GET_HEARBEAT_STATUS;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;
    VMW_TYPE_SPEC output_spec[] = GET_HEARTBEAT_STATUS_OUTPUT_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                        sizeof(VMAFD_HB_STATUS_W),
                        (PVOID *)&pHeartbeatStatus
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pHeartbeatStatus->bIsAlive = *(output_spec[1].data.pUint32);

    dwError = VmAfdUnMarshalHeartbeatStatusArray(
                        *output_spec[3].data.pUint32,
                        output_spec[2].data.pByte,
                        &pHeartbeatStatus->dwCount,
                        &pHeartbeatStatus->pHeartbeatInfoArr
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppHeartbeatStatus = pHeartbeatStatus;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetHeartbeatStatus failed. Error(%u)", dwError);
    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }

    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusW(pHeartbeatStatus);
    }

    goto cleanup;
}

DWORD
VmAfdLocalRefreshSiteName()
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_REFRESH_SITE_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    NULL,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(
        VMAFD_DEBUG_ANY,
        "VmAfdLocalRefreshSiteName failed. Error(%u)",
        dwError
        );

    goto cleanup;
}

DWORD
VmAfdLocalConfigureDNSW(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_CONFIGURE_DNS;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = DNS_CONFIG_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    BAIL_ON_VMAFD_EMPTY_STRING(pwszUserName, dwError);
    BAIL_ON_VMAFD_EMPTY_STRING(pwszPassword, dwError);

    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszUserName;
    input_spec[1].data.pWString = (PWSTR) pwszPassword;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(
        VMAFD_DEBUG_ANY,
        "VmAfdLocalConfigureDNSW failed. Error(%u)",
        dwError
        );

    goto cleanup;
}

DWORD
VmAfdLocalChangePNID(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszPNID
    )
{
    DWORD dwError = 0;
    UINT32 apiType = VMAFD_IPC_CHANGE_PNID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = CHANGE_PNID_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR) pwszUserName;
    input_spec[1].data.pWString = (PWSTR) pwszPassword;
    input_spec[2].data.pWString = (PWSTR) pwszPNID;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}
