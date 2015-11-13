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


typedef struct _VMW_MUTEX
{
    pthread_mutex_t  mtx;
    pthread_mutex_t* pMtx;

} VMW_MUTEX;

typedef struct _VMW_EVENT
{
    pthread_cond_t  cond;
    pthread_cond_t* pCond;

} VMW_EVENT;

typedef struct _VMW_THREAD
{
    pthread_t thread;
    BOOLEAN   bJoin;
} VMW_THREAD;

typedef struct _VMW_LOG_HANDLE
{
    pthread_mutex_t  mutex;
    pthread_mutex_t* pMutex;

    VMW_LOG_TARGET   logTarget;

    union
    {
        FILE*   pFile;
        BOOLEAN bCloseSyslog;
    };

} VMW_LOG_HANDLE;

typedef struct _VMW_CFG_CONNECTION
{
    LONG   refCount;

    HANDLE hConnection;

} VMW_CFG_CONNECTION;

typedef struct _VMW_CFG_KEY
{
    PVMW_CFG_CONNECTION pConnection;

    HKEY hKey;

} VMW_CFG_KEY;

