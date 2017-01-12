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



#if !defined(_WIN32) || defined(HAVE_PTHREADS_WIN32)

typedef struct _VMDIR_MUTEX
{
    BOOLEAN bInitialized;
    pthread_mutex_t critSect;
} VMDIR_MUTEX;

typedef struct _VMDIR_COND
{
    BOOLEAN bInitialized;
    pthread_cond_t cond;
} VMDIR_COND;

#else /* use Windows native threading */

typedef struct _VMDIR_MUTEX
{
    BOOLEAN bInitialized;
    CRITICAL_SECTION critSect;
} VMDIR_MUTEX;

typedef struct _VMDIR_COND_2008
{
    BOOLEAN bInitialized;
    CONDITION_VARIABLE cond;
} VMDIR_COND_2008, *PVMDIR_COND_2008;

typedef struct _VMDIR_COND_2003
{
    HANDLE hSignalEvent;
    HANDLE hBroadcastEvent;
}VMDIR_COND_2003, *PVMDIR_COND_2003;

#ifdef WIN2008

typedef VMDIR_COND_2008  VMDIR_COND;

#else  /* WIN2003 */

typedef VMDIR_COND_2003  VMDIR_COND;

#endif /* WIN2003 */

#endif /* HAVE_DCERPC_WIN32 */

typedef struct _VMDIR_THREAD_START_INFO
{
    VmDirStartRoutine* pStartRoutine;
    PVOID pArgs;
} VMDIR_THREAD_START_INFO, * PVMDIR_THREAD_START_INFO;

typedef struct _VMDIR_SYNCHRONIZE_COUNTER
{
    BOOLEAN                 bInitialized;
    PVMDIR_MUTEX            pMutex;
    PVMDIR_COND             pCond;
    DWORD                   iCondWaitTimeInMilliSec;
    size_t                  iCounter;       // current value
    size_t                  iSyncValue;     // value to wait and synchronize upon
    VMDIR_SYNC_MECHANISM    wakeupMethod;

} VMDIR_SYNCHORNIZE_COUNTER;

#ifndef STRUCT_VMDIR_LOG_CTX
#define STRUCT_VMDIR_LOG_CTX
#ifndef _WIN32
typedef struct _VMDIR_LOG_CTX
{
    PSTR            pszLogFileName;
    FILE*           pFile;
    BOOLEAN         bSyslog;
    PSTR            pszSyslogDaemon;
    int             iLogMask;
    VMDIR_LOG_LEVEL iLogLevel;
} VMDIR_LOG_CTX;
#else
typedef struct _VMDIR_LOG_CTX
{
    PSTR            pszLogFileName;
    FILE*           pFile;
    BOOLEAN         bSyslog;
    PSTR            pszSyslogDaemon;
    int             iLogMask;
    VMDIR_LOG_LEVEL iLogLevel;
    DWORD           dwMaxOldLogs;
    INT64           i64MaxLogSizeBytes;
    PVMDIR_MUTEX    pLogMutex;
    PVMDIR_THREAD   pThread;
    PVMDIR_MUTEX    pThreadMutex;
    PVMDIR_COND     pThreadCond;
    BOOLEAN         bThreadShouldExit;
} VMDIR_LOG_CTX;
#endif
#endif

typedef struct _VMDIR_SASL_INTERACTIVE_DEFAULT
{
    PCSTR   pszRealm;
    PCSTR   pszAuthName;
    PCSTR   pszUser;
    PCSTR   pszPass;
} VMDIR_SASL_INTERACTIVE_DEFAULT, *PVMDIR_SASL_INTERACTIVE_DEFAULT;

//IPC
typedef struct  _VM_DIR_CONNECTION_
{
#if defined _WIN32
	HANDLE hConnection;
#else
	int fd;
#endif
} VM_DIR_CONNECTION;

typedef struct _VM_DIR_SECURITY_CONTEXT_
{
#if defined _WIN32
        PSID pSid;
        BOOL bRoot;
#else
        uid_t uid;
#endif
}VM_DIR_SECURITY_CONTEXT;
