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

#include "includes.h"

static LWCA_OPENSSL_GLOBALS gLwCAOpenSSLGlobals = {0};

static
VOID
LwCAOpenSSLLockingCallback(
    int mode,
    int index,
    const char * file,
    int line
    );

static
VOID
LwCAOpenSSLCleanupMutexes(
    pthread_mutex_t* pSSLMutexes,
    int nSSLMutexes
    );


DWORD
LwCAOpenSSLInitialize(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    pthread_mutex_t* pSSLMutexes = NULL;
    int nSSLMutexes = 0;
    int dwNumMutexes = 0;

    if (gLwCAOpenSSLGlobals.pSSLMutexes == NULL)
    {
        dwNumMutexes = CRYPTO_num_locks();

        dwError = LwCAAllocateMemory(dwNumMutexes * sizeof(pthread_mutex_t), (PVOID*)&pSSLMutexes);
        BAIL_ON_LWCA_ERROR(dwError);

        for (i = 0; i < dwNumMutexes; ++i)
        {
            if (0 != pthread_mutex_init( &(pSSLMutexes[i]), NULL))
            {
                dwError = LWCA_OUT_OF_MEMORY_ERROR;
                BAIL_ON_LWCA_ERROR(dwError);
            }
            ++nSSLMutexes;
        }

        gLwCAOpenSSLGlobals.pSSLMutexes = pSSLMutexes;
        gLwCAOpenSSLGlobals.nSSLMutexes = nSSLMutexes;
        pSSLMutexes = NULL;

        CRYPTO_set_locking_callback(LwCAOpenSSLLockingCallback);
    }

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

cleanup:

    return dwError;

error:

    if (pSSLMutexes)
    {
        LwCAOpenSSLCleanupMutexes(pSSLMutexes, nSSLMutexes);
    }
    goto cleanup;
}

DWORD
LwCAOpenSSLCleanup(
    VOID
    )
{
    EVP_cleanup();
    ERR_free_strings();
    CRYPTO_set_locking_callback(NULL);

    LwCAOpenSSLCleanupMutexes(gLwCAOpenSSLGlobals.pSSLMutexes, gLwCAOpenSSLGlobals.nSSLMutexes);

    gLwCAOpenSSLGlobals.pSSLMutexes = NULL;
    gLwCAOpenSSLGlobals.nSSLMutexes = 0;

    return LWCA_SUCCESS;
}


static
VOID
LwCAOpenSSLCleanupMutexes(
    pthread_mutex_t* pSSLMutexes,
    int nSSLMutexes
    )
{
    int i = 0;
    int dwNumMutexes = nSSLMutexes;

    if (pSSLMutexes)
    {
        for (i = 0; i < dwNumMutexes; ++i)
        {
            pthread_mutex_destroy( &(pSSLMutexes[i]));
        }

        LwCAFreeMemory(pSSLMutexes);
    }
}

static
VOID
LwCAOpenSSLLockingCallback(
    int mode,
    int index,
    const char * file,
    int line
    )
{
    if (gLwCAOpenSSLGlobals.pSSLMutexes && gLwCAOpenSSLGlobals.nSSLMutexes > index)
    {
        if (mode & CRYPTO_LOCK)
        {
             pthread_mutex_lock(&gLwCAOpenSSLGlobals.pSSLMutexes[index]);
        }
        else
        {
             pthread_mutex_unlock(&gLwCAOpenSSLGlobals.pSSLMutexes[index]);
        }
    }
}
