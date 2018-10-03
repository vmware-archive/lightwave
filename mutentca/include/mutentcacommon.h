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

#ifndef __LWCA_COMMON_H__
#define __LWCA_COMMON_H__

#include <pthread.h>
#include <dlfcn.h>

#if !defined(NO_LIKEWISE)
#include <lw/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// We don't want the LikeWise headers since we conflict
// with Unix ODBC, and we are on Unix. Define all types ourselves
#if (defined NO_LIKEWISE &&  !defined _WIN32)

#ifndef LWCA_WCHAR16_T_DEFINED
#define LWCA_WCHAR16_T_DEFINED 1
typedef unsigned short int wchar16_t, *PWSTR;
#endif /* LWCA_WCHAR16_T_DEFINED */

#ifndef LWCA_PSTR_DEFINED
#define LWCA_PSTR_DEFINED 1
typedef char* PSTR;
#endif /* LWCA_PSTR_DEFINED */

#ifndef LWCA_PCSTR_DEFINED
#define LWCA_PCSTR_DEFINED 1
typedef const char* PCSTR;
#endif /* LWCA_PCSTR_DEFINED */

#ifndef LWCA_PCWSTR_DEFINED
#define LWCA_PCWSTR_DEFINED 1
typedef const wchar16_t* PCWSTR;
#endif /* LWCA_PCWSTR_DEFINED */

#ifndef LWCA_BYTE_DEFINED
#define LWCA_BYTE_DEFINED 1
typedef unsigned char BYTE;
#endif /* LWCA_BYTE_DEFINED */

#ifndef LWCA_VOID_DEFINED
#define LWCA_VOID_DEFINED 1

typedef void VOID, *PVOID;
#endif /* LWCA_VOID_DEFINED */

#ifndef LWCA_UINT8_DEFINED
#define LWCA_UINT8_DEFINED 1
typedef uint8_t  UINT8;
#endif /* LWCA_UINT8_DEFINED */

#ifndef LWCA_UINT32_DEFINED
#define LWCA_UINT32_DEFINED 1
typedef uint32_t UINT32;
#endif /* LWCA_UINT32_DEFINED */

#ifndef LWCA_DWORD_DEFINED
#define LWCA_DWORD_DEFINED 1
typedef uint32_t DWORD, *PDWORD;
#endif /* LWCA_DWORD_DEFINED */

#ifndef LWCA_BOOLEAN_DEFINED
#define LWCA_BOOLEAN_DEFINED 1
typedef UINT8 BOOLEAN, *PBOOLEAN;
#endif /* LWCA_BOOLEAN_DEFINED */

#endif /* defined NO_LIKEWISE &&  !defined _WIN32 */

typedef struct _LWCA_CFG_CONNECTION* PLWCA_CFG_CONNECTION;
typedef struct _LWCA_CFG_KEY*        PLWCA_CFG_KEY;

typedef struct _LWCA_STRING_ARRAY
{
    PSTR    *ppData;
    DWORD   dwCount;
} LWCA_STRING_ARRAY, *PLWCA_STRING_ARRAY;

#define LWCA_ASCII_aTof(c)     ( (c) >= 'a' && (c) <= 'f' )
#define LWCA_ASCII_AToF(c)     ( (c) >= 'A' && (c) <= 'F' )
#define LWCA_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )
#define LWCA_ASCII_SPACE(c) \
    ( (c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' )

#define LWCA_MIN(a, b) ((a) < (b) ? (a) : (b))
#define LWCA_MAX(a, b) ((a) > (b) ? (a) : (b))

#define LWCA_SAFE_FREE_STRINGA(PTR)     \
    do {                                \
        if ((PTR)) {                    \
            LwCAFreeStringA(PTR);       \
            (PTR) = NULL;               \
        }                               \
    } while(0)

#define LWCA_SAFE_FREE_STRINGW(PTR)     \
    do {                                \
        if ((PTR)) {                    \
            LwCAFreeStringW(PTR);       \
            (PTR) = NULL;               \
        }                               \
    } while(0)

#define LWCA_SAFE_FREE_MEMORY(PTR)  \
    do {                            \
        if ((PTR)) {                \
            LwCAFreeMemory(PTR);    \
            (PTR) = NULL;           \
        }                           \
    } while(0)

#define LWCA_SECURE_SAFE_FREE_MEMORY(PTR, n)     \
    do {                                         \
        if ((PTR)) {                             \
            if ((n) > 0) {                       \
                memset(PTR, 0, n);               \
            }                                    \
            LwCAFreeMemory(PTR);                 \
            (PTR) = NULL;                        \
        }                                        \
    } while(0)

#define LWCA_LOCK_MUTEX(bInLock, mutex)     \
    do {                                    \
        if (!(bInLock))                     \
        {                                   \
            pthread_mutex_lock(mutex);      \
            (bInLock) = TRUE;               \
        }                                   \
    } while (0)

#define LWCA_TRYLOCK_MUTEX(bInLock, mutex, dwError)                 \
    do {                                                            \
        if (!(bInLock))                                             \
        {                                                           \
            int iResult = pthread_mutex_trylock(mutex);             \
            if (iResult == 0)                                       \
            {                                                       \
                (bInLock) = TRUE;                                   \
            }                                                       \
            else                                                    \
            {                                                       \
                (dwError) = LWCA_ERRNO_TO_LWCAERROR(iResult);       \
            }                                                       \
        }                                                           \
    } while (0)

#define LWCA_UNLOCK_MUTEX(bInLock, mutex)   \
    do {                                    \
        if ((bInLock))                      \
        {                                   \
            pthread_mutex_unlock(mutex);    \
            (bInLock) = FALSE;              \
        }                                   \
    } while (0)

typedef enum
{
    LWCA_LOG_TYPE_CONSOLE = 0,
    LWCA_LOG_TYPE_FILE,
    LWCA_LOG_TYPE_SYSLOG
} LWCA_LOG_TYPE;

#ifndef _LWCA_LOG_LEVEL_DEFINED_
#define _LWCA_LOG_LEVEL_DEFINED_
typedef enum
{
   LWCA_LOG_LEVEL_EMERGENCY = 0,
   LWCA_LOG_LEVEL_ALERT,
   LWCA_LOG_LEVEL_CRITICAL,
   LWCA_LOG_LEVEL_ERROR,
   LWCA_LOG_LEVEL_WARNING,
   LWCA_LOG_LEVEL_NOTICE,
   LWCA_LOG_LEVEL_INFO,
   LWCA_LOG_LEVEL_DEBUG
} LWCA_LOG_LEVEL;
#endif /* _LWCA_LOG_LEVEL_DEFINED_ */

DWORD
LwCAInitLog(
    VOID
    );

VOID
LwCATerminateLogging(
    VOID
    );

VOID
LwCALog(
   LWCA_LOG_LEVEL level,
   const char*      fmt,
   ...
   );

typedef struct _LWCA_LOG_HANDLE* PLWCA_LOG_HANDLE;

extern PLWCA_LOG_HANDLE gpLwCALogHandle;
extern LWCA_LOG_LEVEL   gLwCALogLevel;
extern HANDLE           gpEventLog;
extern LWCA_LOG_TYPE    gLwCALogType;
extern LWCA_LOG_LEVEL LwCALogGetLevel();

#define _LWCA_LOG( Level, Format, ... )             \
    do                                              \
    {                                               \
        LwCALog(                                    \
               (Level),                             \
               (Format),                            \
               ##__VA_ARGS__);                      \
    } while (0)

#define LWCA_LOG_GENERAL( Level, Format, ... ) \
    _LWCA_LOG( (Level), (Format), ##__VA_ARGS__ )

#define LWCA_LOG_ERROR( Format, ... ) \
    LWCA_LOG_GENERAL( LWCA_LOG_LEVEL_ERROR, (Format), ##__VA_ARGS__ )

#define LWCA_LOG_WARNING( Format, ... ) \
    LWCA_LOG_GENERAL( LWCA_LOG_LEVEL_WARNING, (Format), ##__VA_ARGS__ )

#define LWCA_LOG_INFO( Format, ... ) \
    LWCA_LOG_GENERAL( LWCA_LOG_LEVEL_INFO, (Format), ##__VA_ARGS__ )

#define LWCA_LOG_VERBOSE( Format, ... ) \
    LWCA_LOG_GENERAL( LWCA_LOG_LEVEL_DEBUG, (Format), ##__VA_ARGS__ )

#define LWCA_LOG_DEBUG( Format, ... )       \
    LWCA_LOG_GENERAL(                       \
        LWCA_LOG_LEVEL_DEBUG,               \
        "[file: %s][line: %d] (Format)",    \
        __FILE__, __LINE__, ##__VA_ARGS__)

#define BAIL_ON_LWCA_ERROR(dwError)                                         \
    if (dwError)                                                            \
    {                                                                       \
        LWCA_LOG_WARNING("error code: %#010x", dwError);                    \
        goto error;                                                         \
    }

#define BAIL_ON_COAPI_ERROR_WITH_MSG(dwError, errMsg)                       \
    if (dwError)                                                            \
    {                                                                       \
        LWCA_LOG_ERROR("[%s:%d] %s. copenapi error (%d)",                   \
                            __FUNCTION__,                                   \
                            __LINE__,                                       \
                            errMsg,                                         \
                            dwError);                                       \
        dwError = LWCA_COAPI_ERROR;                                         \
        goto error;                                                         \
    }

#define BAIL_ON_CREST_ERROR_WITH_MSG(dwError, errMsg)                       \
    if (dwError)                                                            \
    {                                                                       \
        LWCA_LOG_ERROR("[%s:%d] %s. c-rest-engine error (%d)",              \
                            __FUNCTION__,                                   \
                            __LINE__,                                       \
                            errMsg,                                         \
                            dwError);                                       \
        dwError = LWCA_CREST_ENGINE_ERROR;                                  \
        goto error;                                                         \
    }

#define BAIL_ON_JSON_ERROR_WITH_MSG(dwError, errMsg)                        \
    if (dwError)                                                            \
    {                                                                       \
        LWCA_LOG_ERROR("[%s:%d] %s. jansson api error (%d)",                \
                            __FUNCTION__,                                   \
                            __LINE__,                                       \
                            errMsg,                                         \
                            dwError);                                       \
        dwError = LWCA_JSON_ERROR;                                          \
        goto error;                                                         \
    }

#define BAIL_ON_JSON_PARSE_ERROR(dwError)       \
    if ((dwError))                              \
    {                                           \
        (dwError) = LWCA_JSON_PARSE_ERROR;      \
        goto error;                             \
    }

#define BAIL_ON_LWCA_ERROR_NO_LOG(dwError) \
    if ((dwError)) { goto error; }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif /* IsNullOrEmptyString */

#ifndef LWCA_SAFE_STRING
#define LWCA_SAFE_STRING(str) ((str) ? (str) : "")
#endif /* LWCA_SAFE_STRING */

// Event logs related constants.

#define LWCA_EVENT_SOURCE   "Lightwave MutentCA Service"

SIZE_T
LwCAStringLenA(
    PCSTR pszStr
    );

DWORD
LwCAAllocateMemory(
    DWORD dwSize,
    PVOID *ppMemory
    );

DWORD
LwCAReallocateMemory(
    PVOID        pMemory,
    PVOID*       ppNewMemory,
    DWORD        dwSize
    );

DWORD
LwCAReallocateMemoryWithInit(
    PVOID         pMemory,
    PVOID*        ppNewMemory,
    size_t        dwNewSize,
    size_t        dwOldSize
    );

DWORD
LwCACopyMemory(
    PVOID       pDst,
    size_t      dstSize,
    const void* pSrc,
    size_t      cpySize
    );

DWORD
LwCAAllocateAndCopyMemory(
    PVOID   pBlob,
    size_t  iBlobSize,
    PVOID*  ppOutBlob
    );

VOID
LwCAFreeMemory(
    PVOID pMemory
    );

DWORD
LwCAAllocateStringA(
    PCSTR pszString,
    PSTR * ppszString
    );

DWORD
LwCAAllocateStringWithLengthA(
    PCSTR pszString,
    DWORD dwSize,
    PSTR * ppszString
    );

DWORD
LwCAAllocateStringPrintfA(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    );

VOID
LwCAFreeStringA(
    PSTR pszString
    );

VOID
LwCAFreeStringW(
    PWSTR pszString
    );

DWORD
LwCACopyStringArrayA(
    PSTR            **pppszDst,
    DWORD           dwDstLen,
    PSTR            *ppszSrc,
    DWORD           dwSrcLen
    );

DWORD
LwCACopyStringArray(
    PLWCA_STRING_ARRAY  pStrInputArray,
    PLWCA_STRING_ARRAY* ppStrOutputArray
    );

VOID
LwCAFreeStringA(
    PSTR pszString
    );

VOID
LwCAFreeStringArrayA(
    PSTR* ppszStrings,
    DWORD dwCount
    );

VOID
LwCAFreeStringArray(
    PLWCA_STRING_ARRAY pStrArray
    );

DWORD
LwCAGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

ULONG
LwCAAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

ULONG
LwCAAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

ULONG
LwCAAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

BOOL
LwCAStringIsEqualW (
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    );

BOOLEAN
LwCAIsValidSecret(
    PWSTR pszTheirs,
    PWSTR pszOurs
    );

int
LwCAStringCompareW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    );

int
LwCAStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

DWORD
LwCAStringNCpyA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource,
    size_t count
    );

PSTR
LwCAStringChrA(
    PCSTR str,
    int c
    );

PSTR
LwCAStringTokA(
    PSTR strToken,
    PCSTR strDelimit,
    PSTR* context
    );

DWORD
LwCAStringCountSubstring(
    PCSTR pszHaystack,
    PCSTR pszNeedle,
    int *pnCount
    );

DWORD
LwCAStringCatA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource
    );

VOID
LwCAStringTrimSpace(
    PSTR    pszStr
    );

DWORD
LwCAGetUTCTimeString(
    PSTR *pszTimeString
    );

int
LwCAStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

int
LwCAStringCompareW(
    PCWSTR pszStr1,
    PCWSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

DWORD
LwCAStringToLower(
    PSTR pszString,
    PSTR *ppszNewString
    );

int
LwCAStringToInt(
    PCSTR pszStr
    );

VOID
LwCASetBit(
    unsigned long *flag,
    int bit
    );

LWCA_LOG_LEVEL
LwCALogGetLevel(
    );

VOID
LwCALogSetLevel(
    LWCA_LOG_LEVEL level
    );

DWORD
LwCAOpenFilePath(
    PCSTR szFileName,
    PCSTR szOpenMode,
    FILE** fp
    );

DWORD
LwCACopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    );

// misc.c

typedef VOID*       LWCA_LIB_HANDLE;

DWORD
LwCALoadLibrary(
    PCSTR           pszLibPath,
    LWCA_LIB_HANDLE* ppLibHandle
    );

VOID
LwCACloseLibrary(
    LWCA_LIB_HANDLE  pLibHandle
    );

VOID*
LwCAGetLibSym(
    LWCA_LIB_HANDLE  pLibHandle,
    PCSTR           pszFunctionName
    );

DWORD
LwCABytesToHexString(
    PUCHAR  pData,
    DWORD   length,
    PSTR*   pszHexString,
    BOOLEAN bLowerCase
    );

DWORD
LwCAHexStringToBytes(
    PSTR    pszHexStr,
    PUCHAR* ppData,
    size_t* pLength
    );

DWORD
LwCAGetInstallDirectory(
    PSTR *ppszInstallDir
    );

DWORD
LwCAGetDataDirectory(
    PSTR *ppszDataDir
    );

DWORD
LwCAGetLogDirectory(
    PSTR *ppszLogDir
    );



/////////////////////////////Actual LwCA Common Functions///////////////////

DWORD
LwCACommonInit(
    VOID
    );

DWORD
LwCACommonShutdown(
    VOID
    );

DWORD
LwCAOpenSSLInitialize(
    VOID
    );

DWORD
LwCAOpenSSLCleanup(
    VOID
    );

int
LwCAisBitSet(
    unsigned long flag,
    int bit
    );

void
LwCAClearBit(
    unsigned long flag,
    int bit
    );

void
LwCAToggleBit(
    unsigned long flag,
    int bit
    );

DWORD
LwCAGetLogDirectory(
    PSTR *ppszLogDir
    );

// thread.c

typedef pthread_t LWCA_THREAD;

typedef LWCA_THREAD* PLWCA_THREAD;

typedef DWORD (LwCAStartRoutine)(PVOID);
typedef LwCAStartRoutine* PLWCA_START_ROUTINE;

typedef struct _LWCA_MUTEX
{
    BOOLEAN                 bInitialized;
    pthread_mutex_t         critSect;
} LWCA_MUTEX, *PLWCA_MUTEX;

typedef struct _LWCA_COND
{
    BOOLEAN                 bInitialized;
    pthread_cond_t          cond;
} LWCA_COND, *PLWCA_COND;

typedef struct _LWCA_RWLOCK
{
    pthread_rwlock_t    rwLock;
} LWCA_RWLOCK, *PLWCA_RWLOCK;

typedef struct _LWCA_THREAD_START_INFO
{
    LwCAStartRoutine*      pStartRoutine;
    PVOID                   pArgs;
} LWCA_THREAD_START_INFO, *PLWCA_THREAD_START_INFO;

DWORD
LwCAAllocateMutex(
    PLWCA_MUTEX* ppMutex
    );

VOID
LwCAFreeMutex(
    PLWCA_MUTEX pMutex
    );

DWORD
LwCALockMutex(
    PLWCA_MUTEX pMutex
    );

DWORD
LwCAUnlockMutex(
    PLWCA_MUTEX pMutex
    );

BOOLEAN
LwCAIsMutexInitialized(
    PLWCA_MUTEX pMutex
    );

DWORD
LwCAAllocateCondition(
    PLWCA_COND* ppCondition
    );

VOID
LwCAFreeCondition(
    PLWCA_COND pCondition
    );

DWORD
LwCAConditionWait(
    PLWCA_COND pCondition,
    PLWCA_MUTEX pMutex
    );

DWORD
LwCAConditionTimedWait(
    PLWCA_COND pCondition,
    PLWCA_MUTEX pMutex,
    DWORD dwMilliseconds
    );

DWORD
LwCAConditionSignal(
    PLWCA_COND pCondition
    );

DWORD
LwCACreateThread(
    PLWCA_THREAD pThread,
    BOOLEAN bDetached,
    PLWCA_START_ROUTINE pStartRoutine,
    PVOID pArgs
    );

DWORD
LwCAInitializeMutexContent(
    PLWCA_MUTEX            pMutex
    );

VOID
LwCAFreeMutexContent(
    PLWCA_MUTEX            pMutex
    );

DWORD
LwCAInitializeConditionContent(
    PLWCA_COND             pCondition
    );

VOID
LwCAFreeConditionContent(
    PLWCA_COND             pCondition
    );

DWORD
LwCAThreadJoin(
    PLWCA_THREAD pThread,
    PDWORD pRetVal
    );

VOID
LwCAFreeThread(
    PLWCA_THREAD pThread
    );

// vecs.c

DWORD
LwCAGetVecsMachineCert(
    PSTR*   ppszCert,
    PSTR*   ppszKey
    );

DWORD
LwCAGetVecsMutentCACert(
    PSTR*   ppszCert,
    PSTR*   ppszKey
    );

DWORD
LwCAOpenVmAfdClientLib(
    LWCA_LIB_HANDLE*   pplibHandle
    );

#ifdef __cplusplus
}
#endif

#endif /* __LWCA_COMMON_H__ */
