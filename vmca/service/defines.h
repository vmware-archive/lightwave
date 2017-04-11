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
 * Filename: defines.h
 *
 * Abstract:
 *
 * Definitions/Macros
 *
 */


#define VMCA_OPTION_LOGGING_LEVEL 'l'
#define VMCA_OPTION_LOG_FILE_NAME 'L'
#define VMCA_OPTION_ENABLE_SYSLOG 's'
#define VMCA_OPTION_CONSOLE_LOGGING 'c'
#define VMCA_OPTIONS_VALID "f:l:L:p:sc"

//
// These values are hard-coded in the VMCA.reg file also,
// in case you change them, please change them in VMCA.reg
// file also.
//
#define VMCA_SERVER_VERSION_STRING "VMware Certificate Server  - Version  2.0.0 \nCopyright VMware Inc. All rights reserved."
#ifndef _WIN32
#define VMCA_PATH_SEPARATOR_CHAR '/'
// #define VMCA_PATH_SEPARATOR "/"
// #define VMCA_KEY_PARAMETERS "\\Services\\vmca\\Parameters"
// #define VMCA_ROOT_CERT "RootCert"
// #define VMCA_ROOT_PRIVATE_KEY "RootPrivateKey"
// #define VMCA_ROOT_PRIVATE_KEY_PASS_PHRASE "RootPrivateKeyPassPhrase"
// #define VMCA_ROOT_CERT_DIR "/var/lib/vmware/vmca"

#else
#define VMCA_PATH_SEPARATOR_CHAR '\\'
// #define VMCA_PATH_SEPARATOR "\\"
// #define VMCA_KEY_PARAMETERS "\\Services\\vmca\\Parameters"
// #define VMCA_ROOT_CERT "RootCert"
// #define VMCA_ROOT_PRIVATE_KEY "RootPrivateKey"
// #define VMCA_ROOT_PRIVATE_KEY_PASS_PHRASE "RootPrivateKeyPassPhrase"
// #define VMCA_ROOT_CERT_DIR "C:\\ProgramData\\VMware\\cis\\data\\vmcad"

#define _USE_32BIT_TIME_T // See MSDN http://msdn.microsoft.com/en-us/library/14h5k7ff(v=VS.80).aspx

#define BOOL BOOLEAN

#define VMCA_IF_HANDLE_T RPC_IF_HANDLE
#define VMCA_RPC_BINDING_VECTOR_P_T RPC_BINDING_VECTOR*
#define VMCA_RPC_AUTHZ_HANDLE RPC_AUTHZ_HANDLE
#define VMCA_RPC_BINDING_HANDLE RPC_BINDING_HANDLE
#define VMCA_RPC_C_AUTHN_LEVEL_PKT RPC_C_AUTHN_LEVEL_PKT

// should this service name match VMCA_NCALRPC_END_POINT ??
//#define VMCA_NT_SERVICE_NAME L"VMWareCertificateService"
#define VMCA_NT_SERVICE_NAME L"vmcasvc"


#define VMCA_CLOSE_HANDLE(handle) \
    {                              \
        if ((handle) != NULL)      \
        {                          \
            CloseHandle((handle)); \
            (handle) = NULL;       \
        }                          \
    }

#define VMCA_CLOSE_SERVICE_HANDLE(hServiceHandle) \
    {                                              \
         if ( (hServiceHandle) != NULL )           \
         {                                         \
             CloseServiceHandle((hServiceHandle)); \
             (hServiceHandle) = NULL;              \
         }                                         \
    }

#endif


#ifndef _WIN32
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "Services\\vmafd\\Parameters"
#else
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareAfdService\\Parameters"
#endif

#if 0 /* TBD: Adam */
#define VMDIR_REG_KEY_DC_ACCOUNT_DN "dcAccountDN"
#endif
#define VMDIR_REG_KEY_DC_PASSWORD   "dcAccountPassword"
#define VMDIR_REG_KEY_DC_ACCOUNT    "dcAccount"
#define VMAFD_REG_KEY_DOMAIN_NAME   "DomainName"

// States

typedef DWORD VMCA_FUNC_LEVEL;

#define VMCA_UNKNOWN                0x00000001
#define VMCA_ROOT_INIT_FAILED       0x00000002
#define VMCA_ROOT_INIT_DONE         0x00000004

#define VMCA_FUNC_LEVEL_INITIAL     0x00000000
#define VMCA_FUNC_LEVEL_SELF_CA     0x00000004

#define    GetLastError() errno

#ifndef  __RPC_USER
#define  __RPC_USER
#endif

#ifndef _WIN32
#define VMCA_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMCA_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#define VMCA_ASSERT(x) assert( (x) )

#define VMCA_MAX(x, y) ((x) > (y) ? (x) : (y))

#define _PTHREAD_FUNCTION_RTN_ASSERT(Function, ...)       \
    do {                                                  \
        int error = Function(__VA_ARGS__);                \
        VMCA_ASSERT(!error);                              \
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


#define VMCA_RPC_SAFE_FREE_MEMORY(mem) \
    if ((mem) != NULL) \
    { \
        VMCARpcFreeMemory(mem); \
    }


#define VMCA_SAFE_FREE_HZN_PSTR(PTR)    \
    do {                          \
        if ((PTR)) {              \
            HZNFree(PTR);         \
            (PTR) = NULL;         \
        }                         \
    } while(0)

#define BAIL_ON_VMCA_INVALID_POINTER(p, errCode)  \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMCA_ERROR(errCode);          \
        }

#define BAIL_ON_VMREST_ERROR(dwError)           \
    if (dwError)                                \
    {                                           \
        goto error;                             \
    }

#define BAIL_ON_JSON_PARSE_ERROR(dwError)       \
    if (dwError)                                \
    {                                           \
        goto error;                             \
    }

#define PARSER_CHECK_NULL(input, dwError)       \
    if (input == NULL)                          \
    {                                           \
        dwError = 415;                          \
    }

#define CHECK_BAD_MALLOC(input, dwError)        \
    if (input == NULL)                          \
    {                                           \
        dwError = ERROR_OUTOFMEMORY;            \
    }

#define HANDLE_NULL_PARAM(input, dwerror)       \
    if (input == NULL)                          \
    {                                           \
        dwerror = ERROR_INVALID_PARAMETER;      \
    }

// REST ENGINE CONFIG VALUES
// TRIDENT
#define VMCARESTSSLCERT "/root/mycert.pem"
#define VMCARESTSSLKEY "/root/mycert.pem"
#define VMCARESTPORT "81"
#define VMCARESTDEBUGLOGFILE "/tmp/restServer.log"
#define VMCARESTCLIENTCNT "5"
#define VMCARESTWORKERTHCNT "5"

//VMCA HTTP ENDPOINT URI VALUES
#define VMCA_CRL_URI "vmca/crl"
#define VMCA_ROOT_URI "vmca/root"
#define VMCA_CERTS_URI "vmca/certificates"
#define VMCA_URI "vmca"

// VMCA REST PARAMETER KEYS
#define VMCA_ADD_ROOT_PARAM_KEY_CERT "cert"
#define VMCA_ADD_ROOT_PARAM_KEY_PRIVKEY "privateKey"
#define VMCA_ADD_ROOT_PARAM_KEY_OVERWRITE "overwrite"
#define VMCA_ENUM_CERTS_PARAM_KEY_FLAG "flag"
#define VMCA_ENUM_CERTS_PARAM_KEY_NUMBER "number"
#define VMCA_GET_SIGNED_CERT_PARAM_KEY_CSR "csr"
#define VMCA_GET_SIGNED_CERT_PARAM_KEY_NOT_BF "notBefore"
#define VMCA_GET_SIGNED_CERT_PARAM_KEY_DURATION "duration"
#define VMCA_REVOKE_CERT_PARAM_KEY_CERT "cert"

// VMCA REST RETURN KEYS
#define VMCA_ENUM_CERTS_RETURN_KEY "cert"

//REST AUTH
#define VMCA_DEFAULT_CLOCK_TOLERANCE 60.0
#define VMCA_DEFAULT_SCOPE_STRING "rs_vmca"
#define VMCA_GROUP_PERMISSION_STRING "CAAdmins"
#define VMCA_BASIC_AUTH_STRING "Basic "
#define VMCA_SUCCESS_MESSAGE "{Success: \"success\"}"

#ifdef _WIN32
#define VMCASleep(X) Sleep((X) * 1000)
#else
#define VMCASleep(X) sleep((X))
#endif


#define FILE_CHUNK (64 * 1024)
#define VMCA_TIME_LAG_OFFSET_CRL (10*60)
#define VMCA_TIME_LAG_OFFSET_CERTIFICATE (5*60)
#define VMCA_MIN_CA_CERT_PRIV_KEY_LENGTH (2048)

#define VMCA_LOCK_MUTEX_EXCLUSIVE(pmutex, bLocked) \
if (! (bLocked) ) \
{ \
  pthread_rwlock_wrlock (pmutex); \
  (bLocked) = TRUE; \
}

#define VMCA_LOCK_MUTEX_SHARED(pmutex, bLocked) \
if (! (bLocked) ) \
{ \
  pthread_rwlock_rdlock (pmutex); \
  (bLocked) = TRUE; \
}

#define VMCA_LOCK_MUTEX_UNLOCK(pmutex, bLocked) \
if (bLocked) \
{ \
  pthread_rwlock_unlock (pmutex); \
  (bLocked) = FALSE; \
}
