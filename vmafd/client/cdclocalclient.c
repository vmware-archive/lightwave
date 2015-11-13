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
CdcLocalEnableClientAffinity()
{
    DWORD dwError = 0;
    UINT32 apiType = CDC_IPC_ENABLE_CLIENT_AFFINITY;
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
CdcLocalDisableClientAffinity()
{
    DWORD dwError = 0;
    UINT32 apiType = CDC_IPC_DISABLE_CLIENT_AFFINITY;
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
CdcLocalGetDCNameW(
    PCWSTR pszDomainName,
    GUID_W pDomainGuid,
    PCWSTR pszSiteName,
    DWORD dwFlags,
    PCDC_DC_INFO_W *ppDomainControllerInfo
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pDomainControllerInfo = NULL;
    UINT32 apiType = CDC_IPC_GET_DC_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    VMW_TYPE_SPEC input_spec[] = GET_CDC_NAME_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = GET_CDC_NAME_OUTPUT_PARAMS;

    if (!ppDomainControllerInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    input_spec[0].data.pWString = (PWSTR)pszDomainName;
    input_spec[1].data.pWString = pDomainGuid;
    input_spec[2].data.pWString = (PWSTR)pszSiteName;
    input_spec[3].data.pUint32 = (PUINT32) &dwFlags;

    dwError = VecsLocalIPCRequest(
                    apiType,
                    noOfArgsIn,
                    noOfArgsOut,
                    input_spec,
                    output_spec);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = *(output_spec[0].data.pUint32);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                              sizeof(CDC_DC_INFO_W),
                              (PVOID *)&pDomainControllerInfo
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    pDomainControllerInfo->DcAddressType = *output_spec[3].data.pUint32;

    if (!IsNullOrEmptyString(output_spec[1].data.pWString))
    {
        dwError = VmAfdAllocateStringW(
                    output_spec[1].data.pWString,
                    &pDomainControllerInfo->pszDCName
                );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsNullOrEmptyString(output_spec[2].data.pWString))
    {
        dwError = VmAfdAllocateStringW(
                    output_spec[2].data.pWString,
                    &pDomainControllerInfo->pszDCAddress
                );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsNullOrEmptyString(output_spec[4].data.pWString))
    {
        dwError = VmAfdAllocateStringW(
                    output_spec[4].data.pWString,
                    &pDomainControllerInfo->pszDomainName
                );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsNullOrEmptyString(output_spec[5].data.pWString))
    {
        dwError = VmAfdAllocateStringW(
                    output_spec[5].data.pWString,
                    &pDomainControllerInfo->pszDcSiteName
                );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppDomainControllerInfo = pDomainControllerInfo;

cleanup:

    VmAfdFreeTypeSpecContent(output_spec, noOfArgsOut);
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLocalGetDCName failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
CdcLocalEnumDCEntries(
    PWSTR **ppszDCEntries,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    UINT32 apiType = CDC_IPC_ENUM_DC_ENTRIES;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    PWSTR *pszDCEntries = NULL;
    DWORD dwCount = 0;

    VMW_TYPE_SPEC output_spec[] = ENUM_STORE_OUTPUT_PARAMS;


    if (!ppszDCEntries ||
        !pdwCount
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest (
                                    apiType,
                                    noOfArgsIn,
                                    noOfArgsOut,
                                    NULL,
                                    output_spec
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdUnMarshalStringArray(
                                        *output_spec[2].data.pUint32,
                                        output_spec[1].data.pByte,
                                        &pszDCEntries,
                                        &dwCount
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszDCEntries = pszDCEntries;
    *pdwCount = dwCount;

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;

error:

    if (ppszDCEntries)
    {
        *ppszDCEntries = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (pszDCEntries)
    {
        VecsFreeStringArrayW (
                                pszDCEntries,
                                dwCount
                              );
    }

    goto cleanup;
}

DWORD
CdcLocalGetCurrentState(
    PDWORD pdwState
    )
{
    DWORD dwError = 0;
    DWORD dwState = 0;
    UINT32 apiType = CDC_IPC_GET_CDC_STATE;

    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;

    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_STATE_OUTPUT_PARAMS;


    if (!pdwState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VecsLocalIPCRequest (
                                    apiType,
                                    noOfArgsIn,
                                    noOfArgsOut,
                                    NULL,
                                    output_spec
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = *(output_spec[0].data.pUint32);

    BAIL_ON_VMAFD_ERROR (dwError);

    dwState = *(output_spec[1].data.pUint32);

    *pdwState = dwState;

cleanup:

    VmAfdFreeTypeSpecContent (output_spec, noOfArgsOut);
    return dwError;

error:

    if (pdwState)
    {
        *pdwState = 0;
    }
    goto cleanup;
}

