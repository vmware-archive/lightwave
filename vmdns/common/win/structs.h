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


#define VMDNS_MAX_CONFIG_VALUE_BYTE_LENGTH (2048)

typedef struct _VMW_MUTEX
{
    CRITICAL_SECTION critSect;
} VMW_MUTEX;

typedef struct _VMW_EVENT
{
    CONDITION_VARIABLE cond;
} VMW_EVENT;

typedef struct _VMW_THREAD
{
    HANDLE    hThread;
    BOOLEAN   bDetached;
} VMW_THREAD;

typedef struct _VMDNS_LOG_CTX
{
    PSTR             pszLogFileName;
    FILE            *pFile;
    BOOLEAN          bSyslog;
    PSTR             pszSyslogDaemon;
    DWORD            dwMaxOldLogs;
    INT64            i64MaxLogSizeBytes;
    pthread_mutex_t  pLogMutex;
    pthread_t       *pThread;
    pthread_mutex_t  pThreadMutex;
    pthread_cond_t   pThreadCond;
    BOOLEAN          bThreadShouldExit;
} VMDNS_LOG_CTX, *PVMDNS_LOG_CTX;

typedef struct _VMDNS_COND
{
	BOOLEAN                 bInitialized;
	pthread_cond_t          cond;
} VMDNS_COND;

typedef struct _VMDNS_THREAD_START_INFO
{
	VmDnsStartRoutine*      pStartRoutine;
	PVOID                   pArgs;
} VMDNS_THREAD_START_INFO, *PVMDNS_THREAD_START_INFO;

typedef struct _VMDNS_MUTEX
{
	BOOLEAN                 bInitialized;
	pthread_mutex_t         critSect;
} VMDNS_MUTEX;

typedef struct _VMDNS_CFG_CONNECTION
{
	LONG                    refCount;
#ifndef _WIN32
	HANDLE                  hConnection;
#endif
} VMDNS_CFG_CONNECTION;

typedef struct _VMDNS_CFG_KEY
{
	PVMDNS_CFG_CONNECTION   pConnection;
	HKEY                    hKey;
} VMDNS_CFG_KEY;
