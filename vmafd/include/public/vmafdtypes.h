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
 * Module Name: VMAFD
 *
 * Filename: vmafdtypes.h
 *
 * Abstract:
 *
 * Common types definition
 *
 */

#ifndef __VMAFDTYPES_H__
#define __VMAFDTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _DCE_IDL_

// We don't want the LikeWise headers since we conflict
// with Unix ODBC, and we are on Unix. Define all types ourselves
#if (defined NO_LIKEWISE &&  !defined _WIN32)

#ifndef VMW_WCHAR16_T_DEFINED
#define VMW_WCHAR16_T_DEFINED 1
typedef unsigned short wchar16_t, *PWSTR;
#endif /* VMW_WCHAR16_T_DEFINED */

#ifndef VMW_PSTR_DEFINED
#define VMW_PSTR_DEFINED 1
typedef char* PSTR;
#endif /* VMW_PSTR_DEFINED */

#ifndef VMW_PCSTR_DEFINED
#define VMW_PCSTR_DEFINED 1
typedef const char* PCSTR;
#endif /* VMW_PCSTR_DEFINED */

#ifndef VMW_PCWSTR_DEFINED
#define VMW_PCWSTR_DEFINED 1
typedef const wchar16_t* PCWSTR;
#endif /* VMW_PCWSTR_DEFINED */

#ifndef VMW_VOID_DEFINED
#define VMW_VOID_DEFINED 1
typedef void VOID, *PVOID;
#endif /* VMW_VOID_DEFINED */

#ifndef VMW_UINT8_DEFINED
#define VMW_UINT8_DEFINED 1
typedef uint8_t  UINT8;
#endif /* VMW_UINT8_DEFINED */

#ifndef VMW_UINT32_DEFINED
#define VMW_UINT32_DEFINED 1
typedef uint32_t UINT32;
#endif /* VMW_UINT32_DEFINED */

#ifndef VMW_DWORD_DEFINED
#define VMW_DWORD_DEFINED 1
typedef uint32_t DWORD, *PDWORD;
#endif /* VMW_DWORD_DEFINED */

#ifndef VMW_BOOLEAN_DEFINED
#define VMW_BOOLEAN_DEFINED 1
typedef UINT8 BOOLEAN, *PBOOLEAN;
#endif /* VMW_BOOLEAN_DEFINED */

#endif /* defined NO_LIKEWISE &&  !defined _WIN32 */

// On Unix and we don't have headers that conflict,
// Just use likewise headers

#if (!defined NO_LIKEWISE && !defined _WIN32)
#include <lw/types.h>
#include <dce/rpcbase.h>
#endif // (!defined NO_LIKEWISE && !defined _WIN32)

#endif

#ifndef _WIN32
#define VMAFD_NCALRPC_END_POINT "vmafdsvc"
#define VMAFD_SEPERATOR "/"

#else
// note: keep in sync with /vmafd/main/idl/vmafd.idl
    #define VMAFD_NCALRPC_END_POINT "VMWareAfdService"
	#define wchar16_t wchar_t
    #define VMAFD_SEPERATOR "\\"
#endif

#define VMAFD_RPC_TCP_END_POINT "2020"

#ifdef _DCE_IDL_

cpp_quote("#include <vmafdtypes.h>")
cpp_quote("#if 0")

#endif

#ifndef VMAFD_WSTRING_DEFINED
#define VMAFD_WSTRING_DEFINED 1
typedef
#ifdef _DCE_IDL_
[ptr, string]
#endif
unsigned short *wstring_t;   /* wchar16_t */
#endif /* VMAFD_WSTRING_DEFINED */

#ifndef VMAFD_VECS_HANDLE_DEFINED
#define VMAFD_VECS_HANDLE_DEFINED 1
typedef
#ifdef _DCE_IDL_
[context_handle]
#endif
void *vecs_crl_handle_t;
#endif /* VMAFD_VECS_HANDLE_DEFINED */

#define SYSTEM_CERT_STORE_NAME "MACHINE_SSL_CERT"
#define SYSTEM_CERT_STORE_NAME_W {'M','A','C','H','I','N','E','_','S','S','L','_','C','E','R','T',0}
#define TRUSTED_ROOTS_STORE_NAME "TRUSTED_ROOTS"
#define TRUSTED_ROOTS_STORE_NAME_W {'T','R','U','S','T','E','D','_','R','O','O','T','S',0}
#define CRL_STORE_NAME "TRUSTED_ROOT_CRLS"
#define CRL_STORE_NAME_W {'T','R','U','S','T','E','D','_','R','O','O','T','_','C','R','L','S',0}
#define VECS_MACHINE_CERT_ALIAS_W {'_','_','M','A','C','H','I','N','E','_','C','E','R','T',0}
#define VECS_MACHINE_CERT_ALIAS    "__MACHINE_CERT"

//Authorization Access Masks
#define READ_STORE 0x40000000
#define WRITE_STORE 0x80000000

//CDC_GET_DCNAME_FLAG
#define CDC_FORCE_REFRESH 0x00000001

#ifndef VMAFD_STATUS_DEFINED
#define VMAFD_STATUS_DEFINED 1

#define VECS_SECRET_KEY_LENGTH_MAX  256

typedef enum {
    CERT_ENTRY_TYPE_UNKNOWN = 0,
    CERT_ENTRY_TYPE_PRIVATE_KEY,
    CERT_ENTRY_TYPE_SECRET_KEY,
    CERT_ENTRY_TYPE_TRUSTED_CERT,
    CERT_ENTRY_TYPE_REVOKED_CERT_LIST,
    CERT_ENTRY_TYPE_ENCRYPTED_PRIVATE_KEY
} CERT_ENTRY_TYPE;

typedef enum {
    ENTRY_INFO_LEVEL_UNDEFINED = 0,
    ENTRY_INFO_LEVEL_1,
    ENTRY_INFO_LEVEL_2
} ENTRY_INFO_LEVEL;

typedef enum
{
    VMAFD_ACE_TYPE_UNKNOWN = 0,
    VMAFD_ACE_TYPE_ALLOWED,
    VMAFD_ACE_TYPE_DENIED
} VMAFD_ACE_TYPE;

typedef enum
{
    VMAFD_STATUS_UNKNOWN       = 0,
    VMAFD_STATUS_INITIALIZING,
    VMAFD_STATUS_PAUSED,
    VMAFD_STATUS_RUNNING,
    VMAFD_STATUS_STOPPING,
    VMAFD_STATUS_STOPPED
} VMAFD_STATUS, *PVMAFD_STATUS;

#endif /* VMAFD_STATUS_DEFINED */

#ifndef VMAFD_DOMAIN_STATE_DEFINED
#define VMAFD_DOMAIN_STATE_DEFINED 1

typedef enum
{
    VMAFD_DOMAIN_STATE_NONE       = 0,
    VMAFD_DOMAIN_STATE_CONTROLLER,
    VMAFD_DOMAIN_STATE_CLIENT
} VMAFD_DOMAIN_STATE, *PVMAFD_DOMAIN_STATE;

#endif /* VMAFD_DOMAIN_STATE_DEFINED */


#ifndef VMAFD_CRL_DATA_DEFINED
#define VMAFD_CRL_DATA_DEFINED 1

typedef struct _VMAFD_CRL_DATA
{
    UINT32 dwSize;
#ifdef _DCE_IDL_
    [size_is(dwSize)]
#endif /* _DCE_IDL_ */
    unsigned char *buffer;
} VMAFD_CRL_DATA, *PVMAFD_CRL_DATA;

#endif /* VMAFD_CRL_DATA_DEFINED */

#ifndef VMAFD_CRL_FILE_METADATA_DEFINED
#define VMAFD_CRL_FILE_METADATA_DEFINED 1
#define AUTH_ID_SIZE 24

typedef struct _VMAFD_CRL_FILE_METADATA
{
    UINT32 dwSize;
    wstring_t pwszIssuerName;
    wstring_t pwszLastUpdate;
    wstring_t pwszNextUpdate;
    wstring_t pwszCrlFileName;
    UINT32 dwCRLNumber;
    unsigned char  bAuthID[AUTH_ID_SIZE];
} VMAFD_CRL_FILE_METADATA, *PVMAFD_CRL_FILE_METADATA;

#endif /* VMAFD_CRL_FILE_METADATA_DEFINED */


#ifndef VMAFD_CRL_METADATA_CONTAINER_DEFINED
#define VMAFD_CRL_METADATA_CONTAINER_DEFINED 1

typedef struct _VMAFD_CRL_METADATA_CONTAINER
{
    UINT32 dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif /* _DCE_IDL_ */
    PVMAFD_CRL_FILE_METADATA MetaData;

} VMAFD_CRL_METADATA_CONTAINER, *PVMAFD_CRL_METADATA_CONTAINER;

#endif /* VMAFD_CRL_METADATA_CONTAINER_DEFINED */

#ifndef  VMAFD_CRL_FILE_CONTAINER_DEFINED
#define  VMAFD_CRL_FILE_CONTAINER_DEFINED 1

typedef struct _VMAFD_CRL_FILE_CONTAINER
{
    UINT32 dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif /* _DCE_IDL_ */
       PVMAFD_CRL_DATA crls;
} VMAFD_CRL_FILE_CONTAINER, *PVMAFD_CRL_FILE_CONTAINER;

#endif /* VMAFD_CRL_FILE_CONTAINER_DEFINED */

#ifndef VECS_STORE_PERMISSION_CONTAINER_DEFINED
#define VECS_STORE_PERMISSION_CONTAINER_DEFINED 1

typedef struct _VECS_STORE_PERMISSION_CONTAINER_W
{
    wstring_t pszUserName;
    UINT32 dwAccessMask;
} VECS_STORE_PERMISSION_W, *PVECS_STORE_PERMISSION_W;
#endif

#ifndef VMAFD_CERT_CONTAINER_DEFINED
#define VMAFD_CERT_CONTAINER_DEFINED 1

typedef struct _VMAFD_CERT_CONTAINER
{
    UINT32 dwStoreType;
    UINT32 dwDate;
    UINT32 dwAutoRefresh;
    wstring_t pCert;
    wstring_t pAlias;
    wstring_t pPassword;
    wstring_t pPrivateKey;

} VMAFD_CERT_CONTAINER, *PVMAFD_CERT_CONTAINER;

#endif /* VMAFD_CERT_CONTAINER_DEFINED */

#ifndef VMAFD_CERT_ARRAY_DEFINED
#define VMAFD_CERT_ARRAY_DEFINED 1

typedef struct _VMAFD_CERT_ARRAY
{
       UINT32 dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif /* _DCE_IDL_ */
       PVMAFD_CERT_CONTAINER certificates;

} VMAFD_CERT_ARRAY, *PVMAFD_CERT_ARRAY;

#endif /* VMAFD_CERT_ARRAY_DEFINED */

#ifndef VMAFD_CA_CERT_DEFINED
#define VMAFD_CA_CERT_DEFINED 1

typedef struct _VMAFD_CA_CERT
{
    /* In the future this may become a chain of certs. */
    wstring_t pCert;
    wstring_t pCrl;
    wstring_t pCN;
    wstring_t pSubjectDN;
} VMAFD_CA_CERT, *PVMAFD_CA_CERT;

#endif /* VMAFD_CA_CERT_DEFINED */

#ifndef VMAFD_CA_CERT_ARRAY_DEFINED
#define VMAFD_CA_CERT_ARRAY_DEFINED 1

typedef struct _VMAFD_CA_CERT_ARRAY
{
       UINT32 dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif /* _DCE_IDL_ */
    PVMAFD_CA_CERT pCACerts;
} VMAFD_CA_CERT_ARRAY, *PVMAFD_CA_CERT_ARRAY;

#endif /* VMAFD_CA_CERT_ARRAY_DEFINED */

#ifndef VMAFD_CERT_STORE_ARRAY_DEFINED
#define VMAFD_CERT_STORE_ARRAY_DEFINED 1

typedef struct _VMAFD_CERT_STORE_ARRAY
{
       UINT32 dwCount;
#ifdef _DCE_IDL_
       [size_is(dwCount)]
#endif /* _DCE_IDL_ */
       wstring_t* ppwszStoreNames;

} VMAFD_CERT_STORE_ARRAY, *PVMAFD_CERT_STORE_ARRAY;

#endif /* VMAFD_CERT_STORE_ARRAY_DEFINED */


#ifndef CERTIFICATE_STORE_TYPE_DEFINED
#define CERTIFICATE_STORE_TYPE_DEFINED 1

typedef enum
{

    CERTIFICATE_STORE_TYPE_ALL              = 0,
    CERTIFICATE_STORE_TYPE_PRIVATE          = 1,
    CERTIFICATE_STORE_TYPE_TRUSTED          = 2,
    CERTIFICATE_STORE_TYPE_REVOKED          = 3,
    CERTIFICATE_STORE_TYPE_TRUSTED_ROOTS    = 4,
    CERTIFICATE_STORE_TYPE_PASSWORD         = 5

} CERTIFICATE_STORE_TYPE;

#endif /* CERTIFICATE_STORE_TYPE_DEFINED */



#ifndef CRL_OPEN_MODE_DEFINED
#define CRL_OPEN_MODE_DEFINED 1

typedef enum
{
    CRL_OPEN_MODE_READ  = 0,
    CRL_OPEN_MODE_WRITE = 1

} CRL_OPEN_MODE;

#endif /* CRL_OPEN_MODE_DEFINED */


#ifndef CDC_ADDRESS_TYPE_DEFINED
#define CDC_ADDRESS_TYPE_DEFINED 1

typedef enum
{
    CDC_ADDRESS_TYPE_UNDEFINED = 0,
    CDC_ADDRESS_TYPE_INET,
    CDC_ADDRESS_TYPE_NETBIOS
} CDC_ADDRESS_TYPE;

#endif

#ifndef CDC_DC_INFO_A_DEFINED
#define CDC_DC_INFO_A_DEFINED    1

typedef char* GUID_A;
//TODO: Later change this to actual GUID struct?

typedef struct _CDC_DC_INFO_A
{
    char*             pszDCName;
    char*             pszDCAddress;
    CDC_ADDRESS_TYPE  DcAddressType;
    char*             pszDomainName;
    char*             pszDcSiteName;
}CDC_DC_INFO_A, *PCDC_DC_INFO_A;

#endif

#ifndef CDC_DC_INFO_W_DEFINED
#define CDC_DC_INFO_W_DEFINED    1

typedef wstring_t GUID_W;
//TODO: Later change this to actual GUID struct?

typedef struct _CDC_DC_INFO_W
{
    wstring_t            pszDCName;
    wstring_t            pszDCAddress;
    CDC_ADDRESS_TYPE     DcAddressType;
    wstring_t            pszDomainName;
    wstring_t            pszDcSiteName;
}CDC_DC_INFO_W, *PCDC_DC_INFO_W;

#endif

#ifndef CDC_DC_STATE_DEFINED
#define CDC_DC_STATE_DEFINED    1

typedef enum {
    CDC_DC_STATE_UNDEFINED =0,
    CDC_DC_STATE_NO_DC_LIST,
    CDC_DC_STATE_SITE_AFFINITIZED,
    CDC_DC_STATE_OFF_SITE,
    CDC_DC_STATE_NO_DCS_ALIVE,
    CDC_DC_STATE_LEGACY
} CDC_DC_STATE, *PCDC_DC_STATE;

#endif


typedef UINT32 VMAFD_JOIN_FLAGS;

#define VMAFD_JOIN_FLAGS_ENABLE_NSSWITCH    0x00000001
#define VMAFD_JOIN_FLAGS_ENABLE_PAM         0x00000002
#define VMAFD_JOIN_FLAGS_ENABLE_SSH         0x00000004
#define VMAFD_JOIN_FLAGS_CLIENT_PREJOINED   0x00000008

#ifndef VMAFD_HB_INFO_A_DEFINED
#define VMAFD_HB_INFO_A_DEFINED 1

typedef struct _VMAFD_HB_INFO_A
{
    char*  pszServiceName;
    UINT32 dwPort;
    UINT32 dwLastHeartbeat;
    UINT32  bIsAlive;
} VMAFD_HB_INFO_A, *PVMAFD_HB_INFO_A;

#endif

#ifndef VMAFD_HB_STATUS_A_DEFINED
#define VMAFD_HB_STATUS_A_DEFINED 1

typedef struct _VMAFD_HB_STATUS_A
{
    UINT32             bIsAlive;
    UINT32             dwCount;
    PVMAFD_HB_INFO_A  pHeartbeatInfoArr;
} VMAFD_HB_STATUS_A, *PVMAFD_HB_STATUS_A;

#endif

#ifndef VMAFD_HB_INFO_W_DEFINED
#define VMAFD_HB_INFO_W_DEFINED 1

typedef struct _VMAFD_HB_INFO_W
{
    wstring_t pszServiceName;
    UINT32 dwPort;
    UINT32 dwLastHeartbeat;
    UINT32  bIsAlive;
} VMAFD_HB_INFO_W, *PVMAFD_HB_INFO_W;

#endif

#ifndef VMAFD_HB_STATUS_W_DEFINED
#define VMAFD_HB_STATUS_W_DEFINED 1

typedef struct _VMAFD_HB_STATUS_W
{
    UINT32             bIsAlive;
    UINT32             dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif /* _DCE_IDL_ */
    PVMAFD_HB_INFO_W   pHeartbeatInfoArr;
} VMAFD_HB_STATUS_W, *PVMAFD_HB_STATUS_W;

#endif

#ifndef CDC_DC_STATUS_INFO_A_DEFINED
#define CDC_DC_STATUS_INFO_A_DEFINED    1

typedef struct _CDC_DC_STATUS_INFO_A
{
    UINT32             dwLastPing;
    UINT32             dwLastResponseTime;
    UINT32             dwLastError;
    UINT32             bIsAlive;
    char*              pszSiteName;
} CDC_DC_STATUS_INFO_A, *PCDC_DC_STATUS_INFO_A;
#endif

#ifndef CDC_DC_STATUS_INFO_W_DEFINED
#define CDC_DC_STATUS_INFO_W_DEFINED    1

typedef struct _CDC_DC_STATUS_INFO_W
{
    UINT32             dwLastPing;
    UINT32             dwLastResponseTime;
    UINT32             dwLastError;
    UINT32             bIsAlive;
    wstring_t          pwszSiteName;
} CDC_DC_STATUS_INFO_W, *PCDC_DC_STATUS_INFO_W;
#endif


#ifndef VMAFD_DC_INFO_W_DEFINED
#define VMAFD_DC_INFO_W_DEFINED         1

typedef struct _VMAFD_DC_INFO_W
{
    wstring_t  pwszHostName;
    wstring_t  pwszAddress;
}VMAFD_DC_INFO_W, *PVMAFD_DC_INFO_W;
#endif

#ifndef VMAFD_MAX_DN_LEN
#define VMAFD_MAX_DN_LEN 1024
#endif

typedef struct _VMAFD_SUPERLOG_ENTRY
{
    UINT32 dwErrorCode;
    UINT32 iStartTime;
    UINT32 iEndTime;
    UINT32 dwState;
    UINT32 dwCDCPingTime;
    UINT32 dwCDCLastPing;
    UINT32 bCDCIsAlive;
    UINT32 bHBIsAlive;
    UINT32 dwHBCount;
    unsigned char pszDomainName[VMAFD_MAX_DN_LEN];
    unsigned char pszDCName[VMAFD_MAX_DN_LEN];
    unsigned char pszSiteName[VMAFD_MAX_DN_LEN];
    unsigned char pszDCAddress[VMAFD_MAX_DN_LEN];
} VMAFD_SUPERLOG_ENTRY, *PVMAFD_SUPERLOG_ENTRY;


typedef struct _VMAFD_SUPERLOG_ENTRY_ARRAY
{
    UINT32 dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif
    PVMAFD_SUPERLOG_ENTRY entries;
} VMAFD_SUPERLOG_ENTRY_ARRAY, *PVMAFD_SUPERLOG_ENTRY_ARRAY;


typedef
#ifdef _DCE_IDL_
[context_handle]
#endif
void *vmafd_superlog_cookie_t;



#ifndef _DCE_IDL_
#ifndef VMAFD_SERVER_DEFINED
#define VMAFD_SERVER_DEFINED 1
/*
 * This is an incomplete type which is completed by the implementation
 * of the vmafd client library.
 */
typedef struct _VMAFD_SERVER VMAFD_SERVER, *PVMAFD_SERVER;
#endif /* VMAFD_SERVER_DEFINED */
#endif

typedef struct _VMAFD_DC_ENTRIES_W
{
    UINT32 dwCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif
    wstring_t* ppszEntries;
} CDC_DC_ENTRIES_W, *PCDC_DC_ENTRIES_W;

#ifdef _DCE_IDL_
cpp_quote("#endif")
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VMAFDTYPES_H__ */
