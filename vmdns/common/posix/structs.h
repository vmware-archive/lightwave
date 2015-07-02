/*
 * Copyright (C) 2012 VMware, Inc. All rights reserved.
 *
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

