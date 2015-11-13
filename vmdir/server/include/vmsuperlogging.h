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




#ifndef __VMDIR_SUPER_LOGGING_INTERFACE_H__
#define __VMDIR_SUPER_LOGGING_INTERFACE_H__

#define MAX_SEARCH_OPT_LEN  16
#define MAX_PARAMETERS_LEN  512

typedef struct _VMSUPERLOGGING_SEARCH_INFO
{
    char szAttributes[MAX_PARAMETERS_LEN];
    char szBaseDN[VMDIR_MAX_DN_LEN];
    char szScope[MAX_SEARCH_OPT_LEN];
    char szIndexResults[MAX_PARAMETERS_LEN];
    DWORD dwScanned;
    DWORD dwReturned;
} VMSUPERLOGGING_SEARCH_INFO;

typedef union _VMSUPERLOGGING_OPERATION_INFO
{
    VMSUPERLOGGING_SEARCH_INFO searchInfo;
} VMSUPERLOGGING_OPERATION_INFO;

typedef struct _VMSUPERLOGGING_ENTRY
{
    DWORD dwOperation; // LDAP_REQ_XXXX
    DWORD dwErrorCode; // LDAP-specific error code (not vmdir's)
    DWORD dwClientPort;
    DWORD dwServerPort;
    uint64_t iStartTime;
    uint64_t iEndTime;
    char szLoginDN[VMDIR_MAX_DN_LEN];
    char szClientIPAddress[INET6_ADDRSTRLEN];
    char szServerIPAddress[INET6_ADDRSTRLEN];
    char Parameter[MAX_PARAMETERS_LEN];
    VMSUPERLOGGING_OPERATION_INFO opInfo;
} VMSUPERLOGGING_ENTRY, *PVMSUPERLOGGING_ENTRY;

typedef struct _VMSUPERLOGGING
{
    BOOLEAN bEnabled;
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer;
} VMSUPERLOGGING, *PVMSUPERLOGGING;

DWORD
VmDirSuperLogQueryServerState(
    PVMDIR_SUPERLOG_SERVER_DATA *ppServerData
    );

DWORD
VmDirSuperLoggingInit(
    PVMSUPERLOGGING *ppLogger
    );

DWORD
VmDirEnableSuperLogging(
    PVMSUPERLOGGING pLogger
    );

DWORD
VmDirDisableSuperLogging(
    PVMSUPERLOGGING pLogger
    );

BOOLEAN
VmDirIsSuperLoggingEnabled(
    PVMSUPERLOGGING pLogger
    );

DWORD
VmDirFlushSuperLogging(
    PVMSUPERLOGGING pLogger
    );

DWORD
VmDirGetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    PDWORD pSize
    );

DWORD
VmDirSetSuperLoggingSize(
    PVMSUPERLOGGING pLogger,
    DWORD dwSize
    );

DWORD
VmDirSuperLoggingGetEntries(
    PVMSUPERLOGGING pLogger,
    UINT64 *pEnumerationCookie,
    DWORD dwCount,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppEntries
    );

VOID
VmDirLogOperation(
    PVMSUPERLOGGING pLogger,
    DWORD dwOperation, // LDAP_REQ_XXXX
    VDIR_CONNECTION *pConn,
    DWORD dwErrorCode
    );
#endif
