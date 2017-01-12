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
 * Module Name: Directory ldap head
 *
 * Filename: opensslclient.c
 *
 * Abstract:
 *
 * Open SSL client transport support
 *
 */

#include "includes.h"

static
DWORD
_VmDirReadCertFromFile(
    PSTR    pszFileName,
    X509**  ppX509
    );

static
DWORD
_VmDirGetCertThumbPrint(
    X509*   pX509,
    PBYTE*  ppByte,
    DWORD*  pByteSize
    );

static
int
_VmDirOpenSSLClientCallback(
    X509_STORE_CTX*     pStore,
    void*               pArg
    );

/*
 *  1. acquire a SSL_CTX
 *  2. set server (peer) cert verification
 */
DWORD
VmDirPrepareOpensslClientCtx(
    SSL_CTX**   ppSslCtx,
    PSTR*       ppszTrustCertFile,
    PCSTR       pszLdapURI
    )
{
    DWORD       dwError = 0;
    SSL_CTX*    pSslCtx  = NULL;
    int         iSslRet = 0;
    PSTR        pszLocalErrMsg = NULL;
    PSTR        pszTrustCertFile = NULL;

    if ( !ppSslCtx || !ppszTrustCertFile || !pszLdapURI )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCertficateFileNameFromLdapURI ( (PSTR)pszLdapURI, &pszTrustCertFile);
    if (dwError)
    {
        dwError = VMDIR_ERROR_SSL_CERT_FILE_NOT_FOUND;
    }
    BAIL_ON_VMDIR_ERROR( dwError );
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
                       "cert (%s) used in ssl server cert verification",
                       VDIR_SAFE_STRING(pszTrustCertFile) );

    pSslCtx = SSL_CTX_new(TLSv1_method());
    if (!pSslCtx)
    {
        dwError = VMDIR_ERROR_NO_SSL_CTX;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    iSslRet = SSL_CTX_load_verify_locations( pSslCtx,
                                             pszTrustCertFile,
                                             NULL);
    if (iSslRet != 1 )
    {
        dwError = VMDIR_ERROR_SSL_CERT_FILE_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "VmDirSetOpensslClientTrustCert: CAfile (%s) failed",
                                      VDIR_SAFE_STRING(pszTrustCertFile) );
    }

    // verify server (peer) cert
    SSL_CTX_set_verify( pSslCtx, SSL_VERIFY_PEER, NULL);

    // use our own callback verify function
    // (really should use SSL default function, but with depth 0, it does not seem perform peer cert verifiction.
    //  seeing error 20/X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
    SSL_CTX_set_cert_verify_callback( pSslCtx, _VmDirOpenSSLClientCallback, pszTrustCertFile);

    // depth 0: peer
    SSL_CTX_set_verify_depth( pSslCtx, 0 );

    *ppSslCtx = pSslCtx;
    *ppszTrustCertFile = pszTrustCertFile;

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszTrustCertFile);
    if (pSslCtx)
    {
        SSL_CTX_free(pSslCtx);
    }

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Error message (%s), error code (%u)",
                                         pszLocalErrMsg ? pszLocalErrMsg : "VmDirPrepareOpensslClientCtx() failed",
                                         dwError  );
    goto cleanup;
}

/*
 * For 2013, we only want to do peer certificate verification.
 * 1. get the remote cert from pStore
 * 2. pArg is the file name of local cert that we trust
 * 3. compare digest of remote and local PEM
 */
static
int
_VmDirOpenSSLClientCallback(
    X509_STORE_CTX*     pStore,
    void*               pArg
    )
{
    DWORD   dwError = 0;
    int     iRtn = 0;
    PBYTE   pByteCertRemote = NULL;
    DWORD   pRemoteSize = 0;
    PBYTE   pByteCertLocal = NULL;
    DWORD   pLocalSize = 0;
    X509*   pLocalX509 = NULL;

    // Since we override default verify callback, pStore is fresh and has not yet been accessed.
    // pStore->cert is the target certificate we want to verify.
    if ( pArg && pStore && pStore->cert)
    {
        dwError = _VmDirReadCertFromFile( (PSTR)pArg, &pLocalX509);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirGetCertThumbPrint( pStore->cert, &pByteCertRemote, &pRemoteSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirGetCertThumbPrint( pLocalX509, &pByteCertLocal, &pLocalSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        if ( pRemoteSize > 0            &&
             pRemoteSize == pLocalSize  &&
             memcmp( pByteCertRemote, pByteCertLocal, pLocalSize) == 0 )
        {
            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirOpenSSLCallback, digest compare succeeded");
            iRtn = 1;   // return success
        }
        else
        {
            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirOpenSSLCallback, digest compare failed");
            iRtn = 0;   // return fail
        }
    }

cleanup:

    if (pLocalX509)
    {
        X509_free(pLocalX509);
    }
    VMDIR_SAFE_FREE_MEMORY(pByteCertLocal);
    VMDIR_SAFE_FREE_MEMORY(pByteCertRemote);

    return iRtn;

error:

    goto cleanup;
}


static
DWORD
_VmDirGetCertThumbPrint(
    X509*   pX509,
    PBYTE*  ppByte,
    DWORD*  pByteSize)
{
    DWORD           dwError = 0;
    const EVP_MD *  pDigest = EVP_sha1();
    unsigned char   md[20] = {0}; //sha1 is 20 bytes
    unsigned int    iOutMDSize = 0;
    PBYTE           pBlob = NULL;

    X509_digest(pX509, pDigest, md, &iOutMDSize);

    dwError = VmDirAllocateAndCopyMemory( &md[0], iOutMDSize, (PVOID*)&pBlob);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppByte = pBlob;
    *pByteSize = iOutMDSize;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pBlob);
    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirGetCertThumbPrint failed (%u)", dwError);

    goto cleanup;
}

static
DWORD
_VmDirReadCertFromFile(
    PSTR    pszFileName,
    X509**  ppX509
    )
{
    DWORD   dwError =0;
    X509*   pCert = NULL;
    FILE*   fpCert = NULL;

    if ( IsNullOrEmptyString(pszFileName) || ( ppX509 == NULL ) )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    fpCert = fopen(pszFileName, "r");
    if ( fpCert == NULL)
    {
        dwError = VMDIR_ERROR_IO;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCert = PEM_read_X509(fpCert, NULL, NULL, NULL);
    if ( pCert == NULL )
    {
        dwError = VMDIR_ERROR_GENERIC;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppX509 = pCert;

cleanup:

    if ( fpCert )
    {
        fclose(fpCert);
    }

    return dwError;

error :
    if ( pCert )
    {
        X509_free(pCert);
    }

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirReadCertFromFile failed (%u)", dwError);

    goto cleanup;
}

/*
static
int
VmDirOpenSSLVerify(
    int                 iVerifyOk,
    X509_STORE_CTX*     pStore
    )
{
    DWORD   dwError = 0;
    int     iRtn = iVerifyOk;       // in SSL return 1 means success
    PBYTE   pByteCertRemote = NULL;
    DWORD   pRemoteSize = 0;
    PBYTE   pByteCertLocal = NULL;
    DWORD   pLocalSize = 0;
    X509*   pLocalX509 = NULL;

    if ( !iVerifyOk && pStore)
    {   // we fail the default SSL verify test
        int     errCode = X509_STORE_CTX_get_error(pStore);
        PCSTR   pErrMsg = X509_verify_cert_error_string(errCode);

        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirOpenSSLVerify, (%d)(%s)",
                         errCode, VDIR_SAFE_STRING( pErrMsg) );

        dwError = _VmDirReadCertFromFile( "c:\\rootca.pem", &pLocalX509);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirGetCertThumbPrint( X509_STORE_CTX_get_current_cert(pStore), &pByteCertRemote, &pRemoteSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirGetCertThumbPrint( pLocalX509, &pByteCertLocal, &pLocalSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        if ( pRemoteSize == pLocalSize && memcmp( pByteCertRemote, pByteCertLocal, pLocalSize) == 0 )
        {   // override SSL verify result
            iRtn = 1;
        }
        else
        {
            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirOpenSSLVerify, digest compare failed");
            iRtn = 0;
        }
    }

cleanup:

    if (pLocalX509)
    {
        X509_free(pLocalX509);
    }
    VMDIR_SAFE_FREE_MEMORY(pByteCertLocal);
    VMDIR_SAFE_FREE_MEMORY(pByteCertRemote);

    return iRtn;

error:
    goto cleanup;
}
*/
