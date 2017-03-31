
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



#ifndef __VMCA_H__
#define __VMCA_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <Windows.h>
#else /* linux */
#if !defined(NO_LIKEWISE)
#include <lw/types.h>
#endif
#endif /* ifdef _WIN32 */

#include <vmcatypes.h>

// We don't want the LikeWise headers since we conflict
// with Unix ODBC, and we are on Unix. Define all types ourselves
#if (defined NO_LIKEWISE &&  !defined _WIN32)

#ifndef VMW_WCHAR16_T_DEFINED
#define VMW_WCHAR16_T_DEFINED 1
typedef unsigned short int wchar16_t, *PWSTR;
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


// Types for VMCA
#ifndef _VMCA_TYPES_DEFINED
#define _VMCA_TYPES_DEFINED 1
typedef char* PVMCA_CSR;
typedef char* PVMCA_KEY;
typedef char* PVMCA_CERTIFICATE;
typedef const char* PCVMCA_CSR;
#endif //_VMCA_TYPES_DEFINED

//
// From RFC 2459 , Section 4.2.1.3
// Please don't change the values unless the RFC changes.

// The key usage extension defines the purpose (e.g., encipherment,
//  signature, certificate signing) of the key contained in the
//  certificate.

#ifndef _VMCA_KEY_USAGE_DEFINED
#define _VMCA_KEY_USAGE_DEFINED 1
enum _VMCA_KEY_USAGE {
    VMCA_DIGITAL_SIGNATURE      = 0,
    VMCA_NON_REPUDIATION         = 1,
    VMCA_KEY_ENCIPHERMENT       = 2,
    VMCA_DATA_ENCIPHERMENT      = 3,
    VMCA_KEY_AGREEMENT          = 4,
    VMCA_KEY_CERT_SIGN          = 5,
    VMCA_KEY_CRL_SIGN           = 6,
    VMCA_ENCIPHER_ONLY          = 7,
    VMCA_DECIPHER_ONLY          = 8
};
#endif // _VMCA_KEY_USAGE_DEFINED

#ifndef _VMCA_KEY_USAGE_FLAG_DEFINED
#define _VMCA_KEY_USAGE_FLAG_DEFINED 1

typedef ULONG VMCA_KEY_USAGE_FLAG;

#define VMCA_KEY_USAGE_FLAG_DIGITAL_SIGNATURE 0x00000001
#define VMCA_KEY_USAGE_FLAG_NON_REPUDIATION   0x00000002
#define VMCA_KEY_USAGE_FLAG_KEY_ENCIPHERMENT  0x00000004
#define VMCA_KEY_USAGE_FLAG_DATA_ENCIPHERMENT 0x00000008
#define VMCA_KEY_USAGE_FLAG_KEY_AGREEMENT     0x00000010
#define VMCA_KEY_USAGE_FLAG_KEY_CERT_SIGN     0x00000020
#define VMCA_KEY_USAGE_FLAG_KEY_CRL_SIGN      0x00000040
#define VMCA_KEY_USAGE_FLAG_ENCIPER_ONLY      0x00000080
#define VMCA_KEY_USAGE_FLAG_DECIPHER_ONLY     0x00000100

#endif /* _VMCA_KEY_USAGE_FLAG_DEFINED */

#ifndef _VMCA_OID_ENUM_DEFINED
#define _VMCA_OID_ENUM_DEFINED 1

typedef enum _VMCA_OID {
    VMCA_OID_CN             = 1,
    VMCA_OID_DC             = 2,
    VMCA_OID_COUNTRY        = 3,
    VMCA_OID_LOCALITY       = 4,
    VMCA_OID_STATE          = 5,
    VMCA_OID_ORGANIZATION   = 6,
    VMCA_OID_ORG_UNIT       = 7,
    VMCA_OID_DNS            = 8,
    VMCA_OID_URI            = 9,
    VMCA_OID_EMAIL          = 10,
    VMCA_OID_IPADDRESS      = 11
}VMCA_OID;

#endif // _VMCA_OID_ENUM_DEFINED

//
// if you ever change this , please make sure that it matches the
// internal definition of this in the IDL file.
//

#ifndef _VMCA_ENUM_FLAGS_DEFINED
#define _VMCA_ENUM_FLAGS_DEFINED 1
typedef enum {
      VMCA_ENUM_SUCCESS         = 0,
      VMCA_ENUM_END             = 1,
      VMCA_ENUM_ERROR           = 2
 } VMCA_ENUM_CERT_RETURN_CODE;
#endif //_VMCA_ENUM_FLAGS_DEFINED

#ifndef _VMCA_PKCS_10_DATAA_DEFINED
#define _VMCA_PKCS_10_DATAA_DEFINED 1
typedef struct _VMCA_PKCS_10_REQ_DATAA
{
    PSTR          pszName;
    PSTR          pszDomainName;
    PSTR          pszCountry;
    PSTR          pszLocality;
    PSTR          pszState;
    PSTR          pszOrganization;
    PSTR          pszOU;
    PSTR          pszDNSName;
    PSTR          pszURIName;
    PSTR          pszEmail;
    PSTR          pszIPAddress;
    DWORD         dwKeyUsageConstraints;
} VMCA_PKCS_10_REQ_DATAA,*PVMCA_PKCS_10_REQ_DATAA;
#endif // //_VMCA_PKCS_10_DATAA_DEFINED

#ifndef _VMCA_PKCS_10_DATAW_DEFINED
#define _VMCA_PKCS_10_DATAW_DEFINED 1
typedef struct _VMCA_PKCS_10_REQ_DATAW
{
    PWSTR         pszName;
    PWSTR         pszDomainName;
    PWSTR         pszCountry;
    PWSTR         pszLocality;
    PWSTR         pszState;
    PWSTR         pszOrganization;
    PWSTR         pszOU;
    PWSTR         pszDNSName;
    PWSTR         pszURIName;
    PWSTR         pszEmail;
    PWSTR         pszIPAddress;
    DWORD         dwKeyUsageConstraints;
} VMCA_PKCS_10_REQ_DATAW, *PVMCA_PKCS_10_REQ_DATAW;

#endif //_VMCA_PKCS_10_DATAW_DEFINED

// Functions

DWORD
VMCAAddRootCertificateA(
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PSTR pszPassPhrase,
    PVMCA_KEY pszPrivateKey
);

DWORD
VMCAAddRootCertificateW(
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PWSTR pszPassPhrase,
    PVMCA_KEY pszPrivateKey
);

DWORD
VMCAAddRootCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PSTR pszPassPhrase,
    PVMCA_KEY pszPrivateKey
);

DWORD
VMCAAddRootCertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PWSTR pszPassPhrase,
    PVMCA_KEY pszPrivateKey
);
// VMCAAddRootCeritificate function allows the user to add a Root CA
// certficate to VMware Certificate Authority.
// Arguments :
// pszServerName : VMCA Server Name
// pszPassPhrase : optional , Password that protect a Private key
// pszPrivateKey : Private Key for the Ceritificate

DWORD
VMCAAllocatePKCS10DataA(
  PVMCA_PKCS_10_REQ_DATAA* ppCertRequestData
);

DWORD
VMCAAllocatePKCS10DataW(
  PVMCA_PKCS_10_REQ_DATAW* ppCertRequestData
);
// VMCAAllocatePKCS10Data function allocates a VMCA specific structure
// which carries the payload for creating a certificate
// request. This function allocates and returns a zero
// structure. Please see the definition of VMCA_PKCS_10_REQ_DATA
// for more details
//
// Arguments :
//         ppCertRequestData : returns an allocated PVMA_PKCS_REQ_DATA_Struture
// Returns :
//     Error code

DWORD
VMCACreatePrivateKeyA(
    PSTR pszPassPhrase,
    size_t uiKeyLength,
    PVMCA_KEY* ppPrivateKey,
    PVMCA_KEY* ppPublicKey
);

DWORD
VMCACreatePrivateKeyW(
    PWSTR pwszPassPhrase,
    size_t uiKeyLength,
    PVMCA_KEY* ppPrivateKey,
    PVMCA_KEY* ppPublicKey
);
// VMCAAllocatePrivateKey function creates private-public key pair  and retuns them to user
//
// Arguments :
//          pszPassPhrase   : Optional Pass Word to protect the Key
//          uiKeyLength     : Key Length - Valid values are between 1024 and 16384
//          ppPrivateKey    : returns a PEM encoded Private Key String
//          ppPublicKey     : returns a PEM encoded Public Key String.
//
// Returns :
//      Error Code
//
// Notes :  key length is a complicated problem,  please look at
// RSA's recommendation http://www.rsa.com/rsalabs/node.asp?id=2218
// on Corporate Key lengths.

DWORD
VMCACreateSelfSignedCertificateA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    PVMCA_KEY pszPrivateKey,
    PSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
);

DWORD
VMCACreateSelfSignedCertificateW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PVMCA_KEY pszPrivateKey,
    PWSTR pwszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
);
//  VMCASelfSignedCertificatePrivate allows the user to create
//  a Self Signed Certificate.
//
// Arguments :
//      pCertRequest    :  Parameters for the request
//      pszPrivateKey   :  Private Key for the CSR
//      pszPassPhrase   :  Pass Phrase for the Private Key
//      ppszCertificate : Allocated Certificate
// Returns :
//      Error Code


DWORD
VMCACreateSigningRequestA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    PVMCA_KEY pPrivateKey,
    PSTR pszPassPhrase,
    PVMCA_CSR* ppCSR
);

DWORD
VMCACreateSigningRequestW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PVMCA_KEY pPrivateKey,
    PWSTR pwszPassPhrase,
    PVMCA_CSR* ppCSR
);
// VMCACreateSigningRequest creates a CSR from the user provided parameters
// and allocates a CSR that can be send to the CA for purposes of getting a Signed
// Certificate.
// Argumemnts :
//      pCertRequest : User parameters that go into the certificate
//      pPrivateKey : The Private Key needed for the Certificate
//      pszPassPhrase : Optional Pass Phrase that is protecting the private key
//      ppCSR : retunrs Allocated CSR that can be send to Server for Signing.
//  Returns:
//      Error Code

DWORD
VMCAOpenEnumContextA(
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *Context
    );

DWORD
VMCAOpenEnumContextW(
    PCWSTR pszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *Context
    );

DWORD
VMCAOpenEnumContextHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *Context
);

DWORD
VMCAOpenEnumContextHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *Context
);

DWORD
VMCAGetNextCertificate(
    PVOID Context,
    PVMCA_CERTIFICATE* ppCertificate,
    int *pCurrentIndex,
    VMCA_ENUM_CERT_RETURN_CODE* enumStatus
    );

DWORD
VMCACloseEnumContext(
    PVOID Context
);

VOID
VMCAFreeCertificate(
    PVMCA_CERTIFICATE pCertificate
);
// VMCAFreeCertificate frees the Certificate Objects allocated by the VMCAGetSignedCertificate
//
// Arguments :
//      pCertficate : Points to the Certificate that is to be freed.
// Returns :
//      VOID

// VOID
// VMCAFreeCertificateContainer(
//     PVMCA_CERITIFICATE_CONTAINER pCertContainer
// );
// VMCAFreeCertificate Contianer free the Certificate Container Object
// Arguments :
//  pCertContainer : Ponits to the ceritificate container that needs to be freed.
// Returns :
//  VOID


VOID
VMCAFreeCSR(
    PVMCA_CSR pCSR
);
// VMCAFreeCSR frees the Key Objects allocated by the VMCACreateSigningRequest
//
// Arguments :
//      pCSR : Points to the CSR that is to be freed.
// Returns :
//      VOID
//

VOID
VMCAFreeKey(
    PVMCA_KEY pKey
);
// VMCAFreeKey frees the Key Objects allocated by the VMCAAllocatePrivateKey
//
// Arguments :
//      pKey : Points to the key that is to be freed.
// Returns :
//      VOID
//

VOID
VMCAFreePKCS10DataA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequestData
);

VOID
VMCAFreePKCS10DataW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequestData
);
// VMCAFreePKCS10Data function frees the object
//
// Arguments :
//          pCertRequestData :  Pointer to the memory that is to be freed.
// Returns :
//      none
//

DWORD
VMCAGetCertificateAsStringA(
    PVMCA_CERTIFICATE pCertificate,
    PSTR* ppszCertString
);

DWORD
VMCAGetCertificateAsStringW(
    PVMCA_CERTIFICATE pCertificate,
    PWSTR* ppswzCertString
);
// VMCAGetCertificateAsString is a helper function that
// returns certificate as String that can be printed on
//  screen.
// Arguments :
//      pCertificate : Pointer to Certificate object
//      ppszCertString : returns an allocated printable certificate
// Returns :
//  Error Code

DWORD
VMCAGetRootCACertificateA(
    PCSTR pszServerName,
    PVMCA_CERTIFICATE* ppCertificate
);

DWORD
VMCAGetRootCACertificateW(
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE* ppCertificate
);

DWORD
VMCAGetRootCACertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_CERTIFICATE* ppCertificate
);

DWORD
VMCAGetRootCACertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pszServerName,
    PVMCA_CERTIFICATE* ppCertificate
);

// VMCAGetRootCACertificate gets the Root CA certificate
// that is being used by the VMware Certificate Authority.
//
// Arguments :
//      pszServerName : VMCA Server Name
//      dwCertLength  : Number of Certificates returned in the Container, it will 1 for this call.
//      ppCertContainer : Pointer to a array of CertContainers
// Returns:
//  Error Code

DWORD
VMCASetServerOptionA(
    PCSTR pszServerName,
    unsigned int dwOption
    );

DWORD
VMCASetServerOptionW(
    PCWSTR pwszServerName,
    unsigned int dwOption
    );

DWORD
VMCASetServerOptionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    unsigned int dwOption
    );

DWORD
VMCASetServerOptionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    unsigned int dwOption
    );

// VMCASetServerOption sets server option to enable certain behaviors
//
// Arguments :
//      pszServerName : VMCA Server Name
//      dwOption : Bit flag of options which you want to enable
// Returns:
//  Error Code

DWORD
VMCAUnsetServerOptionA(
    PCSTR pszServerName,
    unsigned int dwOption
    );

DWORD
VMCAUnsetServerOptionW(
    PCWSTR pwszServerName,
    unsigned int dwOption
    );

DWORD
VMCAUnsetServerOptionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    unsigned int dwOption
    );

DWORD
VMCAUnsetServerOptionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    unsigned int dwOption
    );

// VMCAUnsetServerOption unsets server option to disable certain behaviors
//
// Arguments :
//      pszServerName : VMCA Server Name
//      dwOption : Bit flag of options which you want to disable
// Returns:
//  Error Code

DWORD
VMCAGetServerOptionA(
    PCSTR pszServerName,
    unsigned int *pdwOption
    );

DWORD
VMCAGetServerOptionW(
    PCWSTR pwszServerName,
    unsigned int *pdwOption
    );

DWORD
VMCAGetServerOptionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    unsigned int *pdwOption
    );

DWORD
VMCAGetServerOptionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    unsigned int *pdwOption
    );

// VMCAGetServerOption gets the current server option settings (enabled/disabled)
// that is being used by the VMware Certificate Authority.
//
// Arguments :
//      pszServerName : VMCA Server Name
//      pdwOption : Bit flag of options which are currently enabled/disabled
// Returns:
//  Error Code

DWORD
VMCAGetServerVersionA(
    PCSTR pszServerName,
    PSTR* ppszServerVersionString
);

DWORD
VMCAGetServerVersionW(
    PCWSTR pszServerName,
    PSTR* ppszServerVersionString
);

DWORD
VMCAGetServerVersionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PSTR* ppszServerVersionString
);

DWORD
VMCAGetServerVersionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pszServerName,
    PSTR* ppszServerVersionString
);

// VMCA Get Server Version returs a Server Version String from the Server
//
// Arguments :
//      pszServerName : Name of the Server to talk to
//      ppszServerVersionString : Allocates a Server Version String
//  Returns :
//      Error Code


DWORD
VMCAGetSignedCertificateA(
    PCSTR pszServerName,
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    PVMCA_KEY pPrivateKey,
    PSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
);

DWORD
VMCAGetSignedCertificateW(
    PCWSTR pwszServerName,
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PVMCA_KEY pPrivateKey,
    PWSTR pwszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
);

DWORD
VMCAGetSignedCertificateHA(
    PVMCA_SERVER_CONTEXT bInBinding,
    PCSTR pszServerName,
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    PVMCA_KEY pPrivateKey,
    PSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
);

DWORD
VMCAGetSignedCertificateHW(
    PVMCA_SERVER_CONTEXT bInBinding,
    PCWSTR pwszServerName,
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PVMCA_KEY pPrivateKey,
    PWSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
);
// VMCAGetSignedCertificate this function allows the
// users to create a certificate.
// Arguments :
//  pCertRequest : The Structure that defines the fields for the certificate.
//  pszPrivateKey : Private Key for the Certificate
//  pszPassPhrase : optional Pass Phrase for the Private Key
//  ppszCertificate  : returNs an allocated certificate
//
// Returns :
//  Error Code
//

DWORD
VMCAGetSignedCertificateFromCSRA(
    PCSTR pszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
);

DWORD
VMCAGetSignedCertificateFromCSRW(
    PCWSTR pwszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
);

DWORD
VMCAGetSignedCertificateFromCSRHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
);

DWORD
VMCAGetSignedCertificateFromCSRHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
);

// VMCAGetSignedCertificate function allows the user to take
// PKCS#10 Certificate Signing Request and send it to the
// Certitifacate server to get a Signed Certificate back.
//
// Arguments :
//      pszServerName : The Name of the Certificate Server that
//      user wants to communicate with.
//      pCertRequest : Pointer to CSR
//      tmNotBefore  : Time when the certificate is valid from
//      tmNotAfter   : Time to the certificate is valid to
//      dwCertLength : Number of Certificates returned in the Container, it will 1 for this call.
//      ppCertContainer : The Actual Certificate
//  Returns :
//      Error Code
//
// NOTE : This functions arguments *might* change in the next drop of VMCA

DWORD
VMCAReadCertificateFromFile(
    PSTR pszFileName,
    PVMCA_CERTIFICATE* ppCertificate
);
// VMCAReadCertificateFromFile reads a Certificate from a file
//
// Arguments :
//  pszFilename : Full file Path to the certificate File
//  ppCertificate : Certificate, Caller to free this certificate after use
// Returns :
//  Error Code

DWORD
VMCAReadPrivateKeyFromFile(
    PSTR pszFileName,
    PSTR pszPassPhrase,
    PVMCA_KEY* ppPrivateKey
);
// VMCAReadPrivateKeyFromFile reads a private key from a file
//
// Arguments :
//  pszFilename : Full file path to the private key file
//  pszPassPhrase : Optional password to protect the Private Key
//  ppPrivateKey : Private key, Caller to free key after use
// Returns :
//  Error Code


DWORD
VMCAReadPublicKeyFromFile(
    PSTR pszFileName,
    PVMCA_KEY* ppPublicKey
);
// VMCAReadCertificateFromFile reads a Certificate from a file
//
// Arguments :
//  pszFilename : Full file Path to the certificate File
//  ppPublicKey : Public Key , Caller to free this public key after use
// Returns :
//  Error Code


/*
 * @brief Revokes a certificate
 *
 * @param[in] pszServerName CA Server Name
 * @param[in] pszCertificate Certificate that needs to be revoked
 *
 * @return Returns 0 for success
 */
DWORD
VMCARevokeCertificateA(
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszCertificate
);

DWORD
VMCARevokeCertificateW(
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE pszCertificate
);

/*
 * @brief Revokes a certificate
 *
 * @param[in] hInBinding Binding Handle
 * @param[in] pszServerName CA Server Name
 * @param[in,out] pszCertificate Certificate that needs to be revoked
 *
 * @return Returns 0 for success
 */
DWORD
VMCARevokeCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszCertificate
);

DWORD
VMCARevokeCertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pszServerName,
    PVMCA_CERTIFICATE pszCertificate
);

/*
 * @brief Revokes a certificate
 *
 * @param[in] hInBinding Binding Handle
 * @param[in] pszServerName CA Server Name
 * @param[in,out] pszCertificate Certificate that needs to be revoked
 *
 * @return Returns 0 for success
 */
DWORD
VMCARevokeCertA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszCertificate,
    VMCA_CRL_REASON certRevokeReason
);

/*
 * @brief Revokes a certificate
 *
 * @param[in] hInBinding Binding Handle
 * @param[in] pszServerName CA Server Name
 * @param[in,out] pszCertificate Certificate that needs to be revoked
 *
 * @return Returns 0 for success
 */
DWORD
VMCARevokeCertW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pszServerName,
    PVMCA_CERTIFICATE pszCertificate,
    VMCA_CRL_REASON certRevokeReason
);


// VMCARevokeCertificate function revokes an existing certificate.
//
// Arguments :
//      pszServerName : CA Server Name
//      pCertificate : Certificate that needs to be revoked
// Returns :
//      Error Code
//
// NOTE : This API will change when we integrate with User Identity provided
// by Lotus.


DWORD
VMCASetCertValueA(
    VMCA_OID Field,
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    PSTR pwszNewValue
);

DWORD
VMCASetCertValueW(
    VMCA_OID Field,
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PWSTR pwszNewValue
);
// This function Sets the member value of a PKCS_10_REQ,
// if member is pointing to something valid, then it is freed,
// before allocating and copying new structure
//
// Args :
//
//      ppszMemeber - Pointer Pointer to the Memeber variable
//      pszNewValue - Pointer to the NewValue to be set, setting NULL
//                    frees the old object and initializes the pointer to NULL.
//
// Returns :
//  None

DWORD
VMCASetKeyUsageConstraintsA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    DWORD dwKeyUsageMask
);

DWORD
VMCASetKeyUsageConstraintsW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    DWORD dwKeyUsageMask
);
// This function Sets the member value of a PKCS_10_REQ,
// if member is pointing to something valid, then it is freed,
// before allocating and copying new structure
//
// Args :
//      ppszMemeber - Pointer Pointer to the Memeber variable
//      dwKeyUsageMask - Tells us what all things the Key can be used
//                      for in the certificate. The legal values are defined
//                      in the KeyUsage Enum. User can OR those values for the Certificate
//
// Returns :
//  None



DWORD
VMCAValidateCACertificate(
    PVMCA_CERTIFICATE pszCertificate
);
//  VMCAValidateCACertificate checks if the given Certificate has
//  the capability to be a CA Cert
//  Arguments:
//      pCertificate :  Pointer to Certificate
//  Returns :
//      Error code

DWORD
VMCAWriteCertificateToFile(
    PSTR pszCertificateFileName,
    PVMCA_CERTIFICATE pCertificate
);
// VMCAWriteCertificateToFile writes a certificate to
// given file name.
//
// Arguments :
//  pszCertificateFileName : Full path to certificate file Name
//  pCertificate : Pointer to certificate that we want to write
// Returns
//   Error Code
//

DWORD
VMCAWritePrivateKeyToFile(
    PSTR pszPrivateKeyFileName,
    PVMCA_KEY pPrivateKey,
    PSTR pszPassPhraseFileName,
    PWSTR pszPassPhrase
);
// VMCAWritePrivateKeyToFile write a private key and a password to files.
//
// Arguments :
//  pszPrivateKeyFileName : Full file Path to the where private key will be written to
//  pPrivateKey : Pointer to  Private Key
//  pszPassPhraseFileName : Optional Password File Name, if specified the Password will also be stored
//  pszPassPhrase : Password that protects the PrivateKey
// Returns :
//  Error Code


DWORD
VMCAWritePublicKeyToFile(
    PSTR pszPublicKeyFileName,
    PVMCA_KEY pPublicKey
);
// VMCAWritePublicKeyToFile writes a public key to a file.
//
// Arguments :
//      pszPublicKeyFileName : Publci Key File Name
//      pPublicKey : Public Key to write
//  Returns
//       Code



//
// These API's might go away
//
//
DWORD
VMCAInitPKCS10DataA(
    PCSTR pszName,
    PCSTR pszOrganization,
    PCSTR pszOU,
    PCSTR pszState,
    PCSTR pszCountry,
    PCSTR pszEmail,
    PCSTR pszIPAddress,
    PVMCA_PKCS_10_REQ_DATAA pCertRequestData
);

DWORD
VMCAInitPKCS10DataWithDCA(
    PCSTR pszName,
    PCSTR pszDomainName,
    PCSTR pszOrganization,
    PCSTR pszOU,
    PCSTR pszState,
    PCSTR pszCountry,
    PCSTR pszEmail,
    PCSTR pszIPAddress,
    PVMCA_PKCS_10_REQ_DATAA pCertRequestData
    );

// This function initilizes the pCertRequstData with appropriate
// values for the certificate. Most of these values are Optional
// other than Subject and Country. We need that in the Certificate
// for it to be a vaild certificate.
//
// Arguments :
//     The Names of the Arguments indicate what it is, Please
//     look up a certificate defintion tounderstand what they
//     mean.
//
//     Country is a 2 CHAR country code, like US, IN etc.
//     if it is anything other than 2 CHARs this function will
//     fail.
//
// Returns :
//     Error Code

DWORD
VMCAInitPKCS10DataW(
    PCWSTR pwszName,
    PCWSTR pwszOrganization,
    PCWSTR pwszOU,
    PCWSTR pwszState,
    PCWSTR pwszCountry,
    PCWSTR pwszEmail,
    PCWSTR pwszIPAddress,
    PVMCA_PKCS_10_REQ_DATAW pCertRequestData
);

DWORD
VMCAInitPKCS10DataWithDCW(
    PCWSTR pwszName,
    PCWSTR pwszDomainName,
    PCWSTR pwszOrganization,
    PCWSTR pwszOU,
    PCWSTR pwszState,
    PCWSTR pwszCountry,
    PCWSTR pwszEmail,
    PCWSTR pwszIPAddress,
    PVMCA_PKCS_10_REQ_DATAW pCertRequestData
    );

// This function initilizes the pCertRequstData with appropriate
// values for the certificate. Most of these values are Optional
// other than Subject and Country. We need that in the Certificate
// for it to be a vaild certificate.
//
// Arguments :
//     The Names of the Arguments indicate what it is, Please
//     look up a certificate defintion to understand what they
//     mean.
//
//     Country is a 2 CHAR country code, like US, IN etc.
//     if it is anything other than 2 CHARs this function will
//     fail.
//
// Returns :
//     Error Code


DWORD
VMCAFreeVersion(PSTR version);
//
// This function frees a version string, this is useful
// for writing code in Python and Java
//

DWORD
VMCAGetDefaultDomainName(
     PSTR pszHostName,
     DWORD dwPort,
     PSTR* ppDomainName);

// VMCAGetDefaultDomainName talks to the LDAP server
// and retrives the Default Domain Name for this Server
// HostName == NULL means it will connect to localhost
// Port ==0 means it will use default LDAP Port


DWORD
VMCACheckLdapConnection(
    PSTR pszHostName,
    DWORD dwPort);
//
// VMCACheckLdapConnection simply opens a connection to LDAP Server
// which allows certool to proceed if VMDIR is running.
// Please see HandleVMDIRWait to see how this function is used.
//


DWORD
VMCAVerifyCertificateA(
    PCSTR pszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
);

DWORD
VMCAVerifyCertificateW(
    PCWSTR pwszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
);

DWORD
VMCAVerifyCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
);

DWORD
VMCAVerifyCertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
);

VOID
VMCAFreeStringA(PSTR pszString);
//
// this function frees a string, useful for python and java
//


DWORD
VMCAWritePKCS12(
    PSTR pszFileName,
    PSTR pszFriendlyName,
    PSTR pszPassword,
    PSTR pCertificate,
    PSTR privateKey,
    PSTR *ppCACerts,
    int uCACertCount);


DWORD
VMCAGetCertificateCountA(
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD *dwNumCertificates
);

DWORD
VMCAGetCertificateCountW(
    PCWSTR pwszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD *dwNumCertificates
);

DWORD
VMCAGetCertificateCountHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD *dwNumCertificates
);

DWORD
VMCAGetCertificateCountHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD *dwNumCertificates
);

DWORD
VMCAOpenServerA(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMCA_SERVER_CONTEXT *pServerContext
    );

DWORD
VMCAOpenServerW(
    PCWSTR pszNetworkAddress,
    PCWSTR pszUserName,
    PCWSTR pszDomain,
    PCWSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMCA_SERVER_CONTEXT *pServerContext
    );

DWORD
VMCACloseServer(
    PVMCA_SERVER_CONTEXT pServerContext
    );

DWORD
VMCAGetCRLA(
    PCSTR pszServerName,
    PSTR pszExistingCRLFileName,
    PSTR pszNewCRLFileName
    );

DWORD
VMCAGetCRLW(
    PCWSTR pwszServerName,
    PWSTR pszExistingCRLFileName,
    PWSTR pszNewCRLFileName
    );

DWORD
VMCAGetCRLHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PSTR pszExistingCRLFileName,
    PSTR pszNewCRLFileName
    );

DWORD
VMCAGetCRLHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PWSTR pszExistingCRLFileName,
    PWSTR pszNewCRLFileName
    );

// VMCAGetCRL returns a CRL from the VMCA Server,
// it takes FileName of the Existing CRL, and sends
// the Unique ID to VMCA server to check if an new
// CRL exists, if it does, it is downloaded from the Server.
// Arguments :
//
//      pszServerName : Machine where VMCA is running
//      pszExistigCRLFileName : File Name to existing CRL or NULL
//      pszNewCRLFileName : The File where newly downloaded CRL will
//      be stored.


DWORD
VMCAReGenCRLA(
    PCSTR pszServerName
    );

DWORD
VMCAReGenCRLW(
    PCWSTR pwszServerName
    );

DWORD
VMCAReGenCRLHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName
    );

DWORD
VMCAReGenCRLHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName
    );
// VMCA Re-GenCRL allows Admins to regenerate CRL if needed.
// Arguments :
// pszServerName : VMCA Server Name.


DWORD
VMCAGetCRLInfo(
    PSTR pszFileName,
    DWORD *ptmLastUpdate,
    DWORD *ptmNextUpdate,
    DWORD *pdwCRLNumber);
// VMCAGetCRLInfo retrieves the Meta-data from a CRL.
// Arguments :
// pszFileName : CRL file Name
// ptmLastUpdate : Will return time - 32 bit value in UTC
// ptmNextUpdate : will return time - 32 bit value in UTC
// pdwCRLNumber  : will return CRL number for this CRL

DWORD
VMCAGetCRLInfo2(
    PSTR pszFileName,
    time_t *ptmLastUpdate,
    time_t *ptmNextUpdate,
    DWORD *pdwCRLNumber);
// VMCAGetCRLInfo retrieves the Meta-data from a CRL.
// Arguments :
// pszFileName : CRL file Name
// ptmLastUpdate : Will return time - 32 bit value in UTC
// ptmNextUpdate : will return time - 32 bit value in UTC
// pdwCRLNumber  : will return CRL number for this CRL


DWORD
VMCAGetShortError(
    DWORD dwError,
    PSTR *pszErrMsg);
// VMCAGetShortError returns a String which
// describes the Error Code as a String. This is
// supported since SMS folks want to localize based
// on some Error String. This will help the SMS localization Effort
//
// Arguments :
// dwError : VMCA Error code
// pszErrMsg : The error message define, something like
// VMCA_ROOT_CA_MISSING

DWORD
VMCAGetErrorString(
    DWORD dwError,
    PSTR *pszErrMsg);
// VMCAGetError returns the Error string that describes the
// Error.
// Arguments :
// dwError :  VMCA Error Code
// pszErrorMsg : The human readable Error Message , something like
// "The Root CA certificate is missing or failed to Initialize"


DWORD
VMCAPrintCRL(
    PSTR pszFileName,
    PSTR *ppszCRLString
    );
// VMCA Print CRL returns CRL as a String, this
// Function is useful if you want to print the CRL
// In human readable format.


DWORD
VMCAPublishRootCertsA(
    PCSTR pszServername
    );

DWORD
VMCAPublishRootCertsW(
    PCWSTR pwszServername
    );

DWORD
VMCAPublishRootCertsHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServername
    );

DWORD
VMCAPublishRootCertsHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServername
    );

DWORD
VMCAGetCSRFromCertificate(
    PVMCA_CERTIFICATE pszCertificate,
    PVMCA_KEY pszPrivateKey,
    PVMCA_CSR *ppszCSR
    );

DWORD
VMCAGetSignedCertificateForHostA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszHostName,
    PCSTR pszHostIp,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
    );

DWORD
VMCAGetSignedCertificateForHostW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pszHostName,
    PCWSTR pszHostIp,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
    );

// VMCAPublishRootCerts publishes the root certificate to
// the Lotus directory service.
#ifdef UNICODE
#define VMCA_PKCS_10_REQ_DATA               VMCA_PKCS_10_REQ_DATAW
#define PVMCA_PKCS_10_REQ_DATA              PVMCA_PKCS_10_REQ_DATAW
#define VMCAAddRootCertificate              VMCAAddRootCertificateW
#define VMCAAddRootCertificateH             VMCAAddRootCertificateHW
#define VMCAAllocatePKCS10Data              VMCAAllocatePKCS10DataW
#define VMCACreatePrivateKey                VMCACreatePrivateKeyW
#define VMCACreateSelfSignedCertificate     VMCACreateSelfSignedCertificateW
#define VMCACreateSigningRequest            VMCACreateSigningRequestW
#define VMCAOpenEnumContext                 VMCAOpenEnumContextW
#define VMCAOpenEnumContextH                VMCAOpenEnumContextHW
#define VMCAFreePKCS10Data                  VMCAFreePKCS10DataW
#define VMCAGetCertificateAsString          VMCAGetCertificateAsStringW
#define VMCAGetRootCACertificate            VMCAGetRootCACertificateW
#define VMCAGetRootCACertificateH           VMCAGetRootCACertificateHW
#define VMCASetServerOption                 VMCASetServerOptionW
#define VMCASetServerOptionH                VMCASetServerOptionHW
#define VMCAUnsetServerOption               VMCAUnsetServerOptionW
#define VMCAUnsetServerOptionH              VMCAUnsetServerOptionHW
#define VMCAGetServerOption                 VMCAGetServerOptionW
#define VMCAGetServerOptionH                VMCAGetServerOptionHW
#define VMCAGetServerVersion                VMCAGetServerVersionW
#define VMCAGetServerVersionH               VMCAGetServerVersionHW
#define VMCAGetSignedCertificate            VMCAGetSignedCertificateW
#define VMCAGetSignedCertificateH           VMCAGetSignedCertificateHW
#define VMCAGetSignedCertificateFromCSR     VMCAGetSignedCertificateFromCSRW
#define VMCAGetSignedCertificateFromCSRH    VMCAGetSignedCertificateFromCSRHW
#define VMCARevokeCertificate               VMCARevokeCertificateW
#define VMCARevokeCert                      VMCARevokeCertW
#define VMCARevokeCertificateH              VMCARevokeCertificateHW
#define VMCAVerifyCertificate               VMCAVerifyCertificateW
#define VMCAVerifyCertificateH              VMCAVerifyCertificateHW
#define VMCASetCertValue                    VMCASetCertValueW
#define VMCASetKeyUsageConstraints          VMCASetKeyUsageConstraintsW
#define VMCAOpenServer                      VMCAOpenServerW
#define VMCAGetCRL                          VMCAGetCRLW
#define VMCAReGenCRL                        VMCAReGenCRLW
#define VMCAPublishRootCerts                VMCAPublishRootCertsW
#define VMCAInitPKCS10Data                  VMCAInitPKCS10DataW
#define VMCAInitPKCS10Data                  VMCAInitPKCS10DataW
#define VMCAGetCertificateCount             VMCAGetCertificateCountW
#define VMCAGetSignedCertificateForHost     VMCAGetSignedCertificateForHostW
#else
#define VMCA_PKCS_10_REQ_DATA               VMCA_PKCS_10_REQ_DATAA
#define PVMCA_PKCS_10_REQ_DATA              PVMCA_PKCS_10_REQ_DATAA
#define VMCAAddRootCertificate              VMCAAddRootCertificateA
#define VMCAAddRootCertificateH             VMCAAddRootCertificateHA
#define VMCAAllocatePKCS10Data              VMCAAllocatePKCS10DataA
#define VMCACreatePrivateKey                VMCACreatePrivateKeyA
#define VMCACreateSelfSignedCertificate     VMCACreateSelfSignedCertificateA
#define VMCACreateSigningRequest            VMCACreateSigningRequestA
#define VMCAOpenEnumContext                 VMCAOpenEnumContextA
#define VMCAOpenEnumContextH                VMCAOpenEnumContextHA
#define VMCAFreePKCS10Data                  VMCAFreePKCS10DataA
#define VMCAGetCertificateAsString          VMCAGetCertificateAsStringA
#define VMCAGetRootCACertificate            VMCAGetRootCACertificateA
#define VMCAGetRootCACertificateH           VMCAGetRootCACertificateHA
#define VMCASetServerOption                 VMCASetServerOptionA
#define VMCASetServerOptionH                VMCASetServerOptionHA
#define VMCAUnsetServerOption               VMCAUnsetServerOptionA
#define VMCAUnsetServerOptionH              VMCAUnsetServerOptionHA
#define VMCAGetServerOption                 VMCAGetServerOptionA
#define VMCAGetServerOptionH                VMCAGetServerOptionHA
#define VMCAGetServerVersion                VMCAGetServerVersionA
#define VMCAGetServerVersionH               VMCAGetServerVersionHA
#define VMCAGetSignedCertificate            VMCAGetSignedCertificateA
#define VMCARevokeCert                      VMCARevokeCertA
#define VMCAGetSignedCertificateH           VMCAGetSignedCertificateHA
#define VMCAGetSignedCertificateFromCSR     VMCAGetSignedCertificateFromCSRA
#define VMCAGetSignedCertificateFromCSRH    VMCAGetSignedCertificateFromCSRHA
#define VMCARevokeCertificate               VMCARevokeCertificateA
#define VMCARevokeCertificateH              VMCARevokeCertificateHA
#define VMCAVerifyCertificate               VMCAVerifyCertificateA
#define VMCAVerifyCertificateH              VMCAVerifyCertificateHA
#define VMCASetCertValue                    VMCASetCertValueA
#define VMCASetKeyUsageConstraints          VMCASetKeyUsageConstraintsA
#define VMCAOpenServer                      VMCAOpenServerA
#define VMCAGetCRL                          VMCAGetCRLA
#define VMCAReGenCRL                        VMCAReGenCRLA
#define VMCAPublishRootCerts                VMCAPublishRootCertsA
#define VMCAInitPKCS10Data                  VMCAInitPKCS10DataA
#define VMCAGetCertificateCount             VMCAGetCertificateCountA
#define VMCAGetSignedCertificateForHost     VMCAGetSignedCertificateForHostA
#endif

#ifdef __cplusplus
}
#endif


#endif // __VMCA_H__

