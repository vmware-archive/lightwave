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
 * Module   : structs.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Common Utilities (Client & Server)
 *
 *            Private Structures
 */

typedef struct _VMDNS_MUTEX
{
    BOOLEAN                 bInitialized;
    pthread_mutex_t         critSect;
} VMDNS_MUTEX;

typedef struct _VMDNS_COND
{
    BOOLEAN                 bInitialized;
    pthread_cond_t          cond;
} VMDNS_COND;

typedef struct _VMDNS_RWLOCK
{
    pthread_rwlock_t    rwLock;
} VMDNS_RWLOCK;

typedef struct _VMDNS_THREAD_START_INFO
{
    VmDnsStartRoutine*      pStartRoutine;
    PVOID                   pArgs;
} VMDNS_THREAD_START_INFO, *PVMDNS_THREAD_START_INFO;

#define VMDNS_MAX_CONFIG_VALUE_BYTE_LENGTH (2048)

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

typedef struct _RecordTypeMap
{
    VMDNS_RR_TYPE   type;
    PCSTR           pszName;
} VMDNS_RECORD_TYPE_NAME_MAP;

typedef struct _ServiceNameMap
{
    VMDNS_SERVICE_TYPE  type;
    PCSTR               pszName;
    PCSTR               pszUserFriendlyName;
} VMDNS_SERVICE_TYPE_NAME_MAP;

typedef struct _ProtocolNameMap
{
    VMDNS_SERVICE_PROTOCOL  protocol;
    PCSTR                   pszName;
    PCSTR                   pszUserFriendlyName;
} VMDNS_SERVICE_PROTOCOL_NAME_MAP;

typedef BOOLEAN (*VMDNS_RECORD_COMPARE_FUNC) (
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

typedef VOID (*VMDNS_RECORD_CLEAR_FUNC) (
    PVMDNS_RECORD   pRecord
    );

typedef BOOLEAN (*VMDNS_RECORD_VALIDATE_FUNC) (
    PVMDNS_RECORD   pRecord
    );

typedef DWORD (*VMDNS_RECORD_DUPLICATE_FUNC) (
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD*  ppDest
    );

typedef DWORD (*VMDNS_RECORD_COPY_FUNC) (
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

typedef DWORD (*VMDNS_RECORD_TOSTRING_FUNC) (
    PVMDNS_RECORD   pSrc,
    PSTR*           ppStr
    );

typedef DWORD (*VMDNS_RECORD_GETCN_FUNC) (
    PVMDNS_RECORD   pSrc,
    PSTR*           ppStr
    );

typedef DWORD (*VMDNS_RECORD_GETRDLENTH_FUNC) (
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    );

typedef DWORD (*VMDNS_RECORD_SERIALIZE_DATA) (
    VMDNS_RECORD_DATA    RecordData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

typedef DWORD (*VMDNS_RECORD_DESERIALIZE_DATA) (
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA    pRecordData
    );

typedef struct _VMDNS_RECORD_METHODS
{
    VMDNS_RR_TYPE                 type;
    VMDNS_RECORD_COMPARE_FUNC     pfnCompare;
    VMDNS_RECORD_COMPARE_FUNC     pfnMatch;
    VMDNS_RECORD_CLEAR_FUNC       pfnClear;
    VMDNS_RECORD_CLEAR_FUNC       pfnRpcClear;
    VMDNS_RECORD_VALIDATE_FUNC    pfnValidate;
    VMDNS_RECORD_DUPLICATE_FUNC   pfnDuplicate;
    VMDNS_RECORD_DUPLICATE_FUNC   pfnRpcDuplicate;
    VMDNS_RECORD_COPY_FUNC        pfnCopy;
    VMDNS_RECORD_COPY_FUNC        pfnRpcCopy;
    VMDNS_RECORD_TOSTRING_FUNC    pfnToString;
    VMDNS_RECORD_GETCN_FUNC       pfnGetCN;
    VMDNS_RECORD_GETRDLENTH_FUNC  pfnGetRDataLength;
    VMDNS_RECORD_SERIALIZE_DATA   pfnSerialize;
    VMDNS_RECORD_DESERIALIZE_DATA pfnDeSerialize;
} VMDNS_RECORD_METHODS;

#ifdef _WIN32

typedef struct _VMDNS_LOG_CTX
{
    PSTR             pszLogFileName;
    FILE            *pFile;
    BOOLEAN          bSyslog;
    PSTR             pszSyslogDaemon;
    DWORD            dwMaxOldLogs;
	DWORD            dwMaxLogSizeBytes;
    pthread_mutex_t  pLogMutex;
    pthread_t       *pThread;
    pthread_mutex_t  pThreadMutex;
    pthread_cond_t   pThreadCond;
    BOOLEAN          bThreadShouldExit;
} VMDNS_LOG_CTX, *PVMDNS_LOG_CTX;

#endif
