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

#ifndef _VMCACLIENT_H_
#define _VMCACLIENT_H_


// Helper Functions


// DWORD
// VMCAWritePublicKeyToFile(
//     LPSTR pszPublicKeyFileName,
//     LPSTR pszPublicKey
// );

// VOID
// VMCAFreeCertificateContainer(
//     PVMCA_CERTIFICATE_CONTAINER pCertContainer
// );

// DWORD
// VMCAAllocatePKCS10Data(
//   PVMCA_PKCS_10_REQ_DATA* pCertRequestData
// );
// // VMCAAllocatePKCS10Data function allocates a VMCA specific structure
// // which carries the payload for creating a certificate
// // request. This function allocates and returns a zero
// // structure. Please see the definition of VMCA_PKCS_10_REQ_DATA
// // for more details
// //
// // Arguments :
// //         Pointer-Pointer to the Structure to be Allocated
// // Returns :
// //     Error code


typedef struct _VMCA_ENUM_CONTEXT
{
    int iChunkSize;
    int iCurrentUserIndex;
    int iFilterMask;
    int iCurrentServerIndex;
    PVMCA_CERTIFICATE_ARRAY pCertChain;
    PWSTR pwczServerName;
    BOOLEAN bAllocatedHandle;
    handle_t hInBinding;
}VMCA_ENUM_CONTEXT, *PVMCA_ENUM_CONTEXT;


DWORD
VMCAInitPKCS10DataAnsi(
    PCSTR pszName,
    PCSTR pszOrganization,
    PCSTR pszOU,
    PCSTR pszState,
    PCSTR pszCountry,
    PCSTR pszEmail,
    PCSTR pszIPAddress,
    PVMCA_PKCS_10_REQ_DATAW pCertRequestData
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
VMCAInitPKCS10DataUnicode(
    PCWSTR pwszName,
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

// VOID
// VMCAFreePKCS10Data(
//     PVMCA_PKCS_10_REQ_DATA pCertRequestData
// );
// // VMCAFreePKCS10Data function frees the object
// //
// // Arguments :
// //          pCertRequestData :  Pointer to the memory that is to be freed.
// // Returns :
// //      none
// //

// DWORD
// VMCAAllocatePrivateKey(
//     PSTR pszPassPhrase,
//     size_t uiKeyLength,
//     PVMCA_KEY* ppPrivateKey,
//     PVMCA_KEY* ppPublicKey
// );
// // VMCAAllocatePrivateKey function creates private-public key pair  and retuns them to user
// //
// // Arguments :
// //          pszPassPhrase   : Optional Pass Word to protect the Key
// //          uiKeyLength     : Key Length - Valid values are between 1024 and 16384
// //          ppPrivateKey    : PEM encoded Private Key String
// //          ppPublicKey     : PEM encoded Public Key String.
// //
// // Returns :
// //      Error Code
// //
// // Notes : This function makes some assumptions on the users
// // behalf. One of them is that assumption on bit size. This is based on RSA's
// // recommendation http://www.rsa.com/rsalabs/node.asp?id=2218 on
// // Corporate Key lengths.


// VOID
// VMCAFreeKey(
//     PVMCA_KEY pKey
// );
// // VMCAFreeKey frees the Key Objects allocated by the VMCAAllocatePrivateKey
// //
// // Arguments :
// //      pKey : Points to the key that is to be freed.
// // Returns :
// //      Error Code
// //



// DWORD
// VMCAFreeCSR(
//     PVMCA_CSR pCSR
// );
// // VMCAFreeCSR frees the Key Objects allocated by the VMCACreateSigningRequest
// //
// // Arguments :
// //      pCSR : Points to the CSR that is to be freed.
// // Returns :
// //      Error Code
// //



// DWORD
// VMCACreateSigningRequest(
//     PVMCA_PKCS_10_REQ_DATA pCertRequest,
//     PVMCA_KEY pszPrivateKey,
//     PSTR pszPassPhrase,
//     PVMCA_CSR* pAllocatedCSR
// );
// // VMCACreateSigningRequest creates a CSR from the user provided parameters
// // and allocates a CSR that can be send to the CA for purposes of getting a Signed
// // Certificate.
// // Argumemnts :
// //      pCertRequest : User parameters that go into the certificate
// //      pszPrivateKey : The Private Key needed for the Certificate
// //      pszPassPhrase : Optional Pass Phrase that is protecting the certificate
// //      pAllocatedCSR : Allocated CSR that can be send to Server for Signing.
// //  Returns:
// //      Error Code


// DWORD
// VMCACreateSelfSignedCertificate(
//     PVMCA_PKCS_10_REQ_DATA pCertRequest,
//     PSTR pszPrivateKey,
//     PSTR pszPassPhrase,
//     int  bCreateSelfSignedRootCA,
//     PSTR* ppszCertificate
// );

//Core CA Functons


DWORD
VMCAInitEnumCertsHandle(
    RP_PCSTR pszServerName,
    PDWORD   pdwHandle
);


// DWORD
// VMCAGetSignedCertificate(
//     PCSTR pszServerName,
//     PVMCA_CSR pszPEMEncodedCSRRequest,
//     DWORD dwValidFrom,
//     DWORD dwDurationInSeconds,
//     DWORD *dwCertLength,
//     PVMCA_CERTIFICATE_CONTAINER *ppCertContainer
// );
// VMCAGetSignedCertificate function allows the user to take
// PKCS#10 Certificate Signing Request and send it to the
// Certitifacate server to get a Signed Certificate back.
//
// Arguments :
//      pszServerName : The Name of the Certificate Server that
//      user wants to communicate with.
//      dwCSRLength : The Length of the CSR Blob that is being send to the
//      CA Server
//      pszPEMEncodedCSRRequest : As the name implies, this is a PEM encoded
//      CSR Request.
//      dwCertLength : This is the length the Certificate returned from the
//      Server
//      pszPEMEncodedCertificate : The Actual Certificate that is encoded in PEM
//      format.
//  Returns :
//      Error Code
//
// DWORD
// VMCARevokeCertificate(
//     PCSTR pszServerName,
//     PSTR pszCertificate
// );
// VMCARevokeCertificate function revokes an existing certificate.
// Please Note : This API will change when we integrate with User Identity provided
// by Lotus.
//
// Arguments :
//      pszServerName : CA Server Name
//      dwCertLength : Length of the Certificate String
//      pszPEMEncodedCertificate : The PEM Encoded Certificate that needs to be revoked
//      pszPassPhrase : The Pass Phrase used to protect the certificate
//      dwStatus : return value from the Server indicating what happened, if the revoke was done or not
// Returns :
//      Error Code -

// DWORD
// VMCAVerifyCertificate(
//     PCSTR pszServerName,
//     DWORD dwCertLength,
//     PCSTR pszPEMEncodedCertificate,
//     DWORD  *dwStatus
// );
// VMCAVerifyCertificate function allows the user verify a certificate
// by talking to the CA Server.
//
// Arguments :
//      pszServerName : CA Server Name
//      dwCertLength : Length of the Certificate String
//      pszPEMEncodedCertificate : The PEM Encoded Certificate that needs to be verified
//      dwStatus : return value from the Server indicating if the Server is good or not
// Returns :
//      Error Code -



DWORD
VMCAFindCertificates(
    PCSTR pszServerName,
    DWORD  dwSearchQueryLength,
    PCSTR pszSearchQuery,
    DWORD dwMaxCount,
    DWORD  *dwCertificateCount,
    VMCA_CERTIFICATE_CONTAINER ** ppCertContainer
);
// VMCAFindCertificates is allows the user to specify a search query condition and
// all certificates which match that condition are retrived from the Server.
//
// Arguments :
//      pszServerName : CA Server Name
//      dwSearchQueryLength : The Length of the Search Query
//      pszSearchQuery : The Actual Search Query
//      dwMaxCount : Maximum number of certificates that you want in reply, a Zero would allow us to
//      find out how many certificates match the query.
//      dwCertificateCount : The Number of Certificates found that matches the query
//      ppCertContainer : The actual Certificates that match the query
// Returns:
//        Error Code
// Notes :
//  An example of Search Query would be something like "COUNTRY=US;NAME=Anu"
// in that case server would find certificates which have attributes where country
// is US and NAME is Anu. This API is still evolving so some extended attributes
// may not be supported yet in the Query Syntax.
//


// DWORD
// VMCAEnumCerts(
//     PCSTR pszServerName,
//     CERTIFICATE_STATUS dwStatus,
//     DWORD    dwStartIndex,
//     DWORD    dwNumCertificates,
//     PVMCA_CERTIFICATE_ARRAY *ppCertContainer
// );
// VMCAEnumCerts allows the user to enumerate thru all certificates
// issued by the CA Server.
//
// Arguments :
//      pszServerName : CA Server Name
//      dwStartIndex : The Index from where you want the certificates returned
//      dwNumPackages : The number of certificates needed
//      ppCertContiner : Actual Certificates
// Returns :
//  Error Code
//

VOID
VMCASetPKCSMemberA(
    PSTR *ppszMember,
    PCSTR pszNewValue
);

VOID
VMCASetPKCSMemberW(
    PWSTR *ppszMember,
    PCWSTR pszNewValue
);

// This function Sets the member value of a PKCS_10_REQ,
// if member is pointing to something valid, then it is freed,
// before allocating and copying new structure
//
// Args :
//  ppszMemeber - Pointer Pointer to the Memeber variable
//  pszNewValue - Pointer to the NewValue to be set, setting NULL
//  frees the old object and initializes the pointer to NULL.
//
// Returns :
//  None


DWORD
VMCAJavaGenCertA(
    PSTR          pszServerName,
    PSTR          pszName,
    PSTR          pszCountry,
    PSTR          pszLocality,
    PSTR          pszState,
    PSTR          pszOrganization,
    PSTR          pszOU,
    PSTR          pszDNSName,
    PSTR          pszURIName,
    PSTR          pszEmail,
    PSTR          pszIPAddress,
    DWORD         dwKeyUsageConstraints,
    DWORD         dwSelfSigned,
    PVMCA_KEY     pPrivateKey,
    time_t        tmNotBefore,
    time_t        tmNotAfter,
    PVMCA_CERTIFICATE *ppCertificate);

// DWORD
// VMCAGetServerVersion(
//     PCSTR pszServerName,
//     DWORD *dwCertLength,
//     VMCA_CERTIFICATE_CONTAINER **pServerVersion
// );
// VMCA Get Server Version returs a Server Version String from the Server
//
// Arguments :
//      pszServerName : Name of the Server to talk to
//      dwCertLength : The  number of Server Version Strings
//      pServerVersion : since certificate is string using the same structure for
//      our purpose


// DWORD
// VMCAAddRootCertificate(
//     PCSTR pszServerName,
//     PVMCA_CERTIFICATE pszRootCertificate,
//     PSTR pszPassPhrase,
//     PVMCA_KEY pszPrivateKey
// );


// DWORD
// VMCAGetRootCACertificate(
//     PCSTR pszServerName,
//     DWORD *dwCertLength,
//     VMCA_CERTIFICATE_CONTAINER ** ppCertContainer
// );


#endif // _VMCACLIENT_H_
