/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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
 * LwCASecurity provides interface for the following high level functionality
 * with appropriate intermediate level implementations which are hidden
 * from the end user.
 * 1. Create or Add private key
 *    a. Create key(pair)       (plugin hides implementation)
 *      aa. Encrypt key         (plugin hides implementation)
 *      ab. Store encrypted key (plugin hides implementation)
 * 2. Sign certificates (API)
 *    a. Retrieve private key
 *      aa. Get data from store (plugin hides implementation)
 *      ab. Decrypt key         (plugin hides implementation)
 * 3. Verify signatures (API)
 *    a. Retrieve private key   (plugin hides implementation)
 *      aa. Get data from store (plugin hides implementation)
 *      ab. Decrypt key         (plugin hides implementation)
 *
 * This plugin allows Lightwave CA to delegate security, key management
 * and key usage like sign and verify if plugin implementations provide
 * those capabilities. Plugin implementations are free to provide any
 * or all of the capabilities. Plugin user is expected to query capabilities
 * and provide missing implementaions by registering callbacks.
 *
 * Things to remember when using this plugin:
 * 1. All calls return DWORD. 0 for success. otherwise failure.
 * 2. All memory allocated by the plugin should be freed with corresponding
 *    free calls from the plugin.
 *
 * Usage flow as follows
 * 1. Use LwCASecurityVersion and verify version.
 * 2. Use LwCASecurityLoadInterface to load interface.
 * 3. First, call pFnOpenHandle to obtain a handle
 *       - this is an opaque handle. see PLWCA_SECURITY_HANDLE
 *       - this handle must be kept for the lifetime of the plugin
 * 4. Use pFnCloseHandle to close the open handle
 * 5. Use LwCASecurityUnLoadInterface to unload and free plugin specific
 *    resources.
 *    - Unload will close handle if still open.
*/

#ifndef __LWCA_SECURITY_H__
#define __LWCA_SECURITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <openssl/x509.h>

/* types that are used in this file */
#ifndef VMW_DWORD_DEFINED
#define VMW_DWORD_DEFINED 1
typedef uint32_t DWORD, *PDWORD;
#endif /* VMW_DWORD_DEFINED */

#ifndef VMW_PSTR_DEFINED
#define VMW_PSTR_DEFINED 1
typedef char* PSTR;
#endif /* VMW_PSTR_DEFINED */

#ifndef VMW_PCSTR_DEFINED
#define VMW_PCSTR_DEFINED 1
typedef const char* PCSTR;
#endif /* VMW_PCSTR_DEFINED */

#ifndef VMW_PBYTE_DEFINED
#define VMW_PBYTE_DEFINED 1
typedef unsigned char BYTE, *PBYTE;
#endif /* VMW_PBYTE_DEFINED */

#ifndef VMW_PVOID_DEFINED
#define VMW_PVOID_DEFINED 1
typedef void VOID, *PVOID;
#endif /* VMW_PVOID_DEFINED */

#ifndef VMW_BOOLEAN_DEFINED
#define VMW_BOOLEAN_DEFINED 1
typedef uint8_t BOOLEAN, *PBOOLEAN;
#endif /* VMW_BOOLEAN_DEFINED */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define LWCA_SECURITY_VERSION_MAJOR   "1"
#define LWCA_SECURITY_VERSION_MINOR   "0"
#define LWCA_SECURITY_VERSION_RELEASE "0"

typedef struct _LWCA_SECURITY_HANDLE_ *PLWCA_SECURITY_HANDLE;

typedef enum
{
    LWCA_SECURITY_CAP_NONE         = 0x0,
    LWCA_SECURITY_CAP_CRYPT        = 0x1,
    LWCA_SECURITY_CAP_SIGN_VERIFY  = 0x2,
    LWCA_SECURITY_CAP_STORAGE      = 0x4,
    LWCA_SECURITY_CAP_ALL          = LWCA_SECURITY_CAP_CRYPT | \
                                     LWCA_SECURITY_CAP_SIGN_VERIFY | \
                                     LWCA_SECURITY_CAP_STORAGE
}LWCA_SECURITY_CAP;

typedef enum
{
    LWCA_SECURITY_MESSAGE_DIGEST_SHA256,
    LWCA_SECURITY_MESSAGE_DIGEST_SHA512
}LWCA_SECURITY_MESSAGE_DIGEST;

typedef enum
{
    LWCA_SECURITY_SIGN_CERT,
    LWCA_SECURITY_SIGN_REQ,
    LWCA_SECURITY_SIGN_CRL
}LWCA_SECURITY_SIGN_TYPE;

typedef struct _LWCA_SECURITY_SIGN_DATA_
{
    union
    {
        X509     *pX509Cert;
        X509_REQ *pX509Req;
        X509_CRL *pX509Crl;
    }signData;
    LWCA_SECURITY_SIGN_TYPE signType;
}LWCA_SECURITY_SIGN_DATA, *PLWCA_SECURITY_SIGN_DATA;

/* capability helper macros */
#define LWCA_SECURITY_CAP_HAS_CRYPT(cap) \
        ((cap & LWCA_SECURITY_CAP_CRYPT) == LWCA_SECURITY_CAP_CRYPT)
#define LWCA_SECURITY_CAP_HAS_SIGN_VERIFY(cap) \
        ((cap & LWCA_SECURITY_CAP_SIGN_VERIFY) == LWCA_SECURITY_CAP_SIGN_VERIFY)
#define LWCA_SECURITY_CAP_HAS_STORAGE(cap) \
        ((cap & LWCA_SECURITY_CAP_STORAGE) == LWCA_SECURITY_CAP_STORAGE)
#define LWCA_SECURITY_CAP_HAS_ALL(cap) \
        ((cap & LWCA_SECURITY_CAP_ALL) == LWCA_SECURITY_CAP_ALL)


/* function names */
#define LWCA_FN_NAME_SECURITY_GET_VERSION       "LwCASecurityGetVersion"
#define LWCA_FN_NAME_SECURITY_LOAD_INTERFACE    "LwCASecurityLoadInterface"
#define LWCA_FN_NAME_SECURITY_UNLOAD_INTERFACE  "LwCASecurityUnloadInterface"

/* version of the plugin interface */
typedef PCSTR
(*PFN_LWCA_SECURITY_GET_VERSION)(
    VOID
    );

/*
 * initialize. handle returned must be saved.
 * handle is used in all api calls.
 * when done, use pFnCloseHandle to close.
*/
typedef DWORD
(*PFN_LWCA_SECURITY_INITIALIZE)(
    PCSTR pszConfigFile,
    PLWCA_SECURITY_HANDLE *pHandle
    );

/*
 * Return a bit mask of capabilities implemented.
 * If all caps are implemented, return LWCA_SECURITY_CAP_ALL
 * If an implementation choses to skip implementation of a cap, appropriate
 * delegation must be setup via callbacks. See "Capability delegation" below.
*/
typedef DWORD
(*PFN_LWCA_SECURITY_GET_CAPS)(
    PLWCA_SECURITY_HANDLE pHandle,
    LWCA_SECURITY_CAP *pnCap
    );

/*
 * Return error description for errors originating from this plugin.
 * Return human readable error strings. Do not include user supplied
 * data in error strings.
*/
typedef DWORD
(*PFN_LWCA_SECURITY_GET_ERROR_STRING)(
    DWORD dwError,
    PSTR *ppszError
    );

/* close handle */
typedef VOID
(*PFN_LWCA_SECURITY_CLOSE_HANDLE)(
    PLWCA_SECURITY_HANDLE pHandle
    );

/* free memory */
typedef VOID
(*PFN_LWCA_SECURITY_FREE_MEMORY)(
    PVOID pMemory
    );

/*
 * Create private key.
*/
typedef DWORD
(*PFN_LWCA_SECURITY_CREATE_KEY_PAIR)(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,     /* capability overrides can use this for context */
    PCSTR pszKeyId,      /* user supplied id to refer to this key */
    size_t nKeyLength,   /* key length 1024 to 16384 */
    PSTR *ppszPublicKey  /* out: pem encoded public key */
    );

/*
 * Add private key
*/
typedef DWORD
(*PFN_LWCA_SECURITY_ADD_KEY_PAIR)(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,     /* capability overrides can use this for context */
    PCSTR pszKeyId,      /* user supplied id to refer to this key */
    PCSTR pszPrivateKey  /* pem encoded private key */
    );

/*
 * Sign X509 cert, request of crl  with key specified by keyid
*/
typedef DWORD
(*PFN_LWCA_SECURITY_SIGN)(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,     /* capability overrides can use this for context */
    PCSTR pszKeyId,
    PLWCA_SECURITY_SIGN_DATA pSignData,
    LWCA_SECURITY_MESSAGE_DIGEST md
    );

/*
 * Verify X509 cert, request of crl  with key specified by keyid
*/
typedef DWORD
(*PFN_LWCA_SECURITY_VERIFY)(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,     /* capability overrides can use this for context */
    PCSTR pszKeyId,
    PLWCA_SECURITY_SIGN_DATA pSignData,
    BOOLEAN *pbValid
    );

/*
 * Capabilities override (for the following scenarios)
 * 1. implementation does not provide a capability
 * 2. caller needs to override a capability with own implementation
*/

typedef struct _LWCA_BINARY_DATA_
{
    PBYTE pData;
    DWORD dwLength;
}LWCA_BINARY_DATA, *PLWCA_BINARY_DATA;

typedef DWORD
(*PFN_LWCA_SECURITY_CAP_CRYPT_ENCRYPT)(
    PVOID pUserData,
    PLWCA_BINARY_DATA pData,
    PLWCA_BINARY_DATA *ppEncryptedData
    );

typedef DWORD
(*PFN_LWCA_SECURITY_CAP_CRYPT_DECRYPT)(
    PVOID pUserData,
    PLWCA_BINARY_DATA pEncryptedData,
    PLWCA_BINARY_DATA *ppData
    );

typedef DWORD
(*PFN_LWCA_SECURITY_CAP_STORAGE_PUT)(
    PVOID pUserData,
    PCSTR pszKeyId,
    PLWCA_BINARY_DATA pEncryptedData
    );

typedef DWORD
(*PFN_LWCA_SECURITY_CAP_STORAGE_GET)(
    PVOID pUserData,
    PCSTR pszKeyId,
    PLWCA_BINARY_DATA *ppEncryptedData
    );

typedef DWORD
(*PFN_LWCA_SECURITY_CAP_SIGN_VERIFY_SIGN)(
    PVOID pUserData,
    PLWCA_BINARY_DATA pKey,
    PLWCA_SECURITY_SIGN_DATA pSignData
    );

typedef DWORD
(*PFN_LWCA_SECURITY_CAP_SIGN_VERIFY_VERIFY)(
    PVOID pUserData,
    PLWCA_SECURITY_SIGN_DATA pSignData,
    PLWCA_BINARY_DATA pKey,
    BOOLEAN **ppbValid
    );

typedef struct _LWCA_SECURITY_CAP_OVERRIDE_
{
    PFN_LWCA_SECURITY_CAP_CRYPT_ENCRYPT      pFnCryptEncrypt;
    PFN_LWCA_SECURITY_CAP_CRYPT_DECRYPT      pFnCryptDecrypt;
    PFN_LWCA_SECURITY_CAP_STORAGE_PUT        pFnStoragePut;
    PFN_LWCA_SECURITY_CAP_STORAGE_GET        pFnStorageGet;
    PFN_LWCA_SECURITY_CAP_SIGN_VERIFY_SIGN   pFnSignVerifySign;
    PFN_LWCA_SECURITY_CAP_SIGN_VERIFY_VERIFY pFnSignVerifyVerify;
}LWCA_SECURITY_CAP_OVERRIDE, *PLWCA_SECURITY_CAP_OVERRIDE;


typedef DWORD
(*PFN_LWCA_SECURITY_CAP_OVERRIDE)(
    PLWCA_SECURITY_HANDLE pHandle,
    PLWCA_SECURITY_CAP_OVERRIDE pOverride
    );

typedef struct _LWCA_SECURITY_INTERFACE_
{
    PFN_LWCA_SECURITY_INITIALIZE          pFnInitialize;
    PFN_LWCA_SECURITY_GET_CAPS            pFnGetCaps;
    PFN_LWCA_SECURITY_CAP_OVERRIDE        pFnCapOverride;
    PFN_LWCA_SECURITY_ADD_KEY_PAIR        pFnAddKeyPair;
    PFN_LWCA_SECURITY_CREATE_KEY_PAIR     pFnCreateKeyPair;
    PFN_LWCA_SECURITY_SIGN                pFnSign;
    PFN_LWCA_SECURITY_VERIFY              pFnVerify;
    PFN_LWCA_SECURITY_GET_ERROR_STRING    pFnGetErrorString;
    PFN_LWCA_SECURITY_CLOSE_HANDLE        pFnCloseHandle;
    PFN_LWCA_SECURITY_FREE_MEMORY         pFnFreeMemory;
}LWCA_SECURITY_INTERFACE, *PLWCA_SECURITY_INTERFACE;


/* Load Interface*/
typedef DWORD
(*PFN_LWCA_SECURITY_LOAD_INTERFACE)(
    PLWCA_SECURITY_INTERFACE *ppInterface
    );

/* Unload Interface*/
typedef DWORD
(*PFN_LWCA_SECURITY_UNLOAD_INTERFACE)(
    PLWCA_SECURITY_INTERFACE pInterface
    );

/* get version. can be called anytime */
PCSTR
LwCASecurityGetVersion(
    VOID
    );

/* Load. call before using implemented functionality */
DWORD
LwCASecurityLoadInterface(
    PLWCA_SECURITY_INTERFACE *ppInterface
    );

/* finalize. call to unload and free any plugin resources */
DWORD
LwCASecurityUnloadInterface(
    PLWCA_SECURITY_INTERFACE pInterface
    );

#ifdef __cplusplus
}
#endif

#endif //__LWCA_SECURITY_H__
