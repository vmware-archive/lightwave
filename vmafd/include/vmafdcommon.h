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



#ifndef _VMAFD_COMMON_H__
#define _VMAFD_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif


#if defined(_WIN32)
#ifdef LOGGING_EXPORTS
#define LOGGING_API __declspec(dllexport)
#else
#define LOGGING_API __declspec(dllimport)
#endif
#else
#define LOGGING_API
#endif

#include <dce/uuid.h>
#include <dce/dcethread.h>
#include <type_spec.h>
#define VMAFD_MAX_SIZE_BUFFER 2048

#if defined(_WIN32)
typedef unsigned char uuid_t[16];  // typedef dce_uuid_t uuid_t;
#endif

typedef struct _VM_AFD_CONNECTION_ *PVM_AFD_CONNECTION;
typedef struct _VM_AFD_SECURITY_CONTEXT_ *PVM_AFD_SECURITY_CONTEXT;
typedef struct __VMAFD_MESSAGE_BUFFER *PVMAFD_MESSAGE_BUFFER;
typedef struct _VECS_STORE_HANDLE *PVECS_SRV_STORE_HANDLE;

typedef struct _VM_AFD_CONNECTION_CONTEXT_
{
    BOOL                        bAnonymousContext;
    PVM_AFD_CONNECTION          pConnection;
    PVM_AFD_SECURITY_CONTEXT    pSecurityContext;
    PVECS_SRV_STORE_HANDLE      pStoreHandle;
    PSTR                        pszProcessName;
    DWORD                       pid;
} VM_AFD_CONNECTION_CONTEXT;

typedef struct _VM_AFD_CONNECTION_CONTEXT_ *PVM_AFD_CONNECTION_CONTEXT;
//typedef struct _VMAFD_SECURITY_DESCRIPTOR *PVMAFD_SECURITY_DESCRIPTOR;
//typedef struct _VMAFD_SECURITY_DESCRIPTOR_RELATIVE *PVMAFD_SECURITY_DESCRIPTOR_RELATIVE;


typedef struct
{
    PCSTR *pStringList;
    DWORD dwCount; // Current count.
    DWORD dwSize; // Max number of strings we can store currently.
} VMAFD_STRING_LIST, *PVMAFD_STRING_LIST;

// Logging
extern int  vmafd_syslog;
extern int  vmafd_debug;
extern int  vmafd_syslog_level;
extern int  vmafd_console_log;

#define ENTER_LOG() \
    do { \
        VmAfdLog(VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__); \
    }while(0)

#define EXIT_LOG() \
    do { \
        VmAfdLog(VMAFD_DEBUG_DEBUG, "Exiting %s", __FUNCTION__); \
    }while(0)

#ifdef _WIN32
typedef HINSTANCE   VMW_LIB_HANDLE;
#else
#include <dlfcn.h>
typedef VOID*       VMW_LIB_HANDLE;
#endif

VOID
VmAfdFreeTypeSpecContent(
    PVMW_TYPE_SPEC typeSpec,
    DWORD sizeOfArray
    );

DWORD
VmAfdAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

DWORD
VmAfdReallocateMemory(
    PVOID   pMemory,
    PVOID*  ppNewMemory,
    size_t  dwSize
    );

DWORD
VmAfdCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    PCVOID  pSource,
    size_t  maxCount
    );

DWORD
VmAfdReallocateMemoryWithInit(
    PVOID  pMemory,
    PVOID* ppNewMemory,
    size_t dwNewSize,
    size_t dwOldSize
    );

VOID
VmAfdFreeMemory(
    PVOID   pMemory
    );

VOID
VmAfdFreeStringA(
    PSTR    pszString
    );

VOID
VmAfdFreeStringW(
    PWSTR   pwszString
    );

VOID
VmAfdFreeStringArrayA(
    PSTR*   ppszString
    );

VOID
VmAfdFreeStringArrayCountA(
    PSTR*   ppszString,
    DWORD   dwCount
    );

VOID
VmAfdFreeStringArrayW(
    PWSTR* ppwszStrings,
    DWORD  dwCount
    );

VOID
VmAfdFreeStorePermissionArray(
    PVECS_STORE_PERMISSION_W pStorePermissions,
    DWORD dwCount
    );

VOID
VmAfdFreeMutexesArray(
    pthread_mutex_t *pMutexes,
    DWORD dwNumMutexes
    );

VOID
VmAfdFreeDomainControllerInfoA(
    PCDC_DC_INFO_A pDomainControllerInfoA
    );

VOID
VmAfdFreeDomainControllerInfoW(
    PCDC_DC_INFO_W pDomainControllerInfoW
    );


VOID
VmAfdFreeHbInfoA(
    PVMAFD_HB_INFO_A pHbInfoA
    );

VOID
VmAfdFreeHbInfoArrayA(
    PVMAFD_HB_INFO_A pHbInfoArr,
    DWORD dwCount
    );

VOID
VmAfdFreeHbStatusA(
    PVMAFD_HB_STATUS_A pHbStatusA
    );

VOID
VmAfdFreeHbInfoW(
    PVMAFD_HB_INFO_W pHbInfoW
    );

VOID
VmAfdFreeHbInfoArrayW(
    PVMAFD_HB_INFO_W pHbInfoArr,
    DWORD dwCount
    );

VOID
VmAfdFreeHbStatusW(
    PVMAFD_HB_STATUS_W pHbStatusW
    );

VOID
VmAfdFreeCredContextW(
    PVMAFD_CRED_CONTEXT_W pCredContext
    );

VOID
VmAfdFreeCdcStatusInfoA(
    PCDC_DC_STATUS_INFO_A pCdcStatusInfo
    );

VOID
VmAfdFreeCdcStatusInfoW(
    PCDC_DC_STATUS_INFO_W pCdcStatusInfo
    );

DWORD
VmAfdAllocateStringAVsnprintf(
    PSTR*    ppszOut,
    PCSTR    pszFormat,
    ...
    );

ULONG
VmAfdAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmAfdAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

ULONG
VmAfdAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmAfdAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

ULONG
VmAfdAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

ULONG
VmAfdAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    );

ULONG
VmAfdGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

DWORD
VmAfdStringCpyW(
    PWSTR strDestination,
    size_t numberOfElements,
    PCWSTR strSource
    );

DWORD
VmAfdStringNCpyW(
    PWSTR strDestination,
    size_t numberOfElements,
    PCWSTR strSource,
    size_t count
    );

DWORD
VmAfdStringCatW(
    PWSTR strDestination,
    size_t numberOfElements,
    PCWSTR strSource
    );

PWSTR
VmAfdStringChrW(
    PCWSTR str,
    WCHAR wchr
);

int
VmAfdStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

int
VmAfdStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

int
VmAfdStringCompareW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    );

int
VmAfdStringIsEqualW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    );

int
VmAfdStringNCompareW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

SIZE_T
VmAfdStringLenA(
    PCSTR pszStr
    );

PSTR
VmAfdStringChrA(
    PCSTR str,
    int c
    );

PSTR
VmAfdStringRChrA(
    PCSTR str,
    int c
    );

PSTR
VmAfdStringTokA(
    PSTR strToken,
    PCSTR strDelimit,
    PSTR* context
    );

PSTR
VmAfdStringStrA(
    PCSTR str,
    PCSTR strSearch
    );

DWORD
VmAfdStringCpyA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource
    );

DWORD
VmAfdStringNCpyA(
    PSTR strDest,
    size_t numberOfElements,
    PCSTR strSource,
    size_t count
    );

DWORD
VmAfdStringCatA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource
    );

int64_t
VmAfdStringToLA(
    PCSTR nptr,
    PSTR* endptr,
    int base
    );

int
VmAfdStringToIA(
    PCSTR pStr
 );

DWORD
VmAfdStringErrorA(
    PSTR buffer,
    size_t numberOfElements,
    int errnum
    );

PSTR
VmAfdCaselessStrStrA(
    PCSTR pszStr1,
    PCSTR pszStr2
    );

DWORD
VmAfdStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
    );

DWORD
VmAfdStringNPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    size_t maxSize,
    PCSTR pszFormat,
    ...
    );

DWORD
VmAfdLowerCaseStringA(
    PSTR pszString
    );

DWORD
VmAfdUpperCaseStringA(
    PSTR pszString
    );

DWORD
VmAfdLowerCaseStringW(
    PWSTR pwszString
    );

DWORD
VmAfdUpperCaseStringW(
    PWSTR pwszString
    );

LOGGING_API
VOID
VmAfdLog(
    int level,
    const char*      fmt,
    ...
    );

LOGGING_API
DWORD
VmAfdLogInitialize(
    PCSTR logFileName,
    DWORD dwMaximumOldFiles,
    INT64 i64MaxLogSizeBytes
    );

LOGGING_API
VOID
VmAfdLogTerminate(
    VOID
    );

DWORD
VmAfdGetWin32ErrorCode(
    DWORD dwUnixError
    );

DWORD
VmAfdGetErrorString(
    DWORD dwErrorCode,
    PSTR *pszErrMsg
    );

DWORD
VmAfdCommonInit(
    VOID
    );

VOID
VmAfdCommonShutdown(
    VOID
    );

DWORD
VmAfdFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound
    );

DWORD
VmAfdGetFileSize(
    PCSTR   pszFilePath,
    size_t* pFileSize
    );

DWORD
VmAfdOpenFilePath(
    PCSTR   pszFileName,
    PCSTR   pszOpenMode,
    FILE**  fp,
    int mode
    );

DWORD
VmAfdRestrictFilePermissionToSelf(
    PCSTR   pszFileName
    );

DWORD
VmAfdGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostName
    );

DWORD
VmAfdGetHostName(
    PSTR* ppszHostName
    );

BOOLEAN
VmAfdIsLocalHostW(
   PCWSTR pwszHostname
   );

BOOLEAN
VmAfdIsLocalHost(
   PCSTR pszHostname
   );

VOID
VmAfdSleep(
    DWORD dwMilliseconds
    );

DWORD
VmAfdFQDNToDN(
    PCSTR pszFQDN,
    PSTR* ppszDN
    );

DWORD
VmAfdSaveStringToFile(
    PCSTR pcszOutFilePath,
    PCSTR pszContent
    );

DWORD
VmAfdGetTickCount(
    );

DWORD
VmAfdOpenServerConnection(
	PVM_AFD_CONNECTION * ppConnection
	);

VOID
VmAfdCloseServerConnection(
	PVM_AFD_CONNECTION pConnection
	);

VOID
VmAfdFreeServerConnection(
    PVM_AFD_CONNECTION pConnection
    );

DWORD
VmAfdOpenClientConnection(
	PVM_AFD_CONNECTION *ppConnection
	);

VOID
VmAfdCloseClientConnection(
	PVM_AFD_CONNECTION pConnection
	);

VOID
VmAfdFreeClientConnection (
  PVM_AFD_CONNECTION pConnection
  );

DWORD
VmAfdAcceptConnection(
	PVM_AFD_CONNECTION pConnection,
	PVM_AFD_CONNECTION *ppConnection
	);

DWORD
VmAfdReadData(
	PVM_AFD_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	);

DWORD
VmAfdWriteData(
	PVM_AFD_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwRequestSize
	);

DWORD
VmAfdMakeServerRequest(
	PVM_AFD_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwRequestSize,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	);

BOOLEAN
VmAfdIsIPV6AddrFormat(
    PCSTR   pszAddr
    );


DWORD
VmAfdGetMarshalLength(
	PVMW_TYPE_SPEC pInput,
	DWORD noOfArgs,
	PDWORD pdwResponseSize
	);

DWORD
VmAfdMarshal(
	UINT32 apiType,
	UINT32 apiVersion,
	DWORD noOfArgs,
	PVMW_TYPE_SPEC pInput,
	PBYTE pResponse,
	DWORD dwResponseSize
	);

DWORD
VmAfdUnMarshal(
	UINT32 apiType,
	UINT32 apiVersion,
	DWORD noOfArgs,
	PBYTE pResponse,
	DWORD dwResponseSize,
	PVMW_TYPE_SPEC pInput
	);

DWORD
VmAfdMarshalStringArrayGetSize (
                               PWSTR *pwszStringArray,
                               DWORD dwArraySize,
                               PDWORD pdwSizeRequired
                              );


DWORD
VmAfdMarshalStringArray (
                         PWSTR *pwszStringArray,
                         DWORD dwArraySize,
                         DWORD dwBlobSize,
                         PBYTE pMarshalledBlob
                        );

DWORD
VmAfdUnMarshalStringArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PWSTR **ppwszStringArray,
                           PDWORD pdwArraySize
                          );

DWORD
VmAfdMarshalEntryArrayLength (
                             PVMAFD_CERT_ARRAY pCertArray,
                             PDWORD pdwSizeRequired
                             );

DWORD
VmAfdMarshalEntryArray (
                        PVMAFD_CERT_ARRAY pCertArray,
                        DWORD dwBlobSize,
                        PBYTE pMarshalledBlob
                       );

DWORD
VmAfdUnMarshalEntryArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PVMAFD_CERT_ARRAY *ppCertArray
                         );

DWORD
VmAfdMarshalPermissionArrayLength (
                             PVECS_STORE_PERMISSION_W pPermArray,
                             DWORD dwCount,
                             PDWORD pdwSizeRequired
                             );

DWORD
VmAfdMarshalPermissionArray (
                        PVECS_STORE_PERMISSION_W pPermArray,
                        DWORD dwCount,
                        DWORD dwBlobSize,
                        PBYTE pMarshalledBlob
                       );

DWORD
VmAfdUnMarshalPermissionArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PDWORD pdwCount,
                           PVECS_STORE_PERMISSION_W *ppPermArray
                         );


DWORD
VmAfdMarshalHeartbeatStatusArrLength (
                             PVMAFD_HB_INFO_W pInfoArray,
                             DWORD dwCount,
                             PDWORD pdwSizeRequired
                             );

DWORD
VmAfdMarshalHeartbeatStatusArray (
                        PVMAFD_HB_INFO_W pInfoArray,
                        DWORD dwCount,
                        DWORD dwBlobSize,
                        PBYTE pMarshalledBlob
                        );

DWORD
VmAfdUnMarshalHeartbeatStatusArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PDWORD pdwCount,
                           PVMAFD_HB_INFO_W *ppInfoArray
                           );

DWORD
VmAfdMarshalGetDCListArrLength(
              PVMAFD_DC_INFO_W pVmAfdDCInfoList,
              DWORD dwCount,
              PDWORD pdwSizeRequired
              );

DWORD
VmAfdMarshalGetDCList(
              DWORD dwCount,
              PVMAFD_DC_INFO_W pVmAfdDCInfoList,
              DWORD dwBlobSize,
              PBYTE pMarshaledBlob
              );

DWORD
VmAfdUnMarshalGetDCList(
              DWORD dwServerCount,
              DWORD dwBlobSize,
              PBYTE pMarshaledBlob,
              PVMAFD_DC_INFO_W *ppVmAfdDCInfoList
              );

DWORD
VmAfdInitializeConnectionContext(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_CONNECTION_CONTEXT *ppConnectionContext
    );

VOID
VmAfdFreeConnectionContext(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

VOID
VmAfdFreeSecurityContext(
    PVM_AFD_SECURITY_CONTEXT pConnectionContext
    );

DWORD
VmAfdInitializeSecurityContext(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

VOID
VmAfdFreeSecurityContext(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext
    );

DWORD
VmAfdGetSecurityContextSize (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PDWORD pdwSize
    );

DWORD
VmAfdEncodeSecurityContext (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PBYTE pByteSecurityContext,
    DWORD dwBufSize,
    PDWORD pdwBuffUsed
    );

DWORD
VmAfdDecodeSecurityContext (
    PBYTE pByteSecurityContext,
    DWORD dwBufSize,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

BOOL
VmAfdIsRootSecurityContext (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

BOOL
VmAfdEqualsSecurityContext (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext1,
    PVM_AFD_SECURITY_CONTEXT pSecurityContext12
    );

DWORD
VmAfdAllocateContextFromName (
    PCWSTR pszAccountName,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmAfdAllocateNameFromContext (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    );

DWORD
VmAfdCopySecurityContext (
    PVM_AFD_SECURITY_CONTEXT pSecurityContextSrc,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContextDest
    );

DWORD
VmAfdCreateAnonymousConnectionContext (
    PVM_AFD_CONNECTION_CONTEXT *ppConnectionContext
    );

DWORD
VmAfdCreateWellKnownContext (
    VM_AFD_CONTEXT_TYPE contextType,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

BOOL
VmAfdContextBelongsToGroup (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PVM_AFD_SECURITY_CONTEXT pSecurityContextGroup
    );

DWORD
VmAfdCheckAclContext(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PSTR pszSddlAcl,
    BOOL *pbIsAllowed
    );

DWORD
VmAfdGenRandom (
                PDWORD pdwRandomNumber
               );

DWORD
VmAfdGetProcessName(
    DWORD pid,
    PSTR *ppszName
    );

/*DWORD
VmAfdInitializeSecurityDescriptor (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    DWORD dwRevision,
    PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
    );

VOID
VmAfdFreeSecurityDescriptor (
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
    );

DWORD
VmAfdAccessCheck (
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor,
    PVM_AFD_SECURITY_CONTEXT pSecurityContext
    //TODO: A third parameter indicating the access
    //needed
    );

DWORD
VmAfdGetSecurityDescriptorSize (
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor,
    PDWORD pdwSize
    );

DWORD
VmAfdCopySecurityContext (
    PVM_AFD_SECURITY_CONTEXT pSecurityContextSrc,
    PVM_AFD_SECURITY_CONTEXT pSecurityContextDest
    );

DWORD
VmAfdGetSecurityContextSize (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PDWORD pdwSize
    );

DWORD
VmAfdAbsoluteToRelativeSD (
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor,
    PVMAFD_SECURITY_DESCRIPTOR_RELATIVE *ppSecurityDescriptorRelative
    );

DWORD
VmAfdRelativeToAbsoluteSD (
    PVMAFD_SECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptorRelative,
    PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
    );

DWORD
VmAfdGetRelativeSDSize(
    PVMAFD_SECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptorRelative,
    PDWORD pdwSize
    );
*/

#ifdef _WIN32

//cmd line args parsing helpers
BOOLEAN
VmAfdIsCmdLineOption(
    PSTR pArg
    );

VOID
VmAfdGetCmdLineOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    PCSTR* ppszOptionValue
    );

DWORD
VmAfdGetCmdLineIntOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    int* pValue
    );

DWORD
VmAfdAllocateArgsAFromArgsW(
    int argc,
    WCHAR* argv[],
    PSTR** argvA
    );

VOID
VmAfdDeallocateArgsA(
    int argc,
    PSTR argv[]
    );

#endif


DWORD
VmAfdOpenCRL(
    PSTR pszCRLFileName,
    X509_CRL **ppCRL
);

DWORD
VmAfdGetCRLVersion(
    X509_CRL *pCRL,
    DWORD  *pdwVersion
);

DWORD
VmAfdGetCRLName(
    X509_CRL *pCRL,
    PWSTR *ppwszName
);

DWORD
VmAfdConvertASN1Time2String(
    ASN1_TIME *tm,
    PWSTR *ppwszDate
);

DWORD
VmAfdGetCRLLastUpdate(
    X509_CRL *pCRL,
    PWSTR *ppwszDate
);

DWORD
VmAfdGetCRLNextUpdate(
    X509_CRL *pCRL,
    PWSTR *ppwszDate
);

DWORD
VmAfdCloseCRL(
    X509_CRL *pCRL
);

DWORD
VmAfdGetCertSubjectHash(
    X509*   pCert,
    BOOLEAN bOldHash,
    PSTR*   ppszHash
    );

DWORD
VmAfdGetCrlAuthorityHash(
    X509_CRL*   pCrl,
    BOOLEAN bOldHash,
    PSTR*   ppszHash
    );

DWORD
VecsComputeCertFingerPrint(
    X509 *pCert,
    VECS_ENCRYPTION_ALGORITHM encAlgo,
    PSTR* ppszHash
    );

DWORD
VecsComputeCrlFingerPrint(
    X509_CRL *pCrl,
    VECS_ENCRYPTION_ALGORITHM encAlgo,
    PSTR* ppszHash
    );

DWORD
VmAfdFindFileIndex(
    PCSTR pszDirPath,
    PCSTR pszFile,
    BOOLEAN bCrl,
    PLONG pFileIndex
    );

DWORD
VmAfdListFilesInDir(
    PCSTR   pszDirPath,
    DWORD*  pCount,
    PSTR**  pppszFiles
    );

DWORD
VmAfdDeleteFile(
    PCSTR pszFile
);

DWORD
VmAfdCopyFile(
    PCSTR pszSrc,
    PCSTR pszDest
    );

DWORD
VmAfdRenameFile(
    PCSTR pszOldFile,
    PCSTR pszNewFile
);

// certutil.c

VOID
VecsFreeCertArray(
    PVMAFD_CERT_ARRAY pArray
    );

VOID
VecsFreeCrlArray(
    PVMAFD_CRL_FILE_CONTAINER pArray
    );

VOID
VecsFreeCACertArray(
    PVMAFD_CA_CERT_ARRAY pArray
    );

PCSTR
VecsMapEntryType(
    CERT_ENTRY_TYPE entryType
    );

DWORD
VecsPrintCertificate(
    PCSTR pszCertificate
    );

DWORD
VecsPrintCrl(
    PCSTR pszCrl
    );

DWORD
VmAfdPrintCACertificates(
    PVMAFD_CA_CERT_ARRAY pCertArray
    );

DWORD
VmAfdSaveCACertificateAndCrlToFile(
    PVMAFD_CA_CERT pCert,
    PCSTR pszCertFile,
    PCSTR pszCrlFile
    );

//ssl.c

DWORD
VmAfdOpenSSLInit(
    VOID
    );

VOID
VmAfdOpenSSLShutdown(
    VOID
    );

VOID
VmAfdCleanupSSLMutexes(
    VOID
    );


DWORD
VecsPEMToX509Stack(
    PSTR pCertificate,
    STACK_OF(X509) **pskX509certs
    );

DWORD
VecsPEMToX509(
    PSTR pCertificate,
    X509 **ppX509Cert
    );

DWORD
VecsPEMToX509Crl(
    PSTR pCrl,
    X509_CRL **ppX509Crl
    );

DWORD
VecsPEMToRSA (
      PCSTR pszKey,
      RSA **ppKey
      );

DWORD
VecsPEMFiletoX509(
    PCSTR pszPath,
    X509** ppX509Cert
    );

DWORD
VecsPEMFiletoX509Stack(
    PCSTR pszPath,
    STACK_OF(X509) **pskX509certs
    );

DWORD
VecsPEMFiletoX509Crl(
    PCSTR       pszPath,
    X509_CRL**  ppX509Crl
    );

DWORD
VecsHashToAlias(
    unsigned long nSubjectHash,
    PSTR* ppszAlias
    );

DWORD
VecsComputeCertAliasFile(
    PCSTR pszPath,
    PSTR* ppszAlias
    );

DWORD
VecsComputeCertAliasA(
    PSTR pszCertificate,
    PSTR* ppszAlias
    );

DWORD
VecsComputeCrlAliasFromFile(
    PCSTR pszPath,
    PSTR* ppszAlias
    );

DWORD
VecsComputeCrlAliasA(
    PSTR pszCrl,
    PSTR* ppszAlias
    );

DWORD
VecsComputeCrlAliasW(
    PWSTR   pwszCrl,
    PWSTR*  ppwszAlias
    );

DWORD
VecsComputeCertAliasW(
    PWSTR pszCertificate,
    PWSTR *ppszHash
    );

DWORD
VecsComputeCertHash_MD5(
    PSTR  pszCertificate,
    PSTR* ppszHash
    );

DWORD
VecsComputeCertHash_SHA_1(
    PSTR pszCertificate,
    PSTR* ppszHash
    );

DWORD
VecsComputeCrlAuthorityHash_MD5(
    PSTR pszCrl,
    PSTR* ppszHash
    );

DWORD
VecsComputeCrlAuthorityHash_SHA_1(
    PSTR pszCrl,
    PSTR* ppszHash
    );

DWORD
VecsValidateCertificate (
                          PCWSTR pszCertificate
                        );

DWORD
VecsValidateKey (
                      PCWSTR pszKey
                   );

DWORD
VecsCertStackToPEM(
    STACK_OF(X509) *skX509certs,
    PSTR* ppszCertificate
    );

DWORD
VecsCertToPEM(
    X509* pCertificate,
    PSTR* ppszCertificate
);

DWORD
VecsX509CRLToPEM(
    X509_CRL*   pCrl,
    PSTR*       ppszCrl
    );

DWORD
VecsKeyToPEMW(
    RSA* pKey,
    PSTR* ppszKey
);

DWORD
VecsValidateAndFormatCert(
    PCWSTR pszCertificate,
    PWSTR *ppszPEMCertificate
    );

DWORD
VecsValidateAndFormatCrl(
    PCWSTR pwszCrl,
    PWSTR *ppwszPEMCrl
    );

DWORD
VecsValidateAndFormatKey(
    PCWSTR pszKey,
    PCWSTR pszPassword,
    PWSTR *ppszPEMKey
    );

DWORD
VecsDecryptAndFormatKey(
    PCWSTR pszEncrypteKey,
    PCWSTR pszPassword,
    PWSTR *ppszDecryptedPEMKey
    );

DWORD
VecsValidateCertKeyPair(
    PCWSTR pszCertificate,
    PCWSTR pszPrivateKey
    );

VOID
VmAfdReadString(
    PCSTR szPrompt,
    PSTR szString,
    int len,
    BOOLEAN bHideString
    );

//Changes input string
DWORD
VmAfdTrimFQDNTrailingDot(
        PWSTR pwszInputFQDN
        );

// misc.c

BOOLEAN
VmAfdCheckIfIPV6AddressW(
    PCWSTR pwszNetworkAddress
    );

BOOLEAN
VmAfdCheckIfIPV6AddressA(
    PCSTR pszNetworkAddress
    );

BOOLEAN
VmAfdCheckIfIPV4AddressA(
    PCSTR pszNetworkAddress
    );

BOOLEAN
VmAfdCheckIfIPV4AddressW(
    PCWSTR pwszNetworkAddress
    );

BOOLEAN
VmAfdCheckIfServerIsUp(
    PCWSTR pwszNetworkAddress,
    DWORD  dwPort
    );


DWORD
VmAfdAllocateBufferStream(
        size_t dwMaxSize,
        PVMAFD_MESSAGE_BUFFER *ppVmAfdBuffer
        );

VOID
VmAfdFreeBufferStream(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdLockBufferForWrite(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdAllocateBufferStreamWithBuffer(
        PBYTE pBuffer,
        size_t szBufSize,
        size_t szMaxSize,
        BOOL bCanWrite,
        PVMAFD_MESSAGE_BUFFER *ppVmAfdBuffer
        );

DWORD
VmAfdAllocateBufferStreamFromBufferStream(
        PVMAFD_MESSAGE_BUFFER pVmAfdBufferSource,
        BOOL bCanWrite,
        PVMAFD_MESSAGE_BUFFER *ppVmAfdBufferDest
        );

DWORD
VmAfdCopyBufferFromBufferStream(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PBYTE pBuffer,
        PDWORD pdwBufferSize
        );

DWORD
VmAfdWriteBoolToBuffer(
        BOOL bData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteBooleanToBuffer(
        BOOLEAN bData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteCharToBuffer(
        CHAR cData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteUCharToBuffer(
        UCHAR ucData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteUINT16ToBuffer(
        UINT16 uData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteUINT32ToBuffer(
        UINT32 uData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteUINT64ToBuffer(
        UINT64 uData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteINT16ToBuffer(
        INT16 iData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteINT32ToBuffer(
        INT32 iData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteINT64ToBuffer(
        INT64 iData,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteStringToBuffer(
        PSTR pszString,
        UINT8 uStringLength,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdWriteBlobToBuffer(
        PBYTE pBlob,
        DWORD dwSize,
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
        );

DWORD
VmAfdReadBoolFromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PBOOL pbData
        );

DWORD
VmAfdReadBooleanFromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PBOOLEAN pbData
        );

DWORD
VmAfdReadCharFromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PCHAR pcData
        );

DWORD
VmAfdReadUCharFromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PUCHAR pucData
        );

DWORD
VmAfdReadUINT16FromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PUINT16 puData
        );

DWORD
VmAfdReadUINT32FromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PUINT32 puData
        );

DWORD
VmAfdReadUINT64FromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PUINT64 puData
        );

DWORD
VmAfdReadINT16FromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PINT16 piData
        );

DWORD
VmAfdReadINT32FromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PINT32 piData
        );

DWORD
VmAfdReadINT64FromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PINT64 piData
        );

DWORD
VmAfdReadStringFromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PSTR *ppszString,
        PDWORD pdwStringLength,
        PBOOL pbEndOfString
        );

DWORD
VmAfdReadOffsetStringFromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        UINT8 unOffset,
        PSTR *ppszString,
        PDWORD pdwStringLength,
        PBOOL pbEndOfString
        );

DWORD
VmAfdReadBlobFromBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        PBYTE *pBlob,
        PDWORD pdwSize
        );

DWORD
VmAfdIsTokenizedBuffer(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        BOOL bTokenized
        );

DWORD
VmAfdSetBufferTokenizedFlag(
        PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
        BOOL bTokenized
        );


#ifdef __cplusplus
}
#endif

#endif /* _VMAFD_COMMON_H__ */
