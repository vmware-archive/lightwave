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

#include <includes.h>
#include <pthread.h>
#include <openssl/ssl.h>

/*
 * OpenSSL locking code needed for only Win32 because the
 * static OpenSSL library is linked into vmdir. The existing
 * locks created by vmdir are not shared into plugins, making
 * this additional logic necessary.
 */
static pthread_mutex_t *g_mutexes;
static int g_max_locks;

static void srp_gss_locking_cb(
    int mode,
    int type,
    const char *file,
    int line)
{
    if (!g_mutexes ||
        (type < 0 || type > g_max_locks))
    {
        return;
    }
    if (mode & CRYPTO_LOCK)
    {
        pthread_mutex_lock(&g_mutexes[type]);
    }
    else
    {
        pthread_mutex_unlock(&g_mutexes[type]);
    }
}

static unsigned long srp_gss_thread_self(void)
{
    return (unsigned long) ((size_t) pthread_self().p);
}

static BOOL srp_gss_init(void)
{
    pthread_mutex_t *mutexes = NULL;
    int n_locks = 0;
    int i = 0;

    n_locks = CRYPTO_num_locks();
    if (n_locks <= 0)
    {
        return 0;
    }
    mutexes = (pthread_mutex_t *) calloc(n_locks, sizeof(pthread_mutex_t));
    if (!mutexes)
    {
        return 0;
    }
    for (i=0; i<n_locks; i++)
    {
        pthread_mutex_init(&mutexes[i], NULL);
    }
    g_mutexes = mutexes;
    g_max_locks = n_locks;
    CRYPTO_set_locking_callback(srp_gss_locking_cb);
    CRYPTO_set_id_callback(srp_gss_thread_self);
    return 1;
}

static BOOL srp_gss_destroy(void)
{
    int i = 0;

    if (g_mutexes)
    {
        for (i=0; i<g_max_locks; i++)
        {
            pthread_mutex_destroy(&g_mutexes[i]);
        }
        free(g_mutexes);
        g_mutexes = NULL;
    }
    return 1;
}

BOOL __stdcall DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    BOOL result = TRUE;

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            result = srp_gss_init();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            srp_gss_destroy();
            break;
    }
    return result;
}
