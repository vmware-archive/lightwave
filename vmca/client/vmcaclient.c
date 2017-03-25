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

// VMCAClient.c : Defines the exported functions for the DLL application.
//


#include "includes.h"

#define BUFSIZE 1024
#define ENUM_CERT_SIZE 20
#define VMCA_RPC_MAX_RETRY 5

#define VMCA_VERIFY_SERVER(pszServerName )          \
    do                                              \
    {                                               \
        if (( pszServerName == NULL) ||             \
                    (*pszServerName == 0)){  \
            dwError = ERROR_INVALID_PARAMETER;      \
            BAIL_ON_ERROR(dwError);                 \
        }                                           \
    } while (0)

#ifdef _WIN32
#define VMCASleep(X) Sleep((X) * 1000)
#else
#define VMCASleep(X) sleep((X))
#endif

static
DWORD
VMCARevokeCertPrivate(
    handle_t BindingHandle,
    PCWSTR pwszServerName,
    PSTR pszPEMEncodedCertificate,
    VMCA_CRL_REASON certRevokeReason,
    PWSTR pwszSharedSecret
    );

DWORD
VMCASelfSignedCertificatePrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PSTR pszPrivateKey,
    PWSTR pwszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PSTR* ppszCertificate
);

DWORD
VMCACreateSigningRequestPrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    LPSTR pszPrivateKey,
    PWSTR pszPassPhrase,
    LPSTR* ppAllocatedCSR
);

VOID
VMCAFreeCertificateContainer(
    PVMCA_CERTIFICATE_CONTAINER pCertContainer
);

VOID
VMCAFreeCertificateArray(
    PVMCA_CERTIFICATE_ARRAY pCertArray
);

static DWORD
VMCAEnumCertsHW(
    handle_t hInBinding,
    PCWSTR pwszServerName,
    CERTIFICATE_STATUS dwStatus,
    DWORD dwStartIndex,
    DWORD dwNumCertificates,
    PVMCA_CERTIFICATE_ARRAY* ppCerts
);

static VOID
VMCAFreeEnumContext(PVOID pContext);

VOID
VMCASetPKCSMemberW(
    PWSTR *ppszMember,
    PCWSTR pszNewValue
);

VOID
VMCASetPKCSMemberA(
    PSTR *ppszMember,
    PCSTR pszNewValue
);

BOOL
isVMCAErrorCode(DWORD dwError)
{
    if ( ( dwError >= VMCA_ROOT_CA_MISSING) &&
         (dwError <= VMCA_UNKNOW_ERROR))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL
IsDceRpcError(DWORD dwError)
{
    if ((dwError >= (DWORD)rpc_s_mod) &&
        (dwError <= (DWORD)rpc_s_fault_codeset_conv_error))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL
IsTransientRpcError(DWORD dwRpcError)
{
    switch ((int)dwRpcError)
    {
        case rpc_s_too_many_rem_connects:
        return (TRUE);

        default:
        return (FALSE);
    }
}

#define VMCARpcCall(RpcFunc)                                \
do                                                          \
{                                                           \
    int i = 0;                                              \
    for (i = 0; i < VMCA_RPC_MAX_RETRY; ++i)                \
    {                                                       \
        DCETHREAD_TRY                                       \
        {                                                   \
            dwError = RpcFunc;                              \
        }                                                   \
        DCETHREAD_CATCH_ALL(THIS_CATCH)                     \
        {                                                   \
            dwError = VMCADCEGetErrorCode(THIS_CATCH);      \
        }                                                   \
        DCETHREAD_ENDTRY;                                   \
                                                            \
        if (dwError == ERROR_SUCCESS ||                     \
            !IsTransientRpcError(dwError))                  \
        {                                                   \
            break;                                          \
        }                                                   \
        VMCASleep(1);                                       \
    }                                                       \
} while(0)


DWORD
VMCACreateSelfSignedCertificateW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PVMCA_KEY pszPrivateKey,
    PWSTR pwszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
)
//  VMCASelfSignedCertificatePrivate allows the user to create
//  a Self Signed Certificate.
//
// Arguments :
//      pCertRequest    :  Parameters for the request
//      pszPrivateKey   :  Private Key for the CSR
//      pszPassPhrase   :  Pass Phrase for the Private Key
//      tmNotBefore     : Start time
//      tmNotAfter      : Expiration time for a certificate
//      ppszCertificate : Allocated Certificate
// Returns :
//      Error Code
{

    DWORD dwError = 0;

    if( (pCertRequest == NULL)
        || (pszPrivateKey == NULL)
        || (ppCertificate == NULL)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if ( tmNotAfter <= tmNotBefore) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (( tmNotBefore == 0 ) ||
        (tmNotAfter == 0 )) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCASelfSignedCertificatePrivate(
                  pCertRequest,
                  pszPrivateKey,
                  pwszPassPhrase,
                  tmNotBefore,
                  tmNotAfter,
                  (PSTR*) ppCertificate
              );

    BAIL_ON_ERROR(dwError);

error :
    return dwError;
}

DWORD
VMCAAllocatePKCS10DataW(
    PVMCA_PKCS_10_REQ_DATAW* pCertRequestData
)
// This function allocates a VMCA specific structure
// which carries the payload for creating a certificate
// request. This function allocates and returns a zero
// structure. Please see the definition of VMCA_PKCS_10_REQ_DATA
// for more details
//
// Arguments :
//         Pointer-Pointer to the Structure to be Allocated
// Returns :
//     Error code
{
    DWORD dwError = 0;
    if (pCertRequestData == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(sizeof(VMCA_PKCS_10_REQ_DATAW), (PVOID*) pCertRequestData);
    BAIL_ON_ERROR(dwError);

    //TODO: Do we need this memset?
    memset((PVOID) *pCertRequestData, 0, sizeof(VMCA_PKCS_10_REQ_DATAW));
error:
    return dwError;
}


DWORD
VMCASetCertValueW(
    VMCA_OID Field,
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PWSTR pszNewValue
)
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
{


    DWORD dwError = 0;

    if ( pCertRequest == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    switch(Field)
    {
    case VMCA_OID_CN            :
        VMCASetPKCSMemberW(&pCertRequest->pszName,          pszNewValue);
        break;
    case VMCA_OID_DC            :
        VMCASetPKCSMemberW(&pCertRequest->pszDomainName,    pszNewValue);
        break;
    case VMCA_OID_COUNTRY       :
        VMCASetPKCSMemberW(&pCertRequest->pszCountry,       pszNewValue);
        break;
    case VMCA_OID_LOCALITY      :
        VMCASetPKCSMemberW(&pCertRequest->pszLocality,      pszNewValue);
        break;
    case VMCA_OID_STATE         :
        VMCASetPKCSMemberW(&pCertRequest->pszState,         pszNewValue);
        break;
    case VMCA_OID_ORGANIZATION  :
        VMCASetPKCSMemberW(&pCertRequest->pszOrganization,  pszNewValue);
        break;
    case VMCA_OID_ORG_UNIT      :
        VMCASetPKCSMemberW(&pCertRequest->pszOU,            pszNewValue);
        break;
    case VMCA_OID_DNS           :
        VMCASetPKCSMemberW(&pCertRequest->pszDNSName,       pszNewValue);
        break;
    case VMCA_OID_URI           :
        VMCASetPKCSMemberW(&pCertRequest->pszURIName,       pszNewValue);
        break;
    case VMCA_OID_EMAIL         :
        VMCASetPKCSMemberW(&pCertRequest->pszEmail,         pszNewValue);
        break;
    case VMCA_OID_IPADDRESS     :
        VMCASetPKCSMemberW(&pCertRequest->pszIPAddress,     pszNewValue);
        break;
    default :
        dwError = ERROR_INVALID_PARAMETER;
        break;
    }

error :
    return dwError;

}

DWORD
VMCASetKeyUsageConstraintsW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    DWORD dwKeyUsageMask
)
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
{
    DWORD dwError = 0;
    if(pCertRequest == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pCertRequest->dwKeyUsageConstraints = dwKeyUsageMask;

error :
    return dwError;
}


DWORD
VMCACreateSigningRequestW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequestData,
    PSTR pszPrivateKey,
    PWSTR pwszPassPhrase,
    PVMCA_CSR* pAllocatedCSR
)
// This function creates a Signing Request which can be send to a Certificate authority
// for its signature. This function creates something known as PKCS#10 , or a CSR
// (Certificate Signing Reqeuest).
//
// Arguments :
//  pCertRequestData - A ceritificate can have various data fields that your might choose to
//  provide, this blob points to data that user wants to put in the certificate.
//
//  pszPrivateKey - The Private Key to be used in the CSR creation
//
//  pszPassPhrase - Optional  Pass Phrase needed to open the Private key. If the key was protected
//  using a Pass Phrase then that is the Pass Phrase needed here.
//
//  pAllocatedCSR - A pointer to a buffer where the  Actual allocated CSR will be written to.
//
// Returns :
//  Error Code
{
    DWORD dwError = 0;

    if ( pCertRequestData == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (pszPrivateKey == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(pAllocatedCSR == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACreateSigningRequestPrivate(
                  pCertRequestData,
                  pszPrivateKey,
                  pwszPassPhrase,
                  pAllocatedCSR);
    BAIL_ON_ERROR(dwError);

error :
    return dwError;
}


VOID
VMCAFreePKCS10DataW(
    PVMCA_PKCS_10_REQ_DATAW pCertRequestData
)// This function takes an allocated PVMCA_PKCS_10_REQ_DATA structure
// and deep frees it. Make sure that there are no dangling pointers.
//
// Arguments :
//     pCertRequestData : The Request Data Structure pointer
// Returns:
//     None
{
    if ( pCertRequestData != NULL) {
        VMCASetPKCSMemberW(&pCertRequestData->pszName,       NULL);
        VMCASetPKCSMemberW(&pCertRequestData->pszDomainName, NULL);
        VMCASetPKCSMemberW(&pCertRequestData->pszOU,         NULL);
        VMCASetPKCSMemberW(&pCertRequestData->pszState,      NULL);
        VMCASetPKCSMemberW(&pCertRequestData->pszEmail,      NULL);
        VMCASetPKCSMemberW(&pCertRequestData->pszIPAddress,  NULL);
        VMCAFreeMemory(pCertRequestData);
    }
    pCertRequestData = NULL;
}

VOID
VMCAFreePKCS10DataA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequestData
)// This function takes an allocated PVMCA_PKCS_10_REQ_DATA structure
// and deep frees it. Make sure that there are no dangling pointers.
//
// Arguments :
//     pCertRequestData : The Request Data Structure pointer
// Returns:
//     None
{
    if ( pCertRequestData != NULL) {
        VMCASetPKCSMemberA(&pCertRequestData->pszName,       NULL);
        VMCASetPKCSMemberA(&pCertRequestData->pszDomainName, NULL);
        VMCASetPKCSMemberA(&pCertRequestData->pszOU,         NULL);
        VMCASetPKCSMemberA(&pCertRequestData->pszState,      NULL);
        VMCASetPKCSMemberA(&pCertRequestData->pszEmail,      NULL);
        VMCASetPKCSMemberA(&pCertRequestData->pszIPAddress,  NULL);
        VMCAFreeMemory(pCertRequestData);
    }
    pCertRequestData = NULL;
}


VOID
VMCASetPKCSMemberW(
    PWSTR *ppszMember,
    PCWSTR pszNewValue
)
// This function Sets the member value of a PKCS_10_REQ,
// if member is pointing to something valid, then it is freed,
// before allocating and copying new structure
//g
// Args :
//  ppszMemeber - Pointer Pointer to the Memeber variable
//  pszNewValue - Pointer to the NewValue to be set, setting NULL
//  frees the old object and initializes the pointer to NULL.
//
// Returns :
//  None
{

    if (*ppszMember) {
        VMCA_SAFE_FREE_STRINGW(*ppszMember);
    }

    if (pszNewValue != NULL) {
        VMCAAllocateStringW(pszNewValue,ppszMember);
    }
}

DWORD
VMCAInitPKCS10DataA(
    PCSTR pwszName,
    PCSTR pwszOrganization,
    PCSTR pwszOU,
    PCSTR pwszState,
    PCSTR pwszCountry,
    PCSTR pwszEmail,
    PCSTR pwszIPAddress,
    PVMCA_PKCS_10_REQ_DATAA pCertRequestData
)
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

{
    DWORD dwError = 0;

    if (pCertRequestData == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    //
    // Country is always a 2 Letter Country Code
    //
    if (pwszCountry != NULL) {

        SIZE_T lenString = 0;

        lenString = strlen(pwszCountry);
        if (lenString != 2)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_ERROR(dwError);
        }
    }

    VMCASetPKCSMemberA(&pCertRequestData->pszName, pwszName);
    VMCASetPKCSMemberA(&pCertRequestData->pszOrganization, pwszOrganization);
    VMCASetPKCSMemberA(&pCertRequestData->pszOU, pwszOU);
    VMCASetPKCSMemberA(&pCertRequestData->pszState, pwszState);
    VMCASetPKCSMemberA(&pCertRequestData->pszEmail, pwszEmail);
    VMCASetPKCSMemberA(&pCertRequestData->pszIPAddress, pwszIPAddress);
    VMCASetPKCSMemberA(&pCertRequestData->pszCountry, pwszCountry);

error:
    return dwError;
}

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
    )
{

    DWORD dwError = 0;

    dwError = VMCAInitPKCS10DataA(
                        pszName,
                        pszOrganization,
                        pszOU,
                        pszState,
                        pszCountry,
                        pszEmail,
                        pszIPAddress,
                        pCertRequestData
                        );
    BAIL_ON_ERROR (dwError);

    VMCASetPKCSMemberA(&pCertRequestData->pszDomainName, pszDomainName);

cleanup:
    return dwError;
error:
    goto cleanup;
}



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
    )
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
{
    DWORD dwError = 0;

    if (pCertRequestData == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    //
    // Country is always a 2 Letter Country Code
    //
    if (pwszCountry != NULL) {

        SIZE_T lenString = 0;
        dwError = VMCAGetStringLengthW(pwszCountry, &lenString);
        BAIL_ON_ERROR(dwError);

        if (lenString != 2)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_ERROR(dwError);
        }
    }

    VMCASetPKCSMemberW(&pCertRequestData->pszName, pwszName);
    VMCASetPKCSMemberW(&pCertRequestData->pszOrganization, pwszOrganization);
    VMCASetPKCSMemberW(&pCertRequestData->pszOU, pwszOU);
    VMCASetPKCSMemberW(&pCertRequestData->pszState, pwszState);
    VMCASetPKCSMemberW(&pCertRequestData->pszEmail, pwszEmail);
    VMCASetPKCSMemberW(&pCertRequestData->pszIPAddress, pwszIPAddress);
    VMCASetPKCSMemberW(&pCertRequestData->pszCountry, pwszCountry);


error:
    return dwError;
}

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
    )
{

    DWORD dwError = 0;

    dwError = VMCAInitPKCS10DataW(
                        pwszName,
                        pwszOrganization,
                        pwszOU,
                        pwszState,
                        pwszCountry,
                        pwszEmail,
                        pwszIPAddress,
                        pCertRequestData
                        );
    BAIL_ON_ERROR (dwError);

    VMCASetPKCSMemberW(&pCertRequestData->pszDomainName, pwszDomainName);

cleanup:
    return dwError;
error:
    goto cleanup;
}

static DWORD
VMCAAddRootCertificatePrivate(
    handle_t BindingHandle,
    PVMCA_CERTIFICATE pszRootCertificate,
    PWSTR pwszPassPhrase,
    PVMCA_KEY pszPrivateKey,
    PWSTR pwszSharedSecret)
{
    DWORD dwError = 0;
    VMCARpcCall(
        RpcVMCAAddRootCertificate(
                BindingHandle,
                (idl_char*) pszRootCertificate,
                pwszPassPhrase,
                (idl_char*) pszPrivateKey,
                1,
                pwszSharedSecret
                )
        );
    BAIL_ON_ERROR(dwError);

error:

    if (dwError != 0)
    {
        printf("Error: %d, VMCAAddRootCertificatePrivate() failed", dwError);
    }
    return dwError;
}

DWORD
VMCAAddRootCertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PWSTR pwszPassPhrase,
    PVMCA_KEY pszPrivateKey)
{
    handle_t BindingHandleKerb = NULL;
    PWSTR pwszSharedSecret = NULL;
    PSTR pszRpcHandle = NULL;
    DWORD dwError = 0;

    VMCA_VERIFY_SERVER(pwszServerName);

    if ( pszRootCertificate == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(pszPrivateKey == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandleKerb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                        pwszServerName,
                        NULL,
                        &BindingHandleKerb
                        );
        BAIL_ON_ERROR(dwError);
    }
    dwError = VMCAAddRootCertificatePrivate(
                        BindingHandleKerb,
                        pszRootCertificate,
                        pwszPassPhrase,
                        pszPrivateKey,
                        NULL
                        );

    BAIL_ON_ERROR(dwError);
error:

    if (!hInBinding && BindingHandleKerb)
    {
        VMCAFreeBindingHandle(&BindingHandleKerb);
    }

    if (pszRpcHandle)
    {
        DWORD tsts = 0;
        rpc_string_free((unsigned_char_p_t *) &pszRpcHandle, &tsts);
    }

    VMCA_SAFE_FREE_STRINGW(pwszSharedSecret);

    return dwError;
}

DWORD
VMCAAddRootCertificateW(
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PWSTR pwszPassPhrase,
    PVMCA_KEY pszPrivateKey
    )
{
    DWORD dwError = 0;

    dwError = VMCAAddRootCertificateHW(
                  NULL,
                  pwszServerName,
                  pszRootCertificate,
                  pwszPassPhrase,
                  pszPrivateKey);

    return dwError;
}

DWORD
VMCACopyServerString(PVMCA_CERTIFICATE_CONTAINER pContainer, PSTR* ppValueString)
{
    DWORD dwError = 0;
    dwError = VMCAAllocateMemory(pContainer->dwCount + 1, (PVOID*)ppValueString);
    BAIL_ON_ERROR(dwError);

    memset(*ppValueString,0, pContainer->dwCount +1);
    memcpy(*ppValueString, pContainer->pCert, pContainer->dwCount);
error:
    return dwError;
}

static
DWORD VMCAGetSignedCertificatePrivate(
      handle_t BindingHandle,
      PVMCA_CSR pCSR,
      time_t tmNotBefore,
      time_t tmNotAfter,
      DWORD *dwCertLength,
      PVMCA_CERTIFICATE_CONTAINER *ppCertContainer,
      PWSTR pszSharedSecret)
{
    DWORD dwError = 0;
    VMCARpcCall(
         RpcVMCAGetSignedCertificate(
                    BindingHandle,
                    (unsigned char*)pCSR,
                    (unsigned int)tmNotBefore,
                    (unsigned int)tmNotAfter,
                    pszSharedSecret,
                    (unsigned int*)dwCertLength,
                    ppCertContainer
                    )
        );
    BAIL_ON_ERROR(dwError);
error:
    if (dwError != 0)
    {
        printf("Error: %d, VMCAGetSignedCertificatePrivate() failed", dwError);
    }
    return dwError;
}

DWORD
VMCAGetSignedCertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PVMCA_KEY pPrivateKey,
    PWSTR pwszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
    )
{
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

    handle_t BindingHandleKrb = NULL;
    PWSTR pwszSharedSecret = NULL;

    DWORD dwError = 0;
    PVMCA_CERTIFICATE_CONTAINER pCertContainer = NULL;
    PVMCA_CSR pCSR = NULL;
    DWORD dwCertLength = 0;
    PSTR pszRpcHandle = NULL;

    VMCA_VERIFY_SERVER(pwszServerName);
    if (pCertRequest == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (pPrivateKey == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if( tmNotBefore >= tmNotAfter){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if( ppCert == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACreateSigningRequestW(
                  pCertRequest,
                  pPrivateKey,
                  pwszPassPhrase,
                  &pCSR);

    BAIL_ON_ERROR(dwError);

    dwError = VMCAGetSignedCertificatePrivate(
                    BindingHandleKrb,
                    pCSR,
                    tmNotBefore,
                    tmNotAfter,
                    &dwCertLength,
                    &pCertContainer,
                    NULL);

    BAIL_ON_ERROR(dwError);

    dwError = VMCACopyServerString(pCertContainer, ppCert);
    BAIL_ON_ERROR(dwError);

error:

    if (pCSR)
    {
        VMCAFreeCSR(pCSR);
    }
    if (pCertContainer)
    {
        VMCAFreeCertificateContainer(pCertContainer);
    }

    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }

    VMCA_SAFE_FREE_STRINGW(pwszSharedSecret);

    if (pszRpcHandle)
    {
        DWORD tsts = 0;
        rpc_string_free((unsigned_char_p_t *) &pszRpcHandle, &tsts);
    }

    return dwError;

}

DWORD
VMCAGetSignedCertificateW(
    PCWSTR pwszServerName,
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PVMCA_KEY pPrivateKey,
    PWSTR pwszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetSignedCertificateHW(
                  NULL,
                  pwszServerName,
                  pCertRequest,
                  pPrivateKey,
                  pwszPassPhrase,
                  tmNotBefore,
                  tmNotAfter,
                  ppCert);
    return dwError;
}

DWORD
VMCAGetSignedCertificateFromCSRHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert)
{
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

    handle_t BindingHandleKrb = NULL;
    PWSTR pwszSharedSecret = NULL;
    DWORD dwError = 0;
    PVMCA_CERTIFICATE_CONTAINER pCertContainer = NULL;
    PVMCA_CSR pCSR = NULL;
    DWORD dwCertLength = 0;
    PSTR pszRpcHandle = NULL;

    if (pCertRequest == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(pwszServerName == NULL && hInBinding == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(tmNotBefore >= tmNotAfter)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pCSR = (PVMCA_CSR) pCertRequest;
    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAGetSignedCertificatePrivate(
                        BindingHandleKrb,
                        pCSR,
                        tmNotBefore,
                        tmNotAfter,
                        &dwCertLength,
                        &pCertContainer,
                        NULL);

    BAIL_ON_ERROR(dwError);

    dwError = VMCACopyServerString(pCertContainer, ppCert);
    BAIL_ON_ERROR(dwError);
error:
    if (pCertContainer)
    {
        VMCAFreeCertificateContainer(pCertContainer);
    }

    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }

    VMCA_SAFE_FREE_STRINGW(pwszSharedSecret);

    if (pszRpcHandle)
    {
        DWORD tsts = 0;
        rpc_string_free((unsigned_char_p_t *) &pszRpcHandle, &tsts);
    }

    return dwError;

}

DWORD
VMCAGetSignedCertificateFromCSRW(
    PCWSTR pwszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetSignedCertificateFromCSRHW(
                  NULL,
                  pwszServerName,
                  pCertRequest,
                  tmNotBefore,
                  tmNotAfter,
                  ppCert);

    return dwError;
}

static
DWORD
VMCAGetRootCACertificatePrivate(
    handle_t BindingHandleKrb,
    DWORD *dwCertLength,
    PVMCA_CERTIFICATE_CONTAINER *pCertContainer
    )
{
    DWORD dwError = 0;
    VMCARpcCall(
         RpcVMCAGetRootCACertificate(
            BindingHandleKrb,
            (unsigned int*)dwCertLength,
            pCertContainer
            )
        );
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}


DWORD
VMCAGetRootCACertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE* ppCertificate
    )
{
    handle_t BindingHandleKrb = NULL;
    handle_t BindingHandleSharedSecret = NULL;
    DWORD dwError = 0;
    PVMCA_CERTIFICATE_CONTAINER pCertContainer = NULL;
    DWORD dwCertLength = 0;
    PSTR pszRpcHandle = NULL;

    if(pwszServerName == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    if(ppCertificate == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                        pwszServerName,
                        NULL,
                        &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAGetRootCACertificatePrivate(
                        BindingHandleKrb,
                        &dwCertLength,
                        &pCertContainer);
    BAIL_ON_ERROR(dwError);

    dwError = VMCACopyServerString(pCertContainer, ppCertificate);
    BAIL_ON_ERROR(dwError);

error:
    if (pCertContainer) {
        VMCAFreeCertificateContainer(pCertContainer);
    }
    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }
    if(BindingHandleSharedSecret)
    {
        VMCAFreeBindingHandle(&BindingHandleSharedSecret);
    }
    if (pszRpcHandle)
    {
        DWORD tsts = 0;
        rpc_string_free((unsigned_char_p_t *) &pszRpcHandle, &tsts);
    }

    return dwError;
}

DWORD
VMCAGetRootCACertificateW(
    PCWSTR pszServerName,
    PVMCA_CERTIFICATE* ppCertificate
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetRootCACertificateHW(
                  NULL,
                  pszServerName,
                  ppCertificate);

    return dwError;
}


DWORD
VMCARevokeCertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PSTR pszPEMEncodedCertificate
    )
{
// VMCARevokeCertificate function revokes an existing certificate.
// Please Note : This API will change when we integrate with User Identity provided
// by Lotus.
//
// Arguments :
//      pszServerName : CA Server Name
//      pszPEMEncodedCertificate : The PEM Encoded Certificate that needs to be revoked
// Returns :
//      Error Code -
//
//
    DWORD dwError = 0;

    if (IsNullOrEmptyString (pwszServerName) ||
        IsNullOrEmptyString (pszPEMEncodedCertificate))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VMCARevokeCertW(
                    hInBinding,
                    pwszServerName,
                    pszPEMEncodedCertificate,
                    VMCA_CRL_REASON_UNSPECIFIED
                    );

    BAIL_ON_VMCA_ERROR (dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VMCARevokeCertificateW(
    PCWSTR pwszServerName,
    PSTR pszPEMEncodedCertificate
    )
{
    DWORD dwError = 0;

    dwError = VMCARevokeCertificateHW(
                    NULL,
                    pwszServerName,
                    pszPEMEncodedCertificate
                    );
    return dwError;
}

DWORD
VMCAVerifyCertificateHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
    )
{
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
    handle_t BindingHandle = NULL;
    DWORD dwError = 0;

    if(pwszServerName == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(pszPEMEncodedCertificate == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }


    if (hInBinding)
    {
        BindingHandle = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &BindingHandle);
        BAIL_ON_ERROR(dwError);
    }

    VMCARpcCall(
        RpcVMCAVerifyCertificate(
                    BindingHandle,
                    (PSTR)pszPEMEncodedCertificate,
                    (unsigned int*)dwStatus)
        );
    BAIL_ON_ERROR(dwError);

error:

    if (!hInBinding && BindingHandle)
    {
        VMCAFreeBindingHandle(&BindingHandle);
    }

    return dwError;
}

DWORD
VMCAVerifyCertificateW(
    PCWSTR pwszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
)
{
    DWORD dwError = 0;

    dwError = VMCAVerifyCertificateHW(
                  NULL,
                  pwszServerName,
                  pszPEMEncodedCertificate,
                  dwStatus);
    return dwError;
}


DWORD
VMCAFindCertificatesW(
    PCWSTR pszServerName,
    DWORD  dwSearchQueryLength,
    PCWSTR pszSearchQuery,
    DWORD dwMaxCount,
    DWORD  *dwCertificateCount,
    VMCA_CERTIFICATE_CONTAINER ** ppCertContainer
)
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
{
    return VMCA_NOT_IMPLEMENTED;
}

static DWORD
VMCASetServerOptionPrivate(
    handle_t BindingHandleKrb,
    unsigned int dwOption
    )
{
    DWORD dwError = 0;

    VMCARpcCall(
            RpcVMCASetServerOption(
                    BindingHandleKrb,
                    dwOption));
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}

DWORD
VMCASetServerOptionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    unsigned int dwOption
    )
{
    DWORD dwError = 0;
    handle_t BindingHandleKrb = NULL;
    handle_t BindingHandleSharedSecret = NULL;

    if (IsNullOrEmptyString(pwszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                pwszServerName, NULL, &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    // Try with KRB handle
    dwError = VMCASetServerOptionPrivate(BindingHandleKrb, dwOption);

    if (dwError && !isVMCAErrorCode(dwError))
    {
        dwError = CreateBindingHandleSharedKeyW(
                pwszServerName, NULL, &BindingHandleSharedSecret);
        BAIL_ON_ERROR(dwError);

        // Try with shared secret handle Since the server side does not
        // authenticate, this call will let us go in irrespective of
        // if you are root or not.
        dwError = VMCASetServerOptionPrivate(
                BindingHandleSharedSecret, dwOption);
    }

    BAIL_ON_ERROR(dwError);

error:
    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }
    if (BindingHandleSharedSecret)
    {
        VMCAFreeBindingHandle(&BindingHandleSharedSecret);
    }
    return dwError;
}

DWORD
VMCASetServerOptionW(
    PCWSTR pwszServerName,
    unsigned int dwOption
    )
{
    return VMCASetServerOptionHW(NULL, pwszServerName, dwOption);
}

DWORD
VMCASetServerOptionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    unsigned int dwOption
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (IsNullOrEmptyString(pszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateStringWFromA(pszServerName, &pwszServerName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASetServerOptionHW(hInBinding, pwszServerName, dwOption);
    BAIL_ON_VMCA_ERROR(dwError);

error:
    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCASetServerOptionA(
    PCSTR pszServerName,
    unsigned int dwOption
    )
{
    return VMCASetServerOptionHA(NULL, pszServerName, dwOption);
}

static DWORD
VMCAUnsetServerOptionPrivate(
    handle_t BindingHandleKrb,
    unsigned int dwOption
    )
{
    DWORD dwError = 0;

    VMCARpcCall(
            RpcVMCAUnsetServerOption(
                    BindingHandleKrb,
                    dwOption));
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}

DWORD
VMCAUnsetServerOptionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    unsigned int dwOption
    )
{
    DWORD dwError = 0;
    handle_t BindingHandleKrb = NULL;
    handle_t BindingHandleSharedSecret = NULL;

    if (IsNullOrEmptyString(pwszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                pwszServerName, NULL, &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    // Try with KRB handle
    dwError = VMCAUnsetServerOptionPrivate(BindingHandleKrb, dwOption);

    if (dwError && !isVMCAErrorCode(dwError))
    {
        dwError = CreateBindingHandleSharedKeyW(
                pwszServerName, NULL, &BindingHandleSharedSecret);
        BAIL_ON_ERROR(dwError);

        // Try with shared secret handle Since the server side does not
        // authenticate, this call will let us go in irrespective of
        // if you are root or not.
        dwError = VMCAUnsetServerOptionPrivate(
                BindingHandleSharedSecret, dwOption);
    }

    BAIL_ON_ERROR(dwError);

error:
    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }
    if (BindingHandleSharedSecret)
    {
        VMCAFreeBindingHandle(&BindingHandleSharedSecret);
    }
    return dwError;
}

DWORD
VMCAUnsetServerOptionW(
    PCWSTR pwszServerName,
    unsigned int dwOption
    )
{
    return VMCAUnsetServerOptionHW(NULL, pwszServerName, dwOption);
}

DWORD
VMCAUnsetServerOptionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    unsigned int dwOption
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (IsNullOrEmptyString(pszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateStringWFromA(pszServerName, &pwszServerName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAUnsetServerOptionHW(hInBinding, pwszServerName, dwOption);
    BAIL_ON_VMCA_ERROR(dwError);

error:
    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAUnsetServerOptionA(
    PCSTR pszServerName,
    unsigned int dwOption
    )
{
    return VMCAUnsetServerOptionHA(NULL, pszServerName, dwOption);
}

static DWORD
VMCAGetServerOptionPrivate(
    handle_t BindingHandleKrb,
    unsigned int *pdwOption
    )
{
    DWORD dwError = 0;

    VMCARpcCall(
            RpcVMCAGetServerOption(
                    BindingHandleKrb,
                    pdwOption));
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}

DWORD
VMCAGetServerOptionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    unsigned int *pdwOption
    )
{
    DWORD dwError = 0;
    handle_t BindingHandleKrb = NULL;
    handle_t BindingHandleSharedSecret = NULL;

    if (IsNullOrEmptyString(pwszServerName) || !pdwOption)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                pwszServerName, NULL, &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    // Try with KRB handle
    dwError = VMCAGetServerOptionPrivate(BindingHandleKrb, pdwOption);

    if (dwError && !isVMCAErrorCode(dwError))
    {
        dwError = CreateBindingHandleSharedKeyW(
                pwszServerName, NULL, &BindingHandleSharedSecret);
        BAIL_ON_ERROR(dwError);

        // Try with shared secret handle Since the server side does not
        // authenticate, this call will let us go in irrespective of
        // if you are root or not.
        dwError = VMCAGetServerOptionPrivate(
                BindingHandleSharedSecret, pdwOption);
    }

    BAIL_ON_ERROR(dwError);

error:
    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }
    if (BindingHandleSharedSecret)
    {
        VMCAFreeBindingHandle(&BindingHandleSharedSecret);
    }
    return dwError;
}

DWORD
VMCAGetServerOptionW(
    PCWSTR pwszServerName,
    unsigned int *pdwOption
    )
{
    return VMCAGetServerOptionHW(NULL, pwszServerName, pdwOption);
}

DWORD
VMCAGetServerOptionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    unsigned int *pdwOption
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (IsNullOrEmptyString(pszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateStringWFromA(pszServerName, &pwszServerName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetServerOptionHW(hInBinding, pwszServerName, pdwOption);
    BAIL_ON_VMCA_ERROR(dwError);

error:
    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAGetServerOptionA(
    PCSTR pszServerName,
    unsigned int *pdwOption
    )
{
    return VMCAGetServerOptionHA(NULL, pszServerName, pdwOption);
}

static DWORD
VMCAGetServerVersionPrivate(
      handle_t BindingHandleKrb,
      DWORD *dwCertLength,
      PVMCA_CERTIFICATE_CONTAINER *pServerVersion )
{
    DWORD dwError = 0;

    VMCARpcCall(
            RpcVMCAGetServerVersion(
                    BindingHandleKrb,
                    (unsigned int*)dwCertLength,
                    pServerVersion));
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}


DWORD
VMCAGetServerVersionHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PSTR* ppszServerVersionString
    )
{
// VMCA Get Server Version returs a Server Version String from the Server
//
// Arguments :
//      pszServerName : Name of the Server to talk to
//      dwCertLength : The  number of Server Version Strings
//      pServerVersion : since certificate is string using the same structure for
//      our purpose

    handle_t BindingHandleKrb = NULL;
    handle_t BindingHandleSharedSecret = NULL;

    DWORD dwError = 0;
    PVMCA_CERTIFICATE_CONTAINER pServerVersion = NULL;
    DWORD dwCertLength = 0;
    PSTR pszServerString = NULL;

    if(pwszServerName == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if( ppszServerVersionString == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }


    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    // Try with KRB handle
    dwError = VMCAGetServerVersionPrivate(
        BindingHandleKrb,
        &dwCertLength,
        &pServerVersion);

    if ((dwError != 0) && isVMCAErrorCode(dwError) != TRUE) {
        dwError = CreateBindingHandleSharedKeyW(
            pwszServerName,
            NULL,
            &BindingHandleSharedSecret
            );
        BAIL_ON_ERROR(dwError);

        // Try with Shared Secret Handle Since the Server Side does not Authenticate, This call will let us go in
        // irrespective of if you are root or not.
        dwError = VMCAGetServerVersionPrivate(
                    BindingHandleSharedSecret,
                    &dwCertLength,
                    &pServerVersion);
    }

    BAIL_ON_ERROR(dwError);
    dwError = VMCACopyServerString(pServerVersion, &pszServerString);
    BAIL_ON_ERROR(dwError);

    *ppszServerVersionString = pszServerString;

error:
    if(pServerVersion) {
        VMCAFreeCertificateContainer(pServerVersion);
    }
    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }
    if(BindingHandleSharedSecret)
    {
        VMCAFreeBindingHandle(&BindingHandleSharedSecret);
    }

    return dwError;
}

DWORD
VMCAGetServerVersionW(
    PCWSTR pwszServerName,
    PSTR* ppszServerVersionString
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetServerVersionHW(
                  NULL,
                  pwszServerName,
                  ppszServerVersionString);
    return dwError;
}


DWORD
VMCAReadCertificateFromFile(
    PSTR pszFileName,
    PVMCA_CERTIFICATE* ppCertificate
)
{
    DWORD dwError = 0;
    dwError = VMCAReadCertificateFromFilePrivate(
            pszFileName,
            (PSTR*) ppCertificate);
    return dwError;
}

DWORD
VMCAReadPrivateKeyFromFile(
    PSTR pszFileName,
    PSTR pszPassPhrase,
    PVMCA_KEY* ppPrivateKey
)
{
    DWORD dwError = 0;
    dwError = VMCAReadPrivateKeyFromFilePrivate(
            pszFileName,
            pszPassPhrase,
            (PSTR*) ppPrivateKey);
    return dwError;

}


DWORD
VMCAValidateCACertificate(
    PVMCA_CERTIFICATE pszCertificate
)
{
    DWORD dwError = 0;
    dwError = VMCAValidateCACertificatePrivate((PSTR) pszCertificate, NULL, NULL);
    return dwError;
}


DWORD
VMCAReadPublicKeyFromFile(
    PSTR pszFileName,
    PVMCA_KEY* ppPublicKey
)
{
    // Not implimented yet
    return 0;
}


VOID
VMCAFreeCertificateContainer(
    PVMCA_CERTIFICATE_CONTAINER pCertContainer
)
{
    if (pCertContainer) {
        if ( pCertContainer->pCert != NULL) {
            VMCA_RPC_SAFE_FREE_MEMORY(pCertContainer->pCert);
            pCertContainer->pCert = NULL;
        }

        VMCA_RPC_SAFE_FREE_MEMORY(pCertContainer);
    }
    pCertContainer = NULL;
}

VOID
VMCAFreeCertificateArray(
    PVMCA_CERTIFICATE_ARRAY pCertArray
)
{
    unsigned int iEntry  = 0;
    if (pCertArray == NULL) return;

    // free string in each container
    for (; iEntry < pCertArray->dwCount; iEntry++) {
        VMCA_RPC_SAFE_FREE_MEMORY(pCertArray->certificates[iEntry].pCert);
        pCertArray->certificates[iEntry].pCert = NULL;
    }
    VMCA_RPC_SAFE_FREE_MEMORY(pCertArray);
    pCertArray = NULL;
}

static DWORD
VMCAAllocateEnumContext(
    PVOID *ppContext,
    PCWSTR pwszServerName
)
{
    PVMCA_ENUM_CONTEXT pContext = NULL;
    DWORD dwError = 0;

    dwError = VMCAAllocateMemory(sizeof(VMCA_ENUM_CONTEXT),(PVOID*) &pContext);
    BAIL_ON_ERROR(dwError);

    memset(pContext,0, sizeof(VMCA_ENUM_CONTEXT));

    //
    // This number will change based on performance tests.
    pContext->iChunkSize = ENUM_CERT_SIZE;

    dwError = VMCAAllocateStringW(pwszServerName, &pContext->pwczServerName);
    BAIL_ON_ERROR(dwError);

    *ppContext = pContext; 
    pContext = NULL;

cleanup:
    return dwError;

error :
    *ppContext = NULL; 
    VMCAFreeEnumContext(pContext);
    goto cleanup;
}

static VOID
VMCAFreeEnumContext(PVOID pContext)
{
    PVMCA_ENUM_CONTEXT pConTemp = NULL;
    pConTemp = (PVMCA_ENUM_CONTEXT) pContext;

    if (pConTemp != NULL)
    {
        if (pConTemp->pwczServerName != NULL)
        {
            VMCAFreeStringW(pConTemp->pwczServerName);
            pConTemp->pwczServerName = NULL;
        }
        if (pConTemp->pCertChain != NULL)
        {
            VMCAFreeCertificateArray(pConTemp->pCertChain);
            pConTemp->pCertChain = NULL;
        }
        if (pConTemp->bAllocatedHandle && pConTemp->hInBinding)
        {
            VMCAFreeBindingHandle(&pConTemp->hInBinding);
        }
        VMCAFreeMemory(pConTemp);
    }
}

DWORD
VMCAOpenEnumContextHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *ppContext
)
{
    DWORD dwError = 0;
    PVMCA_ENUM_CONTEXT pContext = NULL;

    if(pwszServerName == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(ppContext == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateEnumContext((PVOID*)&pContext, pwszServerName);
    BAIL_ON_ERROR(dwError);

    // assign cert filter
    pContext->iFilterMask = Filter;

    if (hInBinding)
    {
        pContext->hInBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &pContext->hInBinding);
        BAIL_ON_ERROR(dwError);
        pContext->bAllocatedHandle = TRUE;
    }

    dwError = VMCAEnumCertsHW(
                  pContext->hInBinding,
                  pwszServerName,
                  pContext->iFilterMask,
                  pContext->iCurrentServerIndex,
                  pContext->iChunkSize,
                  &pContext->pCertChain);

    BAIL_ON_ERROR(dwError);
    
    *ppContext = pContext;
    pContext = NULL;

error :
    if (pContext)
    {
        VMCAFreeEnumContext(pContext);
        pContext = NULL;
    }

    return dwError;
}

DWORD
VMCAOpenEnumContextW(
    PCWSTR pwszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *ppContext
)
{
    DWORD dwError = 0;

    dwError = VMCAOpenEnumContextHW(
                  NULL,
                  pwszServerName,
                  Filter,
                  ppContext);
    return dwError;
}

DWORD
VMCAGetNextCertificate(
    PVOID pContext,
    PVMCA_CERTIFICATE* ppCertificate,
    int *pCurrentIndex,
    VMCA_ENUM_CERT_RETURN_CODE* enumStatus
    )
{
    DWORD dwError = 0;
    PVMCA_ENUM_CONTEXT pLocalContext = NULL;
    DWORD dwLen;
    DWORD dwCertIndex;

    if(pContext == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(ppCertificate == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(pCurrentIndex == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(enumStatus == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    *enumStatus = VMCA_ENUM_ERROR;
    pLocalContext = (PVMCA_ENUM_CONTEXT) pContext;

    // only return when user requested index if within total certificates range
    dwCertIndex = pLocalContext->iCurrentUserIndex - pLocalContext->iCurrentServerIndex;
    if (!pLocalContext->pCertChain || dwCertIndex >= pLocalContext->pCertChain->dwCount) {
        // dwCertIndex should exactly match pCertChain->dwCount when we get here
        if (pLocalContext->pCertChain != NULL) {
            pLocalContext->iCurrentServerIndex += pLocalContext->pCertChain->dwCount;
            VMCAFreeCertificateArray(pLocalContext->pCertChain);
            pLocalContext->pCertChain = NULL;
        }
        dwError = VMCAEnumCertsHW(
                      pLocalContext->hInBinding,
                      pLocalContext->pwczServerName,
                      pLocalContext->iFilterMask,
                      pLocalContext->iCurrentServerIndex,
                      pLocalContext->iChunkSize,
                      &pLocalContext->pCertChain);

        BAIL_ON_ERROR(dwError);

        // means server has no more, tell client we are done.
        if ( pLocalContext->pCertChain->dwCount == 0 ) {
            *enumStatus = VMCA_ENUM_END;
            dwError = VMCA_ENUM_SUCCESS;
            goto error; /// Let us exit , this is not really an error
        }
        dwCertIndex = 0;
    }
    dwLen = pLocalContext->pCertChain->certificates[dwCertIndex].dwCount;
    dwError = VMCAAllocateStringWithLengthA(pLocalContext->pCertChain->certificates[dwCertIndex].pCert, dwLen, ppCertificate);
    BAIL_ON_ERROR(dwError);
    pLocalContext->iCurrentUserIndex++;

    *pCurrentIndex = pLocalContext->iCurrentUserIndex;
    *enumStatus = VMCA_ENUM_SUCCESS;

error:
    return dwError;
}


DWORD
VMCACloseEnumContext(
    PVOID Context
)
{
    VMCAFreeEnumContext(Context);
    Context = NULL;

    return 0;
}

static DWORD
VMCAEnumCertsHW(
    handle_t hInBinding,
    PCWSTR pwszServerName,
    CERTIFICATE_STATUS dwStatus,
    DWORD dwStartIndex,
    DWORD dwNumCertificates,
    PVMCA_CERTIFICATE_ARRAY* ppCerts
    )
{
    DWORD dwError = 0;

    if(pwszServerName == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    VMCARpcCall(
        RpcVMCAEnumCertificates(
                hInBinding,
                dwStatus,
                dwStartIndex,
                dwNumCertificates,
                ppCerts)
        );
    BAIL_ON_ERROR(dwError);

error:

    return dwError;
}


DWORD
VMCAGetCertificateCountHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD *dwNumCertificates
)
{
    handle_t BindingHandle = NULL;
    DWORD dwError = 0;

    if(pwszServerName == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandle = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &BindingHandle);
        BAIL_ON_ERROR(dwError);
    }

    VMCARpcCall(
         RpcVMCAGetCertificateCount(
                    BindingHandle,
                    (DWORD)dwStatus,
                    dwNumCertificates)
                );
    BAIL_ON_ERROR(dwError);

error:
    if (!hInBinding && BindingHandle)
    {
        VMCAFreeBindingHandle(&BindingHandle);
    }

    return dwError;
}


DWORD
VMCAGetCertificateCountW(
    PCWSTR pszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD *dwNumCertificates
)
{
    DWORD dwError = 0;

    dwError = VMCAGetCertificateCountHW(
                  NULL,
                  pszServerName,
                  dwStatus,
                  dwNumCertificates);

    return dwError;
}


DWORD
VMCAFreeVersion(PSTR version)
{
    if(version != NULL) {
        VMCAFreeStringA(version);
    }
    return 0;
}


/*
This stupid monstorus function(VMCAJavaGenCert) exists because when I
use JNA and return a structre to JVM, the JVM
writes to that structure and destroys many of my assumptions
about the integrity of my data.

Hence this function takes all parameters needed from Java
and just returns a Certificate, hiding all the Memory Allocation
from JVM.

*/


DWORD
VMCAJavaGenCertA(
    PSTR         pszServerName,
    PSTR         pszName,
    PSTR         pszCountry,
    PSTR         pszLocality,
    PSTR         pszState,
    PSTR         pszOrganization,
    PSTR         pszOU,
    PSTR         pszDNSName,
    PSTR         pszURIName,
    PSTR         pszEmail,
    PSTR         pszIPAddress,
    DWORD        dwKeyUsageConstraints,
    DWORD        dwSelfSigned,
    PVMCA_KEY    pPrivateKey,
    time_t       tmNotBefore,
    time_t       tmNotAfter,
    PVMCA_CERTIFICATE *ppCertificate)
{

    DWORD dwError = 0;
    PVMCA_PKCS_10_REQ_DATAA pCertReqData = NULL;
    PVMCA_CSR pCSR = NULL;

    dwError = VMCAAllocatePKCS10DataA(&pCertReqData);
    BAIL_ON_ERROR(dwError);

    if (!IsNullOrEmptyString(pszName)) {
        dwError = VMCASetCertValueA( VMCA_OID_CN, pCertReqData, pszName);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszOrganization)) {
        dwError = VMCASetCertValueA( VMCA_OID_ORGANIZATION,
                            pCertReqData,pszOrganization);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszOU)){
        dwError = VMCASetCertValueA( VMCA_OID_ORG_UNIT, pCertReqData, pszOU);
        BAIL_ON_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pszState)) {
        dwError = VMCASetCertValueA( VMCA_OID_STATE, pCertReqData, pszState);
        BAIL_ON_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pszCountry)) {
        dwError = VMCASetCertValueA( VMCA_OID_COUNTRY, pCertReqData, pszCountry);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszEmail)) {
        dwError = VMCASetCertValueA( VMCA_OID_EMAIL, pCertReqData, pszEmail);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszIPAddress)) {
        dwError = VMCASetCertValueA( VMCA_OID_IPADDRESS, pCertReqData, pszIPAddress);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszDNSName)) {
        dwError = VMCASetCertValueA( VMCA_OID_DNS, pCertReqData, pszDNSName);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCASetKeyUsageConstraintsA(pCertReqData,dwKeyUsageConstraints);
    BAIL_ON_ERROR(dwError);

//
// Create a Signing Request CSR ( PKCS10)
//
    dwError =  VMCACreateSigningRequestA(
                   pCertReqData,
                   pPrivateKey,
                   NULL,
                   &pCSR);
    BAIL_ON_ERROR(dwError);


//
// Send the CSR to server to get it signed
//
    dwError =  VMCAGetSignedCertificateFromCSRA(
                   pszServerName,
                   pCSR,
                   tmNotBefore,
                   tmNotAfter,
                   ppCertificate);
    BAIL_ON_ERROR(dwError);


error :
    VMCAFreePKCS10DataA(pCertReqData);
    VMCAFreeCSR(pCSR);
    return dwError;
}

/*
This stupid monstorus function(VMCAJavaGenCert) exists because when I
use JNA and return a structre to JVM, the JVM
writes to that structure and destroys many of my assumptions
about the integrity of my data.

Hence this function takes all parameters needed from Java
and just returns a Certificate, hiding all the Memory Allocation
from JVM.

*/


DWORD
VMCAJavaGenCertHA(
    PVMCA_SERVER_CONTEXT pContext,
    PSTR         pszServerName,
    PSTR         pszName,
    PSTR         pszCountry,
    PSTR         pszLocality,
    PSTR         pszState,
    PSTR         pszOrganization,
    PSTR         pszOU,
    PSTR         pszDNSName,
    PSTR         pszURIName,
    PSTR         pszEmail,
    PSTR         pszIPAddress,
    DWORD        dwKeyUsageConstraints,
    DWORD        dwSelfSigned,
    PVMCA_KEY    pPrivateKey,
    time_t       tmNotBefore,
    time_t       tmNotAfter,
    PVMCA_CERTIFICATE *ppCertificate)
{

    DWORD dwError = 0;
    PVMCA_PKCS_10_REQ_DATAA pCertReqData = NULL;
    PVMCA_CSR pCSR = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocatePKCS10DataA(&pCertReqData);
    BAIL_ON_ERROR(dwError);

    if (!IsNullOrEmptyString(pszName)) {
        dwError = VMCASetCertValueA( VMCA_OID_CN, pCertReqData, pszName);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszOrganization)) {
        dwError = VMCASetCertValueA( VMCA_OID_ORGANIZATION,
                            pCertReqData,pszOrganization);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszOU)){
        dwError = VMCASetCertValueA( VMCA_OID_ORG_UNIT, pCertReqData, pszOU);
        BAIL_ON_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pszState)) {
        dwError = VMCASetCertValueA( VMCA_OID_STATE, pCertReqData, pszState);
        BAIL_ON_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pszCountry)) {
        dwError = VMCASetCertValueA( VMCA_OID_COUNTRY, pCertReqData, pszCountry);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszEmail)) {
        dwError = VMCASetCertValueA( VMCA_OID_EMAIL, pCertReqData, pszEmail);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszIPAddress)) {
        dwError = VMCASetCertValueA( VMCA_OID_IPADDRESS, pCertReqData, pszIPAddress);
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszDNSName)) {
        dwError = VMCASetCertValueA( VMCA_OID_DNS, pCertReqData, pszDNSName);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCASetKeyUsageConstraintsA(pCertReqData,dwKeyUsageConstraints);
    BAIL_ON_ERROR(dwError);

//
// Create a Signing Request CSR ( PKCS10)
//
    dwError =  VMCACreateSigningRequestA(
                   pCertReqData,
                   pPrivateKey,
                   NULL,
                   &pCSR);
    BAIL_ON_ERROR(dwError);


//
// Send the CSR to server to get it signed
//
    dwError =  VMCAGetSignedCertificateFromCSRHA(
                   pContext,
                   pszServerName,
                   pCSR,
                   tmNotBefore,
                   tmNotAfter,
                   ppCertificate);
    BAIL_ON_ERROR(dwError);


error :
    VMCAFreePKCS10DataA(pCertReqData);
    VMCAFreeCSR(pCSR);
    return dwError;
}


DWORD
VMCAGetCRLHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PWSTR pszExistCRL,
    PWSTR pwszNewCRLFileName
    )
{
    #define FILE_CHUNK (64 * 1024) - 1
    handle_t BindingHandle = NULL;
    DWORD dwError = 0;
    PVMCA_FILE_BUFFER pFileBuffer = NULL;
    PSTR pszNewCRLFileName = NULL;
    DWORD dwCurrentOffset = 0;
    DWORD dwSize = FILE_CHUNK;
    FILE *pNewFile = NULL;
    DWORD dwLastSize = 0;

    if(pwszServerName == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if(pwszNewCRLFileName == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

#ifndef WIN32
    dwError = VMCAAllocateStringAFromW(pwszNewCRLFileName, &pszNewCRLFileName);
    BAIL_ON_ERROR(dwError);

    pNewFile = fopen(pszNewCRLFileName, "w");
#else
    pNewFile = _wfopen(pwszNewCRLFileName, L"w");
#endif 
    if (pNewFile == NULL){
        dwError = VMCA_CRL_LOCAL_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandle = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &BindingHandle);
        BAIL_ON_ERROR(dwError);
    }

    do {
        VMCARpcCall(
            RpcVMCAGetCRL(
                 BindingHandle,
                 NULL ,  // TODO : Replace with Real Cached CRL ID
                 dwCurrentOffset,
                 dwSize,
                 &pFileBuffer)
            );
         BAIL_ON_ERROR(dwError);
        if(pFileBuffer != NULL){
            dwLastSize = pFileBuffer->dwCount;
            dwCurrentOffset += pFileBuffer->dwCount;
            if( dwLastSize > 0) {
                fwrite(pFileBuffer->buffer,1,pFileBuffer->dwCount, pNewFile);
                VMCAFreeMemory(pFileBuffer->buffer);
            }

            VMCAFreeMemory(pFileBuffer);

            pFileBuffer = NULL;
        }

    }while(dwLastSize > 0);

error:
    if (!hInBinding && BindingHandle)
    {
        VMCAFreeBindingHandle(&BindingHandle);
    }

    if(pFileBuffer != NULL){
        VMCAFreeMemory(pFileBuffer);
    }

    if (pszNewCRLFileName)
    {
        VMCA_SAFE_FREE_STRINGA(pszNewCRLFileName);
    }

    if(pNewFile != NULL){
        fclose(pNewFile);
    }

    return dwError;
}

DWORD
VMCAGetCRLW(
    PCWSTR pwszServerName,
    PWSTR pwszExistCRL,
    PWSTR pwszNewCRLFileName
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetCRLHW(
                  NULL,
                  pwszServerName,
                  pwszExistCRL,
                  pwszNewCRLFileName);

    return dwError;
}


DWORD
VMCAReGenCRLHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName
    )
{

    handle_t BindingHandle = NULL;
    DWORD dwError = 0;

    if(pwszServerName == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (hInBinding)
    {
        BindingHandle = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServerName,
                      NULL,
                      &BindingHandle);
        BAIL_ON_ERROR(dwError);
    }

    VMCARpcCall(
        VMCARpcReGenCRL(BindingHandle)
        );
    BAIL_ON_ERROR(dwError);

error:
    if (!hInBinding && BindingHandle)
    {
        VMCAFreeBindingHandle(&BindingHandle);
    }

    return dwError;
}

DWORD
VMCAReGenCRLW(
    PCWSTR pwszServerName
    )
{
    DWORD dwError = 0;

    dwError = VMCAReGenCRLHW(NULL, pwszServerName);

    return dwError;
}

DWORD
VMCAGetCRLInfo2(
    PSTR pszFileName,
    time_t *ptmLastUpdate,
    time_t *ptmNextUpdate,
    DWORD *pdwCRLNumber)
{
    return
        VMCAGetCRLInfoPrivate(pszFileName,
            ptmLastUpdate,
            ptmNextUpdate,
            pdwCRLNumber);
}

DWORD
VMCAGetCRLInfo(
    PSTR pszFileName,
    DWORD *ptmLastUpdate,
    DWORD *ptmNextUpdate,
    DWORD *pdwCRLNumber)
{
    return 0;
}

DWORD
VMCAGetShortError(
    DWORD dwErrorCode,
    PSTR *pszErrMsg)
{
    int i = 0;
    PSTR pszErr = NULL;
    DWORD dwError = 0;
    VMCA_ERROR_CODE_NAME_MAP VMCA_ERROR_Table[] =
                                 VMCA_ERROR_TABLE_INITIALIZER;

    if ( isVMCAErrorCode(dwErrorCode) == TRUE)
    {
        for (i=0;
            i<sizeof(VMCA_ERROR_Table)/sizeof(VMCA_ERROR_Table[0]);
            i++)
        {
            if ( dwErrorCode == VMCA_ERROR_Table[i].code)
            {
                dwError = VMCAAllocateStringA(
                    (PSTR) VMCA_ERROR_Table[i].name,
                    &pszErr);
                BAIL_ON_ERROR(dwError);
                break;
            }
        }
    }

    if (!pszErr && (IsDceRpcError(dwErrorCode) == TRUE))
    {
        VMCAGetDceRpcShortErrorString(dwErrorCode, &pszErr);
    }

    if (!pszErr)
    {
        dwError = VMCAAllocateStringA(UNKNOWN_STRING, &pszErr);
        BAIL_ON_ERROR(dwError);
    }

    *pszErrMsg = pszErr;
cleanup :
        return dwError;

error :

    if ( pszErr) {
        VMCA_SAFE_FREE_STRINGA(pszErr);
    }
    goto cleanup;
}


DWORD
VMCAGetErrorString(
    DWORD dwErrorCode,
    PSTR *pszErrMsg)
{
    int i = 0;
    DWORD dwError = 0;
    PSTR pszErr = NULL;
    VMCA_ERROR_CODE_NAME_MAP VMCA_ERROR_Table[] = VMCA_ERROR_TABLE_INITIALIZER;

    for (i=0; i<sizeof(VMCA_ERROR_Table)/sizeof(VMCA_ERROR_Table[0]); i++)
    {
        if (dwErrorCode == VMCA_ERROR_Table[i].code)
        {
            dwError = VMCAAllocateStringA((PSTR) VMCA_ERROR_Table[i].desc, &pszErr);
            BAIL_ON_ERROR(dwError);
            break;
        }
    }

    if (!pszErr && (IsDceRpcError(dwErrorCode) == TRUE))
    {
        VMCAGetDceRpcErrorString(dwErrorCode, &pszErr);
    }

    if( !pszErr)
    {
        VMCAGetWin32ErrorString(dwErrorCode, &pszErr);
    }

    if (!pszErr)
    {
        dwError = VMCAAllocateStringA(UNKNOWN_STRING, &pszErr);
        BAIL_ON_ERROR(dwError);
    }

    *pszErrMsg = pszErr;

cleanup :
    return dwError;

error :

    VMCA_SAFE_FREE_STRINGA(pszErr);
    goto cleanup;
}


DWORD
VMCAPrintCRL(
    PSTR pszFileName,
    PSTR *ppszCRLString
)
{

    return VMCAPrintCRLPrivate(
        pszFileName,
        ppszCRLString);

}

DWORD
VMCAPublishRootCertsHW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServername
    )
{
    DWORD    dwError = 0;
    handle_t hBinding = NULL;
    wchar16_t serverName[3] = {0};

    if (pwszServername == NULL)
    {
        pwszServername = serverName; // will attempt a connection to local host
    }

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                      pwszServername,
                      NULL,
                      &hBinding);
        BAIL_ON_ERROR(dwError);
    }

    VMCARpcCall(
        RpcVMCAPublishRootCerts(hBinding);
        );
    BAIL_ON_ERROR(dwError);

error:

    if (!hInBinding && hBinding)
    {
        VMCAFreeBindingHandle(&hBinding);
    }

    return dwError;
}


DWORD
VMCAPublishRootCertsW(
    PCWSTR pwszServername
    )
{
    DWORD dwError = 0;

    dwError = VMCAPublishRootCertsHW(
                  NULL,
                  pwszServername);

    return dwError;
}


////////////////// UTF8 stub functions ///////////////////////////////

static DWORD
VMCAAllocatePKCS10WFromPKCS10A(
    PVMCA_PKCS_10_REQ_DATAA pCertRequestA,
    PVMCA_PKCS_10_REQ_DATAW *ppCertRequestW
    )
{
    DWORD dwError = 0;
    PVMCA_PKCS_10_REQ_DATAW pTemp = NULL;

    dwError = VMCAAllocatePKCS10DataW(&pTemp);
    BAIL_ON_VMCA_ERROR(dwError);

    if (pCertRequestA->pszName)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszName, &pTemp->pszName);
        BAIL_ON_VMCA_ERROR(dwError);
    }
    if (pCertRequestA->pszDomainName)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszDomainName, &pTemp->pszDomainName);
        BAIL_ON_VMCA_ERROR (dwError);
    }

    if (pCertRequestA->pszCountry)
    {
    dwError = VMCAAllocateStringWFromA(pCertRequestA->pszCountry, &pTemp->pszCountry);
    BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszLocality)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszLocality, &pTemp->pszLocality);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszState)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszState, &pTemp->pszState);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszOrganization)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszOrganization, &pTemp->pszOrganization);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszOU)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszOU, &pTemp->pszOU);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszDNSName)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszDNSName, &pTemp->pszDNSName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszURIName)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszURIName, &pTemp->pszURIName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszEmail)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszEmail, &pTemp->pszEmail);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pCertRequestA->pszIPAddress)
    {
        dwError = VMCAAllocateStringWFromA(pCertRequestA->pszIPAddress, &pTemp->pszIPAddress);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pTemp->dwKeyUsageConstraints = pCertRequestA->dwKeyUsageConstraints;

    *ppCertRequestW = pTemp;
    pTemp = NULL;

error:
    if (pTemp)
    {
        VMCAFreePKCS10DataW(pTemp);
    }

    return dwError;
}

DWORD
VMCACreateSelfSignedCertificateA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequestA,
    PVMCA_KEY pszPrivateKey,
    PSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pwszPassPhrase = NULL;
    PVMCA_PKCS_10_REQ_DATAW pCertRequestW = NULL;

    if (pszPassPhrase)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszPassPhrase,
                    &pwszPassPhrase);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocatePKCS10WFromPKCS10A(
                pCertRequestA,
                &pCertRequestW);

    dwError = VMCACreateSelfSignedCertificateW(
                pCertRequestW,
                pszPrivateKey,
                pwszPassPhrase,
                tmNotBefore,
                tmNotAfter,
                ppCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    VMCA_SAFE_FREE_STRINGW(pwszPassPhrase);
    VMCAFreePKCS10DataW(pCertRequestW);

    return dwError;
}

DWORD
VMCAAllocatePKCS10DataA(
    PVMCA_PKCS_10_REQ_DATAA* pCertRequestData
    )
{
    DWORD dwError = 0;
    if (pCertRequestData == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(sizeof(VMCA_PKCS_10_REQ_DATAA), (PVOID*) pCertRequestData);
    BAIL_ON_ERROR(dwError);

    //TODO: Do we need this memset?
    memset((PVOID) *pCertRequestData, 0, sizeof(VMCA_PKCS_10_REQ_DATAA));
error:
    return dwError;
}


VOID
VMCASetPKCSMemberA(
    PSTR *ppszMember,
    PCSTR pszNewValue
    )
{
    if (*ppszMember) {
        VMCA_SAFE_FREE_STRINGA(*ppszMember);
    }

    if (pszNewValue != NULL) {
        VMCAAllocateStringA(pszNewValue,ppszMember);
    }
}

DWORD
VMCASetCertValueA(
    VMCA_OID Field,
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    PSTR pszNewValue
)
{
    DWORD dwError = 0;

    if ( pCertRequest == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    switch(Field)
    {
    case VMCA_OID_CN            :
        VMCASetPKCSMemberA(&pCertRequest->pszName,          pszNewValue);
        break;
    case VMCA_OID_DC:
        VMCASetPKCSMemberA(&pCertRequest->pszDomainName,    pszNewValue);
        break;
    case VMCA_OID_COUNTRY       :
        VMCASetPKCSMemberA(&pCertRequest->pszCountry,       pszNewValue);
        break;
    case VMCA_OID_LOCALITY      :
        VMCASetPKCSMemberA(&pCertRequest->pszLocality,      pszNewValue);
        break;
    case VMCA_OID_STATE         :
        VMCASetPKCSMemberA(&pCertRequest->pszState,         pszNewValue);
        break;
    case VMCA_OID_ORGANIZATION  :
        VMCASetPKCSMemberA(&pCertRequest->pszOrganization,  pszNewValue);
        break;
    case VMCA_OID_ORG_UNIT      :
        VMCASetPKCSMemberA(&pCertRequest->pszOU,            pszNewValue);
        break;
    case VMCA_OID_DNS           :
        VMCASetPKCSMemberA(&pCertRequest->pszDNSName,       pszNewValue);
        break;
    case VMCA_OID_URI           :
        VMCASetPKCSMemberA(&pCertRequest->pszURIName,       pszNewValue);
        break;
    case VMCA_OID_EMAIL         :
        VMCASetPKCSMemberA(&pCertRequest->pszEmail,         pszNewValue);
        break;
    case VMCA_OID_IPADDRESS     :
        VMCASetPKCSMemberA(&pCertRequest->pszIPAddress,     pszNewValue);
        break;
    default :
        dwError = ERROR_INVALID_PARAMETER;
        break;
    }

error :
    return dwError;

}

DWORD
VMCASetKeyUsageConstraintsA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequest,
    DWORD dwKeyUsageMask
    )
{
    DWORD dwError = 0;
    if(pCertRequest == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pCertRequest->dwKeyUsageConstraints = dwKeyUsageMask;

error :
    return dwError;
}

DWORD
VMCACreateSigningRequestA(
    PVMCA_PKCS_10_REQ_DATAA pCertRequestA,
    PSTR pszPrivateKey,
    PSTR pszPassPhrase,
    PVMCA_CSR* pAllocatedCSR
    )
{
    DWORD dwError = 0;
    PWSTR pwszPassPhrase = NULL;
    PVMCA_PKCS_10_REQ_DATAW pCertRequestW = NULL;

    if (pszPassPhrase)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszPassPhrase,
                    &pwszPassPhrase);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocatePKCS10WFromPKCS10A(
                pCertRequestA,
                &pCertRequestW);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACreateSigningRequestW(
                pCertRequestW,
                pszPrivateKey,
                pwszPassPhrase,
                pAllocatedCSR);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    VMCA_SAFE_FREE_STRINGW(pwszPassPhrase);
    VMCAFreePKCS10DataW(pCertRequestW);

    return dwError;
}

DWORD
VMCAAddRootCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PSTR pszPassPhrase,
    PVMCA_KEY pszPrivateKey)
{
    DWORD dwError = 0;
    PWSTR pwszPassPhrase = NULL;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }
        
    if (pszPassPhrase)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszPassPhrase,
                    &pwszPassPhrase);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAddRootCertificateHW(
                    hInBinding,
                    pwszServerName,
                    pszRootCertificate,
                    pwszPassPhrase,
                    pszPrivateKey);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    VMCA_SAFE_FREE_STRINGW(pwszPassPhrase);

    return dwError;
}

DWORD
VMCARevokeCertA (
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServername,
    PVMCA_CERTIFICATE pszCertificate,
    VMCA_CRL_REASON certRevokeReason
    )
{
    DWORD dwError = 0;

    PWSTR pwszServerName = NULL;

    if (IsNullOrEmptyString (pszServername) ||
        !pszCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VMCAAllocateStringWFromA (
                    pszServername,
                    &pwszServerName
                    );

    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCARevokeCertW (
                    hInBinding,
                    pwszServerName,
                    pszCertificate,
                    certRevokeReason
                    );

    BAIL_ON_VMCA_ERROR (dwError);

cleanup:
    VMCA_SAFE_FREE_STRINGW (pwszServerName);

    return dwError;

error:
    goto cleanup;
}

DWORD
VMCARevokeCertW (
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszServerName,
    PVMCA_CERTIFICATE pszCertificate,
    VMCA_CRL_REASON certRevokeReason
    )
{
    DWORD dwError = 0;

    handle_t BindingHandleKrb = NULL;
    PWSTR pwszSharedSecret = NULL;
    PSTR pszRpcHandle = NULL;

    if (IsNullOrEmptyString (pwszServerName) ||
        !pszCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    if (hInBinding)
    {
        BindingHandleKrb = hInBinding->hBinding;
    }
    else
    {
        dwError = CreateBindingHandleKrbW(
                        pwszServerName,
                        NULL,
                        &BindingHandleKrb);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCARevokeCertPrivate(
                    BindingHandleKrb,
                    pwszServerName,
                    pszCertificate,
                    certRevokeReason,
                    NULL
                    );
    BAIL_ON_ERROR(dwError);

cleanup:
    if (!hInBinding && BindingHandleKrb)
    {
        VMCAFreeBindingHandle(&BindingHandleKrb);
    }
    VMCA_SAFE_FREE_STRINGW(pwszSharedSecret);
    if (pszRpcHandle)
    {
        DWORD tsts = 0;
        rpc_string_free((unsigned_char_p_t *) &pszRpcHandle, &tsts);
    }

    return dwError;

error:
    goto cleanup;
}

static
DWORD
VMCARevokeCertPrivate(
    handle_t BindingHandle,
    PCWSTR pwszServerName,
    PSTR pszPEMEncodedCertificate,
    VMCA_CRL_REASON certRevokeReason,
    PWSTR pwszSharedSecret
    )
{
    DWORD dwError = 0;
    VMCARpcCall(
         VMCARpcRevokeCertificate(
                                BindingHandle,
                                (PWSTR)pwszServerName,
                                (unsigned char*)pszPEMEncodedCertificate,
                                certRevokeReason,
                                pwszSharedSecret
                                )
        );
    BAIL_ON_ERROR(dwError);
error:
    return dwError;
}

DWORD
VMCAAddRootCertificateA(
    PCSTR pszServerName,
    PVMCA_CERTIFICATE pszRootCertificate,
    PSTR pszPassPhrase,
    PVMCA_KEY pszPrivateKey
    )
{
    DWORD dwError = 0;

    dwError = VMCAAddRootCertificateHA(
                    NULL,
                    pszServerName,
                    pszRootCertificate,
                    pszPassPhrase,
                    pszPrivateKey);

    return dwError;
}

DWORD
VMCAGetSignedCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_PKCS_10_REQ_DATAA pCertRequestA,
    PVMCA_KEY pPrivateKey,
    PSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
    )
{
    DWORD dwError = 0;
    PWSTR pwszPassPhrase = NULL;
    PWSTR pwszServerName = NULL;
    PVMCA_PKCS_10_REQ_DATAW pCertRequestW = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pszPassPhrase)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszPassPhrase,
                    &pwszPassPhrase);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocatePKCS10WFromPKCS10A(
                    pCertRequestA,
                    &pCertRequestW);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetSignedCertificateHW(
                    hInBinding,
                    pwszServerName,
                    pCertRequestW,
                    pPrivateKey,
                    pwszPassPhrase,
                    tmNotBefore,
                    tmNotAfter,
                    ppCert);
error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    VMCA_SAFE_FREE_STRINGW(pwszPassPhrase);
    VMCAFreePKCS10DataW(pCertRequestW);

    return dwError;
}

DWORD
VMCAGetSignedCertificateA(
    PCSTR pszServerName,
    PVMCA_PKCS_10_REQ_DATAA pCertRequestA,
    PVMCA_KEY pPrivateKey,
    PSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetSignedCertificateHA(
                    NULL,
                    pszServerName,
                    pCertRequestA,
                    pPrivateKey,
                    pszPassPhrase,
                    tmNotBefore,
                    tmNotAfter,
                    ppCert);

    return dwError;
}

DWORD
VMCAGetSignedCertificateFromCSRHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetSignedCertificateFromCSRHW(
                    hInBinding,
                    pwszServerName,
                    pCertRequest,
                    tmNotBefore,
                    tmNotAfter,
                    ppCert);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAGetSignedCertificateFromCSRA(
    PCSTR pszServerName,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCert
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetSignedCertificateFromCSRHA(
                    NULL,
                    pszServerName,
                    pCertRequest,
                    tmNotBefore,
                    tmNotAfter,
                    ppCert);

    return dwError;
}


DWORD
VMCAGetRootCACertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PVMCA_CERTIFICATE* ppCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetRootCACertificateHW(
                    hInBinding,
                    pwszServerName,
                    ppCertificate);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAGetRootCACertificateA(
    PCSTR pszServerName,
    PVMCA_CERTIFICATE* ppCertificate
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetRootCACertificateHA(
                    NULL,
                    pszServerName,
                    ppCertificate);

    return dwError;
}

DWORD
VMCARevokeCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PSTR pszPEMEncodedCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCARevokeCertificateHW(
                    hInBinding,
                    pwszServerName,
                    pszPEMEncodedCertificate);
error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}


DWORD
VMCARevokeCertificateA(
    PCSTR pszServerName,
    PSTR pszPEMEncodedCertificate
    )
{
    DWORD dwError = 0;

    dwError = VMCARevokeCertificateHA(
                    NULL,
                    pszServerName,
                    pszPEMEncodedCertificate);
    return dwError;
}

DWORD
VMCAVerifyCertificateHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAVerifyCertificateHW(
                    hInBinding,
                    pwszServerName,
                    pszPEMEncodedCertificate,
                    dwStatus);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAVerifyCertificateA(
    PCSTR pszServerName,
    PCSTR pszPEMEncodedCertificate,
    DWORD  *dwStatus
)
{
    DWORD dwError = 0;

    dwError = VMCAVerifyCertificateHA(
                    NULL,
                    pszServerName,
                    pszPEMEncodedCertificate,
                    dwStatus);
    return dwError;
}

DWORD
VMCAFindCertificatesA(
    PCSTR pszServerName,
    DWORD dwSearchQueryLength,
    PCSTR pszSearchQuery,
    DWORD dwMaxCount,
    DWORD *dwCertificateCount,
    VMCA_CERTIFICATE_CONTAINER ** ppCertContainer
    )
{
    return VMCA_NOT_IMPLEMENTED;
}

DWORD
VMCAGetServerVersionHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PSTR* ppszServerVersionString
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetServerVersionHW(
                    hInBinding,
                    pwszServerName,
                    ppszServerVersionString);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAGetServerVersionA(
    PCSTR pszServerName,
    PSTR* ppszServerVersionString
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetServerVersionHA(
                    NULL,
                    pszServerName,
                    ppszServerVersionString);

    return dwError;
}

DWORD
VMCAOpenEnumContextHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *ppContext
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAOpenEnumContextHW(
                    hInBinding,
                    pwszServerName,
                    Filter,
                    ppContext);
error:
     
    VMCA_SAFE_FREE_STRINGW(pwszServerName);   
    return dwError;
}

DWORD
VMCAOpenEnumContextA(
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS Filter,
    PVOID *ppContext
    )
{
    DWORD dwError = 0;

    dwError = VMCAOpenEnumContextHA(
                    NULL,
                    pszServerName,
                    Filter,
                    ppContext);

    return dwError;
}

DWORD
VMCAGetCertificateCountHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD* pdwNumCertificates
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetCertificateCountHW(
                    hInBinding,
                    pwszServerName,
                    dwStatus,
                    pdwNumCertificates);
error:
     
    VMCA_SAFE_FREE_STRINGW(pwszServerName);   
    return dwError;
}

DWORD
VMCAGetCertificateCountA(
    PCSTR pszServerName,
    VMCA_CERTIFICATE_STATUS dwStatus,
    DWORD* pdwNumCertificates
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetCertificateCountHA(
                    NULL,
                    pszServerName,
                    dwStatus,
                    pdwNumCertificates);
    
    return dwError;
}


DWORD
VMCAGetCRLHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName,
    PSTR pszExistingCRLFileName,
    PSTR pszNewCRLFileName
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszExistingCRLFileName = NULL;
    PWSTR pwszNewCRLFileName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pszExistingCRLFileName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszExistingCRLFileName,
                    &pwszExistingCRLFileName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pszNewCRLFileName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszNewCRLFileName,
                    &pwszNewCRLFileName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetCRLHW(
                    hInBinding,
                    pwszServerName,
                    pwszExistingCRLFileName,
                    pwszNewCRLFileName);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    VMCA_SAFE_FREE_STRINGW(pwszExistingCRLFileName);
    VMCA_SAFE_FREE_STRINGW(pwszNewCRLFileName);
    return dwError;
}

DWORD
VMCAGetCRLA(
    PCSTR pszServerName,
    PSTR pszExistingCRLFileName,
    PSTR pszNewCRLFileName
    )
{
    DWORD dwError = 0;

    dwError = VMCAGetCRLHA(
                    NULL,
                    pszServerName,
                    pszExistingCRLFileName,
                    pszNewCRLFileName);

    return dwError;
}

DWORD
VMCAReGenCRLHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAReGenCRLHW(
                    hInBinding,
                    pwszServerName);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAReGenCRLA(
    PCSTR pszServerName
    )
{
    DWORD dwError = 0;

    dwError = VMCAReGenCRLHA(
                    NULL,
                    pszServerName);

    return dwError;
}

DWORD
VMCAPublishRootCertsHA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszServerName
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VMCAAllocateStringWFromA(
                    pszServerName,
                    &pwszServerName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAPublishRootCertsHW(
                    hInBinding,
                    pwszServerName);

error:

    VMCA_SAFE_FREE_STRINGW(pwszServerName);
    return dwError;
}

DWORD
VMCAPublishRootCertsA(
    PCSTR pszServerName
    )
{
    DWORD dwError = 0;

    dwError = VMCAPublishRootCertsHA(
                    NULL,
                    pszServerName);

    return dwError;
}

DWORD
VMCAGetCSRFromCertificate(
    PVMCA_CERTIFICATE pszCertificate,
    PVMCA_KEY pszPrivateKey,
    PVMCA_CSR *ppszCSR
    )
{
    DWORD dwError = 0;
    PVMCA_CSR pszCSR = NULL;
    X509_REQ *pReq = NULL;
    X509 *pCert = NULL;
    RSA *pPKey = NULL;
    EVP_PKEY *pKey = NULL;

    if(IsNullOrEmptyString(pszCertificate) ||
       IsNullOrEmptyString(pszPrivateKey) ||
       !ppszCSR
      )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VMCAPEMToX509(
                        pszCertificate,
                        &pCert
                        );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCAPEMToPrivateKey(
                       pszPrivateKey,
                       &pPKey
                       );
    BAIL_ON_VMCA_ERROR (dwError);

    pKey = EVP_PKEY_new();

    if (!pKey)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    EVP_PKEY_assign_RSA(pKey, pPKey);

    pPKey = NULL;

    dwError = VMCAGetCSRFromCert(
                                 pCert,
                                 pKey,
                                 &pReq
                                );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCACSRToPEM(
                        pReq,
                        &pszCSR
                        );
    BAIL_ON_VMCA_ERROR (dwError);

    *ppszCSR = pszCSR;

cleanup:

    if (pReq)
    {
        X509_REQ_free(pReq);
    }
    if (pCert)
    {
        X509_free (pCert);
    }
    if (pPKey)
    {
        RSA_free(pPKey);
    }
    if (pKey)
    {
        EVP_PKEY_free(pKey);
    }

    return dwError;

error:
    if (ppszCSR)
    {
        *ppszCSR = NULL;
    }
    VMCA_SAFE_FREE_STRINGA(pszCSR);

    goto cleanup;
}

DWORD
VMCAGetSignedCertificateForHostA(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCSTR pszHostName,
    PCSTR pszHostIp,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMCA_CERTIFICATE pCertificate = NULL;

    if (IsNullOrEmptyString(pCertRequest) || !ppCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAVerifyHostName(pszHostName, pszHostIp, pCertRequest);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetSignedCertificateFromCSRHA(
                        hInBinding,
                        NULL,
                        pCertRequest,
                        tmNotBefore,
                        tmNotAfter,
                        &pCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppCertificate = pCertificate;

cleanup:
    return dwError;

error:
    if (ppCertificate)
    {
        *ppCertificate = NULL;
    }

    if (pCertificate)
    {
        VMCAFreeCertificate(pCertificate);
    }

    goto cleanup;
}

DWORD
VMCAGetSignedCertificateForHostW(
    PVMCA_SERVER_CONTEXT hInBinding,
    PCWSTR pwszHostName,
    PCWSTR pwszHostIp,
    PCVMCA_CSR pCertRequest,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PVMCA_CERTIFICATE* ppCertificate
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR pszHostName = NULL;
    PSTR pszHostIp = NULL;
    PVMCA_CERTIFICATE pCertificate = NULL;

    if (IsNullOrEmptyString(pCertRequest) || !ppCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszHostName)
    {
        dwError = VMCAAllocateStringAFromW(pwszHostName, &pszHostName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszHostIp)
    {
        dwError = VMCAAllocateStringAFromW(pwszHostIp, &pszHostIp);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetSignedCertificateForHostA(
                        hInBinding,
                        pszHostName,
                        pszHostIp,
                        pCertRequest,
                        tmNotBefore,
                        tmNotAfter,
                        &pCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppCertificate = pCertificate;

cleanup:

    VMCA_SAFE_FREE_STRINGA(pszHostName);
    VMCA_SAFE_FREE_STRINGA(pszHostIp);

    return dwError;
error:
    if (ppCertificate)
    {
        *ppCertificate = NULL;
    }

    if (pCertificate)
    {
        VMCAFreeCertificate(pCertificate);
    }

    goto cleanup;
}
