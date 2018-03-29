## Header
```
#include<vmca.h>
```
## 1. Connection:
### VMCAOpenServer
```
DWORD
VMCAOpenServerA(
    [in] PCSTR pszServerName,
    [in] PCSTR pszUserName,
    [in] PCSTR pszDomain,
    [in] PCSTR pszPassword,
    [in] DWORD dwFlags,
    [in] PVOID pReserved,
    [out] PVMCA_SERVER_CONTEXT *ppServerContext
    );
```
### Description
Opens a RPC connection a VMCA running on a Lightwave server
### Parameters
#### Inputs:
*pszServerName*[in]: Name of the Lightwave Server

*pszUserName*[in] : Username of the Lightwave User

*pszDomain*[in] : Lightwave domain

*pszPassword*[in] : Lightwave user password

*dwFlags*[in] : Flags needed to be passed to the server. If you are not sure, pass 0

*pReserved*[in]: Pass in NULL
#### Output:
*ppServerContext* [out] : A connection object to the Lightwave server specified by pszServerName. If the call fails, this is set to NULL. After performing necessary operations the user should close the connection context using VMCACloseServer.
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCACloseServer
```
DWORD
VMCACloseServer(
    [in] PVMCA_SERVER_CONTEXT pServerContext
    );
```
### Definition
Closes an existing connection to a Lightwave Server's VMCA service
### Parameters
*pServerContext[in]: An open server connection to Lightwave Server.

### Return:
Returns 0 on success.

## 2. Root Certificate:
### VMCAAddRootCertificate
```
DWORD
VMCAAddRootCertificateHA(
    [in]           PVMCA_SERVER_CONTEXT hInBinding,
    [in, optional] PCSTR pszServerName,
    [in]           PVMCA_CERTIFICATE pszRootCertificate,
    [in, optional] PSTR pszPassPhrase,
    [in]           PVMCA_KEY pszPrivateKey
);
```
### Definition
This function is used to set the root certificate (signing certificate) for VMCA. 
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

*pszRootCertificate*[in]: A PEM formatted X509 certificate string. This is the certificate that will be set as the signing certificate. This should be a CA signing certificate.

*pszPassPhrase*[in,optional]: Passphrase to decrypt the PrivateKey if the Private key is encrypted.

*pszPrivateKey*[in]: The signing certificate's Private Key string.
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetRootCertificate
```
DWORD
VMCAGetRootCACertificateHA(
    [in]          PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional] PCSTR pszServerName,
    [out]         PVMCA_CERTIFICATE* ppCertificate
);
```
### Definition
This function is used to fetch the root certificate of the Lightwave Server.
### Parameters
#### Inputs:
*hInBinding*[in]: An open connection handle to a Lightwave Server

*pszServerName*[in]: If hInBinding does not exist, the Lightwave Server Name.
#### Output
*pCertificate*[out]: The root certificate of VMCA
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAPublishRootCerts
```
DWORD
VMCAPublishRootCertsHA(
    [in]          PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional] PCSTR pszServername
    );
```
### Definition
Publishes VMCA's root signing certificate to the Lightwave directory
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
## 3. Private and Public KeyPair:
### VMCACreatePrivateKey
```
DWORD
VMCACreatePrivateKeyA(
    [in,optional]  PSTR pszPassPhrase,
    [in]           size_t uiKeyLength,
    [out]          PVMCA_KEY* ppPrivateKey,
    [out]          PVMCA_KEY* ppPublicKey
);
```
### Definition
Creates a private and public key pair.
### Parameters
#### Inputs:
*pszPassphrase*[in,optional]: Optional passphrase if the private key needs to be encrypted.

*uiKeyLength*[in]: Length of the private key
#### Outputs:
*ppPrivateKey*[out]: Generated private key

*ppPublicKey*[out]: Generated public key 
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
## 4. **C**ertificate **S**igning **R**equest (CSR):
### VMCACreateSigningRequest
```
DWORD
VMCACreateSigningRequestA(
    [in]          PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    [in]          PVMCA_KEY pPrivateKey,
    [in,optional] PSTR pszPassPhrase,
    [out]         PVMCA_CSR* ppCSR
);
```
### Definition
Creates a **C**ertificate **S**igning **R**equest **(CSR)**. A CSR can be submitted to a certificate authority to obtain a signed certificate.
### Parameters
#### Inputs:
*pCertRequest*[in]: A certificate request struct with various parameters. Use the [VMCAAllocatePKCS10Data](#VMCAAllocatePKCS10Data) to allocate this structure. Use [VMCAInitPKCS10Data](#VMCAInitPKCS10Data) to initialize the structure with the right data.

*pPrivateKey*[in]: The private key to be used for the certificate

*pszPassphrase*[in,optional]: Passphrase to decrypt to the private key (if the private key is encrypted).
#### Output:
*ppCSR*[out]: PEM formatted generated CSR string. This can be used to obtain a certificate from any certificate authority.
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetCSRFromCertificate
```
DWORD
VMCAGetCSRFromCertificate(
    [in]  PVMCA_CERTIFICATE pszCertificate,
    [in]  PVMCA_KEY pszPrivateKey,
    [out] PVMCA_CSR *ppszCSR
    );
```
### Parameters
#### Inputs:
*pszCertificate*[in]: PEM formatted X509 certificate

*pszPrivateKey*[in]: Private key of the certificate

#### Output:
*ppszCSR*[out]: CSR generated from the certificate
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
## 5. Certificate Generation:
### VMCACreateSelfSignedCertificate
```
DWORD
VMCACreateSelfSignedCertificateA(
    [in]          PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    [in]          PVMCA_KEY pszPrivateKey,
    [in,optional] PSTR pszPassPhrase,
    [in]          time_t tmNotBefore,
    [in]          time_t tmNotAfter,
    [out]         PVMCA_CERTIFICATE* ppCertificate
);
```
### Definition
Creates a self signed certificate
### Parameters
#### Inputs:
*pCertRequest*[in]: A certificate request struct with various parameters. Use the [VMCAAllocatePKCS10Data](#VMCAAllocatePKCS10Data) to allocate this structure. Use [VMCAInitPKCS10Data](#VMCAInitPKCS10Data) to initialize the structure with the right data.

*pPrivateKey*[in]: The private key to be used for the certificate

*pszPassphrase*[in,optional]: Passphrase to decrypt to the private key (if the private key is encrypted).

*tmNotBefore*[in]: Setting when the certificate is valid from

*tmNotAfter[in]: Expiry of the certificate
#### Output:
*ppCertificate*[out]: Generated self signed certificate in PEM format
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetSignedCertificate
```
DWORD
VMCAGetSignedCertificateHA(
    [in]          PVMCA_SERVER_CONTEXT bInBinding,
    [in,optional] PCSTR pszServerName,
    [in]          PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    [in]          PVMCA_KEY pPrivateKey,
    [in,optional] PSTR pszPassPhrase,
    [in]          time_t tmNotBefore,
    [in]          time_t tmNotAfter,
    [out]         PVMCA_CERTIFICATE* ppCert
);
```
### Definition
Gets a signed certificate from VMCA
### Parameters:
#### Inputs:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

*pCertRequest*[in]: A certificate request struct with various parameters. Use the [VMCAAllocatePKCS10Data](#VMCAAllocatePKCS10Data) to allocate this structure. Use [VMCAInitPKCS10Data](#VMCAInitPKCS10Data) to initialize the structure with the right data.

*pPrivateKey*[in]: The private key to be used for the certificate

*pszPassphrase*[in,optional]: Passphrase to decrypt to the private key (if the private key is encrypted).

*tmNotBefore*[in]: Setting when the certificate is valid from

*tmNotAfter[in]: Expiry of the certificate
#### Output:
*ppCertificate*[out]: Generated self signed certificate in PEM format
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
```
DWORD
VMCAGetSignedCertificateForHostA(
    [in]          PVMCA_SERVER_CONTEXT hInBinding,
    [in]          PCSTR pszHostName,
    [in]          PCSTR pszHostIp,
    [in]          PCVMCA_CSR pCertRequest,
    [in]          time_t tmNotBefore,
    [in]          time_t tmNotAfter,
    [out]         PVMCA_CERTIFICATE* ppCert
    );
```
### Definition
Gets a signed certificate from VMCA for a specific host.
### Parameters:
#### Inputs:
*hInBinding*[in]: A connection handle to VMCA

*pszHostname*[in]: Hostname for which the certificate needs to be generated

*pszHostIp*[in]: IP of the host for which the certificate needs to be generated

*pCertRequest*[in]: PEM formatted CSR string. You could use one of the CSR generation to generate the CSR.

*tmNotBefore*[in]: Setting when the certificate is valid from

*tmNotAfter[in]: Expiry of the certificate
#### Output:
*ppCertificate*[out]: Generated self signed certificate in PEM format
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetSignedCertificateFromCSR
```
DWORD
VMCAGetSignedCertificateFromCSRHA(
    [in]          PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional] PCSTR pszServerName,
    [in]          PCVMCA_CSR pCertRequest,
    [in]          time_t tmNotBefore,
    [in]          time_t tmNotAfter,
    [out]         PVMCA_CERTIFICATE* ppCert
);
```
### Definition
Generates a VMCA signed certificate using a CSR.
### Parameters:
#### Inputs:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)
*pCertRequest*[in]: PEM formatted CSR string. You could use one of the CSR generation to generate the CSR.

*tmNotBefore*[in]: Setting when the certificate is valid from

*tmNotAfter[in]: Expiry of the certificate
#### Output:
*ppCertificate*[out]: Generated self signed certificate in PEM format
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
## 6. View Certificates:
### VMCAOpenEnumContext
```
DWORD
VMCAOpenEnumContextHA(
    [in]           PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional]  PCSTR pszServerName,
    [in]           VMCA_CERTIFICATE_STATUS Filter,
    [out]          PVOID *Context
);
```
### Definition
Creates an enumerator handle. This can be used to enumerate all the certificates issued by VMCA. This enumeration context should then be closed by using [VMCACloseEnumContext](#VMCACloseEnumContext)
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

*Filter*[in]: Filter to the query. The values are:
    *  VMCA_CERTIFICATE_ACTIVE
    *  VMCA_CERTIFICATE_REVOKED
    *  VMCA_CERTIFICATE_EXPIRED
    *  VMCA_CERTIFICATE_ALL

#### Output:
*Context* [out]: A context handle which can be used to enumerate
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetNextCertificate
```
DWORD
VMCAGetNextCertificate(
    [in] PVOID Context,
    [out] PVMCA_CERTIFICATE* ppCertificate,
    [out] int *pCurrentIndex,
    [out] VMCA_ENUM_CERT_RETURN_CODE* enumStatus
    );
```
### Definition
Gets the next certificate using the enumerator handle
### Parameters
#### Input:
*Context*[in]: Enumeration handle
#### Outputs:
*ppCertificate*[out]: Next certificate returned by the enumerator

*pCurrentIndex*[out]: Current index of the certificate

*enumStatus*[out]: Enumeration error if any
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCACloseEnumContext
```
DWORD
VMCACloseEnumContext(
    [in] PVOID Context
);
```
### Definition
Closes an open enum context
### Parameter
*Context*[in]: The enumeration context that needs to be closed
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
## 7. Certificate Revocation:
```
DWORD
VMCARevokeCertA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszCertificate,
    VMCA_CRL_REASON certRevokeReason
);
```
### Definition
This function is used to set the root certificate (signing certificate) for VMCA. 
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
## 8. **C**ertificate **R**evocation **L**ist:
### VMCAGetCRL
```
DWORD
VMCAGetCRLHA(
    [in]          PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional] PCSTR pszServerName,
    [in]          PSTR pszExistingCRLFileName,
    [in]          PSTR pszNewCRLFileName
    );
```
### Definition
Retrieves the current CRL from VMCA
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

*pszExistingCRLFileName*[in]: Filename of the most recent CRL that the client has knowledge of

*pszNewCRLFileName*[in]: Filename into which the new CRL needs to be written to
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAReGenCRL
```
DWORD
VMCAReGenCRLHA(
    [in]          PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional] PCSTR pszServerName
    );
```
### Definition
Forces VMCA to regenerate the CRL
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetCRLInfo
```
DWORD
VMCAGetCRLInfo(
    [in]  PSTR pszFileName,
    [out] DWORD *ptmLastUpdate,
    [out] DWORD *ptmNextUpdate,
    [out] DWORD *pdwCRLNumber);

DWORD
VMCAGetCRLInfo2(
    [in]  PSTR pszFileName,
    [out] DWORD *ptmLastUpdate,
    [out] DWORD *ptmNextUpdate,
    [out] DWORD *pdwCRLNumber);
```
### Definition
Reads and parses a CRL file and gets information from it.
### Parameters
*pszFileName*[in]: CRL file

*ptmLastUpdate*[out]: Last updated time

*ptmNextUpdate*[out]: When the CRL will be updated next

*pdwCRLNumber*[in]: CRL number
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAPrintCRL
```
DWORD
VMCAPrintCRL(
    [in]  PSTR pszFileName,
    [out] PSTR *ppszCRLString
    );
```
### Definition
Utility function to read the CRL string from a file
### Parameters
#### Input:
*pszFileName*[in]: CRL filename
#### Output:
*ppszCRLString*[out]: CRL string read from the file
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)

## 9. Utility:
### VMCAFreeString
```
VOID
[in] VMCAFreeStringA(PSTR pszString);
```
### Defintion
Frees the string specified
### Parameter
*pszString*[in]: String to be freed
### VMCAAllocatePKCS10Data
```
DWORD
VMCAAllocatePKCS10DataA(
  [out] PVMCA_PKCS_10_REQ_DATAA* ppCertRequestData
);
```
### Definition
Allocates the PKCS10Data struct. This can then be passed to InitPKCS10Data to initialize with necessary values
### Parameters
*ppCertRequestData*[out]: Allocated CertRequestData structure
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAInitPKCS10Data
```
DWORD
VMCAInitPKCS10DataWithDCA(
    [in]     PCSTR pszName,
    [in]     PCSTR pszDomainName,
    [in]     PCSTR pszOrganization,
    [in]     PCSTR pszOU,
    [in]     PCSTR pszState,
    [in]     PCSTR pszCountry,
    [in]     PCSTR pszEmail,
    [in]     PCSTR pszIPAddress,
    [in,out] PVMCA_PKCS_10_REQ_DATAA pCertRequestData
    );
```
### Definition
Populates PKCS10Data struct with necessary values
### Parameters
#### Inputs:
Various certificate fields
#### Output:
*pCertRequestData*[in, out]: PKCS10Data struct populated with the right values. This struct needs to be allocated using AllocatePKCS10Data functions
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAFreeCertificate
```
VOID
VMCAFreeCertificate(
    [in] PVMCA_CERTIFICATE pCertificate
);
```
### Defintion
Frees the certificate specified
### Parameter
*pCertificate*[in]: Certificate to be freed
### VMCAFreeCSR
```
VOID
VMCAFreeCSR(
    [in] PVMCA_CSR pCSR
);
```
### Defintion
Frees the CSR specified
### Parameter
*pCSR*[in]: CSR to be freed
### VMCAFreeKey
```
VOID
VMCAFreeKey(
    [in] PVMCA_KEY pKey
);
```
### Defintion
Frees the Key specified
### Parameter
*pKey*[in]: Key to be freed
### VMCAFreePKCS10Data
```
VOID
VMCAFreePKCS10DataA(
    [in] PVMCA_PKCS_10_REQ_DATAA pCertRequestData
);
```
### Defintion
Frees the PKCS10Data struct specified
### Parameter
*pCertRequestData*[in]: PKCS10Data struct to be freed
### VMCAGetCertificateAsString
```
DWORD
VMCAGetCertificateAsStringA(
    [in]  PVMCA_CERTIFICATE pCertificate,
    [out] PSTR* ppszCertString
);
```
### Definition
Converts a PEM format X509 certificate to human readable form
### Parameters
#### Input:
*pCertificate*[in]: PEM format certificate
#### Output:
*ppszCertificateString*[out]: Human readable form certificate
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAReadCertificateFromFile
```
DWORD
VMCAReadCertificateFromFile(
    [in]  PSTR pszFileName,
    [out] PVMCA_CERTIFICATE* ppCertificate
);
```
### Definition
Reads a certificate from a File
#### Parameters
##### Input:
*pszFileName*[in]: Input file name
#### Output"
*ppCertificate*[out]: Certificate from the file
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAReadPrivateKeyFromFile
```
DWORD
VMCAReadPrivateKeyFromFile(
    [in]           PSTR pszFileName,
    [in,optional]  PSTR pszPassPhrase,
    [out]          PVMCA_KEY* ppPrivateKey
);
```
### Definition
Reads a Private Key from a File
#### Parameters
##### Input:
*pszFileName*[in]: Input file name

*pszPassphrase*[in,optional]: Passphrase to decrypt the file
#### Output"
*ppPrivateKey*[out]: Private Key from the file
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAReadPublicKeyFromFile
```
DWORD
VMCAReadPublicKeyFromFile(
    [in]  PSTR pszFileName,
    [out] PVMCA_KEY* ppPublicKey
);
```
### Definition
Reads a Public key from a File
#### Parameters
##### Input:
*pszFileName*[in]: Input file name
#### Output"
*ppPublicKey*[out]: Public key read from the file
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAWriteCertificateToFile
```
DWORD
VMCAWriteCertificateToFile(
    [in] PSTR pszCertificateFileName,
    [in] PVMCA_CERTIFICATE pCertificate
);
```
### Definition
Writes a PEM formatted certificate string to file
### Parameters
*pszCertificateFileName*[in]: Name of file to which the certificate needs to be written to

*pCertificate*[in]: Certificate to write to file
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAWritePrivateKeyToFile
```
DWORD
VMCAWritePrivateKeyToFile(
    [in]          PSTR pszPrivateKeyFileName,
    [in]          PVMCA_KEY pPrivateKey,
    [in,optional] PSTR pszPassPhraseFileName,
    [in,optional] PWSTR pszPassPhrase
);
```
### Definition
Writes a private key string to file
### Parameters
*pszPrivateKeyFileName*[in]: Name of file to which the key needs to be written to

*pPrivateKey*[in]: Key to write to file

### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAWritePublicKeyToFile
```
DWORD
VMCAWritePublicKeyToFile(
    [in] PSTR pszPublicKeyFileName,
    [in] PVMCA_KEY pPublicKey
);
```
### Definition
Writes a public key string to file
### Parameters
*pszPublickKeyFileName*[in]: Name of file to which the key needs to be written to

*pPublicKey*[in]: Key to write to file
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetShortError
```
DWORD
VMCAGetShortError(
    [in]  DWORD dwError,
    [out] PSTR *pszErrMsg
);
```
### Definition
Converts the VMCA returned error into a string message
### Parameters
#### Input
*dwError*[in]: VMCA returned error code
#### Output
*pszErrMsg*[out]: Error message
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetErrorString
```
DWORD
VMCAGetErrorString(
    [in]  DWORD dwError,
    [out] PSTR *pszErrMsg
);
```
### Definition
Converts the VMCA returned error into a string message
### Parameters
#### Input
*dwError*[in]: VMCA returned error code
#### Output
*pszErrMsg*[out]: Error message
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
## 10. Server Configuration:
### VMCASetServerOption
```
DWORD
VMCASetServerOptionHA(
    [in]           PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional]  PCSTR pszServerName,
    [in]           unsigned int dwOption
    );
```
### Definition
This function is used to set the server option
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

*dwOption*[in]: Options to be set

### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAUnsetServerOption
```
DWORD
VMCAUnsetServerOptionHA(
    [in]           PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional]  PCSTR pszServerName,
    [in]           unsigned int dwOption
    );
```
### Definition
This function is used to unset the server option
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

*dwOption*[in]: Options to un set
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetServerOption
```
DWORD
VMCAGetServerOptionHA(
    [in]           PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional]  PCSTR pszServerName,
    [out]          unsigned int* pdwOption
    );
```
### Definition
This function is used to get the server option
### Parameters:
#### Inputs:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)
#### Output:
*pdwOption*[out]: Options to get
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
#### VMCAGetServerVersion
```
DWORD
VMCAGetServerVersionHA(
    [in]           PVMCA_SERVER_CONTEXT hInBinding,
    [in,optional]  PCSTR pszServerName,
    [out]          PSTR* ppszServerVersionString
);
```
### Definition
This function is used to get the server Version as a string
### Parameters:
#### Inputs:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)
#### Output:
*ppszServerVersionString*[out]: Version of the server

### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)

## 10. Certificate Configuration:
### VMCASetCertValue
```
DWORD
VMCASetCertValueA(
    VMCA_OID Field,
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    PSTR pwszNewValue
);
```
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCASetKeyUsageContraints
```
DWORD
VMCASetKeyUsageConstraintsA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    DWORD dwKeyUsageMask
);
```
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
#### VMCAValidateCACertificate
```
DWORD
VMCAValidateCACertificate(
    PVMCA_CERTIFICATE pszCertificate
);
```
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAVerifyCertificate
```
DWORD
VMCAVerifyCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
);
```
### Definition
This function is used to Verify a given certificate 
### Parameters:
*hInBinding*[in]: A connection handle to VMCA

*pszServerName*[in,optional]: Lightwave Servername  (if hInBinding is not available)

### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)
### VMCAGetCertificateCount
```
DWORD
VMCAGetCertificateCountHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD *dwNumCertificates
);
```
### Definition
Gets the total number of certificates issued by this Certificate Authority
### Parameters
#### Inputs:
*hInBinding*[in]: A connection handle to a Lightwave Server

*pszServerName*[in,optional]: If hInBinding is missing, this is the name of the Lightwave Server 
### Return
On Success, the function returns 0. In all other cases returns a non-zero error code. For the list of errors, refer to [vmca_error.h](../../vmca/include/vmca_error.h)