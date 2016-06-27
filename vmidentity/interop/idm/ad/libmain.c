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
 * Module Name:
 *
 *        libmain.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        Library Entrypoint
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Jonathan Brown (brownj@vmware.com)
 *
 */

#include "includes.h"

DWORD
IDMInitialize(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD dwCleanupError = 0;
    BOOLEAN bLocked = FALSE;
    PCSTR pszLocalStateDir = VMSTS_DB_DIR;
    PCSTR pszCacheFilename = "krb5_cc_idm";

    dwError = pthread_mutex_init(&pgIdmAuthMutex->mutex, NULL);
    BAIL_ON_ERROR(dwError);

    IDM_RWMUTEX_LOCK_EXCLUSIVE(&pgIdmKrbContext->mutex_rw, bLocked, dwError);
    BAIL_ON_ERROR(dwError);

    if (pgIdmKrbContext->state != IDM_KRB_CONTEXT_STATE_UNKNOWN)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMAllocateStringPrintf(
                    &pgIdmKrbContext->pszCachePath,
                    "%s/%s",
                    pszLocalStateDir,
                    pszCacheFilename);
    BAIL_ON_ERROR(dwError);

    pgIdmKrbContext->state = IDM_KRB_CONTEXT_STATE_INITIAL;

cleanup:

    IDM_RWMUTEX_UNLOCK(&pgIdmKrbContext->mutex_rw, bLocked, dwCleanupError);
    if(!dwError)
    {
        dwError = dwCleanupError;
    }
    return dwError;

error:

    goto cleanup;
}

VOID
IDMShutdown(
    VOID
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    pthread_mutex_destroy(&pgIdmAuthMutex->mutex);


    IDM_RWMUTEX_LOCK_EXCLUSIVE(&pgIdmKrbContext->mutex_rw, bLocked, dwError);
    BAIL_ON_ERROR(dwError);

    IDM_SAFE_FREE_MEMORY(pgIdmKrbContext->pszAccount);
    IDM_SAFE_FREE_MEMORY(pgIdmKrbContext->pszCachePath);
    IDM_SAFE_FREE_MEMORY(pgIdmKrbContext->pszDomain);

    pgIdmKrbContext->expiryTime = 0;

    pgIdmKrbContext->state = IDM_KRB_CONTEXT_STATE_UNKNOWN;

cleanup:

    IDM_RWMUTEX_UNLOCK(&pgIdmKrbContext->mutex_rw, bLocked, dwError);

    return;

error:

    goto cleanup;
}

