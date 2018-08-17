/*
 * Copyright ©2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
#include "includes.h"

static
DWORD
VmDirRESTTestIfHostIPMatch(
    PSTR        pszOrigin,
    BOOLEAN     *pbMatch
    );

DWORD
VmDirRESTSetCORSHeaders(
    PVDIR_REST_OPERATION    pRestOp,
    PREST_RESPONSE*         ppResponse
    )
{
    DWORD   dwError = 0;

    if (!pRestOp || !ppResponse)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //determine if Origin header is present & set CORS headers accordingly
    if (pRestOp->bisValidOrigin)
    {
        dwError = VmRESTSetHttpHeader (ppResponse,
                      "Access-Control-Allow-Origin", pRestOp->pszOrigin);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmRESTSetHttpHeader (ppResponse,
                      "Access-Control-Allow-Headers",
                      "Origin, X-Requested-With, Content-Type, Authorization");
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmRESTSetHttpHeader (ppResponse,
                      "Access-Control-Allow-Methods",
                      "GET, OPTIONS, PUT, DELETE, PATCH");
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

DWORD
VmDirRESTIsValidOrigin(
    PSTR        pszOrigin,
    BOOLEAN     *pbIsValidOrigin
    )
{
    DWORD dwError = 0;
    BOOLEAN  bIsValidOrigin = FALSE;
    PSTR pszDomainName = NULL;

    if (IsNullOrEmptyString(pszOrigin) || !pbIsValidOrigin)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringStartsWith (pszOrigin, HTTP_PROTOCOL_PREFIX, FALSE))
    {
        // get the part of origin after "https://"
        PSTR  pszOriginValue = pszOrigin + strlen(HTTP_PROTOCOL_PREFIX);

        if (VmDirIsIPAddrFormat(pszOriginValue))
        {
            dwError = VmDirRESTTestIfHostIPMatch(pszOrigin, &bIsValidOrigin);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirRESTGetDomainName(&pszDomainName);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirStringEndsWith(
                    pszOriginValue,
                    pszDomainName,
                    FALSE /* case insensitive */
                    ))
            {
                bIsValidOrigin = TRUE;
            }
        }
    }
    *pbIsValidOrigin = bIsValidOrigin;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

static
DWORD
VmDirRESTTestIfHostIPMatch(
    PSTR        pszOrigin,
    BOOLEAN     *pbMatch
    )
{
    DWORD dwError = 0;
    BOOLEAN bMatch = FALSE;
    char pszAddr[INET_ADDRSTRLEN];
    struct ifaddrs *addrs = NULL, *pCur = NULL;
    struct sockaddr_in *pAddr = NULL;

    if (IsNullOrEmptyString(pszOrigin) || !pbMatch)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Compare with current IP addrss
    if (getifaddrs(&addrs) < 0)
    {
        dwError = VMDIR_ERROR_REST_IP_UNKNOWN;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pCur = addrs;

    while (pCur)
    {
        if (pCur->ifa_addr  != NULL)
        {
            pAddr = (struct sockaddr_in *)pCur->ifa_addr;
            if (!inet_ntop(AF_INET, &(pAddr->sin_addr), pszAddr, INET_ADDRSTRLEN))
            {
               dwError = VMDIR_ERROR_REST_IP_UNKNOWN;
               BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (!strncasecmp(pszOrigin + strlen(HTTP_PROTOCOL_PREFIX), pszAddr, strlen(pszAddr)))
            {
                    bMatch = TRUE;
                    break;
            }
        }
        pCur = pCur->ifa_next;
    }

    *pbMatch = bMatch;

cleanup:
    if (addrs)
    {
        freeifaddrs(addrs);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}
