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



#include "includes.h"

static
DWORD
InitializeResourceLimit(
    VOID
);

static
DWORD
VmAfdGetDbPath(
    PSTR *ppszDbPath
    );

static
DWORD
InitializeDatabase(
    VOID
);

static
DWORD
InitializeSystemStores(
    VOID
    );

static
VOID
InitializeGlobals(
    PVMAFD_GLOBALS pGlobals)
{
    pGlobals->status = VMAFD_STATUS_INITIALIZING;
    pGlobals->pRPCServerThread = NULL;

    pthread_mutex_init(&pGlobals->mutex, NULL);
    pthread_cond_init(&pGlobals->statusCond, NULL);

    pGlobals->mutexConnection = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pGlobals->pCertUpdateMutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pGlobals->mutexStoreState = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    pGlobals->mutexCreateStore = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    pGlobals->rwlockStoreMap = (pthread_rwlock_t) PTHREAD_RWLOCK_INITIALIZER;
}

static
DWORD
VmAfdVmDirClientInit(
    VOID)
{
    DWORD dwError = 0;
    CHAR pszPath[VMAFD_PATH_MAX];

    dwError = VmDirGetVmDirLogPath(pszPath, "vmafdvmdirclient.log");
    BAIL_ON_VMAFD_ERROR(dwError);

    VmDirLogInitialize( pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL);

error:

    return dwError;
}

/*
 * Initialize vmafdd components
 */
DWORD
VmAfdInit(
    VOID
    )
{
    DWORD dwError = 0;

    InitializeGlobals(&gVmafdGlobals);

    dwError = VmAfdVmDirClientInit();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLogInitialize(
                    gVmafdGlobals.pszLogFile,
                    gVmafdGlobals.dwMaxOldLogs,
                    gVmafdGlobals.dwMaxLogSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = InitializeResourceLimit();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfCfgInit();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = InitializeDatabase();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = InitializeSystemStores();
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdOpenSSLInit();
    BAIL_ON_VMAFD_ERROR (dwError);

    if (gVmafdGlobals.bEnableRPC)
    {
      dwError = VmAfdRpcServerInit();
      BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdIpcServerInit();
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfSrvInitHeartbeatTable();
    BAIL_ON_VMAFD_ERROR (dwError);

    // One of the decisions is not  to check return value to prevent failure of AFD because of SL
    dwError = VmAfdSuperLoggingInit(&(gVmafdGlobals.pLogger));
    BAIL_ON_VMAFD_ERROR (dwError);

error:

    return dwError;
}

/*
 * Set process resource limits
 */
static
DWORD
InitializeResourceLimit(
    VOID
    )
{
    DWORD           dwError = 0;
    BAIL_ON_VMAFD_ERROR(dwError);

#if !defined(_WIN32) && !defined(PLATFORM_VMWARE_ESX)
    struct rlimit   VMLimit = {0};

    // unlimited virtual memory
    VMLimit.rlim_cur = RLIM_INFINITY;
    VMLimit.rlim_max = RLIM_INFINITY;

    if ( setrlimit(RLIMIT_AS, &VMLimit)     // virtual memory
         ||
         setrlimit(RLIMIT_CORE, &VMLimit)   // core file size
         ||
         setrlimit(RLIMIT_NPROC, &VMLimit)  // thread
       )
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VMLimit.rlim_cur = VMAFD_OPEN_FILES_MAX;
    VMLimit.rlim_max = VMAFD_OPEN_FILES_MAX;
    if (setrlimit(RLIMIT_NOFILE, &VMLimit)!=0)
    {
       //If VMAFD_OPEN_FILES_MAX is too large to set, try to set soft limit to hard limit
       if (getrlimit(RLIMIT_NOFILE, &VMLimit)==0)
       {
            VMLimit.rlim_cur = VMLimit.rlim_max;
            setrlimit(RLIMIT_NOFILE, &VMLimit);
       }
    }
#endif

error:

    return dwError;
}

/*
 * Main initialization loop.
 */
DWORD
VmAfdInitLoop(
    PVMAFD_GLOBALS pGlobals)
{
    DWORD dwError = 0;
    int sts = 0;
    time_t now = 0;
    struct timespec timeout = {0};

    while (1)
    {
        switch (VmAfdSrvGetStatus())
        {
            case VMAFD_STATUS_INITIALIZING:
                pthread_mutex_lock(&pGlobals->mutex);
                now = time(NULL);
                timeout.tv_sec = now + 60;
                timeout.tv_nsec = 0;
                sts = 0;
                while (pGlobals->status == VMAFD_STATUS_INITIALIZING && sts == 0)
                {
                    sts = pthread_cond_timedwait(&pGlobals->statusCond,
                                                 &pGlobals->mutex,
                                                 &timeout);
                }
                pthread_mutex_unlock(&pGlobals->mutex);

                if (VmAfdSrvGetStatus() == VMAFD_STATUS_INITIALIZING)
                {
                    VmAfdSrvSetStatus(VMAFD_STATUS_RUNNING);
                }

                break;

            case VMAFD_STATUS_RUNNING:
                pthread_mutex_lock(&pGlobals->mutex);
                while (pGlobals->status == VMAFD_STATUS_RUNNING)
                {
                    pthread_cond_wait(&pGlobals->statusCond,
                                      &pGlobals->mutex);
                }
                pthread_mutex_unlock(&pGlobals->mutex);
                break;

            case VMAFD_STATUS_STOPPING:
                goto cleanup;

            case VMAFD_STATUS_UNKNOWN:
                pthread_mutex_lock(&pGlobals->mutex);
                while (pGlobals->status == VMAFD_STATUS_UNKNOWN)
                {
                    pthread_cond_wait(&pGlobals->statusCond,
                                      &pGlobals->mutex);
                }
                pthread_mutex_unlock(&pGlobals->mutex);
                break;

            case VMAFD_STATUS_PAUSED:
            case VMAFD_STATUS_STOPPED:
            default:
                break;
        }
    }

cleanup:

    return dwError;
}

#ifdef _WIN32
static
DWORD
VmAfdGetDbPath(
                PSTR *ppszDbPath
               )
{
    DWORD dwError = 0;
    PSTR pszDbBasePath = NULL;
    PSTR pszDbPath = NULL;
    DWORD dwPathLength = 0;

    dwError = VecsSrvGetDBBasePath(
                                   &pszDbBasePath
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwPathLength = VmAfdStringLenA(pszDbBasePath) +
                   VmAfdStringLenA(VMAFD_CERT_DB_FILE) + 1;

    dwError = VmAfdAllocateMemory(
                                  dwPathLength,
                                  (PVOID *)&pszDbPath
                                 );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdStringPrintFA(
                                 pszDbPath,
                                 dwPathLength,
                                 "%s%s",
                                 pszDbBasePath,
                                 VMAFD_CERT_DB_FILE
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszDbPath = pszDbPath;

cleanup:
    VMAFD_SAFE_FREE_STRINGA (pszDbBasePath);
    return dwError;

error:
    if (ppszDbPath)
    {
        *ppszDbPath = NULL;
    }

    VMAFD_SAFE_FREE_STRINGA (pszDbPath);

    goto cleanup;
}

#else
static
VOID
VmAfdMoveOldDbFile(
                   VOID
                  )
{
    DWORD dwError = 0;
    PSTR pszDbPath = NULL;
    PSTR pszDbOldPath = NULL;
    BOOLEAN bOldFileExists = FALSE;
    BOOLEAN bNewFieExists = FALSE;

    dwError = VmAfdAllocateStringA(
                                    VMAFD_CERT_DB,
                                    &pszDbPath
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringA(
                                   VMAFD_OLD_CERT_DB,
                                   &pszDbOldPath
                                   );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdFileExists(pszDbOldPath,&bOldFileExists);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdFileExists(pszDbPath, &bNewFieExists);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bOldFileExists && !bNewFieExists)
    {
        dwError = VmAfdCopyFile(pszDbOldPath, pszDbPath);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdDeleteFile(pszDbOldPath);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszDbOldPath);
    VMAFD_SAFE_FREE_STRINGA(pszDbPath);
    return;

error:

    goto cleanup;
}

static
DWORD
VmAfdGetDbPath(
                PSTR *ppszDbPath
               )
{
    DWORD dwError = 0;
    PSTR pszDbPath = NULL;

    VmAfdMoveOldDbFile();

    dwError = VmAfdAllocateStringA(
                                    VMAFD_CERT_DB,
                                    &pszDbPath
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszDbPath = pszDbPath;

cleanup:
    return dwError;

error:
    if (ppszDbPath)
    {
        *ppszDbPath = NULL;
    }

    VMAFD_SAFE_FREE_STRINGA (pszDbPath);

    goto cleanup;
}
#endif


static
DWORD
InitializeDatabase(
    VOID
)
{
    DWORD dwError = 0 ;
    DWORD dwVersion = 0;

    PSTR pszDbPath = NULL;

    dwError = VmAfdGetDbPath(&pszDbPath);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsDbInitialize(pszDbPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbInitialize(pszDbPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsDbGetDbVersion(&dwVersion);
    BAIL_ON_VMAFD_ERROR(dwError);
    if (dwVersion != VECS_DB_CURRENT_VERSION)
    {
#ifndef _WIN32
        dwError = VecsDbCleanupPermissions();
        BAIL_ON_VMAFD_ERROR(dwError);
#endif
    }

    dwError = VecsDbSetDbVersion(VECS_DB_CURRENT_VERSION);
    BAIL_ON_VMAFD_ERROR(dwError);

error :

    VMAFD_SAFE_FREE_STRINGA (pszDbPath);
    return dwError;
}

static
DWORD
InitializeSystemStores(
    VOID
    )
{
    DWORD dwError = 0;
    PVECS_SRV_STORE_HANDLE pStore = NULL;
    WCHAR wszSystemStoreName[] = SYSTEM_CERT_STORE_NAME_W;
    WCHAR wszTrustedStoreName[] = TRUSTED_ROOTS_STORE_NAME_W;
    WCHAR wszCRLStoreName[] = CRL_STORE_NAME_W;
    WCHAR wszEveryone[] = GROUP_EVERYONE_W;
    PVM_AFD_CONNECTION_CONTEXT pRootConnectionContext = NULL;

    dwError = VmAfdCreateAnonymousConnectionContext(
                                  &pRootConnectionContext
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsSrvCreateCertStoreWithAuth (
                    wszSystemStoreName,
                    NULL,
                    pRootConnectionContext,
                    &pStore
                    );
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pStore)
    {
        VecsSrvCloseCertStoreHandle(
                              pStore,
                              pRootConnectionContext
                              );
        pStore = NULL;
    }

    dwError = VecsSrvCreateCertStoreWithAuth (
                    wszTrustedStoreName,
                    NULL,
                    pRootConnectionContext,
                    &pStore
                    );
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pStore)
    {
        dwError = VecsSrvSetPermission(
                                       pStore,
                                       wszEveryone,
                                       READ_STORE,
                                       VMAFD_ACE_TYPE_ALLOWED,
                                       pRootConnectionContext
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);
        VecsSrvCloseCertStoreHandle(
                          pStore,
                          pRootConnectionContext
                          );
        pStore = NULL;
    }

    dwError = VecsSrvCreateCertStoreWithAuth (
                    wszCRLStoreName,
                    NULL,
                    pRootConnectionContext,
                    &pStore
                    );
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pStore)
    {
        dwError = VecsSrvSetPermission(
                                       pStore,
                                       wszEveryone,
                                       READ_STORE,
                                       VMAFD_ACE_TYPE_ALLOWED,
                                       pRootConnectionContext
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

cleanup:

    if (pStore && pRootConnectionContext)
    {
        VecsSrvCloseCertStoreHandle(
                          pStore,
                          pRootConnectionContext
                          );
    }
    if (pRootConnectionContext)
    {
        VmAfdFreeConnectionContext (pRootConnectionContext);
    }
    return dwError;

error:
    goto cleanup;
}
