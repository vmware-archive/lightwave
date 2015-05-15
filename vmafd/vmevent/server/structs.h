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
 * Module Name: ThinAppRepoService
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Structure definitions
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
typedef PSECURITY_DESCRIPTOR PSECURITY_DESCRIPTOR_ABSOLUTE;
#endif

typedef struct _EVENTLOG_SERVER_GLOBALS
{
    PSECURITY_DESCRIPTOR_ABSOLUTE   gSecurityDescriptor;
    dcethread*                      pRPCServerThread;

} EVENTLOG_SERVER_GLOBALS, *PEVENTLOG_SERVER_GLOBALS;

typedef struct _EVENTLOG_ENDPOINT
{
    PCSTR protocol;
    PCSTR endpoint;
} EVENTLOG_ENDPOINT, *PEVENTLOG_ENDPOINT;

#ifdef _WIN32

typedef struct _VMEVENT_NTSERVICE_DATA
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    HANDLE stopServiceEvent;
} VMEVENT_NTSERVICE_DATA, *PVMEVENT_NTSERVICE_DATA;


typedef struct _VMEVENT_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    // static fields initialized during server startup.
    // their values never change, so no access protection necessary.
    PSTR                            pszLogFile;

    // following fields are protected by mutex
    pthread_t                       thread;
    pthread_mutex_t                 mutex;

    pthread_cond_t                  statusCond;
    VMEVENT_STATUS                    status;

    dcethread*                      pRPCServerThread;

    BOOLEAN                         bRegisterTcpEndpoint;
    DWORD                           iPort;

} VMEVENT_GLOBALS, *PVMEVENT_GLOBALS;

#endif //_WIN32

#ifdef __cplusplus
}
#endif
