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
 * Module Name: dns Main
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * dns Main module
 *
 * Private Structures
 *
 */

typedef enum _VMDNS_CONFIG_VALUE_TYPE
{
    VMDNS_CONFIG_VALUE_TYPE_STRING = 0,
    VMDNS_CONFIG_VALUE_TYPE_DWORD,
    VMDNS_CONFIG_VALUE_TYPE_BOOLEAN
} VMDNS_CONFIG_VALUE_TYPE;

typedef struct _VMDNS_RPC_ENDPOINT
{
    PCSTR pszEndPointType;
    PCSTR pszEndPointName;
} VMDNS_RPC_ENDPOINT, *PVMDNS_RPC_ENDPOINT;

typedef struct _VMCA_SASL_INTERACTIVE_DEFAULT
{
    PCSTR   pszRealm;
    PCSTR   pszAuthName;
    PCSTR   pszUser;
    PCSTR   pszPass;
} VMDNS_SASL_INTERACTIVE_DEFAULT, *PVMDNS_SASL_INTERACTIVE_DEFAULT;

#ifdef _WIN32

typedef struct _VMDNS_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE   hServiceStatus;
    HANDLE                  stopServiceEvent;
} VMDNS_NTSERVICE_DATA, *PVMDNS_NTSERVICE_DATA;

#endif

typedef struct _VMDNS_DIR_SYNC_CONTEXT
{
    PVMDNS_THREAD               pSyncThread;
    PVMDNS_COND                 pRefreshEvent;
    PVMDNS_MUTEX                pThreadLock;
    BOOL                        bShutdown;
    BOOL                        bRunning;
} VMDNS_DIR_SYNC_CONTEXT, *PVMDNS_DIR_SYNC_CONTEXT;

