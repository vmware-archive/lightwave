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


typedef struct __VMAFD_MESSAGE_BUFFER
{
     size_t szMaxSize;
     size_t szCurrentSize;
     size_t szLength; //write cursor
     size_t szCursor; //read cursor
     BOOL  bCanWrite;
     BOOL  bTokenizeDomainName;
     PBYTE pMessage;
} VMAFD_MESSAGE_BUFFER;

typedef struct  _VM_AFD_CONNECTION_
{
#if defined _WIN32
    HANDLE hConnection;
#else
    int fd;
#endif
    DWORD pid;
} VM_AFD_CONNECTION;

typedef struct _VM_AFD_SECURITY_CONTEXT_
{
#if defined _WIN32
    PSID pSid;
#else
    uid_t uid;
#endif
} VM_AFD_SECURITY_CONTEXT;

typedef DWORD (*PFN_IPC_OPEN_CONNECTION) (VM_AFD_CONNECTION **ppConnection);
typedef VOID (*PFN_IPC_CLOSE_CONNECTION) (VM_AFD_CONNECTION *pConnection);
typedef DWORD (*PFN_IPC_ACCEPT_CONNECTION) (VM_AFD_CONNECTION *pConnection, VM_AFD_CONNECTION **ppConnection);
typedef DWORD (*PFN_IPC_READ_DATA) (VM_AFD_CONNECTION *pConnection, PBYTE *pResponse, PDWORD pdwResponseSize);
typedef DWORD (*PFN_IPC_WRITE_DATA) (VM_AFD_CONNECTION *pConnection, PBYTE pRequest, DWORD dwRequestSize);
typedef VOID (*PFN_IPC_FREE_CONNECTION) (VM_AFD_CONNECTION *pConnection);
typedef DWORD (*PFN_IPC_INITIALIZE_CONNECTION_CONTEXT) (VM_AFD_CONNECTION *pConnection, VM_AFD_CONNECTION_CONTEXT **ppConnectionContext);
typedef VOID (*PFN_IPC_FREE_CONNECTION_CONTEXT) (VM_AFD_CONNECTION_CONTEXT *pConnectionContext);
typedef BOOL (*PFN_IPC_CHECK_ROOT_CONTEXT) (VM_AFD_CONNECTION_CONTEXT *pSecurityContext);
typedef DWORD (*PFN_IPC_INITIALIZE_SECURITY) (VM_AFD_CONNECTION *pConnection, VM_AFD_SECURITY_CONTEXT **ppSecurityContext);
typedef VOID (*PFN_IPC_FREE_SECURITY_CONTEXT) (VM_AFD_SECURITY_CONTEXT *pSecurityContext);
typedef DWORD (*PFN_IPC_GET_CONTEXT_SIZE) (VM_AFD_SECURITY_CONTEXT *pSecurityContext, PDWORD pdwSize);
typedef DWORD (*PFN_IPC_ENCODE_CONTEXT) (VM_AFD_SECURITY_CONTEXT *pSecurityContext, PBYTE pByteSecurityContext, DWORD dwBuffSize, PDWORD pdwBuffUsed);
typedef DWORD (*PFN_IPC_DECODE_CONTEXT) (PBYTE pByteSecurityContext, DWORD dwBuffSize, VM_AFD_SECURITY_CONTEXT **ppSecurityContext);
typedef BOOL (*PFN_IPC_COMPARE_CONTEXT) (VM_AFD_SECURITY_CONTEXT *pSecurityContext1 , VM_AFD_SECURITY_CONTEXT *pSecurityContext2);
typedef DWORD (*PFN_IPC_ALLOCATE_FROM_NAME) (PCWSTR , VM_AFD_SECURITY_CONTEXT **ppSecurityContext);
typedef DWORD (*PFN_IPC_COPY_CONTEXT) (VM_AFD_SECURITY_CONTEXT *pSecurityContextSrc, VM_AFD_SECURITY_CONTEXT **ppSecurityContextDst);
typedef DWORD (*PFN_IPC_CREATE_ANONYMOUS_CONNECTION_CONTEXT) (VM_AFD_CONNECTION_CONTEXT **ppConnectionContext);
typedef DWORD (*PFN_IPC_CREATE_WELLKNOWN_CONTEXT) (VM_AFD_CONTEXT_TYPE contextType, VM_AFD_SECURITY_CONTEXT **ppSecurityContext);
typedef DWORD (*PFN_IPC_CHECK_ACL_CONTEXT) (VM_AFD_CONNECTION_CONTEXT *pConnectionContext, PSTR pszSddlAcl, BOOL *pIsAllowed);

typedef struct _VM_AFD_VTABLE_
{
    PFN_IPC_OPEN_CONNECTION pfnOpenServerConnection;
    PFN_IPC_CLOSE_CONNECTION pfnCloseServerConnection;
    PFN_IPC_OPEN_CONNECTION pfnOpenClientConnection;
    PFN_IPC_CLOSE_CONNECTION pfnCloseClientConnection;
    PFN_IPC_ACCEPT_CONNECTION pfnAcceptConnection;
    PFN_IPC_FREE_CONNECTION pfnFreeConnection;
    PFN_IPC_READ_DATA pfnReadData;
    PFN_IPC_WRITE_DATA pfnWriteData;
    PFN_IPC_INITIALIZE_SECURITY pfnInitializeSecurityContext;
    PFN_IPC_FREE_SECURITY_CONTEXT pfnFreeSecurityContext;
    PFN_IPC_GET_CONTEXT_SIZE pfnGetSecurityContextSize;
    PFN_IPC_ENCODE_CONTEXT pfnEncodeSecurityContext;
    PFN_IPC_DECODE_CONTEXT pfnDecodeSecurityContext;
    PFN_IPC_CHECK_ROOT_CONTEXT pfnIsRootSecurityContext;
    PFN_IPC_COMPARE_CONTEXT pfnEqualsContext;
    PFN_IPC_ALLOCATE_FROM_NAME pfnAllocateContextFromName;
    PFN_IPC_COPY_CONTEXT pfnCopySecurityContext;
    PFN_IPC_CREATE_ANONYMOUS_CONNECTION_CONTEXT pfnCreateAnonymousConnectionContext;
    PFN_IPC_CREATE_WELLKNOWN_CONTEXT pfnCreateWellKnownContext;
    PFN_IPC_COMPARE_CONTEXT pfnContextBelongsToGroup;
    PFN_IPC_CHECK_ACL_CONTEXT pfnCheckAclContext;
} VM_AFD_VTABLE;

#ifndef _WIN32
typedef struct _VMAFD_LOG_CTX
{
    PSTR             pszLogFileName;
    FILE            *pFile;
    BOOLEAN          bSyslog;
    PSTR             pszSyslogDaemon;
} VMAFD_LOG_CTX, *PVMAFD_LOG_CTX;
#else
typedef struct _VMAFD_LOG_CTX
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
} VMAFD_LOG_CTX, *PVMAFD_LOG_CTX;
#endif

typedef struct _VMAFD_SSL_GLOBALS
{
    DWORD dwNumMutexes;
    pthread_mutex_t* pMutexes;
} VMAFD_SSL_GLOBALS;
