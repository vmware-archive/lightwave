/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

BOOLEAN
VmIsIPV6AddrFormat(
    PCSTR   pszAddr
    )
{
    struct sockaddr_in6 sa = {0};
    return (inet_pton(AF_INET6, pszAddr, &(sa.sin6_addr)) != 0);
}

DWORD
VmFormatUrl(
    PCSTR    pszScheme,
    PCSTR    pszHost,
    DWORD    dwPort,
    PCSTR    pszPath,
    PCSTR    pszQuery,
    PSTR*    ppszUrl
    )
{
    DWORD dwError = 0;
    PSTR pszUrl = NULL;
    PSTR pszPort = NULL;
    PSTR pszHostPrefix = "";
    PSTR pszHostSuffix = "";

    if (IsNullOrEmptyString(pszScheme) ||
        IsNullOrEmptyString(pszHost) ||
        IsNullOrEmptyString(pszPath))
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (VmIsIPV6AddrFormat(pszHost))
    {
        pszHostPrefix = "[";
        pszHostSuffix = "]";
    }

    if (dwPort)
    {
        dwError = VmAllocateStringPrintf(
                      &pszPort,
                      ":%d",
                      dwPort);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateStringPrintf(
                  &pszUrl,
                  "%s://%s%s%s%s%s%s",
                  pszScheme,
                  pszHostPrefix,
                  pszHost,
                  pszHostSuffix,
                  (pszPort ? pszPort : ""),
                  pszPath,
                  (pszQuery ? pszQuery : ""));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppszUrl = pszUrl;

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszPort);
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pszUrl);
    goto cleanup;
}
