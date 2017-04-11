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

#include "stdafx.h"

#ifdef _WIN32
BOOL
IsRpcOperationAllowed(
    handle_t IDL_handle,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    DWORD dwAccessDesired
    )
{
    return TRUE;
}
#endif

DWORD
EventLogRpcCloneEventEntryContents(
    PEVENTLOG_ENTRY  pPkgEntrySrc,
    PEVENTLOG_ENTRY  pPkgEntryDst
    )
{
    DWORD dwError = 0;

    struct
    {
        PWSTR  pwszSrc;
        PWSTR* ppwszDst;
    }
    values[] =
    {
        {
            VMEVENT_SF_INIT(.pwszSrc, pPkgEntrySrc->pszMessage),
            VMEVENT_SF_INIT(.ppwszDst, &pPkgEntryDst->pszMessage)
        }
    };
    DWORD iValue = 0;

    for (; iValue < sizeof(values)/sizeof(values[0]); iValue++)
    {
        if (values[iValue].pwszSrc)
        {
            EventLogRpcAllocateStringW(
                            values[iValue].pwszSrc,
                            values[iValue].ppwszDst);
            BAIL_ON_VMEVENT_ERROR(dwError);
        }
    }
    pPkgEntryDst->dwEventId = pPkgEntrySrc->dwEventId;
    pPkgEntryDst->dwEventType = pPkgEntrySrc->dwEventType;
cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
EventLogRpcCloneEventContainer(
    PEVENTLOG_CONTAINER  pPkgContainer,
    PEVENTLOG_CONTAINER* ppPkgContainerRpc
    )
{
    DWORD dwError = 0;
    PEVENTLOG_CONTAINER pPkgContainerRpc = NULL;

    dwError = EventLogRpcAllocateMemory(
            sizeof(*pPkgContainerRpc),
            (PVOID*)&pPkgContainerRpc);
    BAIL_ON_VMEVENT_ERROR(dwError);

    pPkgContainerRpc->dwCount = pPkgContainer->dwCount;
    pPkgContainerRpc->dwTotalCount = pPkgContainer->dwTotalCount;

    if (pPkgContainer->dwCount > 0)
    {
        DWORD iEntry = 0;

        dwError = EventLogRpcAllocateMemory(
                     pPkgContainer->dwCount *
                             sizeof(pPkgContainer->pPkgEntries[0]),
                     (PVOID*)&pPkgContainerRpc->pPkgEntries);
        BAIL_ON_VMEVENT_ERROR(dwError);

        for (; iEntry < pPkgContainer->dwCount; iEntry++)
        {
            PEVENTLOG_ENTRY pPkgEntrySrc =
                                        &pPkgContainer->pPkgEntries[iEntry];
            PEVENTLOG_ENTRY pPkgEntryDst =
                                        &pPkgContainerRpc->pPkgEntries[iEntry];

            dwError = EventLogRpcCloneEventEntryContents(pPkgEntrySrc, pPkgEntryDst);
            BAIL_ON_VMEVENT_ERROR(dwError);
        }
    }

    *ppPkgContainerRpc = pPkgContainerRpc;

cleanup:

    return dwError;

error:

    *ppPkgContainerRpc = NULL;

    if (pPkgContainerRpc)
    {
        EventLogRpcFreeEventContainer(pPkgContainerRpc);
    }

    goto cleanup;
}

DWORD
RpcEventLogAdd(
    handle_t IDL_handle,
    unsigned int eventID,
    unsigned int eventType,
    RP_PWSTR pszMessage
    )
{
    DWORD dwError = 0;

    if (!IsRpcOperationAllowed(
                    IDL_handle,
                    gEventLogServerGlobals.gSecurityDescriptor,
                    GENERIC_READ))
    {
       dwError = ERROR_ACCESS_DENIED;
       BAIL_ON_VMEVENT_ERROR(dwError);
    }
    dwError = EventLogServerAddEvent(
                 eventID,
                 eventType,
                 pszMessage);
    BAIL_ON_VMEVENT_ERROR(dwError);

error:
    return dwError;
}

DWORD
RpcEventLogInitEnumHandle(
    handle_t IDL_handle,
    unsigned int * pdwHandle
    )
{
    DWORD dwError = 0;

    if (!IsRpcOperationAllowed(
                    IDL_handle,
                    gEventLogServerGlobals.gSecurityDescriptor,
                    GENERIC_READ))
    {
       dwError = ERROR_ACCESS_DENIED;
       BAIL_ON_VMEVENT_ERROR(dwError);
    }

    dwError = EventLogServerInitEnumEventsHandle(
                    pdwHandle
                    );
    BAIL_ON_VMEVENT_ERROR(dwError);

error:

    return dwError;
}

DWORD
RpcEventLogEnumEvents(
    handle_t IDL_handle,
    unsigned int dwHandle,
    unsigned int dwStartIndex,
    unsigned int dwNumPackages,
    EVENTLOG_CONTAINER **ppPkgContainer
    )
{
    DWORD dwError = 0;
    PEVENTLOG_CONTAINER pPkgContainer = NULL;
    PEVENTLOG_CONTAINER pPkgContainerRpc = NULL;

    if (!IsRpcOperationAllowed(
                IDL_handle,
                gEventLogServerGlobals.gSecurityDescriptor,
                GENERIC_READ))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    dwError = EventLogServerEnumEvents(
                    dwHandle,
                    dwStartIndex,
                    dwNumPackages,
                    &pPkgContainer);
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = EventLogRpcCloneEventContainer(pPkgContainer, &pPkgContainerRpc);

    BAIL_ON_VMEVENT_ERROR(dwError);

    *ppPkgContainer = pPkgContainerRpc;

cleanup:

    if (pPkgContainer)
    {
        EventLogFreeEventContainer(pPkgContainer);
    }

    printf("Error = %d\n", dwError);
    return dwError;

error:

    *ppPkgContainer = NULL;

    if (pPkgContainerRpc)
    {
        EventLogRpcFreeEventContainer(pPkgContainerRpc);
    }

    goto cleanup;
}
