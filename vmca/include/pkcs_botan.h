#ifndef __PKCS_BOTAN_H__
#define __PKCS_BOTAN_H__

#include <macros.h>
#include <public/vmca.h>
#include "vmcacommon.h"

#ifdef __cplusplus
extern "C" {
#endif

DWORD
VMCAAllocatePrivateKeyPrivate(
    PWSTR pszPassPhrase,
    size_t uiKeyLength,
    PSTR* ppPrivateKey,
    PSTR* ppPublicKey
);

// This function creates private-public key pair  and retuns them to user
//
// Arguments :
//          pszPassPhrase   : Optional Pass Word to protect the Key
//          uiKeyLength     : Key Length - Valid values are between 1024 and 16384
//          ppPrivateKey    : PEM encoded Private Key String
//          ppPublicKey     : PEM encoded Public Key String.
//
// Returns :
//      Error Code
//
// Notes : This function makes some assumptions on the users
// behalf. One of them is that assumption on bit size. This is based on RSA's
// recommendation http://www.rsa.com/rsalabs/node.asp?id=2218 on
// Corporate Key lengths.


DWORD
VMCACreateSigningRequestPrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PSTR pszPrivateKey,
    PWSTR pszPassPhrase,
    PSTR* ppAllocatedCSR
);
// VMCACreateSigningRequestPrivate function creates a
// CSR that you can send to a CA for signing .
//
// Arguments :
//      pCertRequest    :  Parameters for the request
//      pszPrivateKey   :  Private Key for the CSR
//      pszPassPhrase   :  Pass Phrase for the Private Key
//      ppAllocatedCSR  :  CSR that is allocated.
// Returns :
//      Error Code
DWORD
VMCASelfSignedCertificatePrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PSTR pszPrivateKey,
    PWSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PSTR* ppszCertificate
);
//  VMCASelfSignedCertificatePrivate allows the user to create
//  a Self Signed Certificate.
//
// Arguments :
//      pCertRequest    :  Parameters for the request
//      pszPrivateKey   :  Private Key for the CSR
//      pszPassPhrase   :  Pass Phrase for the Private Key
//      bCreateSelfSignedRootCA : if this flag is true, the Certificate will have CA privilages turned on
//      ppszCertificate : Allocated Certificate
// Returns :
//      Error Code

DWORD
VMCACheckforRevokedCertPrivate(
    PSTR pszCRLFile,
    PVMCA_CERTIFICATE pszCertificate,
    PVMCA_CERTIFICATE pszRootCA,
    BOOL *bRevoked);


#ifdef __cplusplus
}
#endif


#endif //__PKCS_BATON_H__
