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
 * Module Name:
 *
 * prototypes.h
 *
 * Abstract:
 *
 * Function prototypes definition.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef char* PVMCA_CSR;
typedef char* PVMCA_KEY;
typedef char* PVMCA_CERTIFICATE;

typedef enum
{
    VMCA_ADMINISTRATORS,
    VMCA_USERS
} VMCA_USER_TYPE;

/* dirsync.c */

DWORD
VMCASrvDirSyncInit(
    VOID
    );

PVMCA_DIR_SYNC_PARAMS
VMCASrvAcquireDirSyncParams(
	PVMCA_DIR_SYNC_PARAMS pSyncInfo
	);

VOID
VMCASrvReleaseDirSyncParams(
	PVMCA_DIR_SYNC_PARAMS pSyncInfo
	);

VOID
VMCASrvDirSyncShutdown(
    VOID
    );

DWORD
VMCASrvNotifyDirSync(
    VOID
    );

/* init.c */

DWORD
VMCAInitialize(
    BOOL overrideLogLevel,
    BOOL overrideLogType
    );

DWORD
VMCASrvInitCA(
	VOID
	);

VOID
VMCAShutdown(
    VOID
    );

#if 0
DWORD
VMCACreateCA(
    PSTR pszCACertificate,
    PSTR pszPrivateKey,
    PSTR pszPassPhrase,
    PVMCA_X509_CA *pCA
)
VOID
VMCAFreeKey(
PVMCA_KEY pKey
);

#endif

/* state.c */

VOID
VMCASrvSetState(
    VMCA_SERVER_STATE state
    );

VMCA_SERVER_STATE
VMCASrvGetState(
    VOID
    );

VOID
VMCASrvSetFuncLevel(
	VMCA_FUNC_LEVEL dwFuncLevel
    );

VMCA_FUNC_LEVEL
VMCASrvGetFuncLevel(
    VOID
    );

DWORD
VMCASrvSetCA(
	PVMCA_X509_CA pCA
	);

DWORD
VMCASrvValidateCA(
	VOID
	);

DWORD
VMCASrvGetCA(
	PVMCA_X509_CA* ppCA
	);

PVMCA_DIR_SYNC_PARAMS
VMCASrvGetDirSyncParams(
	VOID
	);

PVMCA_THREAD
VMCASrvGetDirSvcThread(
	VOID
	);

VOID
VMCASrvCleanupGlobalState(
	VOID
	);

/* rpc.c */
void *
VMCAListenRpcServer(
    void * pArg
    );

DWORD
VMCAStopRpcServer(
    VOID
    );

/* rpcmemory.c */

DWORD
VMCARpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VMCARpcFreeMemory(
    PVOID pMemory
    );


/* service.c */

DWORD
VMCARPCInit(
    VOID
    );

VOID
VMCARPCShutdown(
    VOID
    );


DWORD
VMCAServiceInit(
    VOID
    );

VOID
VMCAServiceShutdown(
    VOID
    );


/* signal.c */

VOID
VMCABlockSelectedSignals(
    VOID
    );

DWORD
VMCAHandleSignals(
    VOID
    );

// thread.c

DWORD
VMCACreateThread(
    PFN_VMCA_THR_FUNC pfnThrFunc,
    PVOID             pData,
    PVMCA_THREAD*     ppThread
    );

PVMCA_THREAD
VMCAAcquireThread(
	PVMCA_THREAD pThread
	);

DWORD
VMCANotifyThread(
	PVMCA_THREAD pThread
	);

DWORD
VMCAWaitNotifyThread(
	PVMCA_THREAD_DATA pThrData,
	DWORD             dwSecs
	);

DWORD
VMCACheckThreadShutdown(
	PVMCA_THREAD_DATA pThrData,
	PBOOLEAN          pbShutdown
	);

VOID
VMCAReleaseThread(
	PVMCA_THREAD pThread
	);

/* enumpkgs.c */
DWORD
VMCAServerInitRpcEnumPackagesHandle(
    PDWORD pdwHandle
    );

DWORD
VMCAServerEnumPackages(
    DWORD    dwHandle,
    DWORD    dwStartIndex,
    DWORD    dwNumPackages,
    PVMCA_CERTIFICATE_CONTAINER * ppCertContainer
    );

unsigned int
VMCAEnumCertificates(
    unsigned int dwStartIndex,
    unsigned int dwNumCertificates,
    CERTIFICATE_STATUS dwStatus,
    VMCA_CERTIFICATE_ARRAY** ppCertArray
    );

unsigned int
VMCAAddRootCertificate(
    unsigned char *pszRootCertificate,
    PWSTR pszPassPhrase,
    unsigned char *pszPrivateKey,
    unsigned int dwOverWrite
    );


unsigned int
VMCAGetSignedCertificate(
    unsigned char *pszPEMEncodedCSRRequest,
    unsigned int dwValidFrom,
    unsigned int dwDurationInSeconds,
    PVMCA_CERTIFICATE_CONTAINER * ppCertContainer
    );

unsigned int
VMCAGetRootCACertificate(
    unsigned int *dwCertLength,
    PVMCA_CERTIFICATE *ppCertificate
    );

DWORD
VMCARevokeCertificate(
    unsigned char *pszCertificate
    );

DWORD
VMCAGetServerVersion(
    PSTR* serverVersion
    );

DWORD
VMCAGetCertificateCount(
    unsigned int dwStatus,
    unsigned int *dwNumCertificates
    );

DWORD
VMCAVerifyCertificate(
    unsigned char *pszPEMEncodedCertificate,
    unsigned int *dwStatus
    );


VOID
VMCARpcFreeCertificateContainer(
    PVMCA_CERTIFICATE_CONTAINER pCertContainer
    );

VOID
VMCAFreeCertificateContainer(
    PVMCA_CERTIFICATE_CONTAINER pCertContainer
    );

DWORD
VMCAStartRpcServer(
    );

// copied from VMCA.h since there are some name
// conflicts between functions in the service
// and vmca.h

VOID
VMCAFreeKey(
    PVMCA_KEY pKey
    );
 //VMCAFreeKey frees the Key Objects allocated by the VMCAAllocatePrivateKey

 //Arguments :
 //     pKey : Points to the key that is to be freed.
 //Returns :
 //     Error Code

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
VMCACheckAccess(
    handle_t IDL_handle, 
    BOOL bAdminAccess
    );
// VMCACheckAccess checks if the person has sufficient privilege to make the call.

// utils.c
DWORD
VMCASetApplicationVersion(
    DWORD dwVersion
    );

DWORD
VMCAGetApplicationVersion(
    VOID
    );

VMCA_DB_CERTIFICATE_STATUS
VMCAMapToDBStatus(
    CERTIFICATE_STATUS st
    );

DWORD
VMCAUpdateCRL(
    PSTR pszCertficate,
    UINT32 uReason
);

unsigned int
VMCAGetCRL(
    unsigned int dwFileOffset,
    unsigned int dwSize,
    VMCA_FILE_BUFFER **ppCRLData
    );

unsigned int
VMCAReGenCRL(
    );

DWORD
VMCACreateDirectory(
    PSTR pszDirectoryName,
    BOOL bRestrictedAccess
    );

DWORD
VMCACreateDataDirectory(
    VOID
    );

DWORD
VMCAInsertCertificate(
    PVMCA_DB_CERTIFICATE_ENTRY pEntry
    );

DWORD
VMCADecodeCert(
    PSTR pszPKCS10Request,
    PVMCA_DB_CERTIFICATE_ENTRY* ppEntry
    );

VOID
VMCAFreeDBEntry(
    PVMCA_DB_CERTIFICATE_ENTRY pEntry
    );

VOID
VMCARpcFreeCertificateContainer(
    PVMCA_CERTIFICATE_CONTAINER pCertContainer
    );

DWORD
VMCARpcAllocateCertificateContainer(
    PSTR pszCert,
    PVMCA_CERTIFICATE_CONTAINER *ppCertContainer
    );

DWORD
VMCAAllocateCertificateContainer(
    PSTR pszCert,
    PVMCA_CERTIFICATE_CONTAINER *ppCertContainer
    );

DWORD
VMCARpcAllocateString(
    PSTR  pszSrc,
    PSTR* ppszDst
    );

DWORD
VMCASetKeyPerm(
    PSTR pszPrivateKeyFileName
    );

VOID
VMCAFreeCertificateArray(
    PVMCA_CERTIFICATE_ARRAY pCertArray
);

VOID
VMCARpcFreeCertificateArray(
    PVMCA_CERTIFICATE_ARRAY pCertArray
);

DWORD
VMCACopyTempCRLtoCRL(
    VOID
    );

DWORD
VMCASrvPublishRootCerts(
    VOID
    );

DWORD
VMCABackupRootCAFiles(
    PCSTR pszRootCACertFile,
    PCSTR pszRootCAPrivateKeyFile,
    PCSTR pszRootCAPasswordFile
    );

//vmcaservice.c
DWORD
VmcaSrvRevokeCertificate(
                         PCWSTR pwszServerName,
                         PVMCA_CERTIFICATE pszCertificate,
                         VMCA_CRL_REASON certRevokeReason
                        );

DWORD
VmcaSrvValidateCRLReason(
                        DWORD dwCrlReason,
                        PVMCA_CRL_REASON pCertRevokeReason
                       );

DWORD
VmcaSrvReGenCRL (
                 X509_CRL **ppCrl
                );

DWORD
VMCASrvGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    );

DWORD
VMCALdapAccessCheck(
    PCSTR szAuthPrinc,
    VMCA_USER_TYPE userType
    );

//utils.c

DWORD
VMCAHeartbeatInit(
    PVMAFD_HB_HANDLE *ppHandle
    );

VOID
VMCAStopHeartbeat(
    PVMAFD_HB_HANDLE pHandle
    );

//vmcaHTTPCallback.c
#if 0
#ifndef _WIN32
DWORD
VMCARESTGetCRL(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCARESTGetRootCACertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCARESTSrvPublishRootCerts(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCARESTAddRootCertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCARESTEnumCertificates(
    VMCA_HTTP_REQ_OBJ request,
    PCSTR pszFlag,
    PCSTR pszNumber,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCARESTGetSignedCertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCARESTRevokeCertificate(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCARESTGetServerVersion(
    VMCA_HTTP_REQ_OBJ request,
    PSTR* ppStatusCode,
    PSTR* ppResponsePayload
    );

DWORD
VMCAHandleEnumCertsParam(
    PSTR pszKey1,
    PSTR pszVal1,
    PSTR pszKey2,
    PSTR pszVal2,
    PSTR* ppszFlag,
    PSTR* ppszNum
    );

//restauth.c
DWORD
VMCARESTGetAccessToken(
      PREST_REQUEST pRESTRequest,
      PVMCA_ACCESS_TOKEN* ppAccessToken
      );

VOID
VMCAFreeAuthorizationParam(
      PVMCA_AUTHORIZATION_PARAM pAuthorization
      );

VOID
VMCAFreeAccessToken(
      PVMCA_ACCESS_TOKEN pAccessToken
      );

//oidcutil.c
DWORD
VMCAVerifyOIDC(
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    );

VOID
VMCAFreeOIDC(
    PVMCA_ACCESS_TOKEN pAccessToken
    );


#endif

//restbasicauth.c
DWORD
VMCARESTVerifyBasicAuth(
    PREST_REQUEST pRequest,
    PREST_RESPONSE* ppResponse
    );

DWORD
base64_decode(
    PCSTR pszInput,
    PSTR* ppszOutput,
    int* pnLength
    );

uint32_t
base64_encode(
    const unsigned char* pszInput,
    const size_t nInputLength,
    PSTR* ppszOutput
    );

//restnegauth.c
DWORD
VMCARESTVerifyKrbAuth(
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    );

VOID
VMCARESTFreeKrb(
    PVMCA_ACCESS_TOKEN pAccessToken
    );
#endif

#ifdef __cplusplus
}
#endif
