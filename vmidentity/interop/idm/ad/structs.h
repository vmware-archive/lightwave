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
 *        structs.h
 *
 * Abstract:
 *
 *        Identity Manager - Active Directory Integration
 *
 *        Structure definitions 
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

typedef struct _SID_ENTRY {
    PSID pSid;
    PWSTR pszName;
    struct _SID_ENTRY * pNext;
}SID_ENTRY, *PSID_ENTRY;

typedef struct _SID_CACHE {
/*
 * Remove CRITICAL_SECTION, and just use
 * pthread_mutex_t once using pthreads-win32.
 */
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t cs;
#endif
    PSID_ENTRY pStartEntry;
    BOOL bInitialized;
}SID_CACHE, *PSID_CACHE;


typedef struct _IDM_WELL_KNOWN_SID
{
    char *Sid;
    char *CommonName;
} IDM_WELL_KNOWN_SID;

#ifndef _WIN32

typedef enum
{
    IDM_KRB_CONTEXT_STATE_UNKNOWN = 0,
    IDM_KRB_CONTEXT_STATE_INITIAL,
    IDM_KRB_CONTEXT_STATE_JOINED

} IDM_KRB_CONTEXT_STATE;

typedef struct _IDM_KRB_CONTEXT
{
    pthread_rwlock_t      mutex_rw;

    IDM_KRB_CONTEXT_STATE state;

    PSTR           pszAccount;
    PSTR           pszDomain;

    PSTR           pszCachePath;
    krb5_timestamp expiryTime;

} IDM_KRB_CONTEXT, *PIDM_KRB_CONTEXT;

typedef struct _IDM_AUTH_MUTEX
{
    pthread_mutex_t mutex;
} IDM_AUTH_MUTEX, *PIDM_AUTH_MUTEX;
#endif

