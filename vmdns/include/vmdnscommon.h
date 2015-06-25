/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
 * Module Name:  vmdnscommon.h
 *
 * Abstract: VMware Domain Name Service.
 *
 * Created on: Sep 18, 2012
 *
 */

#ifndef _VMDNS_COMMON_H__
#define _VMDNS_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dce/uuid.h>
#include <dce/dcethread.h>

#include "vmdnsdefines.h"
#include "vmdnsbuffer.h"
//#if defined(_WIN32)
//typedef unsigned char uuid_t[16];  // typedef dce_uuid_t uuid_t;
//#endif

// Logging
extern int  vmdns_syslog;
extern int  vmdns_debug;

typedef enum
{
    VMDNS_LOG_TYPE_CONSOLE = 0,
    VMDNS_LOG_TYPE_FILE,
    VMDNS_LOG_TYPE_SYSLOG
} VMDNS_LOG_TYPE;

#ifndef _VMDNS_LOG_LEVEL_DEFINED_
#define _VMDNS_LOG_LEVEL_DEFINED_
typedef enum
{
   VMDNS_LOG_LEVEL_EMERGENCY = 0,
   VMDNS_LOG_LEVEL_ALERT,
   VMDNS_LOG_LEVEL_CRITICAL,
   VMDNS_LOG_LEVEL_ERROR,
   VMDNS_LOG_LEVEL_WARNING,
   VMDNS_LOG_LEVEL_NOTICE,
   VMDNS_LOG_LEVEL_INFO,
   VMDNS_LOG_LEVEL_DEBUG
} VMDNS_LOG_LEVEL;
#endif

DWORD
VmDnsLogInitialize(
	PCSTR   pszLogFileName,
	DWORD   dwMaximumOldFiles,
	DWORD   dwMaxLogSizeBytes
	);

void
VmDnsLogTerminate();

void
VmDnsLog(
   VMDNS_LOG_LEVEL level,
   const char*      fmt,
   ...);

typedef struct _VMDNS_LOG_HANDLE* PVMDNS_LOG_HANDLE;

extern PVMDNS_LOG_HANDLE gpVMDNSLogHandle;
extern VMDNS_LOG_LEVEL   gVMDNSLogLevel;
extern HANDLE           gpEventLog;
extern VMDNS_LOG_TYPE    gVMDNSLogType;
extern VMDNS_LOG_LEVEL VMDNSLogGetLevel();

#define VMDNS_LOG_( Level, Format, ... ) \
    do                                             \
    {                                              \
        VmDnsLog(                                   \
               Level,                              \
               Format,                             \
               ##__VA_ARGS__);                     \
    } while (0)

#define VMDNS_LOG_GENERAL_( Level, Format, ... ) \
    VMDNS_LOG_( Level, Format, ##__VA_ARGS__ )

#define VMDNS_LOG_ERROR( Format, ... )   \
    VMDNS_LOG_GENERAL_( VMDNS_LOG_LEVEL_ERROR, Format, ##__VA_ARGS__ )
#define VMDNS_LOG_WARNING( Format, ... ) \
    VMDNS_LOG_GENERAL_( VMDNS_LOG_LEVEL_WARNING, Format, ##__VA_ARGS__ )
#define VMDNS_LOG_INFO( Format, ... )    \
    VMDNS_LOG_GENERAL_( VMDNS_LOG_LEVEL_INFO, Format, ##__VA_ARGS__ )
#define VMDNS_LOG_VERBOSE( Format, ... ) \
    VMDNS_LOG_GENERAL_( VMDNS_LOG_LEVEL_DEBUG, Format, ##__VA_ARGS__ )
#define VMDNS_LOG_DEBUG( Format, ... )       \
    VMDNS_LOG_GENERAL_(                      \
        VMDNS_LOG_LEVEL_DEBUG,               \
    Format " [file: %s][line: %d]",     \
    ##__VA_ARGS__, __FILE__, __LINE__ )

// Read write lock

typedef struct _VMDNS_RWLOCK* PVMDNS_RWLOCK;

#define VMDNS_FREE_RWLOCK(pLock) \
{ \
    VmDnsFreeRWLock(pLock); \
    pLock = NULL; \
}

DWORD
VmDnsAllocateRWLock(
    PVMDNS_RWLOCK* ppLock
    );

VOID
VmDnsFreeRWLock(
    PVMDNS_RWLOCK pLock
    );

void VmDnsLockRead(
    PVMDNS_RWLOCK  pLock
    );

int VmDnsTryLockRead(
    PVMDNS_RWLOCK  pLock
    );

void VmDnsUnlockRead(
    PVMDNS_RWLOCK  pLock
    );

void VmDnsLockWrite(
    PVMDNS_RWLOCK  pLock
    );

int VmDnsTryLockWrite(
    PVMDNS_RWLOCK  pLock
    );

void VmDnsUnlockWrite(
    PVMDNS_RWLOCK  pLock
    );

#define VMDNS_LOCKREAD(pLock) \
{ \
    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "LOCKRD at [%s,%s,%d]", __FILE__, __FUNCTION__, __LINE__); \
    VmDnsLockRead(pLock); \
}

#define VMDNS_TRYLOCKREAD(pLock) \
( \
    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "TRYLOCKRD at [%s,%s,%d]", __FILE__, __FUNCTION__, __LINE__), \
    VmDnsTryLockRead(pLock) \
)

#define VMDNS_UNLOCKREAD(pLock) \
{ \
    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "UNLOCKWR at [%s,%s,%d]", __FILE__, __FUNCTION__, __LINE__); \
    VmDnsUnlockRead(pLock); \
}

#define VMDNS_LOCKWRITE(pLock) \
{ \
    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "LOCKWR at [%s,%s,%d]", __FILE__, __FUNCTION__, __LINE__); \
    VmDnsLockWrite(pLock); \
}

#define VMDNS_TRYLOCKWRITE(pLock) \
( \
    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "TRYLOCKWR at [%s,%s,%d]", __FILE__, __FUNCTION__, __LINE__), \
    VmDnsTryLockWrite(pLock) \
)

#define VMDNS_UNLOCKWRITE(pLock) \
{ \
    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "UNLOCKWR at [%s,%s,%d]", __FILE__, __FUNCTION__, __LINE__); \
    VmDnsUnlockWrite(pLock); \
}

DWORD
VmDnsAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

DWORD
VmDnsReallocateMemory(
    PVOID   pMemory,
    PVOID*  ppNewMemory,
    size_t  dwSize
    );

DWORD
VmDnsCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    PVOID   pSource,
    size_t  maxCount
    );

DWORD
VmDnsReallocateMemoryWithInit(
    PVOID  pMemory,
    PVOID* ppNewMemory,
    size_t dwNewSize,
    size_t dwOldSize
    );

VOID
VmDnsFreeMemory(
    PVOID   pMemory
    );

VOID
VmDnsFreeStringA(
    PSTR    pszString
    );

VOID
VmDnsFreeStringArrayA(
    PSTR*   ppszString
    );

VOID
VmDnsFreeStringCountedArrayA(
    PSTR* ppszString,
    DWORD dwCount
    );

DWORD
VmDnsAllocateStringAVsnprintf(
    PSTR*    ppszOut,
    PCSTR    pszFormat,
    ...
    );

ULONG
VmDnsAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmDnsAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

ULONG
VmDnsAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmDnsAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

ULONG
VmDnsAllocateStringPrintfVA(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

ULONG
VmDnsAllocateStringPrintfA(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    );

ULONG
VmDnsGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

ULONG
VmDnsStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

ULONG
VmDnsStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

ULONG
VmDnsStringCompareW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    );

SIZE_T
VmDnsStringLenA(
    PCSTR pszStr
);

PSTR
VmDnsStringChrA(
   PCSTR str,
   int c
);

PSTR
VmDnsStringRChrA(
   PCSTR str,
   int c
);

PSTR
VmDnsStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
);

PSTR
VmDnsStringStrA(
   PCSTR str,
   PCSTR strSearch
);

DWORD
VmDnsStringCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

DWORD
VmDnsStringNCpyA(
   PSTR strDest,
   size_t numberOfElements,
   PCSTR strSource,
   size_t count
);

DWORD
VmDnsStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

int64_t
VmDnsStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
);

int
VmDnsStringToIA(
   PCSTR pStr
);

DWORD
VmDnsStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
);

PSTR
VmDnsCaselessStrStrA(
    PCSTR pszStr1,
    PCSTR pszStr2
    );

DWORD
VmDnsStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
);

DWORD
VmDnsStringNPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    size_t maxSize,
    PCSTR pszFormat,
    ...
);

DWORD
VmDnsRpcAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

ULONG
VmDnsRpcAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

VOID
VmDnsRpcFreeStringA(
    PSTR    pszString
    );

VOID
VmDnsRpcFreeMemory(
    PVOID pMemory
    );

DWORD
VmDnsRpcCopyRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    );

VOID
VmDnsRpcFreeRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    );

DWORD
VmDnsRpcCopyZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfoSrc,
    PVMDNS_ZONE_INFO pZoneInfoDest
    );

DWORD
VmDnsRpcCopyZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfoArray
    );

VOID
VmDnsRpcFreeZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfo
    );

VOID
VmDnsRpcFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray
    );

#ifdef _WIN32

//cmd line args parsing helpers
BOOLEAN
VmDnsIsCmdLineOption(
    PSTR pArg
);

VOID
VmDnsGetCmdLineOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    PCSTR* ppszOptionValue
);

DWORD
VmDnsGetCmdLineIntOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    int* pValue
);

DWORD
VmDnsAllocateArgsAFromArgsW(
    int argc,
    WCHAR* argv[],
    PSTR** argvA
);

VOID
VmDnsDeallocateArgsA(
    int argc,
    PSTR argv[]
);

#endif

/* mutexes/threads/conditions */
typedef struct _VMDNS_MUTEX* PVMDNS_MUTEX;

typedef struct _VMDNS_COND* PVMDNS_COND;

typedef pthread_t VMDNS_THREAD;

typedef VMDNS_THREAD* PVMDNS_THREAD;

typedef DWORD (VmDnsStartRoutine)(PVOID);
typedef VmDnsStartRoutine* PVMDNS_START_ROUTINE;

DWORD
VmDnsAllocateMutex(
    PVMDNS_MUTEX* ppMutex
);

VOID
VmDnsFreeMutex(
    PVMDNS_MUTEX pMutex
);

DWORD
VmDnsLockMutex(
    PVMDNS_MUTEX pMutex
);

DWORD
VmDnsUnlockMutex(
    PVMDNS_MUTEX pMutex
);

BOOLEAN
VmDnsIsMutexInitialized(
    PVMDNS_MUTEX pMutex
);

DWORD
VmDnsAllocateCondition(
    PVMDNS_COND* ppCondition
);

VOID
VmDnsFreeCondition(
    PVMDNS_COND pCondition
);

DWORD
VmDnsConditionWait(
    PVMDNS_COND pCondition,
    PVMDNS_MUTEX pMutex
);

DWORD
VmDnsConditionTimedWait(
    PVMDNS_COND pCondition,
    PVMDNS_MUTEX pMutex,
    DWORD dwMilliseconds
);

DWORD
VmDnsConditionSignal(
    PVMDNS_COND pCondition
);

DWORD
VmDnsCreateThread(
    PVMDNS_THREAD pThread,
    BOOLEAN bDetached,
    PVMDNS_START_ROUTINE pStartRoutine,
    PVOID pArgs
);

DWORD
VmDnsThreadJoin(
    PVMDNS_THREAD pThread,
    PDWORD pRetVal
);

VOID
VmDnsFreeThread(
    PVMDNS_THREAD pThread
);

struct _VMDNS_CFG_CONNECTION;
struct _VMDNS_CFG_CONNECTION;
typedef struct _VMDNS_CFG_CONNECTION*   PVMDNS_CFG_CONNECTION;
typedef struct _VMDNS_CFG_KEY*          PVMDNS_CFG_KEY;

DWORD
VmDnsConfigOpenConnection(
    PVMDNS_CFG_CONNECTION*  ppConnection
    );

DWORD
VmDnsConfigOpenRootKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PCSTR                   pszKeyName,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    );

DWORD
VmDnsOpenKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubKey,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    );

DWORD
VmDnsConfigOpenKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubKey,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    );

DWORD
VmDnsConfigReadStringValue(
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubkey,
    PCSTR                   pszName,
    PSTR*                   ppszValue
    );

DWORD
VmDnsConfigReadDWORDValue(
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubkey,
    PCSTR                   pszName,
    PDWORD                  pdwValue
    );

VOID
VmDnsConfigCloseKey(
    PVMDNS_CFG_KEY          pKey
    );

VOID
VmDnsConfigCloseConnection(
    PVMDNS_CFG_CONNECTION   pConnection
    );

PVMDNS_CFG_CONNECTION
VmDnsConfigAcquireConnection(
    PVMDNS_CFG_CONNECTION   pConnection
    );

VOID
VmDnsConfigFreeConnection(
    PVMDNS_CFG_CONNECTION   pConnection
    );

/* record */

DWORD
VmDnsCreateRecord(
    PWSTR               pwszName,
    VMDNS_RR_TYPE       type,
    PVMDNS_RECORD_DATA  pRecordData,
    PVMDNS_RECORD*      ppRecord
    );

BOOLEAN
VmDnsCompareRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    );

BOOLEAN
VmDnsMatchRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pTemplate
    );

VOID
VmDnsClearRecord(
    PVMDNS_RECORD   pRecord
    );

VOID
VmDnsRpcClearRecord(
    PVMDNS_RECORD   pRecord
    );

DWORD
VmDnsDuplicateRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD*  ppDest
    );

DWORD
VmDnsRpcDuplicateRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD*  ppDest
    );

DWORD
VmDnsCopyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRpcCopyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    );

DWORD
VmDnsRecordToString(
    PVMDNS_RECORD   pSrc,
    PSTR*           ppStr
    );

DWORD
VmDnsRecordGetCN(
    PVMDNS_RECORD   pSrc,
    PSTR*           ppStr
    );

VOID
VmDnsClearRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    );

DWORD
VmDnsCreateSoaRecord(
    PVMDNS_ZONE_INFO    pZoneInfo,
    PVMDNS_RECORD*      ppRecord
    );

DWORD
VmDnsWriteDomainNameToBuffer(
    PSTR pszDomainName,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsReadDomainNameFromBuffer(
      PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
      PSTR *ppszDomainName
      );

DWORD
VmDnsWriteRecordToBuffer(
    PVMDNS_RECORD pDnsRecord,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsReadRecordFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD *ppDnsRecord
    );

DWORD
VmDnsSerializeDnsRecord(
    PVMDNS_RECORD       pDnsRecord,
    PBYTE*              ppBytes,
    DWORD*              pdwSize
    );

DWORD
VmDnsDeserializeDnsRecord(
    PBYTE               pBytes,
    DWORD               dwBytes,
    PVMDNS_RECORD      *ppDnsRecord
    );

DWORD
VmDnsParseRecordType(
    PSTR            pszRecordType,
    VMDNS_RR_TYPE*  pType
    );

DWORD
VmDnsParseServiceType(
    PSTR                pszServiceType,
    VMDNS_SERVICE_TYPE* pType,
    PSTR*               ppszName
    );

DWORD
VmDnsParseServiceProtocol(
    PSTR                    pszServiceType,
    VMDNS_SERVICE_PROTOCOL* pProtocol,
    PSTR*                   ppszName
    );

#define VMDNS_FREE_RECORD(pRecord) \
    if (pRecord) \
    { \
        VmDnsClearRecord(pRecord); \
        VmDnsFreeMemory(pRecord); \
        pRecord = NULL; \
    }

#define VMDNS_FREE_RECORD_ARRAY(pRecords) \
    if (pRecords) \
    { \
        VmDnsClearRecordArray(pRecords); \
        VmDnsFreeMemory(((PVMDNS_RECORD_ARRAY)pRecords)->Records); \
        VmDnsFreeMemory(pRecords); \
    }

VOID
VmDnsClearZoneInfo(
    PVMDNS_ZONE_INFO    pZoneInfo
    );

VOID
VmDnsClearZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY  pZoneInfoArray
    );

#define VMDNS_FREE_ZONE_INFO(pZoneInfo) \
{ \
    if (pZoneInfo) \
    { \
        VmDnsClearZoneInfo(pZoneInfo); \
        VMDNS_SAFE_FREE_MEMORY(pZoneInfo); \
    } \
}

#define VMDNS_FREE_ZONE_INFO_ARRAY(pZoneInfoArray) \
    if (pZoneInfoArray) \
    { \
        VmDnsClearZoneInfoArray(pZoneInfoArray); \
        VmDnsFreeMemory(pZoneInfoArray->ZoneInfos); \
        VmDnsFreeMemory(pZoneInfoArray); \
    }

#ifdef _WIN32
#define POSIX_TO_WIN32_ERROR(errno) errno
#else
#define POSIX_TO_WIN32_ERROR(errno) LwErrnoToWin32Error(errno)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _VMDNS_COMMON_H__ */
