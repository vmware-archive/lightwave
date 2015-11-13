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



/*
 * Module Name:
 *
 * prototypes.h
 *
 * Abstract:
 *
 * Function prototypes definition.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif


/* authzn.c */

BOOL
IsRpcOperationAllowed(
    handle_t IDL_handle,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    DWORD dwAccessDesired
    );

DWORD
ConstructSDForEventLogServ(
    PSECURITY_DESCRIPTOR_ABSOLUTE * ppSD
    );

/* init.c */

DWORD
EventLogInitialize(
    VOID);

VOID
EventLogShutdown(
    VOID
    );

/* rpc.c */

DWORD
EventLogStartRpcServer(
	);

PVOID
EventLogListenRpcServer(
    PVOID pInfo
    );

DWORD
EventLogStopRpcServer(
    VOID
    );

/* rpcmemory.c */

DWORD
EventLogRpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
EventLogRpcFreeMemory(
    PVOID pMemory
    );

/* rpcstring.c */

DWORD
EventLogRpcAllocateStringW(
    RP_PWSTR  pwszSrc,
    RP_PWSTR* ppwszDst
    );

DWORD
EventLogRpcAllocateUnicodeStringFromAnsi(
    RP_PCSTR  pszSrc,
    RP_PWSTR* ppwszDst
    );

//

VOID
EventLogBlockSelectedSignals(
    VOID
    );

DWORD
EventLogHandleSignals(
    VOID
    );

#ifndef _WIN32
/* securitydescriptor.c */

DWORD
ConstructSecurityDescriptor(
    DWORD  dwAllowUserCount,
    PWSTR* ppwszAllowUsers,
    DWORD  dwDenyUserCount,
    PWSTR* ppwszDenyUsers,
    BOOLEAN bReadOnly,
    PSECURITY_DESCRIPTOR_RELATIVE* ppRelative,
    PDWORD pdwRelativeSize
    );

/* service.c */
#endif

DWORD
EventLogRPCInit(
    VOID
    );

VOID
EventLogRPCShutdown(
    VOID
    );


DWORD
EventLogServiceInit(
    VOID
    );

VOID
EventLogServiceShutdown(
    VOID
    );

/* utils.c */
DWORD
EventLogGetApplicationVersion(
    VOID
    );

VOID
EventLogRpcFreeEventEntryArray(
   PEVENTLOG_ENTRY pPkgEntryArray,
   DWORD               dwCount
   );


VOID
EventLogRpcFreeEventEntryContents(
    PEVENTLOG_ENTRY pPkgEntry
    );


VOID
EventLogRpcFreeEventContainer(
    PEVENTLOG_CONTAINER pContainer
    );

VOID
EventLogRpcFreeEventEntryArray(
   PEVENTLOG_ENTRY pPkgEntryArray,
   DWORD               dwCount
   );

VOID
EventLogRpcFreeEventEntryContents(
    PEVENTLOG_ENTRY pPkgEntry
    );

VOID
EventLogFreeEventContainer(
    PEVENTLOG_CONTAINER pContainer
    );

VOID
EventLogFreeEventEntryArray(
    PEVENTLOG_ENTRY pPkgEntryArray,
    DWORD               dwCount
    );

VOID
EventLogFreeEventEntryContents(
    PEVENTLOG_ENTRY pPkgEntry
    );

VOID
EventLogRpcFreeEventContainer(
    PEVENTLOG_CONTAINER pContainer
    );


/* enumpkgs.c */
DWORD
EventLogServerAddEvent(
   DWORD eventID,
   DWORD eventType,
   RP_PWSTR pszMessage
   );

DWORD
EventLogServerInitEnumEventsHandle(
    PDWORD pdwHandle
    );

DWORD
EventLogServerEnumEvents(
    DWORD    dwHandle,
    DWORD    dwStartIndex,
    DWORD    dwNumPackages,
    PEVENTLOG_CONTAINER * ppPkgContainer
    );



DWORD
EventLogRpcCloneEventContainer(
    PEVENTLOG_CONTAINER  pPkgContainer,
    PEVENTLOG_CONTAINER* ppPkgContainerRpc
    );

DWORD
EventLogRpcCloneEventEntryContents(
    PEVENTLOG_ENTRY  pPkgEntrySrc,
    PEVENTLOG_ENTRY  pPkgEntryDst
    );

DWORD
EventLogParseArgs(
    int         argc,
    char*       argv[],
    PBOOLEAN    pbConsoleMode
    );

DWORD
EventLogAllocateArgsAFromArgsW(
    int argc,
    WCHAR* argv[],
    PSTR** argvA
);

VOID
EventLogDeallocateArgsA(
    int argc,
    PSTR argv[]
);
#ifdef _WIN32
VOID
VMEventSrvSetStatus(
    VMEVENT_STATUS state
    );
VMEVENT_STATUS
VMEventSrvGetStatus(
    VOID
    );
#endif


#ifdef __cplusplus
}
#endif
