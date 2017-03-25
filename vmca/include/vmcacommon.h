/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 * Module Name: VMCAService
 *
 * Filename: VMCAcommon.h
 *
 * Abstract:
 *
 * Common utilities between VMCA Service Components.
 *
 * Public header for libVMCAcommon.so
 *
 */

#ifndef __VMCA_COMMON_H__
#define __VMCA_COMMON_H__


#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// make sure no conflict with windows.h for double def
#ifdef _WIN32
#ifdef wchar16_t
#undef wchar16_t
#endif
typedef unsigned short wchar16_t;
#define UINT32        unsigned long
#define mode_t        int
#endif // _WIN32

// Treat wchar16_t as "unsigned short" (2 bytes char) as in Windows Unicode
typedef char             RP_STR,   *RP_PSTR;
typedef char const       RP_CSTR,  *RP_PCSTR;
typedef wchar16_t        RP_WSTR,  *RP_PWSTR;
typedef wchar16_t const  RP_CWSTR, *RP_PCWSTR;

typedef void*            LPVOID;
typedef char             *NPSTR, *LPSTR;
typedef char const       *LPCSTR;

typedef struct _VMCA_LDAP_CONTEXT*  PVMCA_LDAP_CONTEXT;

typedef struct _VMW_CFG_CONNECTION* PVMW_CFG_CONNECTION;
typedef struct _VMW_CFG_KEY*        PVMW_CFG_KEY;

#define FILE_EXISTS 1
#define FILE_DOES_NOT_EXIST 0

#define VMCA_ASCII_aTof(c)     ( (c) >= 'a' && (c) <= 'f' )
#define VMCA_ASCII_AToF(c)     ( (c) >= 'A' && (c) <= 'F' )
#define VMCA_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )

#define VMCA_SAFE_FREE_STRINGA(PTR)    \
    do {                          \
        if ((PTR)) {              \
            VMCAFreeStringA(PTR); \
            (PTR) = NULL;         \
        }                         \
    } while(0)

#define VMCA_SAFE_FREE_STRINGW(PTR)    \
    do {                          \
        if ((PTR)) {              \
            VMCAFreeStringW(PTR); \
            (PTR) = NULL;         \
        }                         \
    } while(0)

#define VMCA_SAFE_FREE_MEMORY(PTR)\
    do {                          \
        if ((PTR)) {              \
            VMCAFreeMemory(PTR);  \
            (PTR) = NULL;         \
        }                         \
    } while(0)

#if defined(_WIN32) && defined(IAMNOTDEFINED)

#define VMCA_SAFE_FREE_MUTEX(mutex)      \
    do {                                  \
        if ((mutex)) {                    \
            VMCAFreeMutex(mutex);        \
            (mutex) = NULL;               \
        }                                 \
    } while(0)

#define VMCA_SAFE_FREE_CONDITION(cond)   \
    do {                                  \
        if ((cond)) {                     \
            VMCAFreeCondition(cond);     \
            (cond) = NULL;                \
        }                                 \
    } while(0)

#define VMCA_LOCK_MUTEX(bInLock, mutex) \
    do {                                 \
        if (!(bInLock))                  \
        {                                \
            VMCALockMutex(mutex);       \
            (bInLock) = TRUE;            \
        }                                \
    } while (0)

#define VMCA_UNLOCK_MUTEX(bInLock, mutex) \
    do {                                  \
        if ((bInLock))                    \
        {                                 \
            VMCAUnLockMutex(mutex);      \
            (bInLock) = FALSE;            \
        }                                 \
    } while (0)

#else
#define VMCA_LOCK_MUTEX(bInLock, mutex) \
    do {                                \
        if (!(bInLock))                 \
        {                               \
            pthread_mutex_lock(mutex);  \
            (bInLock) = TRUE;           \
        }                               \
    } while (0)

#define VMCA_TRYLOCK_MUTEX(bInLock, mutex, dwError) \
    do {                                \
        if (!(bInLock))                 \
        {                               \
            int iResult = pthread_mutex_trylock(mutex);       \
            if (iResult == 0)                                 \
            {                                                 \
                (bInLock) = TRUE;                             \
            }                                                 \
            else                                              \
            {                                                 \
                if (iResult == EBUSY)                         \
                {                                             \
                    (dwError) = ERROR_BUSY;                   \
                }                                             \
                else                                          \
                {                                             \
                    (dwError) = LwErrnoToWin32Error(iResult); \
                }                                             \
            }                                                 \
        }                                                     \
    } while (0)

#define VMCA_UNLOCK_MUTEX(bInLock, mutex) \
    do {                                  \
        if ((bInLock))                    \
        {                                 \
            pthread_mutex_unlock(mutex);  \
            (bInLock) = FALSE;            \
        }                                 \
    } while (0)
#endif

typedef enum
{
    VMCA_LOG_TYPE_CONSOLE = 0,
    VMCA_LOG_TYPE_FILE,
    VMCA_LOG_TYPE_SYSLOG
} VMCA_LOG_TYPE;

#ifndef _VMCA_LOG_LEVEL_DEFINED_
#define _VMCA_LOG_LEVEL_DEFINED_
typedef enum
{
   VMCA_LOG_LEVEL_EMERGENCY = 0,
   VMCA_LOG_LEVEL_ALERT,
   VMCA_LOG_LEVEL_CRITICAL,
   VMCA_LOG_LEVEL_ERROR,
   VMCA_LOG_LEVEL_WARNING,
   VMCA_LOG_LEVEL_NOTICE,
   VMCA_LOG_LEVEL_INFO,
   VMCA_LOG_LEVEL_DEBUG
} VMCA_LOG_LEVEL;
#endif

DWORD
VMCAInitLog();

VOID
VMCATerminateLogging();

void
VMCALog(
   VMCA_LOG_LEVEL level,
   const char*      fmt,
   ...);

typedef struct _VMCA_LOG_HANDLE* PVMCA_LOG_HANDLE;

extern PVMCA_LOG_HANDLE gpVMCALogHandle;
extern VMCA_LOG_LEVEL   gVMCALogLevel;
extern HANDLE           gpEventLog;
extern VMCA_LOG_TYPE    gVMCALogType;
extern VMCA_LOG_LEVEL VMCALogGetLevel();

#define VMCA_LOG_( Level, Format, ... ) \
    do                                             \
    {                                              \
        VMCALog(                                   \
               Level,                              \
               Format,                             \
               ##__VA_ARGS__);                     \
    } while (0)

#define VMCA_LOG_GENERAL_( Level, Format, ... ) \
    VMCA_LOG_( Level, Format, ##__VA_ARGS__ )

#define VMCA_LOG_ERROR( Format, ... )   \
    VMCA_LOG_GENERAL_( VMCA_LOG_LEVEL_ERROR, Format, ##__VA_ARGS__ )
#define VMCA_LOG_WARNING( Format, ... ) \
    VMCA_LOG_GENERAL_( VMCA_LOG_LEVEL_WARNING, Format, ##__VA_ARGS__ )
#define VMCA_LOG_INFO( Format, ... )    \
    VMCA_LOG_GENERAL_( VMCA_LOG_LEVEL_INFO, Format, ##__VA_ARGS__ )
#define VMCA_LOG_VERBOSE( Format, ... ) \
    VMCA_LOG_GENERAL_( VMCA_LOG_LEVEL_DEBUG, Format, ##__VA_ARGS__ )
#define VMCA_LOG_DEBUG( Format, ... )       \
    VMCA_LOG_GENERAL_(                      \
        VMCA_LOG_LEVEL_DEBUG,               \
    Format " [file: %s][line: %d]",     \
    ##__VA_ARGS__, __FILE__, __LINE__ )

#define BAIL_ON_VMCA_ERROR(dwError)                                         \
    if (dwError)                                                            \
    {                                                                       \
        VMCA_LOG_WARNING("error code: %#010x", dwError);                    \
        goto error;                                                         \
    }


#define BAIL_ON_SSL_ERROR(dwError, ERROR_CODE)                              \
    if (dwError == 0)                                                       \
    {                                                                       \
        dwError = ERROR_CODE;                                               \
        VMCA_LOG_WARNING("error code: %#010x", dwError);                    \
        if (ERROR_CODE == VMCA_CERT_IO_FAILURE)                             \
        {                                                                   \
            printf(" Failed at %s %d \n", __FUNCTION__, __LINE__);          \
        }                                                                   \
        goto error;                                                         \
    } else {                                                                \
        dwError = 0;                                                        \
    }                                                                       \


#define BAIL_ON_NULL(ptr, dwError , ERROR_CODE)                                       \
    if (ptr == NULL)                                                           \
    {                                                                       \
        dwError = ERROR_CODE;                                               \
        VMCA_LOG_DEBUG("error code: %#010x", dwError);                      \
        goto error;                                                         \
    } else {                                                                \
        dwError = 0;                                                        \
    }                                                                       \




#define BAIL_ON_VMCA_ERROR_NO_LOG(dwError) \
    if (dwError) { goto error; }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#ifndef VMCA_SAFE_LOG_STRING
#define VMCA_SAFE_LOG_STRING(str) ((str) ? (str) : "")
#endif

#define VMCA_REG_KEY_VMCADBPATH              "DbPath"
#define VMCA_REG_KEY_LOGFILEPATH             "LogFilePath"
#define VMCA_REG_KEY_CERTFILEPATH            "CertFilePath"
#define VMCA_REG_KEY_RPCTCPENDPOINT          "RpcTcpEndPoint"
#define VMCA_REG_KEY_LOGLEVEL                "LogLevel"
#define VMCA_REG_KEY_ENABLEEVENTLOGS         "EnableEventLogs"
#define VMCA_REG_KEY_ENABLESERVICE           "EnableService"
#define VMCA_REG_KEY_SERVER_OPTION           "ServerOption"

#define VMCA_RPC_TCP_END_POINT               "2014"
#define VMCA_NCALRPC_END_POINT               "vmcasvc"

// Event logs related constants.

#define VMCA_EVENT_SOURCE                    "VMware Certificate-Service"


#if 0
/* mutexes/threads/conditions */
typedef struct _VMCA_MUTEX* PVMCA_MUTEX;

typedef struct _VMCA_COND* PVMCA_COND;

typedef pthread_t VMCA_THREAD;

typedef VMCA_THREAD* PVMCA_THREAD;

typedef DWORD (VMCAStartRoutine)(PVOID);
typedef VMCAStartRoutine* PVMCA_START_ROUTINE;

DWORD
VMCAAllocateMutex(
    PVMCA_MUTEX* ppMutex
);

VOID
VMCAFreeMutex(
    PVMCA_MUTEX pMutex
);

DWORD
VMCALockMutex(
    PVMCA_MUTEX pMutex
);

DWORD
VMCAUnLockMutex(
    PVMCA_MUTEX pMutex
);

BOOLEAN
VMCAIsMutexInitialized(
    PVMCA_MUTEX pMutex
);

DWORD
VMCAAllocateCondition(
    PVMCA_COND* ppCondition
);

VOID
VMCAFreeCondition(
    PVMCA_COND pCondition
);

DWORD
VMCAConditionWait(
    PVMCA_COND pCondition,
    PVMCA_MUTEX pMutex
);

DWORD
VMCAConditionTimedWait(
    PVMCA_COND pCondition,
    PVMCA_MUTEX pMutex,
    DWORD dwMilliseconds
);

DWORD
VMCAConditionSignal(
    PVMCA_COND pCondition
);

DWORD
VMCACreateThread(
    PVMCA_THREAD pThread,
    BOOLEAN bDetached,
    PVMCA_START_ROUTINE pStartRoutine,
    PVOID pArgs
);

DWORD
VMCAThreadJoin(
    PVMCA_THREAD pThread,
    PDWORD pRetVal
);

VOID
VMCAFreeVMCAThread(
    PVMCA_THREAD pThread
);
#endif

SIZE_T
VMCAStringLenA(
    PCSTR pszStr
    );

DWORD
VMCAAllocateMemory(
    DWORD dwSize,
    PVOID * ppMemory
    );

DWORD
VMCAReallocateMemory(
    PVOID        pMemory,
    PVOID*       ppNewMemory,
    DWORD        dwSize
    );

VOID
VMCAFreeMemory(
    PVOID pMemory
    );

DWORD
VMCAAllocateStringA(
    RP_PCSTR pszString,
    RP_PSTR * ppszString
    );

DWORD
VMCAAllocateStringWithLengthA(
    RP_PCSTR pszString,
    DWORD dwSize,
    RP_PSTR * ppszString
    );

DWORD
VMCAAllocateStringPrintfA(
    RP_PSTR* ppszString,
    RP_PCSTR pszFormat,
    ...
    );

VOID
VMCAFreeStringA(
    RP_PSTR pszString
    );

VOID
VMCAFreeStringW(
    RP_PWSTR pszString
    );

VOID
VMCAFreeStringArrayA(
    PSTR* ppszStrings,
    DWORD dwCount
    );


DWORD
VMCAGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

ULONG
VMCAAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

ULONG
VMCAAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

ULONG
VMCAAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

BOOL
VMCAStringIsEqualW (
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    );

BOOLEAN
VMCAIsValidSecret(
    PWSTR pszTheirs,
    PWSTR pszOurs
    );

int
VMCAStringCompareW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
);

int
VMCAStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

DWORD
VMCAStringNCpyA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource,
    size_t count
    );

PSTR
VMCAStringChrA(
    PCSTR str,
    int c
    );

PSTR
VMCAStringTokA(
    PSTR strToken,
    PCSTR strDelimit,
    PSTR* context
    );

DWORD
VMCAStringCountSubstring(
    PSTR pszHaystack,
    PCSTR pszNeedle,
    int** ppnCount
);

DWORD
VMCAStringCatA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource
    );

DWORD
VMCAGetUTCTimeString(
    PSTR *pszTimeString
    );

DWORD
VMCALogSetTypeFile(
    PVMCA_LOG_HANDLE plogHandle,
    const char*      inName
    );

DWORD
VMCALogSetTypeSyslog(
    PVMCA_LOG_HANDLE plogHandle
    );

DWORD
VMCALogSetTypeConsole(
    PVMCA_LOG_HANDLE plogHandle
    );

DWORD
VMCALogInitialize(
    PVMCA_LOG_HANDLE plogHandle
    );

VOID
VMCALogShutdown(
    PVMCA_LOG_HANDLE plogHandle
    );

VOID
VMCALogMessage(
    PVMCA_LOG_HANDLE plogHandle,
    VMCA_LOG_LEVEL   level,
    const char*      fmt,
    ...
    );

VMCA_LOG_LEVEL
VMCALogLevelFromInt(
    int intLoglevel
    );

VMCA_LOG_LEVEL
VMCALogLevelFromTag(
    const char* level
    );

const char*
VMCALogLevelToTag(
    VMCA_LOG_LEVEL level
    );

VMCA_LOG_LEVEL
VMCALogGetLevel(
    );

VOID
VMCALogSetLevel(
    VMCA_LOG_LEVEL level
    );

VMCA_LOG_TYPE
VMCALogGetType(
    PVMCA_LOG_HANDLE plogHandle
    );

const char*
VMCALogGetFilename(
    PVMCA_LOG_HANDLE plogHandle
    );

DWORD
VMCAOpenFilePath(
    PCSTR szFileName,
    PCSTR szOpenMode,
    FILE** fp
    );

DWORD
VMCACopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    );

DWORD
VMCARestrictDirectoryAccess(
    PCSTR pszDirectoryName
    );

DWORD
VMCAVerifyHostName(
    PCSTR pszHostName,
    PCSTR pszHostIp,
    PCSTR pCertRequest
    );

/////////////////////////////Actual VMCA Common Functions///////////////////

#ifndef _VMCA_CA_STRUCT_DEFINED
#define _VMCA_CA_STRUCT_DEFINED 1

typedef struct _X509_CA
{
    LONG refCount;

    X509 *pCertificate;
    EVP_PKEY *pKey;
    PSTR pszCertificate;
    STACK_OF(X509) *skCAChain;

}VMCA_X509_CA, *PVMCA_X509_CA;

#endif // _VMCA_CA_STRUCT_DEFINED

#ifndef _VMCA_PKCS_10_DATAW_DEFINED
#define _VMCA_PKCS_10_DATAW_DEFINED 1
typedef struct _VMCA_PKCS_10_REQ_DATA
{
    PCWSTR          pszName;
    PCWSTR          pszCountry;
    PCWSTR          pszLocality;
    PCWSTR          pszState;
    PCWSTR          pszOrganization;
    PCWSTR          pszOU;
    PCWSTR          pszDNSName;
    PCWSTR          pszURIName;
    PCWSTR          pszEmail;
    PCWSTR          pszIPAddress;
    DWORD         dwKeyUsageConstraints;
} VMCA_PKCS_10_REQ_DATAW,*PVMCA_PKCS_10_REQ_DATAW;
#endif //_VMCA_PKCS_10_DATAW_DEFINED

DWORD
VMCACommonInit();

DWORD
VMCACommonShutdown();

DWORD
VMCAIntializeOpenSSL();

DWORD
VMCACleanupOpenSSL();

DWORD
VMCAPrivateKeyToPEM(
    EVP_PKEY* pPrivateKey,
    PSTR* ppPrivateKey
);

DWORD
VMCAPublicKeyToPEM(
    EVP_PKEY* pPubKey,
    PSTR* ppPublicKey
);

DWORD
VMCACRLToPEM(
    X509_CRL* pCrl,
    PSTR* ppCrl
);


DWORD
VMCACertToPEM(
    X509* pCertificate,
    PSTR* ppCertificate
);

DWORD
VMCACertStackToPEM(
    STACK_OF(X509) *skX509certs,
    PSTR* ppszCertificate
    );

DWORD
VMCACSRToPEM(
        X509_REQ* pCSR,
        PSTR* ppCSR
);

DWORD
VMCACSRToFile(
        X509_REQ* pREQ,
        PSTR pszFileName
);

DWORD
VMCACRLToFile(
        X509_CRL* pCRL,
        PSTR pszFileName
);

DWORD
VMCAPEMToX509Stack(
    PCSTR pCertificate,
    STACK_OF(X509) **pskX509certs
    );

DWORD
VMCAPEMToX509(
    PSTR pCertificate,
    X509 **ppX509Cert
);

DWORD
VMCAPEMToPrivateKey(
    PSTR pKey,
    RSA **ppPrivateKey
);

DWORD
VMCAPEMToPublicKey(
    PSTR pKey,
    RSA **ppPublicKey
);

DWORD
VMCAPEMToCSR(
    PCSTR pCSR,
    X509_REQ **ppReq
);

DWORD
VMCAFileToCRL(
    PSTR pszFileName,
    X509_CRL **ppCRL
);

DWORD
VMCAAllocatePrivateKeyPrivate(
    PWSTR pszPassPhrase,
    size_t uiKeyLength,
    PSTR* ppPrivateKey,
    PSTR* ppPublicKey
);

DWORD
VMCAReadCertificateFromFilePrivate(
    PSTR pszFileName,
    PSTR *ppszCertificate
);

DWORD
VMCAReadCertificateChainFromFile(
    PSTR pszFileName,
    PSTR *ppszCertificate
);

DWORD
VMCAReadPrivateKeyFromFilePrivate(
    PSTR pszFileName,
    PSTR pszPassPhrase,
    PSTR* ppszPrivateKey
);

DWORD
VMCAGetCertificateAsStringA(
    PSTR pCertificate,
    PSTR *ppCertString
);

DWORD
VMCAWritePublicKeyToFile(
    PSTR pszPublicKeyFileName,
    PSTR pszPublicKey
);

DWORD
VMCAWritePrivateKeyToFile(
    PSTR pszPrivateKeyFileName,
    PSTR pszPrivateKey,
    PSTR pszPassPhraseFileName,
    PWSTR pszPassPhrase
);

DWORD
VMCAWriteCertificateChainToFile(
    PSTR pszFileName,
    PSTR pszCertificate
    );

DWORD
VMCAWriteCertificateToFile(
    PSTR pszFileName,
    PSTR pszCertificate
);

DWORD
VMCAWriteCSRToFile(
    PSTR pszFileName,
    PSTR pszCSR
);

DWORD
VMCAValidateCACertificatePrivate(
    PSTR pszCertificate,
    PSTR pszPassPhrase,
    PSTR pszPrivateKey
);

DWORD
VMCACreateCertificateName(
        PVMCA_PKCS_10_REQ_DATAW pCertRequest,
        X509_NAME *pCertName
);

DWORD
VMCAAddExtension(
    STACK_OF(X509_EXTENSION) *pStack,
    int NID,
    PSTR pszValue
);

DWORD
VMCACreateSigningRequestPrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PSTR pszPrivateKey,
    PWSTR pszPassPhrase,
    PSTR* ppAllocatedCSR
);

BOOL
VMCAIsSelfSignedCert(
                     X509* pCertificate
                    );

DWORD
VMCAValidateAndFormatCert(
    PCSTR pszCertificate,
    PSTR *ppszPEMCertificate
    );

DWORD
VMCAVerifyCertificateChain(
          STACK_OF(X509) *skX509Certs
          );

DWORD
VMCAParseCertChain(
    PSTR pszCertChainString,
    PSTR **pppszCertChainArray,
    PDWORD pdwCount,
    STACK_OF(X509) **pskX509certs
    );

DWORD
VMCAGetCSRFromCert(
    X509 *pCert,
    EVP_PKEY *pKey,
    X509_REQ  **ppREQ
    );

DWORD
VMCAReadX509FromFile(
    PSTR pszFileName,
    X509 ** ppCertificate
    );

DWORD
VMCAReadPKEYFromFile(
    PSTR pszFileName,
    PSTR pszPassPhrase,
    EVP_PKEY ** ppPrivateKey
    );

DWORD
VMCAGetHostName(
    PSTR* ppszHostName
);
DWORD
VMCAGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostname
    );

int
VMCAStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

int
VMCAStringCompareW(
    PCWSTR pszStr1,
    PCWSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

DWORD
VMCAGetCertificateTimeStrings(
        time_t tmNotBefore,
        time_t tmNotAfter,
        PSTR *ppszNotBefore,
        PSTR *ppszNotAfter
);

DWORD
VMCAGenearteX509Serial(
    ASN1_INTEGER **pAi
);

DWORD
VMCASelfSignedCertificatePrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PSTR pszPrivateKey,
    PWSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PSTR* ppszCertificate
);

DWORD
VMCACreateCA(
    PSTR pszCACertificate,
    PSTR pszPrivateKey,
    PSTR pszPassPhrase,
    PVMCA_X509_CA *pCA
);

PVMCA_X509_CA
VMCAAcquireCA(
    PVMCA_X509_CA pCA
    );

VOID
VMCAReleaseCA(
    PVMCA_X509_CA pCA
);


DWORD
VMCAGetCertificateName(
    X509 *pCert,
    PSTR *ppszCertName
);

DWORD
VMCAGetCertificateSerial(
    X509 *pCert,
    PSTR *ppszSerialNumber
);

DWORD
VMCAGetCertificateTime(
    X509 *pCert,
    PSTR *ppszNotBefore,
    PSTR *ppszNotAfter
);

DWORD
VMCAGetIssuerName(
    X509 *pCert,
    PSTR *ppszIssuerName
);

DWORD
VMCAVerifyExtensions(
    STACK_OF(X509_EXTENSION) *pExtension
);

DWORD
VMCACopyExtensions(
    X509 *pCertificate,
    X509 *pCACertificate,
    X509_REQ *pRequest
);


DWORD
VMCAVerifyCertificateName(
    X509 *pCertificate
);

DWORD
VMCAVerifySubjectAltNames(
    X509 *pCertificate
);


DWORD
VMCASignedRequestPrivate(
    PVMCA_X509_CA pCA,
    PSTR pszPKCS10Request,
    PSTR *ppszCertificate,
    time_t tmNotBefore,
    time_t tmNotAfter
);

DWORD
VMCAWIntegerToASN1Integer(
                            PWSTR pwszInteger,
                            ASN1_INTEGER *asnInteger
                         );

DWORD
VMCACheckFileExists(
    PSTR pszFileName,
    DWORD *pdwExists
);

DWORD
VMCAReadCRLFromFile(
    PSTR pszFileName,
    X509_CRL **ppszCrlData
);

DWORD
VMCACreateNewCRL(
    PVMCA_X509_CA pCA,
    X509_CRL **ppszCrlData
);

DWORD
VMCACheckNotAlreadyRevoked(
    X509 *pCert,
    X509_CRL *pCrl
);

DWORD
VMCACheckNotAlreadyRevoked_Serial(
    ASN1_INTEGER *asnSerial,
    X509_CRL *pCrl
);


DWORD
VMCACreateRevokedFromCert(
    X509 *pCert,
    X509_REVOKED **pRevoked
);

DWORD
VMCACreateRevokedFromCert_Reason(
    ASN1_INTEGER *asnSerial,
    DWORD dwRevokedDate,
    VMCA_CRL_REASON certRevokeReason,
    X509_REVOKED **pRevoked
);


DWORD
VMCAAddCertToCRL(
                X509_CRL *pCrl,
                X509 *pCerts,
                DWORD dwCertCount
);

DWORD
VMCAAddCertToCRL_Reason(
                X509_CRL *pCrl,
                PWSTR pwszSerial,
                DWORD dwRevokedDate,
                VMCA_CRL_REASON certRevokeReason
);


DWORD
VMCAOpenCRLPrivate(
    PVMCA_X509_CA pCA,
    PSTR pszCRLFileName,
    X509_CRL **pX509Crl
);


DWORD
VMCASortCRL(
    X509_CRL *pCrl
);

DWORD
VMCACrlSign(
    X509_CRL *pCrl,
    PVMCA_X509_CA pCA
);

DWORD
VMCACrlFree(
    X509_CRL *pCrl
);

DWORD
VMCAGetNextCrlNumber(
    X509_CRL *pCrl,
    DWORD *pdwNextNum
);

DWORD
VMCAASN1ToTimeT(
        ASN1_TIME *pTime,
        time_t *ptmTime
);

DWORD
VMCAGetNextUpdateTime(
    X509_CRL *pCrl,
    time_t *pNextUpdateTime
);

DWORD
VMCAUpdateTimeStamps(
    X509_CRL *pCrl,
    time_t tmLastUpdate,
    time_t tmNextUpdate,
    DWORD nCrlNum
);

int
VMCAGetDefaultValidityPeriod();

DWORD
VMCAUpdateAuthorityKeyIdentifier(
    X509_CRL *pCrl,
    PVMCA_X509_CA pCA
    );

DWORD
VMCAGetInstallDirectory(PSTR *ppszInstallDir);
// VMCAGetInstallDirectory returns the Installation Directory of VMCA


DWORD
VMCAGetDataDirectory(PSTR *ppszDataDir);
// VMCAGetDataDirectory returns the Data Directory of VMCA


DWORD
VMCAGetRootCertificateFilePath(PSTR *pszRootPath);

DWORD
VMCAGetPrivateKeyPath(PSTR *pszPrivPath);

DWORD
VMCAGetPrivateKeyPasswordPath(PSTR *pszPrivPath);

DWORD
VMCAGetCertsDBPath(PSTR *pszPrivPath);

DWORD
VMCAGetCRLNamePath(PSTR *pszPrivPath);

DWORD
VMCAGetTempCRLNamePath(PSTR *pszPrivPath);

DWORD
VMCAStringToLower(
    PSTR pszString,
    PSTR *ppszNewString
    );

void
VMCASetBit(unsigned long *flag, int bit);

// This is the shared Key Protocol used by VMDIR and KRB to protect the KRB interface.
// We are using the same protocol in VMCA to protect against scenerios where KRB *may*
// not be configured correctly. Say a network with DNS incorrect configured.
//
// We will remove this code path once we have DNS working with cloud VM.
// This protocol will *ONLY* work if this VMDIR key is present, in 2013
// deployment this  Key should be present.

#define VMCA_SHARED_SECRET_LEN    20

#ifndef _WIN32
#define VMCA_SHARED_SECRET_KEY     "Services\\Vmdir"
#else
#define VMCA_SHARED_SECRET_KEY     "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#endif
#define VMCA_SHARED_ROOT_KEY        "dcAccountPassword"

DWORD
VMCAGetSharedSecret(
    PWSTR* ppszPassword
    );

DWORD
VMCASetSharedSecret(
    PWSTR pszPassword);

int
VMCAisBitSet(unsigned long flag, int bit);

void
VMCAClearBit(unsigned long flag, int bit);

void
VMCAToggleBit(unsigned long flag, int bit);

DWORD
VMCAGetCRLInfoPrivate(
    PSTR pszFileName,
    time_t *ptmLastUpdate,
    time_t *ptmNextUpdate,
    DWORD  *pdwCRLNumber);

DWORD
VMCAPrintCRLPrivate(
    PSTR pszFileName,
    PSTR *ppszCRLString
);

DWORD
VMCAGetLogDirectory(
                    PSTR *ppszLogDir
);

DWORD
VMCALdapConnect(
    PSTR   pszHostName,
    DWORD  dwPort,
    PSTR   pszUserName,
    PSTR   pszPassword,
    PVMCA_LDAP_CONTEXT* ppContext
    );

DWORD
VMCAGetDSERootAttribute(
    PVMCA_LDAP_CONTEXT pConnection,
    PSTR  pszAttribute,
    PSTR* ppAttrValue
    );

DWORD
VMCAGetDefaultDomainName2(
    PVMCA_LDAP_CONTEXT pConnection,
    PSTR* ppDomainName
    );

DWORD
VMCAGetDSEServerName(
    PVMCA_LDAP_CONTEXT pConnection,
    PSTR* ppServerName
    );

DWORD
VMCAUpdatePkiCAAttribute(
    PVMCA_LDAP_CONTEXT pConnection,
    PSTR  pszServerNameDN,
    X509* pCertificate
    );

DWORD
VMCAUpdateCrlCAAttribute(
    PVMCA_LDAP_CONTEXT pConnection,
    PSTR pszConfigurationDN,
    PSTR  pszCRL
    );

VOID
VMCALdapClose(
    PVMCA_LDAP_CONTEXT pHandle
    );

DWORD
VMCALdapGetMemberships(
    PVMCA_LDAP_CONTEXT pConnection,
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    );
//
// Config
//

DWORD
VmwConfigOpenConnection(
    PVMW_CFG_CONNECTION* ppConnection
    );

DWORD
VmwConfigOpenRootKey(
    PVMW_CFG_CONNECTION pConnection,
    PCSTR               pszKeyName,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMW_CFG_KEY*       ppKey
    );

DWORD
VmwConfigOpenKey(
    PVMW_CFG_CONNECTION pConnection,
    PVMW_CFG_KEY        pKey,
    PCSTR               pszSubKey,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMW_CFG_KEY*       ppKey
    );

DWORD
VmwConfigReadStringValue(
    PVMW_CFG_KEY        pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    );

DWORD
VmwConfigReadDWORDValue(
    PVMW_CFG_KEY        pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    );

VOID
VmwConfigCloseKey(
    PVMW_CFG_KEY pKey
    );

VOID
VmwConfigCloseConnection(
    PVMW_CFG_CONNECTION pConnection
    );

DWORD
VMCAGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszPassword
    );

DWORD
VMCAConfigGetDword(
    PCSTR pcszValueName,
    DWORD* pdwOutput
    );

DWORD
VMCAConfigSetDword(
    PCSTR  pcszValueName,
    DWORD  dwInput
    );

BOOLEAN
VMCAConfigIsServerOptionEnabled(
    VMCA_SERVER_OPTION  option
    );

DWORD
VMCAAccountDnToUpn(
    PSTR dn,
    PSTR *retUpn
    );

DWORD
VMCAGetDceRpcShortErrorString(
    DWORD dwRpcError,
    PSTR* szErrorMessage
    );

DWORD
VMCAGetDceRpcErrorString(
    DWORD dwRpcError,
    PSTR* szErrorMessage
    );

DWORD
VMCAGetWin32ErrorString(
    DWORD dwWin32Error,
    PSTR* szErrorMessage
    );

#ifndef WIN32

DWORD
VMCAGetWin32ErrorCode(
    DWORD dwUnixError
    );

PCSTR
VMCAGetWin32ErrorDesc(
    DWORD dwUnixError
    );

#endif

#ifdef __cplusplus
}
#endif

#endif /* __VMCA_COMMON_H__ */
