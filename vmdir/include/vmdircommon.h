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



#ifndef _VMDIR_COMMON_H__
#define _VMDIR_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#ifndef __RPCDCE_H__
typedef unsigned char uuid_t[16];  // typedef dce_uuid_t uuid_t;
#endif
#define LW_STRICT_NAMESPACE
#include <lw/types.h>
#include <lw/hash.h>

#ifndef LOGGING_API
#ifdef LOGGING_EXPORTS
#define LOGGING_API __declspec(dllexport)
#else
#define LOGGING_API __declspec(dllimport)
#endif
#endif
#else
#ifndef LOGGING_API
#define LOGGING_API
#endif
#endif

#include <dce/uuid.h>
#include <dce/dcethread.h>

#include <ldap.h>
#include <openssl/ssl.h>
#include <type_spec.h>

#define VMDIR_SIZE_1024         1024
#define VMDIR_SIZE_2048         2048
#define VMDIR_SIZE_4096         4096
#define VMDIR_SIZE_8192         8192
#define VMDIR_SIZE_9216         9216

#define MAX_PATH 260
#define MAX_INSTALL_PARAMETER_LEN 260

#define VMDIR_GUID_STR_LEN             (32 + 4 /* -s */ + 1 /* \0 */) // "%08x-%04x-%04x-%04x-%04x%08x"
#define VMDIR_SSL_DISABLED_PROTOCOL_LEN 64
#define VMDIR_SSL_CIPHER_SUITE_LEN     256
#define VMDIR_MAX_DN_LEN               1024 // including \0
#define VMDIR_MAX_PASSWORD_LEN         128 /* As specified in schema for userPassword and replBindPassword attributes */
#define VMDIR_MAX_I64_ASCII_STR_LEN     (19 + 1 /* null byte terminator */) /* Max value for i64_t is 9,223,372,036,854,775,807 */

#define VMDIR_MAX_FILE_NAME_LEN        1024

#define VMDIR_LOG_MAX_OLD_FILES (10)

#define VMDIR_LOG_MAX_SIZE_BYTES (1024*1024*10)

#define VMKDC_RANDPWD_MAX_RETRY 128 /* Prevents RpcVmDirCreateUser() from looping forever */

// Versions and DFLs
#define VMDIR_DFL_UNKNOWN "UNKNOWN"
#define VMDIR_DFL_5_5 "5.5"
#define VMDIR_DFL_6_0 "6.0"
#define VMDIR_DFL_6_5 "6.5"
#define VMDIR_DFL_6_6 "6.6"
#define VMDIR_DFL_7_0 "7.0"
#define VMDIR_DFL_DEFAULT 1
#define VMDIR_DFL_MODDN 3

// Special SELF sid for internal use (not assigned to object as attribute)
#define VMDIR_SELF_SID "S-1-7-32-666"

/* mutexes/threads/conditions */
typedef struct _VMDIR_MUTEX* PVMDIR_MUTEX;
typedef struct _VMDIR_RWLOCK* PVMDIR_RWLOCK;
typedef struct _VM_DIR_CONNECTION_ *PVM_DIR_CONNECTION;
typedef struct _VM_DIR_SECURITY_CONTEXT_ *PVM_DIR_SECURITY_CONTEXT;

typedef struct _VMDIR_IPC_DATA_CONTAINER
{
    UINT32 dwCount;
    unsigned char * data;
} VMDIR_IPC_DATA_CONTAINER, *PVMDIR_IPC_DATA_CONTAINER;

typedef struct _DEQUE_NODE
{
    PVOID               pElement;
    struct _DEQUE_NODE* pPrev;
    struct _DEQUE_NODE* pNext;
} DEQUE_NODE, *PDEQUE_NODE;

typedef struct _DEQUE
{
    PDEQUE_NODE pHead;
    PDEQUE_NODE pTail;
    size_t      iSize;
} DEQUE, * PDEQUE;

typedef struct _VDIR_LINKED_LIST_NODE
{
    PVOID                           pElement;
    struct _VDIR_LINKED_LIST_NODE*  pPrev;
    struct _VDIR_LINKED_LIST_NODE*  pNext;
    struct _VDIR_LINKED_LIST*       pList;

} VDIR_LINKED_LIST_NODE, *PVDIR_LINKED_LIST_NODE;

typedef struct _VDIR_LINKED_LIST
{
    PVDIR_LINKED_LIST_NODE  pHead;
    PVDIR_LINKED_LIST_NODE  pTail;
    size_t                  iSize;

} VDIR_LINKED_LIST, *PVDIR_LINKED_LIST;

typedef struct _VMDIR_TSSTACK
{
    PVMDIR_MUTEX pMutex;
    size_t       iSize;
    size_t       iCapacity;
    PVOID*       ppBuffer;
} VMDIR_TSSTACK, *PVMDIR_TSSTACK;

typedef struct _VMDIR_KEYTAB_HANDLE
{
    int ktType; // 1=file, 2=memory,...
    FILE *ktfp;
    int ktOffset; // offset from start of container where kt data begins
    int ktMode; //1 = "r", 2 = "rw", 3="a"
} VMDIR_KEYTAB_HANDLE, *PVMDIR_KEYTAB_HANDLE;

struct _VMDIR_KRBKEY;

typedef struct _VMDIR_KEYTAB_ENTRY
{
    int entrySize;
    int princType;
    char *realm;
    char **nameComponents;
    int nameComponentsLen;
    int timeStamp;
    int kvno;
    int keyType;
    struct _VMDIR_KRBKEY *key;
} VMDIR_KEYTAB_ENTRY, *PVMDIR_KEYTAB_ENTRY;

typedef enum _VMDIR_FIRST_REPL_CYCLE_MODE
{
    FIRST_REPL_CYCLE_MODE_COPY_DB = 1,
    FIRST_REPL_CYCLE_MODE_USE_COPIED_DB,
    FIRST_REPL_CYCLE_MODE_OBJECT_BY_OBJECT,
} VMDIR_FIRST_REPL_CYCLE_MODE;

typedef struct _VMDIR_CIRCULAR_BUFFER
{
    //
    // Maximum number of entries.
    //
    DWORD dwCapacity;

    //
    // The spot where we will write the next entry.
    //
    DWORD dwHead;

    //
    // Current number of entries.
    //
    DWORD dwSize;

    //
    // Size of individual elements in the buffer.
    //
    DWORD dwElementSize;

    //
    // Actual objects.
    //
    PBYTE CircularBuffer;

    //
    // Lock for making our operations thread-safe.
    //
    PVMDIR_MUTEX mutex;
} VMDIR_CIRCULAR_BUFFER, *PVMDIR_CIRCULAR_BUFFER;

typedef const VMDIR_CIRCULAR_BUFFER* PCVMDIR_CIRCULAR_BUFFER;
typedef BOOLEAN (*CIRCULAR_BUFFER_SELECT_CALLBACK)(PVOID pElement, PVOID pContext);

typedef struct
{
    PCSTR *pStringList;
    DWORD dwCount; // Current count.
    DWORD dwSize; // Max number of strings we can store currently.
} VMDIR_STRING_LIST, *PVMDIR_STRING_LIST;

#ifdef _WIN32
typedef HINSTANCE   VMDIR_LIB_HANDLE;
#else
#include <dlfcn.h>
typedef VOID*       VMDIR_LIB_HANDLE;
#endif

ULONG
VmDirRpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VmDirRpcFreeMemory(
    void *p
    );

VOID
VmDirRpcClientFreeMemory(
    void *p
    );

DWORD
VmDirAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

DWORD
VmDirReallocateMemory(
    PVOID   pMemory,
    PVOID*  ppNewMemory,
    size_t  dwSize
    );

DWORD
VmDirCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    const void* pSource,
    size_t  maxCount
    );

DWORD
VmDirAllocateAndCopyMemory(
    PVOID   pBlob,
    size_t  iBlobSize,
    PVOID*  ppOutBlob
    );

DWORD
VmDirReallocateMemoryWithInit(
    PVOID  pMemory,
    PVOID* ppNewMemory,
    size_t dwNewSize,
    size_t dwOldSize
    );

VOID
VmDirFreeMemory(
    PVOID   pMemory
    );

VOID
VmDirFreeStringA(
    PSTR    pszString
    );

VOID
VmDirFreeStringArrayA(
    PSTR*   ppszString
    );

VOID
VmDirFreeStringArrayW(
    PWSTR* ppwszStrings,
    DWORD  dwCount
    );

DWORD
VmDirVsnprintf(
    PSTR*    ppszOut,
    PCSTR    pszFormat,
    va_list  args
    );

ULONG
VmDirLengthRequiredSid(
    IN UCHAR SubAuthorityCount
    );

ULONG
VmDirInitializeSid(
    PSID Sid,
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    UCHAR SubAuthorityCount
    );

ULONG
VmDirSetSidSubAuthority(
    PSID Sid,
    DWORD nSubAuthority,
    DWORD subAuthorityValue
    );

ULONG
VmDirAllocateSidFromCString(
    PCSTR pszSidString,
    PSID* Sid
    );

ULONG
VmDirAllocateCStringFromSid(
    PSTR* ppszStringSid,
    PSID pSid
    );

VOID
VmDirFreeTypeSpecContent(
    PVMW_TYPE_SPEC specInput,
    DWORD sizeOfArray
    );

ULONG
VmDirAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmDirAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

DWORD
VmDirAllocateStringOfLenA(
    PCSTR   pszSource,
    SIZE_T  sLength,
    PSTR*   ppszDestination
    );

ULONG
VmDirAllocateMultiStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

ULONG
VmDirAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmDirAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

ULONG
VmDirAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

ULONG
VmDirAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    );

ULONG
VmDirGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

LONG
VmDirStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

LONG
VmDirStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

BOOLEAN
VmDirStringEndsWith(
    PCSTR   pszStr,
    PCSTR   pszSuffix,
    BOOLEAN bIsCaseSensitive
    );

SIZE_T
VmDirStringLenA(
    PCSTR pszStr
);

PSTR
VmDirStringChrA(
   PCSTR str,
   int c
);

PSTR
VmDirStringRChrA(
   PCSTR str,
   int c
);

PSTR
VmDirStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
);

PSTR
VmDirStringStrA(
   PCSTR str,
   PCSTR strSearch
);

PSTR
VmDirStringCaseStrA(
   PCSTR    pszSource,
   PCSTR    pszPattern
);

DWORD
VmDirStringCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

DWORD
VmDirStringNCpyA(
   PSTR strDest,
   size_t numberOfElements,
   PCSTR strSource,
   size_t count
);

DWORD
VmDirStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

DWORD
VmDirStringNCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource,
   size_t number
);

int64_t
VmDirStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
);

int VmDirStringToIA(
   PCSTR pStr
);

DWORD
VmDirStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
);

PSTR
VmDirCaselessStrStrA(
    PCSTR pszStr1,
    PCSTR pszStr2
    );

DWORD
VmDirStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
);

DWORD
VmDirStringNPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    size_t maxSize,
    PCSTR pszFormat,
    ...
);

VOID
VmdDirNormalizeString(
    PSTR    pszString
    );

#ifdef _WIN32

DWORD
VmDirGetProgramDataEnvVar(
    _TCHAR *pEnvName,   // [in]
    _TCHAR **ppEnvValue // [out]
    );

#endif

LOGGING_API
void
VmDirLog(
   ULONG        level,
   const char*  fmt,
   ...);

LOGGING_API
void
VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    ULONG           iMask,
    const char*     fmt,
    ...);

LOGGING_API
DWORD
VmDirLogInitialize(
   PCSTR            pszLogFileName,
   BOOLEAN          bUseSysLog,
   PCSTR            pszSyslogName,
   VMDIR_LOG_LEVEL  iInitLogLevel,
   ULONG            iInitLogMask
   );

LOGGING_API
DWORD
VmDirLogInternalInitialize(
   PCSTR            pszLogFileName,
   BOOLEAN          bUseSysLog,
   PCSTR            pszSyslogName,
   VMDIR_LOG_LEVEL  iLogLevel,
   ULONG            iInitLogMask,
   DWORD            dwMaximumOldLogs,
   INT64            i64MaximumLogSizeBytes
   );


LOGGING_API
void
VmDirLogSetLevel(
    VMDIR_LOG_LEVEL iNewLogLevel
    );

LOGGING_API
VMDIR_LOG_LEVEL
VmDirLogGetLevel(
    VOID
    );

LOGGING_API
void
VmDirLogTerminate(
    VOID
    );

LOGGING_API
void
VmDirLogSetMask(
    ULONG       iNewLogMask
    );

LOGGING_API
ULONG
VmDirLogGetMask(
    VOID
    );

PCSTR
VmDirSearchDomainDN(
    PCSTR pszNormObjectDN
    );

DWORD
VmDirDomainDNToName(
    PCSTR pszDomainDN,
    PSTR* ppszDomainName);

DWORD
VmDirSrvCreateDomainDN(
    PCSTR pszFQDomainName,
    PSTR* ppszDomainDN
    );

DWORD
VmDirConnectLDAPServerWithMachineAccount(
    PCSTR  pszHostName,
    PCSTR  pszDomain,
    LDAP** ppLd
    );

DWORD
VmDirGetDomainFuncLvlInternal(
    LDAP*  pLd,
    PCSTR  pszDomain,
    PDWORD pdwFuncLvl
    );

#if defined(HAVE_DCERPC_WIN32)
int uuid_parse(char *str, uuid_t ret_uuid);
#endif

DWORD
VmDirUuidGenerate(
    uuid_t* pGuid
);

DWORD
VmDirUuidToStringLower(
    uuid_t* pGuid,
    PSTR pStr,
    DWORD sizeOfBuffer
);

DWORD
VmKdcGenerateMasterKey(
    PBYTE *ppMasterKey,
    PDWORD pMasterKeyLen,
    PBYTE *ppEncMasterKey,
    PDWORD pEncMasterKeyLen);

DWORD
VmKdcStringToKeys(
    PSTR upnName,
    PSTR password,
    PBYTE *ppMasterKey,
    PDWORD pMasterKeyLen);


DWORD
VmKdcStringToKeysEncrypt(
    PSTR upnName,
    PSTR password,
    PBYTE pKey,
    DWORD keyLen,
    DWORD kvno,
    PBYTE *ppUpnKeys,
    PDWORD pUpnKeysLen);

DWORD
VmDirMigrateKrbUPNKey(
    PBYTE   pOldUpnKeys,
    DWORD   oldUpnKeysLen,
    PBYTE   pOldMasterKey,
    DWORD   oldMasterKeyLen,
    PBYTE   pNewMasterKey,
    DWORD   newMasterKeyLen,
    PBYTE*  ppNewUpnKeys,
    PDWORD  pNewUpnKeysLen
    );

DWORD
VmKdcGenerateRandomPassword(
    DWORD pwLen,
    PSTR *ppRandPwd);

// cmd line args parsing helpers
typedef enum
{
    CL_NO_PARAMETER,
    CL_STRING_PARAMETER,
    CL_INTEGER_PARAMETER
} VMDIR_COMMAND_LINE_PARAMETER_TYPE;

typedef struct
{
    char                                Switch; // e.g., 's', for "-s".
    const char*                         LongSwitch; // e.g., "silent", for "--silent".
    VMDIR_COMMAND_LINE_PARAMETER_TYPE   Type; // If this flag takes a parameter (and, if so, what kind).
    PVOID                               Ptr; // Ptr to store parameter value
} VMDIR_COMMAND_LINE_OPTION, *PVMDIR_COMMAND_LINE_OPTION;

typedef VOID (*USAGE_FUNCTION)(PVOID pContext);
typedef DWORD (*POST_VALIDATION_CALLBACK)(PVOID pContext);

typedef struct
{
    //
    // This is called after all parameters have been parsed and allows for the client
    // to do cross-parameter validation.
    //
    POST_VALIDATION_CALLBACK    ValidationRoutine;

    //
    // We call this if the app should print its usage to the command line (i.e., the
    // user gave incorrect parameters to the command).
    //
    USAGE_FUNCTION              ShowUsage;

    //
    // Argument context for callback functions
    //
    PVOID                       pvContext;
} VMDIR_PARSE_ARG_CALLBACKS, *PVMDIR_PARSE_ARG_CALLBACKS;

DWORD
VmDirParseArguments(
    VMDIR_COMMAND_LINE_OPTION Options[],
    PVMDIR_PARSE_ARG_CALLBACKS Callbacks,
    int argc,
    PSTR *argv
    );

BOOLEAN
VmDirIsCmdLineOption(
    PSTR pArg
);

VOID
VmDirGetCmdLineOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    PCSTR* ppszOptionValue
);

DWORD
VmDirGetCmdLineIntOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    int* pValue
);

DWORD
VmDirGetCmdLineInt64Option(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    int64_t* pValue
);

#ifdef _WIN32

DWORD
VmDirAllocateArgsAFromArgsW(
    int argc,
    WCHAR* argv[],
    PSTR** argvA
);

VOID
VmDirDeallocateArgsA(
    int argc,
    PSTR argv[]
);

DWORD
VmDirGetEnvironmentVariable(
    PCSTR pszVariableName,
    PSTR  pszBuffer,
    DWORD dwBufLen
);

DWORD
VmDirMDBGetHomeDir(
    _TCHAR *lpHomeDirBuffer
    );
#endif


#if !defined(_WIN32) || defined(HAVE_PTHREADS_WIN32)
typedef struct _VMDIR_COND* PVMDIR_COND;
#else
#ifdef WIN2008
typedef struct _VMDIR_COND_2008  VMDIR_COND;
typedef VMDIR_COND *PVMDIR_COND_2008;
typedef VMDIR_COND *PVMDIR_COND;
#else
typedef struct _VMDIR_COND_2003 VMDIR_COND;
typedef VMDIR_COND *PVMDIR_COND_2003;
typedef VMDIR_COND *PVMDIR_COND;
#endif
#endif

#if !defined(_WIN32) || defined(HAVE_PTHREADS_WIN32)
#include <pthread.h>
#define PTHREAD_SELF() ((size_t) pthread_self().p)
typedef pthread_t VMDIR_THREAD;
#else
typedef HANDLE VMDIR_THREAD;
#define PTHREAD_SELF() ((size_t) GetCurrentThreadId())
#endif

typedef VMDIR_THREAD* PVMDIR_THREAD;

typedef DWORD (VmDirStartRoutine)(PVOID);
typedef VmDirStartRoutine* PVMDIR_START_ROUTINE;

typedef struct _VMDIR_SYNCHRONIZE_COUNTER* PVMDIR_SYNCHRONIZE_COUNTER;


typedef enum
{
    SYNC_SIGNAL,
    SYNC_BROADCAST

} VMDIR_SYNC_MECHANISM;

#define VMDIR_NAME                          "vmdir"
#define VMAFD_NAME                          "vmafd"

#ifndef _WIN32
#ifndef VMDIR_CONFIG_SASL2_LIB_PATH
#define VMDIR_CONFIG_SASL2_LIB_PATH        "/opt/likewise/lib64/sasl2"
#endif
#else
#define VDMIR_CONFIG_SASL2_KEY_PATH         "SOFTWARE\\Carnegie Mellon\\Project Cyrus\\SASL Library"
#endif

#ifndef _WIN32
#define VMDIR_CONFIG_PARAMETER_KEY_PATH     "Services\\Vmdir"
#define VMDIR_CONFIG_PARAMETER_V1_KEY_PATH  "Services\\Vmdir\\Parameters"
#define VMDIR_LINUX_DB_PATH                 VMDIR_DB_DIR "/"
#else
#define VMDIR_CONFIG_PARAMETER_KEY_PATH     "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#define VMDIR_CONFIG_PARAMETER_V1_KEY_PATH  "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService\\Parameters"
#define VMDIR_CONFIG_SOFTWARE_KEY_PATH      "SOFTWARE\\VMware, Inc.\\VMware Directory Services"
#define WIN_SYSTEM32_PATH                   "c:\\windows\\system32"
#endif

#ifndef _WIN32
#define VMAFD_CONFIG_KEY_ROOT               "Services\\Vmafd"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH     "Services\\Vmafd\\Parameters"
#define VMAFD_REG_KEY_PATH                  "Path"
#else
#define VMAFD_CONFIG_PARAMETER_KEY_PATH     "SYSTEM\\CurrentControlSet\\Services\\VMwareAfdService\\Parameters"
#define VMAFD_CONFIG_SOFTWARE_KEY_PATH      "SOFTWARE\\VMware, Inc.\\VMware Afd Services"
#endif

#define VMDIR_REG_KEY_SITE_GUID             "SiteGuid"
#define VMDIR_REG_KEY_LDU_GUID              "LduGuid"
#define VMDIR_REG_KEY_KEYTAB_FILE           "KeytabPath"

#define VMAFD_REG_KEY_DOMAIN_NAME           "DomainName"
#define VMAFD_REG_KEY_DC_NAME               "DCName"
#define VMDIR_REG_KEY_DC_ACCOUNT            "dcAccount"
#define VMDIR_REG_KEY_DC_ACCOUNT_DN         "dcAccountDN"
#define VMDIR_REG_KEY_DC_ACCOUNT_PWD        "dcAccountPassword"
#define VMDIR_REG_KEY_DC_ACCOUNT_OLD_PWD    "dcAccountOldPassword"
#define VMDIR_REG_KEY_MACHINE_GUID          "MachineGuid"
#define VMDIR_REG_KEY_CONFIG_PATH           "ConfigPath"
#define VMDIR_REG_KEY_DATA_PATH             "DataPath"
#define VMDIR_REG_KEY_LOG_PATH              "LogsPath"
#define VMDIR_REG_KEY_INSTALL_PATH          "InstallPath"
#define VMDIR_REG_KEY_MAXIMUM_OLD_LOGS      "MaximumOldLogs"
#define VMDIR_REG_KEY_MAXIMUM_LOG_SIZE      "MaximumLogSize"
#define VMDIR_REG_KEY_MAXIMUM_DB_SIZE_MB    "MaximumDbSizeMb"
#define VMDIR_REG_KEY_DISABLE_VECS_INTEGRATION    "DisableVECSIntegration"
#define VMDIR_REG_KEY_SSL_DISABLED_PROTOCOLS "SslDisabledProtocols"
#define VMDIR_REG_KEY_SSL_CIPHER_SUITE       "SslCipherSuite"
#define VMDIR_REG_KEY_DIRTY_SHUTDOWN         "DirtyShutdown"
#define VMDIR_REG_KEY_ENABLE_RENAME          "EnableRename"
#define VMAFD_REG_KEY_KRB5_CONF              "Krb5Conf"
#define VMDIR_REG_KEY_LDAP_PORT               "LdapPort"
#define VMDIR_REG_KEY_ALLOW_INSECURE_AUTH     "AllowInsecureAuthentication"
#define VMDIR_REG_KEY_ADMIN_PASSWD            "AdministratorPassword"
#define VMDIR_REG_KEY_LDAP_LISTEN_PORTS       "LdapListenPorts"
#define VMDIR_REG_KEY_LDAPS_LISTEN_PORTS      "LdapsListenPorts"
#define VMDIR_REG_KEY_LDAP_CONNECT_PORTS      "LdapConnectPorts"
#define VMDIR_REG_KEY_LDAPS_CONNECT_PORTS     "LdapsConnectPorts"
#define VMDIR_REG_KEY_LDAP_RECV_TIMEOUT_SEC   "LdapRecvTimeoutSec"
#define VMDIR_REG_KEY_ALLOW_ADMIN_LOCKOUT     "AllowAdminLockout"
#define VMDIR_REG_KEY_MAX_OP_THREADS          "MaxLdapOpThrs"
#define VMDIR_REG_KEY_DISABLE_VECS            "DisableVECSIntegration"
#define VMDIR_REG_KEY_MAX_INDEX_SCAN          "MaxIndexScan"
#define VMDIR_REG_KEY_SMALL_CANDIDATE_SET     "SmallCandidateSet"
#define VMDIR_REG_KEY_MAX_SIZELIMIT_SCAN      "MaxSizeLimitScan"
#define VMDIR_REG_KEY_ALLOW_IMPORT_OP_ATTR    "AllowImportOperationalAttrs"
#define VMDIR_REG_KEY_LDAP_SEARCH_TIMEOUT_SEC "LdapSearchTimeoutSec"
#define VMDIR_REG_KEY_TRACK_LAST_LOGIN_TIME   "TrackLastLoginTime"
#define VMDIR_REG_KEY_SUPPRES_TRACK_LLT       "SuppressTrackLLTContainer"
#define VMDIR_REG_KEY_URGENT_REPL_TIMEOUT_MSEC "UrgentReplTimeoutMilliSec"
#define VMDIR_REG_KEY_PAGED_SEARCH_READ_AHEAD "PagedSearchReadAhead"
#define VMDIR_REG_KEY_COPY_DB_WRITES_MIN      "CopyDbWritesMin"
#define VMDIR_REG_KEY_COPY_DB_INTERVAL_IN_SEC "CopyDbIntervalInSec"
#define VMDIR_REG_KEY_COPY_DB_BLOCK_WRITE_IN_SEC "CopyDbBlockWriteInSec"
#define VMDIR_REG_KEY_OVERRIDE_PASS_SCHEME    "OverridePassScheme"
//
// The expiration period for deleted entries. Any entries older than this will
// be permanently expunged once the reaping thread runs. The default is 45 days.
//
#define VMDIR_REG_KEY_TOMBSTONE_EXPIRATION_IN_SEC "TombstoneExpirationPeriodInSec"
//
// How often the reaping thread wakes up to look for old entries. The default is
// 24 hours.
//
#define VMDIR_REG_KEY_TOMBSTONE_REAPING_FREQ_IN_SEC  "TombstoneReapingThreadFreqInSec"

#ifdef _WIN32
#define VMDIR_DEFAULT_KRB5_CONF             "C:\\ProgramData\\MIT\\Kerberos5\\krb5.ini"
#else
#define VMDIR_DEFAULT_KRB5_CONF             "/etc/krb5.lotus.conf"
#endif

DWORD
VmDirAllocateMutex(
    PVMDIR_MUTEX* ppMutex
);

VOID
VmDirFreeMutex(
    PVMDIR_MUTEX pMutex
);

DWORD
VmDirLockMutex(
    PVMDIR_MUTEX pMutex
);

DWORD
VmDirUnLockMutex(
    PVMDIR_MUTEX pMutex
);

BOOLEAN
VmDirIsMutexInitialized(
    PVMDIR_MUTEX pMutex
);

DWORD
VmDirAllocateRWLock(
    PVMDIR_RWLOCK*  ppLock
    );

DWORD
VmDirInitializeRWLockContent(
    PVMDIR_RWLOCK   pLock
    );

VOID
VmDirFreeRWLock(
    PVMDIR_RWLOCK   pLock
    );

VOID
VmDirFreeRWLockContent(
    PVMDIR_RWLOCK   pLock
    );

DWORD
VmDirRWLockReadLock(
    PVMDIR_RWLOCK   pLock,
    DWORD           dwMilliSec
    );

DWORD
VmDirRWLockWriteLock(
    PVMDIR_RWLOCK   pLock,
    DWORD           dwMilliSec
    );

DWORD
VmDirRWLockUnlock(
    PVMDIR_RWLOCK   pLock
    );

BOOLEAN
VmDirIsRWLockInitialized(
    PVMDIR_RWLOCK   pLock
    );

DWORD
VmDirAllocateCondition(
    PVMDIR_COND* ppCondition
);

VOID
VmDirFreeCondition(
    PVMDIR_COND pCondition
);

DWORD
VmDirConditionWait(
    PVMDIR_COND pCondition,
    PVMDIR_MUTEX pMutex
);

DWORD
VmDirConditionTimedWait(
    PVMDIR_COND pCondition,
    PVMDIR_MUTEX pMutex,
    DWORD dwMilliseconds
);

DWORD
VmDirConditionSignal(
    PVMDIR_COND pCondition
);

DWORD
VmDirConditionBroadcast(
    PVMDIR_COND pCondition
);

#if defined(_WIN32) && !defined(HAVE_PTHREADS_WIN32)
#ifdef WIN2008
DWORD
VmDirAllocateCondition2008(
    PVMDIR_COND_2008* ppCondition
);

VOID
VmDirFreeCondition2008(
    PVMDIR_COND_2008 pCondition
);


DWORD
VmDirConditionTimedWait2008(
    PVMDIR_COND_2008 pCondition,
    PVMDIR_MUTEX pMutex,
    DWORD dwMilliseconds
);

DWORD
VmDirConditionWait2008(
    PVMDIR_COND_2008 pCondition,
    PVMDIR_MUTEX pMutex
);


DWORD
VmDirConditionSignal2008(
    PVMDIR_COND_2008 pCondition
);

DWORD
VmDirConditionBroadcast2008(
    PVMDIR_COND_2008 pCondition
);

#define VmDirAllocateCondition                  VmDirAllocateCondition2008
#define VmDirFreeCondition                      VmDirFreeCondition2008
#define VmDirConditionWait                      VmDirConditionWait2008
#define VmDirConditionTimedWait                 VmDirConditionTimedWait2008
#define VmDirConditionSignal                    VmDirConditionSignal2008
#define VmDirConditionBroadcast                 VmDirConditionBroadcast2008


#else

DWORD
VmDirAllocateCondition2003(
    PVMDIR_COND_2003* ppCondition
);

VOID
VmDirFreeCondition2003(
    PVMDIR_COND_2003 pCondition
);


DWORD
VmDirConditionTimedWait2003(
    PVMDIR_COND_2003 pCondition,
    PVMDIR_MUTEX pMutex,
    DWORD dwMilliseconds
);

DWORD
VmDirConditionWait2003(
    PVMDIR_COND_2003 pCondition,
    PVMDIR_MUTEX pMutex
);


DWORD
VmDirConditionSignal2003(
    PVMDIR_COND_2003 pCondition
);

DWORD
VmDirConditionBroadcast2003(
    PVMDIR_COND_2003 pCondition
);

#define VmDirAllocateCondition                  VmDirAllocateCondition2003
#define VmDirFreeCondition                      VmDirFreeCondition2003
#define VmDirConditionWait                      VmDirConditionWait2003
#define VmDirConditionTimedWait                 VmDirConditionTimedWait2003
#define VmDirConditionSignal                    VmDirConditionSignal2003
#define VmDirConditionBroadcast                 VmDirConditionBroadcast2003


#endif

#endif /* !defined(HAVE_PTHREADS_WIN32) */

DWORD
VmDirCreateThread(
    PVMDIR_THREAD pThread,
    BOOLEAN bDetached,
    PVMDIR_START_ROUTINE pStartRoutine,
    PVOID pArgs
);

DWORD
VmDirThreadJoin(
    PVMDIR_THREAD pThread,
    PDWORD pRetVal
);

VOID
VmDirFreeVmDirThread(
    PVMDIR_THREAD pThread
);

DWORD
VmDirAllocateSyncCounter(
    PVMDIR_SYNCHRONIZE_COUNTER*     ppSyncCounter,
    size_t                          iSyncValue,
    VMDIR_SYNC_MECHANISM            wakeupMethod,
    DWORD                           iCondWaitTimeInMillionSec
    );

VOID
VmDirFreeSyncCounter(
    PVMDIR_SYNCHRONIZE_COUNTER      pSyncCounter
    );

DWORD
VmDirSyncCounterWaitEvent(
    PVMDIR_SYNCHRONIZE_COUNTER      pSyncCounter,
    PBOOLEAN                        pbWaitTimeOut
    );

DWORD
VmDirSyncCounterChange(
    PVMDIR_SYNCHRONIZE_COUNTER      pSyncCounter,
    int                             iValue
    );

DWORD
VmDirSyncCounterIncrement(
    PVMDIR_SYNCHRONIZE_COUNTER      pSyncCounter
    );

DWORD
VmDirSyncCounterDecrement(
    PVMDIR_SYNCHRONIZE_COUNTER      pSyncCounter
    );

ULONG
VmDirGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostName
    );

BOOLEAN
VmDirIsIPAddrFormat(
    PCSTR   pszAddr
    );

BOOLEAN
VmDirIsIPV6AddrFormat(
    PCSTR   pszAddr
    );

int
VmDirCompareStrByLen(
    const void * pszStr1, // PSTR *
    const void * pszStr2  // PSTR *
    );

ULONG
VmDirGetAddrInfo(
  PCSTR pszHostname,
  struct addrinfo** ppResult
  );

ULONG
VMDirGetNameInfo(
    const struct sockaddr*     pSockaddr,
    socklen_t           sockaddrLength,
    PCHAR               pHostName,
    DWORD               dwBufferSize
    );

ULONG
VmDirGetHostName(
    PSTR pszHostName,
    DWORD dwBufLen
);

DWORD
VmDirGetNetworkInfoFromSocket(
    ber_socket_t fd,
    PSTR pszAddress,
    DWORD dwAddressLen,
    PDWORD pdwPort,
    BOOLEAN bPeerInfo
    );

DWORD
VmDirAllocateStringFromSocket(
    int fd,
    BOOLEAN bLocalIsServer,
    PSTR *ppszInfo
    );

DWORD
VmDirGenerateGUID(
    PSTR*   ppszGuid
    );

VOID
VmDirSleep(
    DWORD dwMilliseconds
);

DWORD
dequeCreate(
    PDEQUE* ppDeque
    );

VOID
dequeFree(
    PDEQUE pDeque
    );

PDEQUE_NODE
dequeHeadNode(
    PDEQUE pDeque
    );

PVOID
dequeHead(
    PDEQUE pDeque
    );

PVOID
dequeTail(
    PDEQUE pDeque
    );

DWORD
dequePush(
    PDEQUE pDeque,
    PVOID pElement
    );

DWORD
dequePop(
    PDEQUE pDeque,
    PVOID* ppElement
    );

DWORD
dequePopLeft(
    PDEQUE pDeque,
    PVOID* ppElement
    );

size_t
dequeGetSize(
    PDEQUE pDeque
    );

BOOLEAN
dequeIsEmpty(
    PDEQUE pDeque
    );

DWORD
VmDirLinkedListCreate(
    PVDIR_LINKED_LIST*  ppLinkedList
    );

DWORD
VmDirLinkedListGetHead(
    PVDIR_LINKED_LIST       pLinkedList,
    PVDIR_LINKED_LIST_NODE* ppHead
    );

DWORD
VmDirLinkedListGetTail(
    PVDIR_LINKED_LIST       pLinkedList,
    PVDIR_LINKED_LIST_NODE* ppTail
    );

DWORD
VmDirLinkedListInsertHead(
    PVDIR_LINKED_LIST       pLinkedList,
    PVOID                   pElement,
    PVDIR_LINKED_LIST_NODE* ppHead
    );

DWORD
VmDirLinkedListInsertTail(
    PVDIR_LINKED_LIST       pLinkedList,
    PVOID                   pElement,
    PVDIR_LINKED_LIST_NODE* ppTail
    );

DWORD
VmDirLinkedListRemove(
    PVDIR_LINKED_LIST       pLinkedList,
    PVDIR_LINKED_LIST_NODE  pNode
    );

size_t
VmDirLinkedListGetSize(
    PVDIR_LINKED_LIST   pLinkedList
    );

BOOLEAN
VmDirLinkedListIsEmpty(
    PVDIR_LINKED_LIST   pLinkedList
    );

VOID
VmDirFreeLinkedList(
    PVDIR_LINKED_LIST   pLinkedList
    );

DWORD
VmDirRun(
    PCSTR pszCmd
    );

DWORD
VmDirGetLotusServerName(
    PCSTR   pszServerName,
    PSTR*   ppOutServerName
    );

DWORD
VmDirWriteDCAccountPassword(
    PCSTR pszPassword,
    DWORD dwLength /* Length of the string, not including null */
    );

DWORD
VmDirWriteDCAccountOldPassword(
    PCSTR pszPassword,
    DWORD dwLength /* Length of the string, not including null */
    );

DWORD
VmDirReadDCAccountPassword(
    PSTR* ppszPassword
    );

DWORD
VmDirReadDCAccountOldPassword(
    PSTR* ppszPassword
    );

DWORD
VmDirRegReadDCAccount(
    PSTR* ppszDCAccount
    );

DWORD
VmDirRegReadDCAccountDn(
    PSTR* ppszDCAccount
    );

DWORD
VmDirRegReadKrb5Conf(
    PSTR* ppszKrb5Conf
    );

DWORD
VmDirReplURIToHostname(
    PSTR    pszRepURI,
    PSTR*   ppszPartnerHostName
    );

DWORD
VmDirGetRegKeyValue(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PSTR    pszValue,
    size_t  valueLen
    );

DWORD
VmDirGetRegKeyValueDword(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PDWORD  pdwValue,
    DWORD   dwDefaultValue
    );

DWORD
VmDirSetRegKeyValueDword(
    PCSTR pszConfigParamKeyPath,
    PCSTR pszKey,
    DWORD dwValue
    );

DWORD
VmDirSetRegKeyValueString(
    PCSTR pszConfigParamKeyPath,
    PCSTR pszKey,
    PCSTR pszValue,
    DWORD dwLength /* Should not include +1 for terminating null */
    );

DWORD
VmDirGetRegKeyValueQword(
    PCSTR   pszConfigParamKeyPath,
    PCSTR   pszKey,
    PINT64  pi64Value
    );

DWORD
VmDirLoadLibrary(
    PCSTR           pszLibPath,
    VMDIR_LIB_HANDLE* ppLibHandle
    );

VOID
VmDirCloseLibrary(
    VMDIR_LIB_HANDLE  pLibHandle
    );

#ifdef _WIN32
FARPROC WINAPI
#else
VOID*
#endif
VmDirGetLibSym(
    VMDIR_LIB_HANDLE  pLibHandle,
    PCSTR           pszFunctionName
    );

DWORD
VmDirKeyTabOpen(
    PSTR ktName,
    PSTR ktOpenMode, // "r", "rw"
    PVMDIR_KEYTAB_HANDLE *ppKeyTab);

DWORD
VmDirKeyTabWriteKeys(
    PVMDIR_KEYTAB_HANDLE pKeyTab,
    PCSTR pszUpnName,
    PBYTE pUpnKeys,
    DWORD upnKeysLen,
    PBYTE pMasterKey,
    DWORD masterKeyLen);

VOID
VmDirKeyTabClose(
    PVMDIR_KEYTAB_HANDLE pKeyTab);

DWORD
VmDirKeyTabRead(
    PVMDIR_KEYTAB_HANDLE pKt,
    PVMDIR_KEYTAB_ENTRY *ppRetData);

DWORD
VmDirKeyTabWrite(
    PVMDIR_KEYTAB_HANDLE pKt,
    PVMDIR_KEYTAB_ENTRY pKtEntry);

DWORD
VmDirKeyTabRewind(
    PVMDIR_KEYTAB_HANDLE pKeyTab);

DWORD
VmDirKeyTabWriteKeysBlob(
    PVMDIR_KEYTAB_HANDLE pKeyTab,
    PCSTR pszUpnName,
    PBYTE pUpnKeys,
    DWORD upnKeysLen,
    PBYTE pMasterKey,
    DWORD masterKeyLen,
    PBYTE *ppBlob,
    PDWORD pdwBlobLen);

DWORD
VmDirKeyTabMakeRecord(
    PVMDIR_KEYTAB_HANDLE pKt,
    PVMDIR_KEYTAB_ENTRY pKtEntry,
    PBYTE *ppKtRecord,
    PDWORD pdwRecordLen);


DWORD
VmDirDestroyDefaultKRB5CC(
    VOID
    );

DWORD
VmDirAllocASCIILowerToUpper(
    PCSTR   pszInputStr,
    PSTR*   ppszOutputStr);

DWORD
VmDirAllocASCIIUpperToLower(
    PCSTR   pszInputStr,
    PSTR*   ppszOutputStr);

DWORD
VmDirMigrateUserKey(
    PBYTE pOldUpnKeys,
    DWORD oldUpnKeysLen,
    PBYTE pOldMasterKey,
    DWORD oldMasterKeyLen,
    PBYTE pNewMasterKey,
    DWORD newMasterKeyLen,
    PBYTE* ppNewUpnKeys,
    PDWORD pNewUpnKeysLen);

DWORD
VmDirFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound
    );

#ifdef _WIN32

DWORD
VmDirOpensslSetServerCertPath(_TCHAR *lpPath);

DWORD
VmDirOpensslSetServerKeyPath(_TCHAR *lpPath);

#endif

DWORD
VmDirCertificateFileNameFromHostName(
    PCSTR   pszPartnerHostName,
    PSTR *  ppszFileName);

DWORD
VmDirCertficateFileNameFromLdapURI(
    PSTR    pszRepURI,
    PSTR *  ppszCertFileName);

DWORD
VmDirReadStringFromFile(
    PCSTR   pszFile,
    PSTR    szString,
    int     len
    );

VOID
VmDirReadString(
    PCSTR szPrompt,
    PSTR szString,
    int len,
    BOOLEAN bHideString
    );

DWORD
VmDirPrepareOpensslClientCtx(
    SSL_CTX**   ppSslCtx,
    PSTR*       ppszTrustCertFile,
    PCSTR       pszLdapURI
    );

//client/ldapbind.c
DWORD
VmDirSASLGSSAPIBind(
     LDAP**     ppLd,
     PCSTR      pszURI
     );

DWORD
VmDirSASLSRPBind(
     LDAP**     ppLd,
     PCSTR      pszURI,
     PCSTR      pszUPN,
     PCSTR      pszPass
     );

DWORD
VmDirSSLBind(
     LDAP**     ppLd,
     PCSTR      pszURI,
     PCSTR      pszAccountDN,
     PCSTR      pszPass
     );

DWORD
VmDirSafeLDAPBind(
    LDAP**      ppLd,
    PCSTR       pszHost,
    PCSTR       pszUPN,         // opt, if exists, will try SRP mech
    PCSTR       pszPassword     // opt, if exists, will try SRP mech
    );

DWORD
VmDirAnonymousLDAPBind(
    LDAP**      ppLd,
    PCSTR       pszLdapURI
    );

int
VmDirCreateSyncRequestControl(
    PCSTR pszInvocationId,
    USN lastLocalUsnProcessed,
    PCSTR pszUtdVector,
    LDAPControl *syncReqCtrl
    );

VOID
VmDirDeleteSyncRequestControl(
    LDAPControl *syncReqCtrl
    );

DWORD
VmDirMapLdapError(
    int ldapErrorCode
    );

// common/tsstack.c
VOID
VmDirFreeTSStack(
    PVMDIR_TSSTACK pStack
    );

DWORD
VmDirAllocateTSStack(
    size_t            iCapacity,
    PVMDIR_TSSTACK*   ppStack
    );

DWORD
VmDirPushTSStack(
    PVMDIR_TSSTACK   pStack,
    PVOID            pElement    // pStack take over ownership
    );

DWORD
VmDirPopTSStack(
    PVMDIR_TSSTACK   pStack,
    PVOID*           ppElement
    );

size_t
VmDirGetTSStackSize(
    PVMDIR_TSSTACK   pStack
    );

BOOLEAN
VmDirIsEmptyTSStack(
    PVMDIR_TSSTACK   pStack
    );

BOOLEAN
VmDirHaveLegacy(
    VOID
    );

// util.c
int
VmDirCompareVersion(
    PSTR    pszVerA,
    PSTR    pszVerB
    );

uint64_t
VmDirGetTimeInMilliSec(
    VOID
    );

DWORD
VmDirAllocateUserCreateParamsWFromA(
    PVMDIR_USER_CREATE_PARAMS_A  pCreateParamsA,
    PVMDIR_USER_CREATE_PARAMS_W* ppCreateParamsW
    );

DWORD
VmDirAllocateUserCreateParamsAFromW(
    PVMDIR_USER_CREATE_PARAMS_W  pCreateParamsW,
    PVMDIR_USER_CREATE_PARAMS_A* ppCreateParamsA
    );

VOID
VmDirFreeUserCreateParamsA(
    PVMDIR_USER_CREATE_PARAMS_A pCreateParams
    );

VOID
VmDirFreeUserCreateParamsW(
    PVMDIR_USER_CREATE_PARAMS_W pCreateParams
    );

DWORD
VmDirLdapURI2Host(
    PCSTR   pszURI,
    PSTR*   ppszHost
    );

DWORD
VmDirUPNToNameAndDomain(
    PCSTR   pszUPN,
    PSTR*   ppszName,
    PSTR*   ppszDomain
    );

//IPC
//networkutil.c
DWORD
VmDirOpenServerConnection(
	PVM_DIR_CONNECTION * ppConnection
	);

VOID
VmDirCloseServerConnection(
	PVM_DIR_CONNECTION pConnection
	);

VOID
VmDirShutdownServerConnection(
	PVM_DIR_CONNECTION pConnection
	);


VOID
VmDirFreeServerConnection(
  PVM_DIR_CONNECTION pConnection
  );

DWORD
VmDirOpenClientConnection(
	PVM_DIR_CONNECTION *ppConnection
	);

VOID
VmDirCloseClientConnection(
	PVM_DIR_CONNECTION pConnection
	);

VOID
VmDirFreeClientConnection (
  PVM_DIR_CONNECTION pConnection
  );

DWORD
VmDirAcceptConnection(
	PVM_DIR_CONNECTION pConnection,
	PVM_DIR_CONNECTION *ppConnection
	);

DWORD
VmDirReadData(
	PVM_DIR_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	);

DWORD
VmDirWriteData(
	PVM_DIR_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwRequestSize
	);

DWORD
VmDirMakeServerRequest(
	PVM_DIR_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwRequestSize,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	);

//marshalutil.c

DWORD
VmDirGetMarshalLength(
	PVMW_TYPE_SPEC pInput,
	DWORD noOfArgs,
	PDWORD pdwResponseSize
	);

DWORD
VmDirMarshal(
	UINT32 apiType,
	UINT32 apiVersion,
	DWORD noOfArgs,
	PVMW_TYPE_SPEC pInput,
	PBYTE pResponse,
	DWORD dwResponseSize
	);

DWORD
VmDirUnMarshal(
	UINT32 apiType,
	UINT32 apiVersion,
	DWORD noOfArgs,
	PBYTE pResponse,
	DWORD dwResponseSize,
	PVMW_TYPE_SPEC pInput
	);

DWORD
VmDirMarshalStringArrayGetSize (
                               PWSTR *pwszStringArray,
                               DWORD dwArraySize,
                               PDWORD pdwSizeRequired
                              );


DWORD
VmDirMarshalStringArray (
                         PWSTR *pwszStringArray,
                         DWORD dwArraySize,
                         DWORD dwBlobSize,
                         PBYTE pMarshalledBlob
                        );

DWORD
VmDirUnMarshalStringArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PWSTR **ppwszStringArray,
                           PDWORD pdwArraySize
                          );

DWORD
VmDirMarshalContainerLength(
    PVMDIR_IPC_DATA_CONTAINER pContainer,
    PDWORD pdwSizeRequired
    );

DWORD
VmDirMarshalContainer(
    PVMDIR_IPC_DATA_CONTAINER pContainer,
    DWORD dwBlobSize,
    PBYTE pMarshalledBlob
    );

DWORD
VmDirUnMarshalContainer(
    DWORD dwBlobSize,
    PBYTE pMarshalledBlob,
    PVMDIR_IPC_DATA_CONTAINER *ppContainer
    );

VOID
VmDirFreeIpcContainer(
    PVMDIR_IPC_DATA_CONTAINER pContainer
    );

//securityutil.c
DWORD
VmDirInitializeSecurityContext(
  PVM_DIR_CONNECTION pConnection,
  PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
  );

VOID
VmDirFreeSecurityContext(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext
    );

DWORD
VmDirGetSecurityContextSize (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PDWORD pdwSize
    );

DWORD
VmDirEncodeSecurityContext (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PBYTE pByteSecurityContext,
    DWORD dwBufSize,
    PDWORD pdwBuffUsed
    );

DWORD
VmDirDecodeSecurityContext (
    PBYTE pByteSecurityContext,
    DWORD dwBufSize,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

BOOL
VmDirIsRootSecurityContext (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext
    );

BOOL
VmDirEqualsSecurityContext (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext1,
    PVM_DIR_SECURITY_CONTEXT pSecurityContext12
    );

DWORD
VmDirAllocateContextFromName (
    PCWSTR pszAccountName,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmDirAllocateNameFromContext (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    );

DWORD
VmDirCopySecurityContext (
    PVM_DIR_SECURITY_CONTEXT pSecurityContextSrc,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContextDest
    );

DWORD
VmDirCreateRootSecurityContext (
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmDirCreateWellKnownContext (
    VM_DIR_CONTEXT_TYPE contextType,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    );

BOOL
VmDirContextBelongsToGroup (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PVM_DIR_SECURITY_CONTEXT pSecurityContextGroup
    );

DWORD
VmDirAllocateNameFromContext (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    );

DWORD
VmDirGetMaxDbSizeMb(
    PDWORD pMaxDbSizeMb
    );

// why PSTR? pass in a buffer but no length?
DWORD
VmDirGetLocalLduGuid(
    PSTR pszLduGuid
    );

// why PSTR? pass in a buffer but no length?
DWORD
VmDirGetLocalSiteGuid(
    PSTR pszSiteGuid
    );

// following functions are in libvmdirclient but should not be published in vmdirclient.h
DWORD
VmDirGetUsnFromPartners(
    PCSTR pszHostName,
    USN   *pUsn
    );

VOID
VmDirRpcFreeSuperLogEntryLdapOperationArray(
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pRpcEntries
    );

DWORD
VmDirCircularBufferCreate(
    DWORD dwCapacity,
    DWORD dwElementSize,
    PVMDIR_CIRCULAR_BUFFER *ppCircularBuffer
    );

VOID
VmDirCircularBufferFree(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmDirCircularBufferReset(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmDirCircularBufferGetSize(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pSize
    );

DWORD
VmDirCircularBufferGetCapacity(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pdwCapacity
    );

DWORD
VmDirCircularBufferSetCapacity(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCapacity
    );

PVOID
VmDirCircularBufferGetNextEntry(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    );

DWORD
VmDirCircularBufferSelectElements(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCount,
    CIRCULAR_BUFFER_SELECT_CALLBACK Callback,
    PVOID pContext
    );

DWORD
VmDirStringListInitialize(
    PVMDIR_STRING_LIST *ppStringList,
    DWORD dwInitialCount
    );

VOID
VmDirStringListFreeContent(
    PVMDIR_STRING_LIST pStringList
    );

VOID
VmDirStringListFree(
    PVMDIR_STRING_LIST pStringList
    );

DWORD
VmDirStringListAdd(
    PVMDIR_STRING_LIST pStringList,
    PCSTR pszString
    );

DWORD
VmDirStringListRemove(
    PVMDIR_STRING_LIST pStringList,
    PCSTR pszString
    );

BOOLEAN
VmDirStringListContains(
    PVMDIR_STRING_LIST pStringList,
    PCSTR pszString
    );

DWORD
VmDirStringListAddStrClone(
    PCSTR               pszStr,
    PVMDIR_STRING_LIST  pStrList
    );

int
VmDirQsortCaseExactCompareString(
    const void* ppStr1,
    const void* ppStr2
    );

int
VmDirQsortCaseIgnoreCompareString(
    const void* ppStr1,
    const void* ppStr2
    );

DWORD
VmDirCopyStrArray(
    PSTR*   ppszOrgArray,
    PSTR**  pppszCopyArray
    );

DWORD
VmDirMergeStrArray(
    PSTR*   ppszSetCur,
    PSTR*   ppszSetNew,
    PSTR**  pppszOutSet
    );

DWORD
VmDirGetStrArrayDiffs(
    PSTR*   ppszArray1,
    PSTR*   ppszArray2,
    PSTR**  pppszNotArray1,
    PSTR**  pppszNotArray2
    );

BOOLEAN
VmDirIsStrArraySuperSet(
    PSTR*   ppszSuper,
    PSTR*   ppszSub
    );

BOOLEAN
VmDirIsStrArrayIdentical(
    PSTR*   ppszSet1,
    PSTR*   ppszSet2
    );

VOID
VmDirFreeStrArray(
    PSTR*   ppszArray
    );

VOID
VmDirNoopHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

VOID
VmDirSimpleHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

#ifdef _WIN32

DWORD
VmDirGetCfgPath(
    PSTR*   ppszCfgPath
    );

#endif

DWORD
VmDirGetSingleAttributeFromEntry(
    LDAP*        pLd,
    LDAPMessage* pEntry,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
    );

DWORD
VmDirGetDCDNList(
    LDAP* pLd,
    PCSTR pszDomainDN,
    PVMDIR_STRING_LIST*  ppDCList
    );

DWORD
VmDirDnLastRDNToCn(
    PCSTR   pszDN,
    PSTR*   ppszCN
    );

DWORD
VmDirStringToTokenList(
    PCSTR pszStr,
    PCSTR pszDelimiter,
    PVMDIR_STRING_LIST *ppStrList
    );

DWORD
VmDirUTDVectorToStruct(
    PCSTR   pszStr,
    PVMDIR_REPL_UTDVECTOR*  ppVector
    );

DWORD
VmDirStrToNameAndNumber(
    PCSTR   pszStr,
    CHAR    del,
    PSTR*   ppszName,
    USN*    pUSN
    );

VOID
VmDirFreeReplVector(
    PVMDIR_REPL_UTDVECTOR  pVector
    );

DWORD
VmDirStringListFromMultiString(
    PCSTR pszString,
    DWORD dwCountHint, // 0 if caller doesn't know
    PVMDIR_STRING_LIST *ppStrList
    );

DWORD
VmDirMultiStringFromStringList(
    PVMDIR_STRING_LIST pStrList,
    PSTR *ppszString,
    PDWORD pdwByteCount
    );

DWORD
VmDirMapVersionToMaxDFL(
    PCSTR	pszVersion,
    PDWORD	pdwDFL
    );

VOID
VmDirLdapUnbind(
    LDAP** ppLd
    );

DWORD
VmDirSetDomainFuncLvlInternal(
    LDAP* pLd,
    PCSTR pszDomainName,
    DWORD dwFuncLvl
    );

DWORD
VmDirGetServerAccountDN(
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszServerDN
    );

DWORD
VmDirLdapGetSingleAttribute(
    LDAP*   pLD,
    PCSTR   pszDN,
    PCSTR   pszAttr,
    PBYTE*  ppByte,
    DWORD*  pdwLen
    );

DWORD
VmDirLdapGetAttributeValues(
    LDAP* pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    LDAPControl** ppSrvCtrls,
    BerValue*** pppBerValues
    );

DWORD
VmDirGetServerName(
    PCSTR pszHostName,
    PSTR* ppszServerName
    );

#ifdef __cplusplus
}
#endif

#include <vmdircommon_schema.h>

#endif /* _VMDIR_COMMON_H__ */
