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
EventLogUpperCaseStringA(
    PSTR pszString)
{
    DWORD dwError = 0;
    size_t iLen = 0;
    size_t i = 0;

    iLen = EventLogStringLenA(pszString);

    BAIL_ON_VMEVENT_INVALID_POINTER(pszString, dwError);

    for (i=0; i<iLen; i++)
    {
        VMEVENT_ASCII_LOWER_TO_UPPER(pszString[i]);
    }

error:

    return dwError;
}

DWORD
EventLogLowerCaseStringA(
    PSTR pszString)
{
    DWORD dwError = 0;
    size_t iLen = 0;
    size_t i = 0;

    BAIL_ON_VMEVENT_INVALID_POINTER(pszString, dwError);

    iLen = EventLogStringLenA(pszString);

    for (i=0; i<iLen; i++)
    {
        VMEVENT_ASCII_UPPER_TO_LOWER(pszString[i]);
    }

error:

    return dwError;
}

DWORD
EventLogGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostname
    )
{
    DWORD  dwError = 0;
    struct addrinfo* pHostInfo = NULL;
    CHAR   szCanonicalHostname[NI_MAXHOST+1] = "";
    PSTR   pszCanonicalHostname = NULL;
    struct addrinfo hints = {0};

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_CANONNAME;

    dwError = getaddrinfo(
                      pszHostname,
                      NULL,
                      &hints,
                      &pHostInfo);
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = getnameinfo(
                      pHostInfo->ai_addr,
                      (socklen_t)(pHostInfo->ai_addrlen),
                      szCanonicalHostname,
                      NI_MAXHOST,
                      NULL,
                      0,
                      NI_NAMEREQD);
    BAIL_ON_VMEVENT_ERROR(dwError);

    if (!IsNullOrEmptyString(&szCanonicalHostname[0]))
    {
        dwError = EventLogAllocateStringA(
                    &szCanonicalHostname[0],
                    &pszCanonicalHostname);
    }
    else
    {
        dwError = ERROR_NO_DATA;
    }
    BAIL_ON_VMEVENT_ERROR(dwError);

    *ppszCanonicalHostname = pszCanonicalHostname;

cleanup:

    if (pHostInfo)
    {
        freeaddrinfo(pHostInfo);
    }

    return dwError;

error:

    *ppszCanonicalHostname = NULL;

    VMEVENT_SAFE_FREE_MEMORY(pszCanonicalHostname);

    goto cleanup;
}
