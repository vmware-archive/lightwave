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



/*
 * Module Name: VMCA common
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * structure definitions
 *
 */

 #ifndef __COMMON_STRUCT__
 #define __COMMON_STRUCT__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VMCA_ERROR_CODE_NAME_DESC
{
    int         code;
    const char* name;
    const char* desc;

} VMCA_ERROR_CODE_NAME_DESC, *PVMCA_ERROR_CODE_NAME_DESC;

//
// threading definition
//


#if 0
typedef struct _VMCA_MUTEX
{
    BOOLEAN bInitialized;
    pthread_mutex_t critSect;
} VMCA_MUTEX;

typedef struct _VMCA_COND
{
    BOOLEAN bInitialized;
    pthread_cond_t cond;
} VMCA_COND;
#endif

typedef struct _VMCA_LOG_GLOBALS
{
    pthread_mutex_t mutex;

    BOOLEAN         bShutdownInProgress;

} VMCA_LOG_GLOBALS, *PVMCA_LOG_GLOBALS;


// common struct
#if 0
typedef struct _VMCA_THREAD_START_INFO
{
    VMCAStartRoutine* pStartRoutine;
    PVOID pArgs;
} VMCA_THREAD_START_INFO, * PVMCA_THREAD_START_INFO;
#endif

typedef struct _VMCA_FILE_LOG_HANDLE
{
    FILE*            fp;
    char*            fileName;
} VMCA_FILE_LOG_HANDLE, *PVMCA_FILE_LOG_HANDLE;

typedef struct _VMCA_SYS_LOG_HANDLE
{
    char*            msg;
} VMCA_SYS_LOG_HANDLE, *PVMCA_SYS_LOG_HANDLE;

typedef struct _VMCA_LOG_HANDLE
{
    VMCA_LOG_TYPE    logType;
    BOOL             initialized;
    union
    {
        VMCA_FILE_LOG_HANDLE fLog;
        VMCA_SYS_LOG_HANDLE  sLog;
    };

} VMCA_LOG_HANDLE;


typedef struct _VMCA_LOG_LEVEL_TABLE
{
    VMCA_LOG_LEVEL  level;
    PCSTR           pTag;
    int             levelAsInt;
} VMCA_LOG_LEVEL_TABLE, *PVMCA_LOG_LEVEL_TABLE;

#if 0
typedef struct _VMCA_THREAD_INFO
{
    VMCA_THREAD                 tid;
    BOOLEAN                     bJoinThr;       // join by main thr

    // mutexUsed is real mutex used (i.e. it may not == mutex)
    PVMCA_MUTEX mutex;
    PVMCA_MUTEX mutexUsed;

    // conditionUsed is real condition used (i.e. it may not == condition)
    PVMCA_COND               condition;
    PVMCA_COND               conditionUsed;

    struct _VDIR_THREAD_INFO*   pNext;

} REPO_THREAD_INFO, *PVDIR_THREAD_INFO;
#endif

typedef struct _VMCA_LDAP_CONTEXT
{
    LDAP* pConnection;

} VMCA_LDAP_CONTEXT;

//
// CONFIG
//

#define VMW_MAX_CONFIG_VALUE_BYTE_LENGTH (2048)

typedef struct _VMW_CFG_CONNECTION
{
    LONG   refCount;

#ifndef _WIN32
    HANDLE hConnection;
#endif

} VMW_CFG_CONNECTION;

typedef struct _VMW_CFG_KEY
{
    PVMW_CFG_CONNECTION pConnection;

    HKEY hKey;

} VMW_CFG_KEY;

typedef struct _VMCA_SASL_INTERACTIVE_DEFAULT
{
    PCSTR   pszRealm;
    PCSTR   pszAuthName;
    PCSTR   pszUser;
    PCSTR   pszPass;
} VMCA_SASL_INTERACTIVE_DEFAULT, *PVMCA_SASL_INTERACTIVE_DEFAULT;

typedef struct _VMCA_OPENSSL_GLOBALS
{
    int nSslMutexes;
    pthread_mutex_t* pSslMutexes;
} VMCA_OPENSSL_GLOBALS;

#ifdef __cplusplus
}
#endif


#endif //__COMMON_STRUCT__

