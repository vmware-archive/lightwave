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

#define LWCA_TIME_SECS_PER_MINUTE           ( 60)
#define LWCA_TIME_SECS_PER_HOUR             ( 60 * LWCA_TIME_SECS_PER_MINUTE)
#define LWCA_TIME_SECS_PER_DAY              ( 24 * LWCA_TIME_SECS_PER_HOUR)
#define LWCA_TIME_SECS_PER_WEEK             (  7 * LWCA_TIME_SECS_PER_DAY)
#define LWCA_TIME_SECS_PER_YEAR             (366 * LWCA_TIME_SECS_PER_DAY)

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

#define LWCA_SAFE_FREE_HZN_PSTR(PTR)    \
    do {                                \
        if ((PTR)) {                    \
            HZNFree(PTR);               \
            (PTR) = NULL;               \
        }                               \
    } while(0)

#define BAIL_ON_LWCA_INVALID_POINTER(p, errCode)    \
    if (p == NULL) {                                \
        errCode = LWCA_ERROR_INVALID_PARAMETER;     \
        BAIL_ON_LWCA_ERROR(errCode);                \
    }

#define BAIL_ON_LWCA_INVALID_PARAMETER(input, dwError)  \
    if (input == NULL)                                  \
    {                                                   \
        dwError = LWCA_ERROR_INVALID_PARAMETER;         \
    }

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

typedef struct _LWCA_REQ_CONTEXT
{
    PSTR            pszAuthPrincipal;
} LWCA_REQ_CONTEXT, *PLWCA_REQ_CONTEXT;

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

typedef struct _LWCA_PKCS_10_REQ_DATA
{
    PSTR                pszName;
    PSTR                pszDomainName;
    PLWCA_STRING_ARRAY  pCountryList;
    PLWCA_STRING_ARRAY  pLocalityList;
    PLWCA_STRING_ARRAY  pStateList;
    PLWCA_STRING_ARRAY  pOrganizationList;
    PLWCA_STRING_ARRAY  pOUList;
    PLWCA_STRING_ARRAY  pDNSList;
    PLWCA_STRING_ARRAY  pURIList;
    PLWCA_STRING_ARRAY  pEmailList;
    PLWCA_STRING_ARRAY  pIPAddressList;
    DWORD               dwKeyUsageConstraints;
} LWCA_PKCS_10_REQ_DATA,*PLWCA_PKCS_10_REQ_DATA;

enum _LWCA_KEY_USAGE {
    LWCA_DIGITAL_SIGNATURE      = 0,
    LWCA_NON_REPUDIATION        = 1,
    LWCA_KEY_ENCIPHERMENT       = 2,
    LWCA_DATA_ENCIPHERMENT      = 3,
    LWCA_KEY_AGREEMENT          = 4,
    LWCA_KEY_CERT_SIGN          = 5,
    LWCA_KEY_CRL_SIGN           = 6,
    LWCA_ENCIPHER_ONLY          = 7,
    LWCA_DECIPHER_ONLY          = 8
};

/* ../common/config.c */

DWORD
LwCASrvGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    );

/* ../common/jsonutils.c */

typedef struct json_t   _LWCA_JSON_OBJECT;
typedef _LWCA_JSON_OBJECT   *PLWCA_JSON_OBJECT;

DWORD
LwCAJsonLoadObjectFromFile(
    PCSTR                   pcszFilePath,
    PLWCA_JSON_OBJECT       *ppJsonConfig
    );

DWORD
LwCAJsonGetObjectFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    PLWCA_JSON_OBJECT       *ppJsonValue
    );

DWORD
LwCAJsonGetStringFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    PSTR                    *ppszValue
    );

DWORD
LwCAJsonGetStringArrayFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    PLWCA_STRING_ARRAY      *ppStrArrValue
    );

DWORD
LwCAJsonGetTimeFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    time_t                  *ptValue
    );

VOID
LwCAJsonCleanupObject(
    PLWCA_JSON_OBJECT       pJson
    );

/* ../common/util.c */

BOOLEAN
LwCAUtilIsValueIPAddr(
    PCSTR           pszValue
    );

BOOLEAN
LwCAUtilIsValuePrivateOrLocalIPAddr(
    PSTR            pszValue
    );

DWORD
LwCAUtilIsValueFQDN(
    PCSTR           pszValue,
    PBOOLEAN        pbIsValid
    );

BOOLEAN
LwCAUtilDoesValueHaveWildcards(
    PCSTR            pszValue
    );

DWORD
LwCAUtilIsValueInWhitelist(
    PCSTR                           pszValue,
    PCSTR                           pszAuthUPN,
    PCSTR                           pcszRegValue,
    PBOOLEAN                        pbInWhitelist
    );

DWORD
LwCACreateCertificate(
    PCSTR               pcszCertificate,
    PLWCA_CERTIFICATE   *ppCertificate
    );

DWORD
LwCACreateCertArray(
    PSTR                     *ppszCertificates,
    DWORD                    dwCount,
    PLWCA_CERTIFICATE_ARRAY  *ppCertArray
    );

DWORD
LwCACopyCertArray(
    PLWCA_CERTIFICATE_ARRAY     pCertArray,
    PLWCA_CERTIFICATE_ARRAY     *ppCertArray
    );

DWORD
LwCACreateKey(
    PBYTE       pData,
    DWORD       dwLength,
    PLWCA_KEY   *ppKey
    );

DWORD
LwCACopyKey(
    PLWCA_KEY pKey,
    PLWCA_KEY *ppKey
    );

VOID
LwCAFreeCertificate(
    PLWCA_CERTIFICATE pCertificate
    );

VOID
LwCAFreeCertificates(
    PLWCA_CERTIFICATE_ARRAY pCertArray
    );

VOID
LwCAFreeKey(
    PLWCA_KEY pKey
    );

DWORD
LwCADbCreateCAData(
    PCSTR                           pcszIssuer,
    PCSTR                           pcszSubject,
    PLWCA_CERTIFICATE_ARRAY         pCertificates,
    PLWCA_KEY                       pEncryptedPrivateKey,
    PLWCA_KEY                       pEncryptedEncryptionKey,
    PCSTR                           pcszTimeValidFrom,
    PCSTR                           pcszTimeValidTo,
    LWCA_CA_STATUS                  status,
    PLWCA_DB_CA_DATA                *ppCAData
    );

DWORD
LwCADbCreateCertData(
    PCSTR               pcszSerialNumber,
    PCSTR               pcszIssuer,
    PCSTR               pcszTimeValidFrom,
    PCSTR               pcszTimeValidTo,
    PCSTR               pcszRevokedReason,
    PCSTR               pcszRevokedDate,
    LWCA_CERT_STATUS    status,
    PLWCA_DB_CERT_DATA  *ppCertData
    );

VOID
LwCADbFreeCAData(
    PLWCA_DB_CA_DATA pCAData
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

// pkcs_openssl.c

DWORD
LwCAGetCommonNameFromSubject(
    PLWCA_CERTIFICATE   pCert,
    PSTR                *ppszCommonName
    );

DWORD
LwCAPEMToX509(
    PCSTR       pCertificate,
    X509        **ppX509Cert
    );

DWORD
LwCAValidateCertificate(
    X509    *pCert,
    PCSTR   pcszPrivateKey,
    PCSTR   pcszPassPhrase
    );

DWORD
LwCAGetCertSubjectName(
    X509 *pCert,
    PSTR *ppszSubjectName
    );

DWORD
LwCAGetCertIssuerName(
    X509 *pCert,
    PSTR *ppszIssuerName
    );

DWORD
LwCACheckCACert(
    X509 *pCert
    );

#ifdef __cplusplus
}
#endif

#endif /* _LWCA_SRV_COMMON_H_ */
