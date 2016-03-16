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




#ifndef _SLCLI_SUPERLOG_WRAPPER_H_
#define _SLCLI_SUPERLOG_WRAPPER_H_

typedef struct _VMAFD_SERVER_CONTEXT
{
    handle_t hBinding;
} VMAFD_SERVER_CONTEXT, *PVMAFD_SERVER_CONTEXT;

typedef struct _SUPERLOG_WRAPPER
{
    PSTR pszNetworkAddress;
    PSTR pszDomain;
    PVMAFD_SERVER_CONTEXT pServerContext;
    PVMAFD_SERVER pServer;
} SUPERLOG_WRAPPER, *PSUPERLOG_WRAPPER;


DWORD
CreateSuperlogWrapper(
        PSTR pszNetworkAddress,
        PSTR pszDomain,
        PSTR pszUserName,
        PSTR pszPassword,
        PSUPERLOG_WRAPPER *ppSuperlogWrapper
        );


DWORD
EnableSuperlog(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

DWORD
IsSuperlogEnabled(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PBOOLEAN pbEnabled
        );

DWORD
DisableSuperlog(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

DWORD
SetSuperlogBufferSize(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        DWORD dwSize
        );

DWORD
GetSuperlogBufferSize(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PDWORD pdwSize
        );

DWORD
RetrieveSuperlogEntries(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries,
        UINT32 **ppEnumerationCookie,
        DWORD dwCount,
        BOOLEAN flush
        );

DWORD
ClearSuperlogBuffer(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );

VOID
FreeSuperlogWrapper(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        );


#endif /* _SLCLI_SUPERLOG_WRAPPER_H_ */
