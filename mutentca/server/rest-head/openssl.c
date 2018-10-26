/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"

#define BAIL_ON_OPENSSL_ERROR(dwError, callerFunction)          \
    if (dwError)                                                \
    {                                                           \
        pszCaller = (callerFunction);                           \
        LWCA_LOG_ERROR("[%s,%d]",__FILE__, __LINE__);           \
        goto openssl_error;                                     \
    }

static
VOID
_LwCALogOpenSSLError(
    int     iSslErr,
    PCSTR   pszCaller
    );

static
VOID
_LwCAOpensslLockingFunc(
    int         mode,
    int         lockNum,
    const char *file,
    int         line
    );

static
unsigned long
_LwCAOpensslIdFunc(
    VOID
    );

static
VOID
_LwCACtxSetOptions(
    SSL_CTX*    pSslCtx
    );

static
DWORD
_LwCAInitSslCtxViaVECS(
    SSL_CTX* pSslCtx
    )
{
    DWORD       dwError = 0;
    int         iSslRet = 0;
    PCSTR       pszCaller = NULL;
    PSTR        pszCert = NULL;
    PSTR        pszKey = NULL;
    BIO*        pCertBIO = NULL;
    BIO*        pKeyBIO = NULL;
    X509*       pX509 = NULL;
    EVP_PKEY*   pEVPKey = NULL;

    dwError = LwCAGetVecsMachineCert(&pszCert, &pszKey);
    BAIL_ON_LWCA_ERROR(dwError);

    pCertBIO = BIO_new_mem_buf(pszCert, -1);
    pKeyBIO  = BIO_new_mem_buf(pszKey, -1);
    if (pCertBIO == NULL || pKeyBIO == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (PEM_read_bio_X509(pCertBIO, &pX509, NULL, NULL) == NULL)
    {
        LWCA_LOG_ERROR("PEM_read_bio_X509 failed.");
        dwError = LWCA_OPENSSL_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (PEM_read_bio_PrivateKey(pKeyBIO, &pEVPKey, NULL, NULL)== NULL)
    {
        LWCA_LOG_ERROR("PEM_read_bio_PrivateKey failed.");
        dwError = LWCA_OPENSSL_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // set server certificate
    iSslRet = SSL_CTX_use_certificate( pSslCtx, pX509 );
    if (iSslRet != 1)
    {
        LWCA_LOG_ERROR("SSL_CTX_use_certificate failed, error code (%d)", iSslRet);
        dwError = LWCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR( TRUE, "SSL_CTX_use_certificate");
    }

    // set server private key
    iSslRet = SSL_CTX_use_PrivateKey(pSslCtx, pEVPKey);
    if (iSslRet != 1)
    {
        LWCA_LOG_ERROR("SSL_CTX_use_PrivateKey failed, error code (%d)", iSslRet);
        dwError = LWCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_CTX_use_certificate");
    }

cleanup:

    if (pCertBIO)
    {
        BIO_free(pCertBIO);
    }
    if (pKeyBIO)
    {
        BIO_free(pKeyBIO);
    }
    if (pX509)
    {
        X509_free(pX509);
    }
    if (pEVPKey)
    {
        EVP_PKEY_free(pEVPKey);
    }
    LWCA_SAFE_FREE_MEMORY(pszCert);
    LWCA_SAFE_FREE_MEMORY(pszKey);

    return dwError;

error:
    goto cleanup;

openssl_error:
    _LwCALogOpenSSLError(iSslRet, pszCaller);

    goto error;
}

static
DWORD
_LwCAInitSslCtx(
    SSL_CTX*    pSslCtx
    )
{
    DWORD dwError = 0;

    // Add support for SslCtxViaFile in the future if required
    dwError = _LwCAInitSslCtxViaVECS(pSslCtx);

    return dwError;
}

/*
 * Initialize openssl libraries and create a default SSL_CTX - gLwCAServerGlobals.pSslCtx
 */
DWORD
LwCAOpensslInit(
    VOID
    )
{
    DWORD       dwError = 0;
    SSL_CTX*    pSslCtx = NULL;
    int         iSslRet = 0;
    PCSTR       pszCaller = NULL;
    DWORD       dwSize = 0;
    EC_KEY*     pEcdhKey = NULL;

    dwError = LwCAAllocateMemory( CRYPTO_num_locks() * sizeof(pthread_mutex_t),
                                   (PVOID*) &(gLwCAOpensslGlobals.pMutexBuf) );
    BAIL_ON_LWCA_ERROR(dwError);

    gLwCAOpensslGlobals.dwMutexBufSize = CRYPTO_num_locks();

    for (dwSize = 0; dwSize < gLwCAOpensslGlobals.dwMutexBufSize; dwSize++)
    {
        pthread_mutex_init( &(gLwCAOpensslGlobals.pMutexBuf[dwSize]), NULL);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // http://www.openssl.org/docs/crypto/threads.html
    // To make openssl functions thread safe, we need to register at least following two functions.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    CRYPTO_set_locking_callback(_LwCAOpensslLockingFunc);
    CRYPTO_set_id_callback(_LwCAOpensslIdFunc);

    // always return 1
    iSslRet = SSL_library_init();
    if ( iSslRet != 1 )
    {
        dwError = LWCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_library_init");
    }

    // load error strings for ssl and crypto API
    SSL_load_error_strings();

    // support SSLV23 and above context.  We disable SSLv23 optoin in _LwCACtxSetOptions later.
    pSslCtx = SSL_CTX_new(SSLv23_method());
    if (!pSslCtx)
    {
        dwError = LWCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_CTX_new");
    }

    (VOID)_LwCACtxSetOptions(pSslCtx);

    pEcdhKey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (NULL != pEcdhKey)
    {
        if (SSL_CTX_set_tmp_ecdh(pSslCtx, pEcdhKey) != 1)
        {
            LWCA_LOG_WARNING("SSL_CTX_set_tmp_ecdh call failed");
        }

        EC_KEY_free(pEcdhKey);
        pEcdhKey = NULL;
    }

    // let ssl layer handle possible retry
    SSL_CTX_set_mode(pSslCtx, SSL_MODE_AUTO_RETRY);

    dwError = _LwCAInitSslCtx(pSslCtx);
    if (dwError)
    {
        // soft fail - Lotus will not listen on LDAPs port.
        dwError = 0;
        goto error;
    }

    iSslRet = SSL_CTX_check_private_key(pSslCtx);
    if (iSslRet != 1)
    {
        dwError = LWCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_CTX_check_private_key");
    }

    gLwCAServerGlobals.pSslCtx = pSslCtx;
    gLwCAOpensslGlobals.bSSLInitialized = TRUE;

cleanup:
    return dwError;

error:
    if (pSslCtx)
    {
        SSL_CTX_free(pSslCtx);
    }

    if (gLwCAOpensslGlobals.bSSLInitialized == FALSE)
    {
        LWCA_LOG_WARNING("SSL port init failed.");
    }

    goto cleanup;

openssl_error:
    _LwCALogOpenSSLError(iSslRet, pszCaller);

    goto error;
}

/*
 * shutdown openssl during server shutdown
 */
VOID
LwCAOpensslShutdown(
    VOID
    )
{
    DWORD   dwSize = 0;

    if (gLwCAServerGlobals.pSslCtx)
    {
        SSL_CTX_free(gLwCAServerGlobals.pSslCtx);
    }

    ERR_remove_state(0);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());

    for (dwSize = 0; dwSize < gLwCAOpensslGlobals.dwMutexBufSize; dwSize++)
    {
        pthread_mutex_destroy( &(gLwCAOpensslGlobals.pMutexBuf[dwSize]));
    }
    gLwCAOpensslGlobals.dwMutexBufSize = 0;
    LWCA_SAFE_FREE_MEMORY( gLwCAOpensslGlobals.pMutexBuf );

    return;
}

#define MAX_SSL_ERROR_BUF_LEN   120

static
VOID
_LwCALogOpenSSLError(
    int     iSslErr,
    PCSTR   pszCaller
    )
{
    PCSTR           pszMsg = NULL;
    unsigned long   ulErr = 0;

    LWCA_LOG_ERROR("Failed SSL function (%s), return value (%d)", LWCA_SAFE_STRING(pszCaller), iSslErr);

    for (ulErr = ERR_get_error(); ulErr != 0; ulErr = ERR_get_error())
    {
        char errbuf[MAX_SSL_ERROR_BUF_LEN] = {0};

        ERR_error_string_n(ulErr, errbuf, MAX_SSL_ERROR_BUF_LEN);

        switch (ulErr)
        {
            case SSL_ERROR_NONE:
                pszMsg = "SSL_ERROR_NONE";
                break;
            case SSL_ERROR_ZERO_RETURN:
                pszMsg = "SSL_ERROR_ZERO_RETURN";
                break;
            case SSL_ERROR_WANT_READ:
                pszMsg = "SSL_ERROR_WANT_READ";
                break;
            case SSL_ERROR_WANT_WRITE:
                pszMsg = "SSL_ERROR_WANT_WRITE";
                break;
            case SSL_ERROR_WANT_CONNECT:
                pszMsg = "SSL_ERROR_WANT_CONNECT";
                break;
            case SSL_ERROR_WANT_ACCEPT:
                pszMsg = "SSL_ERROR_WANT_ACCEPT";
                break;
            case SSL_ERROR_WANT_X509_LOOKUP:
                pszMsg = "SSL_ERROR_WANT_X509_LOOKUP";
                break;
            case SSL_ERROR_SYSCALL:
                pszMsg = "SSL_ERROR_SYSCALL";
                break;
            case SSL_ERROR_SSL:
                pszMsg = "SSL_ERROR_SSL";
                break;
            default:
                pszMsg = "Others";
                break;
        }

        LWCA_LOG_ERROR("SSL error (%lu)(%s)(%s)", ulErr, pszMsg, errbuf);
    }

    return;
}

/**
 * OpenSSL locking function.
 *
 * @param    mode    lock mode
 * @param    lockNum lock number
 * @param    file    source file name
 * @param    line    source file line number
 * @return   none
 */
static
VOID
_LwCAOpensslLockingFunc(
    int         mode,
    int         lockNum,
    const char *file,
    int         line
    )
{
    if (mode & CRYPTO_LOCK)
    {
        pthread_mutex_lock( &(gLwCAOpensslGlobals.pMutexBuf[lockNum]) );
    }
    else
    {
        pthread_mutex_unlock( &(gLwCAOpensslGlobals.pMutexBuf[lockNum]) );
    }
}

/**
 * OpenSSL uniq id function.
 *
 * @return    thread id
 */
static
unsigned long
_LwCAOpensslIdFunc(
    VOID
    )
{
    return ((unsigned long) pthread_self());
}

static
VOID
_LwCACtxSetOptions(
    SSL_CTX*    pSslCtx
    )
{
    /*
     *  Disable options for well known protocols:
     *  { "SSLv2",      SSL_OP_NO_SSLv2 }
     *  { "SSLv3",      SSL_OP_NO_SSLv3 }
     *  { "TLSv1",      SSL_OP_NO_TLSv1 }
     *  { "TLSv1.1",    SSL_OP_NO_TLSv1_1 }
     *  { "TLSv1.2",    SSL_OP_NO_TLSv1_2 }
     */

    /*
     * Only allowing TLSv1.2 and higher version protocols
     * Disabling all the other options
     */
    long sslOptions = SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1|SSL_OP_NO_TLSv1_1;

    SSL_CTX_set_options(pSslCtx, sslOptions); // ignore return options bitmask

    return;
}
