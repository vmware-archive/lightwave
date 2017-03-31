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



#ifndef _VMKDC_COMMON_H__
#define _VMKDC_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dce/uuid.h>
#include <dce/dcethread.h>
#if defined(_WIN32)
typedef unsigned char uuid_t[16];  // typedef dce_uuid_t uuid_t;
#endif

// Logging
extern int slap_debug;
extern int  ldap_syslog;
#define vmkdc_ldap_debug slap_debug

DWORD
VmKdcAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

DWORD
VmKdcReallocateMemory(
    PVOID   pMemory,
    PVOID*  ppNewMemory,
    size_t  dwSize
    );

DWORD
VmKdcCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    PCVOID  pSource,
    size_t  maxCount
    );

DWORD
VmKdcReallocateMemoryWithInit(
    PVOID  pMemory,
    PVOID* ppNewMemory,
    size_t dwNewSize,
    size_t dwOldSize
    );

VOID
VmKdcFreeMemory(
    PVOID   pMemory
    );

VOID
VmKdcFreeStringA(
    PSTR    pszString
    );

VOID
VmKdcFreeStringArrayA(
    PSTR*   ppszString
    );

DWORD
VmKdcAllocateStringAVsnprintf(
    PSTR*    ppszOut,
    PCSTR    pszFormat,
    ...
    );

ULONG
VmKdcLengthRequiredSid(
    IN UCHAR SubAuthorityCount
    );

ULONG
VmKdcInitializeSid(
    PSID Sid,
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    UCHAR SubAuthorityCount
    );

ULONG
VmKdcSetSidSubAuthority(
    PSID Sid,
    DWORD nSubAuthority,
    DWORD subAuthorityValue
    );

ULONG
VmKdcAllocateSidFromCString(
    PCSTR pszSidString,
    PSID* Sid
    );

ULONG
VmKdcAllocateCStringFromSid(
    PSTR* ppszStringSid,
    PSID pSid
    );

DWORD
VmKdcSetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Group,
    BOOLEAN IsGroupDefaulted
    );

ULONG
VmKdcLengthSid(
    PSID Sid
    );

DWORD
VmKdcCreateAcl(
    PACL Acl,
    ULONG AclLength,
    ULONG AclRevision
    );

DWORD
VmKdcAddAccessAllowedAceEx(
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

DWORD
VmKdcSetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    BOOLEAN IsDaclPresent,
    PACL Dacl,
    BOOLEAN IsDaclDefaulted
    );

BOOLEAN
VmKdcValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor
    );

DWORD
VmKdcAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE SelfRelativeSecurityDescriptor,
    PULONG BufferLength
    );

ULONG
VmKdcAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmKdcAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

ULONG
VmKdcAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmKdcAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

ULONG
VmKdcAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

ULONG
VmKdcAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    );

ULONG
VmKdcGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

ULONG
VmKdcStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

ULONG
VmKdcStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

SIZE_T
VmKdcStringLenA(
    PCSTR pszStr
);

PSTR
VmKdcStringChrA(
   PCSTR str,
   int c
);

PSTR
VmKdcStringRChrA(
   PCSTR str,
   int c
);

PSTR
VmKdcStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
);

PSTR
VmKdcStringStrA(
   PCSTR str,
   PCSTR strSearch
);

DWORD
VmKdcStringCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

DWORD
VmKdcStringNCpyA(
   PSTR strDest,
   size_t numberOfElements,
   PCSTR strSource,
   size_t count
);

DWORD
VmKdcStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

int64_t
VmKdcStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
);

int VmKdcStringToIA(
   PCSTR pStr
);

DWORD
VmKdcStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
);

PSTR
VmKdcCaselessStrStrA(
    PCSTR pszStr1,
    PCSTR pszStr2
    );

DWORD
VmKdcStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
);

DWORD
VmKdcStringNPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    size_t maxSize,
    PCSTR pszFormat,
    ...
);

DWORD
VmKdcQuerySecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformationNeeded,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorInput,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorOutput,
    PULONG Length
    );

DWORD
VmKdcSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_RELATIVE SelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR_ABSOLUTE AbsoluteSecurityDescriptor,
    PULONG AbsoluteSecurityDescriptorSize,
    PACL pDacl,
    PULONG pDaclSize,
    PACL pSacl,
    PULONG pSaclSize,
    PSID Owner,
    PULONG pOwnerSize,
    PSID PrimaryGroup,
    PULONG pPrimaryGroupSize
    );

BOOLEAN
VmKdcAccessCheck(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PACCESS_TOKEN AccessToken,
    ACCESS_MASK DesiredAccess,
    ACCESS_MASK PreviouslyGrantedAccess,
    PGENERIC_MAPPING GenericMapping,
    PACCESS_MASK GrantedAccess,
    PDWORD pAccessError
    );

DWORD
VmKdcGetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Owner,
    PBOOLEAN pIsOwnerDefaulted
    );

DWORD
VmKdcGetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Group,
    PBOOLEAN pIsGroupDefaulted
    );

DWORD
VmKdcGetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsDaclPresent,
    PACL* Dacl,
    PBOOLEAN pIsDaclDefaulted
    );

DWORD
VmKdcGetSaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsSaclPresent,
    PACL* Sacl,
    PBOOLEAN pIsSaclDefaulted
    );

BOOLEAN
VmKdcValidRelativeSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    ULONG SecurityDescriptorLength,
    SECURITY_INFORMATION RequiredInformation
    );

DWORD
VmKdcSetSecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE InputSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE ObjectSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE NewObjectSecurityDescriptor,
    PULONG NewObjectSecurityDescripptorLength,
    PGENERIC_MAPPING GenericMapping
    );

DWORD
VmKdcCreateSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    ULONG Revision
    );

VOID
VmKdcReleaseAccessToken(
    PACCESS_TOKEN* AccessToken
    );

DWORD
VmKdcSetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Owner,
    BOOLEAN IsOwnerDefaulted
    );

DWORD
VmKdcCreateWellKnownSid(
    WELL_KNOWN_SID_TYPE wellKnownSidType,
    PSID pDomainSid,
    PSID pSid,
    DWORD* pcbSid
);

VOID
VmKdcMapGenericMask(
    PDWORD pdwAccessMask,
    PGENERIC_MAPPING pGenericMapping
);

DWORD
VmKdcQueryAccessTokenInformation(
    HANDLE hTokenHandle,
    TOKEN_INFORMATION_CLASS tokenInformationClass,
    PVOID pTokenInformation,
    DWORD dwTokenInformationLength,
    PDWORD pdwReturnLength
);

DWORD
VmKdcAllocateSddlCStringFromSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    DWORD dwRequestedStringSDRevision,
    SECURITY_INFORMATION securityInformation,
    PSTR* ppStringSecurityDescriptor
);

#ifdef _WIN32

//cmd line args parsing helpers
BOOLEAN
VmKdcIsCmdLineOption(
    PSTR pArg
);

VOID
VmKdcGetCmdLineOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    PCSTR* ppszOptionValue
);

DWORD
VmKdcGetCmdLineIntOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    int* pValue
);

DWORD
VmKdcAllocateArgsAFromArgsW(
    int argc,
    WCHAR* argv[],
    PSTR** argvA
);

VOID
VmKdcDeallocateArgsA(
    int argc,
    PSTR argv[]
);

#endif

/* mutexes/threads/conditions */
typedef struct _VMKDC_MUTEX* PVMKDC_MUTEX;

typedef struct _VMKDC_COND* PVMKDC_COND;

#ifndef _WIN32
typedef pthread_t VMKDC_THREAD;
#else
typedef HANDLE VMKDC_THREAD;
#endif

typedef VMKDC_THREAD* PVMKDC_THREAD;

typedef PVOID (VmKdcStartRoutine)(PVOID);
typedef VmKdcStartRoutine* PVMKDC_START_ROUTINE;

DWORD
VmKdcAllocateMutex(
    PVMKDC_MUTEX* ppMutex
);

VOID
VmKdcFreeMutex(
    PVMKDC_MUTEX pMutex
);

DWORD
VmKdcLockMutex(
    PVMKDC_MUTEX pMutex
);

DWORD
VmKdcUnLockMutex(
    PVMKDC_MUTEX pMutex
);

BOOLEAN
VmKdcIsMutexInitialized(
    PVMKDC_MUTEX pMutex
);

DWORD
VmKdcAllocateCondition(
    PVMKDC_COND* ppCondition
);

VOID
VmKdcFreeCondition(
    PVMKDC_COND pCondition
);

DWORD
VmKdcConditionWait(
    PVMKDC_COND pCondition,
    PVMKDC_MUTEX pMutex
);

DWORD
VmKdcConditionTimedWait(
    PVMKDC_COND pCondition,
    PVMKDC_MUTEX pMutex,
    DWORD dwMilliseconds
);

DWORD
VmKdcConditionSignal(
    PVMKDC_COND pCondition
);

DWORD
VmKdcCreateThread(
    PVMKDC_THREAD pThread,
    BOOLEAN bDetached,
    PVMKDC_START_ROUTINE pStartRoutine,
    PVOID pArgs
);

DWORD
VmKdcThreadJoin(
    PVMKDC_THREAD pThread,
    PDWORD pRetVal
);

VOID
VmKdcFreeVmKdcThread(
    PVMKDC_THREAD pThread
);

#ifdef __cplusplus
}
#endif

#endif /* _VMKDC_COMMON_H__ */
