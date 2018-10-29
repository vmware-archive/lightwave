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

#ifndef _LWCA_SRV_COMMON_H_
#define _LWCA_SRV_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "Services\\vmafd\\Parameters"
#define LWCA_CONFIG_PARAMETER_KEY_PATH  "Services\\mutentca\\Parameters"

#if 0 /* TBD: Adam */
#define VMDIR_REG_KEY_DC_ACCOUNT_DN "dcAccountDN"
#endif
#define VMDIR_REG_KEY_DC_PASSWORD   "dcAccountPassword"
#define VMDIR_REG_KEY_DC_ACCOUNT    "dcAccount"
#define VMAFD_REG_KEY_DOMAIN_NAME   "DomainName"

#define LwCASleep(X) sleep((X))

#define FILE_CHUNK (64 * 1024)

// States

typedef DWORD LWCA_FUNC_LEVEL;

#define LWCA_FUNC_LEVEL_INITIAL     0x00000000
#define LWCA_FUNC_LEVEL_SELF_CA     0x00000004

#define    GetLastError() errno

#define LWCA_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue

#define LWCA_ASSERT(x) assert( (x) )

#define _PTHREAD_FUNCTION_RTN_ASSERT(Function, ...)       \
    do {                                                  \
        int error = Function(__VA_ARGS__);                \
        LWCA_ASSERT(!error);                              \
    } while (0)

#define INITIALIZE_SRW_LOCK(pRWLock, pRWLockAttr) \
    _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_init, pRWLock, pRWLockAttr)

#define ENTER_READERS_SRW_LOCK(bHasLock, pRWLock)                 \
    do {                                                          \
        assert (!bHasLock);                                           \
        _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_rdlock, pRWLock); \
        bHasLock = true;                                              \
    } while(0)

#define LEAVE_READERS_SRW_LOCK(bHasLock, pRWLock)                 \
    do {                                                          \
        if (bHasLock) {                                               \
            _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_unlock, pRWLock); \
            bHasLock = false;                                             \
        }                                                             \
    } while(0)

#define ENTER_WRITER_SRW_LOCK(bHasLock, pRWLock)                  \
    do {                                                          \
        assert (!bHasLock);                                           \
        _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_wrlock, pRWLock); \
        bHasLock = true;                                              \
    } while(0)

#define LEAVE_WRITER_SRW_LOCK(bHasLock, pSRWLock)  LEAVE_READERS_SRW_LOCK(bHasLock, pSRWLock)

#define SQL_BUFFER_SIZE          1024

#define LWCA_MAX_INT_CA_CERT_DURATION  (LWCA_TIME_SECS_PER_YEAR * 1)

#define LWCA_SAFE_FREE_HZN_PSTR(PTR)    \
    do {                                \
        if ((PTR)) {                    \
            HZNFree(PTR);               \
            (PTR) = NULL;               \
        }                               \
    } while(0)

#define LWCA_LOCK_MUTEX_EXCLUSIVE(pmutex, bLocked) \
    if (! (bLocked) ) \
    { \
        pthread_rwlock_wrlock (pmutex); \
        (bLocked) = TRUE; \
    }

#define LWCA_LOCK_MUTEX_SHARED(pmutex, bLocked) \
    if (! (bLocked) ) \
    { \
        pthread_rwlock_rdlock (pmutex); \
        (bLocked) = TRUE; \
    }

#define LWCA_LOCK_MUTEX_UNLOCK(pmutex, bLocked) \
    if (bLocked) \
    { \
        pthread_rwlock_unlock (pmutex); \
        (bLocked) = FALSE; \
    }

typedef struct _LWCA_DIR_SYNC_PARAMS
{
    LONG refCount;

    pthread_mutex_t  mutex;
    pthread_mutex_t* pMutex;

    DWORD   dwSyncIntervalSecs;

    time_t  lastUpdateTime;

    BOOLEAN bRefresh;

} LWCA_DIR_SYNC_PARAMS, *PLWCA_DIR_SYNC_PARAMS;

typedef enum
{
    LWCAD_STARTUP = 0,
    LWCAD_NORMAL,
    LWCAD_SHUTDOWN
} LWCA_SERVER_STATE;

typedef struct _LWCA_SERVER_GLOBALS
{
    pthread_mutex_t                 mutex;

    pthread_rwlock_t                svcMutex;

    FILE*                           fLwCALog;

    LWCA_SERVER_STATE               mutentcadState;

    LWCA_FUNC_LEVEL                 dwFuncLevel;

    PLWCA_DIR_SYNC_PARAMS           pDirSyncParams;
    PLWCA_THREAD                    pDirSyncThr;

    HANDLE                          gpEventLog;

    SSL_CTX*                        pSslCtx;
} LWCA_SERVER_GLOBALS, *PLWCA_SERVER_GLOBALS;

extern LWCA_SERVER_GLOBALS gLwCAServerGlobals;

/* ../common/config.c */

DWORD
LwCASrvGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    );

/* ../common/util.c */

BOOLEAN
LwCAUtilIsValueIPAddr(
    PCSTR           pcszValue
    );

BOOLEAN
LwCAUtilIsValuePrivateOrLocalIPAddr(
    PCSTR           pcszValue
    );

DWORD
LwCAUtilIsValueFQDN(
    PCSTR           pcszValue,
    PBOOLEAN        pbIsValid
    );

BOOLEAN
LwCAUtilDoesValueHaveWildcards(
    PCSTR            pcszValue
    );

DWORD
LwCADbCreateCertData(
    PCSTR                   pcszSerialNumber,
    PCSTR                   pcszTimeValidFrom,
    PCSTR                   pcszTimeValidTo,
    DWORD                   revokedReason,
    PCSTR                   pcszRevokedDate,
    LWCA_CERT_STATUS        status,
    PLWCA_DB_CERT_DATA      *ppCertData
    );


VOID
LwCADbFreeCertData(
    PLWCA_DB_CERT_DATA pCertData
    );

VOID
LwCADbFreeCertDataArray(
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray
    );

/* ../common/state.c */

VOID
LwCASrvSetState(
    LWCA_SERVER_STATE state
    );

LWCA_SERVER_STATE
LwCASrvGetState(
    VOID
    );

VOID
LwCASrvSetFuncLevel(
    LWCA_FUNC_LEVEL dwFuncLevel
    );

LWCA_FUNC_LEVEL
LwCASrvGetFuncLevel(
    VOID
    );

PLWCA_DIR_SYNC_PARAMS
LwCASrvGetDirSyncParams(
    VOID
    );

PLWCA_THREAD
LwCASrvGetDirSvcThread(
    VOID
    );

VOID
LwCASrvCleanupGlobalState(
    VOID
    );

/* security/security.c */
DWORD
LwCASecurityInitCtx(
    PLWCA_JSON_OBJECT pConfig
    );

DWORD
LwCASecurityAddKeyPair(
    PCSTR pszKeyId,
    PCSTR pszPrivateKey
    );

DWORD
LwCASecurityCreateKeyPair(
    PCSTR pszKeyId,
    PSTR *ppszPublicKey
    );

DWORD
LwCASecuritySignX509Cert(
    PCSTR pcszKeyId,
    X509 *pCert
    );

DWORD
LwCASecuritySignX509Request(
    PCSTR    pcszKeyId,
    X509_REQ *pReq
    );

DWORD
LwCASecuritySignX509Crl(
    PCSTR    pcszKeyId,
    X509_CRL *pCrl
    );

VOID
LwCASecurityFreeCtx(
   VOID
   );

/* security/storage.c */
DWORD
LwCASecurityGetEncryptedKey(
    PCSTR pszCAId,
    PLWCA_KEY *ppEncryptedKey
    );

/*
 * normally there is no need to call this
 * but if there was an error during create key pair,
 * call this at error handling to make sure cache is
 * cleared
*/
DWORD
LwCASecurityRemoveEncryptedKeyFromCache(
    PCSTR pszCAId
    );

#ifdef __cplusplus
}
#endif

#endif /* _LWCA_SRV_COMMON_H_ */
