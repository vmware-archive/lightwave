/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifndef __COMMON_STRUCT__
#define __COMMON_STRUCT__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _LWCA_ERROR_CODE_NAME_DESC
{
    int         code;
    const char* name;
    const char* desc;

} LWCA_ERROR_CODE_NAME_DESC, *PLWCA_ERROR_CODE_NAME_DESC;

// common struct

typedef struct _LWCA_LOG_GLOBALS
{
    pthread_mutex_t mutex;

    BOOLEAN         bShutdownInProgress;

} LWCA_LOG_GLOBALS, *PLWCA_LOG_GLOBALS;

typedef struct _LWCA_FILE_LOG_HANDLE
{
    FILE*            fp;
    char*            fileName;
} LWCA_FILE_LOG_HANDLE, *PLWCA_FILE_LOG_HANDLE;

typedef struct _LWCA_SYS_LOG_HANDLE
{
    char*            msg;
} LWCA_SYS_LOG_HANDLE, *PLWCA_SYS_LOG_HANDLE;

typedef struct _LWCA_LOG_HANDLE
{
    LWCA_LOG_TYPE    logType;
    BOOL             initialized;
    union
    {
        LWCA_FILE_LOG_HANDLE fLog;
        LWCA_SYS_LOG_HANDLE  sLog;
    };

} LWCA_LOG_HANDLE;


typedef struct _LWCA_LOG_LEVEL_TABLE
{
    LWCA_LOG_LEVEL  level;
    PCSTR           pTag;
    int             levelAsInt;
} LWCA_LOG_LEVEL_TABLE, *PLWCA_LOG_LEVEL_TABLE;

typedef struct _LWCA_LDAP_CONTEXT
{
    LDAP* pConnection;

} LWCA_LDAP_CONTEXT;

//
// CONFIG
//

#define LWCA_MAX_CONFIG_VALUE_BYTE_LENGTH (2048)

typedef struct _LWCA_CFG_CONNECTION
{
    LONG   refCount;

    HANDLE hConnection;

} LWCA_CFG_CONNECTION;

typedef struct _LWCA_CFG_KEY
{
    PLWCA_CFG_CONNECTION pConnection;

    HKEY hKey;

} LWCA_CFG_KEY;

typedef struct _LWCA_SASL_INTERACTIVE_DEFAULT
{
    PCSTR   pszRealm;
    PCSTR   pszAuthName;
    PCSTR   pszUser;
    PCSTR   pszPass;
} LWCA_SASL_INTERACTIVE_DEFAULT, *PLWCA_SASL_INTERACTIVE_DEFAULT;

typedef struct _LWCA_OPENSSL_GLOBALS
{
    int nSSLMutexes;
    pthread_mutex_t* pSSLMutexes;
} LWCA_OPENSSL_GLOBALS;

#ifdef __cplusplus
}
#endif


#endif //__COMMON_STRUCT__

