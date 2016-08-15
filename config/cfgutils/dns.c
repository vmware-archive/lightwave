/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
VmwDeploySetForwarders(
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszForwarders
    )
{
    DWORD  dwError = 0;
    PSTR   pszServername = "localhost";
    PCSTR  pszDelim = ",";
    PVMDNS_SERVER_CONTEXT pBinding = NULL;
    PCSTR  pszReadCursor = pszForwarders;

    if (IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszForwarders) ||
        !pszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    VMW_DEPLOY_LOG_INFO("Setting DNS Forwarders [%s]", pszForwarders);

    dwError = VmDnsOpenServerA(
                    pszServername,
                    pszUsername,
                    pszDomain,
                    pszPassword,
                    0,
                    NULL,
                    &pBinding);
    BAIL_ON_DEPLOY_ERROR(dwError);

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = 0;
        size_t len_delim = 0;
        char szForwarder[128];

        len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (len_name > sizeof(szForwarder)-1)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_DEPLOY_ERROR(dwError);
            }

            strncpy(&szForwarder[0], pszReadCursor, len_name);
            szForwarder[len_name] = '\0';

            if (!VmwDeployIsIPAddress(szForwarder))
            {
               VMW_DEPLOY_LOG_ERROR(
                   "Error: An invalid DNS forwarder [%s] was specified",
                   szForwarder);

               dwError = ERROR_INVALID_PARAMETER;
               BAIL_ON_DEPLOY_ERROR(dwError);
            }

            pszReadCursor += len_name;

            dwError = VmDnsAddForwarderA(pBinding, szForwarder);
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        len_delim = strspn(pszReadCursor, pszDelim);

        pszReadCursor += len_delim;
    }

cleanup:

    if (pBinding)
    {
        VmDnsCloseServer(pBinding);
    }

    return dwError;

error:

    goto cleanup;
}

