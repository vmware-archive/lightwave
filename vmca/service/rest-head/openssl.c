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

#define BAIL_ON_OPENSSL_ERROR(dwError, callerFunction)               \
    if (dwError)                                                     \
    {                                                                \
        pszCaller = (callerFunction);                                \
        VMCALog(VMCA_LOG_LEVEL_ERROR, "[%s,%d]",__FILE__, __LINE__); \
        goto openssl_error;                                          \
    }

static
VOID
_VMCALogOpenSSLError(
    int     iSslErr,
    PCSTR   pszCaller
    );

static
VOID
_VMCAOpensslLockingFunc(
    int         mode,
    int         lockNum,
    const char *file,
    int         line
    );

static
unsigned long
_VMCAOpensslIdFunc(
    VOID
    );

static
VOID
_VMCACtxSetOptions(
    SSL_CTX*    pSslCtx
    );


#ifdef _WIN32

static  _TCHAR  RSA_SERVER_CERT[MAX_PATH];
static  _TCHAR  RSA_SERVER_KEY[MAX_PATH];

#endif


static
DWORD
_VMCAInitSslCtxViaFile(
    SSL_CTX* pSslCtx
    )
{
    DWORD       dwError = 0;
    int         iSslRet = 0;
    PCSTR       pszCaller = NULL;
    BOOLEAN     bFileExists = FALSE;

    dwError = VMCAFileExists(RSA_SERVER_CERT, &bFileExists);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!bFileExists)
    {
        VMCALog(VMCA_LOG_LEVEL_ERROR, "Certificate [%s] does not exist", RSA_SERVER_CERT);
        dwError = VMCA_ERROR_NO_FILE_OR_DIRECTORY;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAFileExists(RSA_SERVER_KEY, &bFileExists);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!bFileExists)
    {
        VMCALog(VMCA_LOG_LEVEL_ERROR, "Private key [%s] does not exist", RSA_SERVER_KEY);
        dwError = VMCA_ERROR_NO_FILE_OR_DIRECTORY;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // set server certificate
    iSslRet = SSL_CTX_use_certificate_file(
                pSslCtx,
                RSA_SERVER_CERT,
                SSL_FILETYPE_PEM);
    if (iSslRet != 1)
    {
        VMCALog(
            VMCA_LOG_LEVEL_ERROR,
            "SSL_Ctx_use_certificate_file failed. Certificate file (%s) error code (%d)",
            RSA_SERVER_CERT,
            iSslRet);
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_CTX_use_certificate_file");
    }

    // set server private key
    iSslRet = SSL_CTX_use_PrivateKey_file(
                pSslCtx,
                RSA_SERVER_KEY,
                SSL_FILETYPE_PEM);
    if (iSslRet != 1)
    {
        VMCALog(
                VMCA_LOG_LEVEL_ERROR,
                "SSL_CTX_use_PrivateKey_file failed. Key file (%s) error code (%d)",
                RSA_SERVER_KEY,
                iSslRet );
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_CTX_use_PrivateKey_file");
    }


cleanup:
    return dwError;

error:
    goto cleanup;

openssl_error:
    _VMCALogOpenSSLError(iSslRet, pszCaller);

    goto error;
}

static
DWORD
_VMCAInitSslCtxViaVECS(
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

    dwError = VMCAGetVecsMachineCert(&pszCert, &pszKey);
    BAIL_ON_VMCA_ERROR(dwError);

    pCertBIO = BIO_new_mem_buf(pszCert, -1);
    pKeyBIO  = BIO_new_mem_buf(pszKey, -1);
    if (pCertBIO == NULL || pKeyBIO == NULL)
    {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (PEM_read_bio_X509(pCertBIO, &pX509, NULL, NULL) == NULL)
    {
        VMCA_LOG_ERROR("PEM_read_bio_X509 failed.");
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (PEM_read_bio_PrivateKey(pKeyBIO, &pEVPKey, NULL, NULL)== NULL)
    {
        VMCA_LOG_ERROR("PEM_read_bio_PrivateKey failed.");
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // set server certificate
    iSslRet = SSL_CTX_use_certificate( pSslCtx, pX509 );
    if (iSslRet != 1)
    {
        VMCA_LOG_ERROR("SSL_CTX_use_certificate failed, error code (%d)", iSslRet);
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR( TRUE, "SSL_CTX_use_certificate");
    }

    // set server private key
    iSslRet = SSL_CTX_use_PrivateKey(pSslCtx, pEVPKey);
    if (iSslRet != 1)
    {
        VMCA_LOG_ERROR("SSL_CTX_use_PrivateKey failed, error code (%d)", iSslRet);
        dwError = VMCA_OPENSSL_ERROR;
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
    VMCA_SAFE_FREE_MEMORY(pszCert);
    VMCA_SAFE_FREE_MEMORY(pszKey);

    return dwError;

error:
    goto cleanup;

openssl_error:
    _VMCALogOpenSSLError(iSslRet, pszCaller);

    goto error;
}

static
DWORD
_VMCAInitSslCtx(
    SSL_CTX*    pSslCtx
    )
{
    DWORD   dwError = 0;

    if (gVMCAServerGlobals.bDisableVECSIntegration == FALSE)
    {
        dwError = _VMCAInitSslCtxViaVECS(pSslCtx);
    }

    if (gVMCAServerGlobals.bDisableVECSIntegration == TRUE || dwError!=0)
    {
        dwError = _VMCAInitSslCtxViaFile(pSslCtx);
    }

    return dwError;
}

/*
 * Initialize openssl libraries and create a default SSL_CTX - gVMCAServerGlobals.gpVMCASslCtx
 */
DWORD
VMCAOpensslInit(
    VOID
    )
{
    DWORD       dwError = 0;
    SSL_CTX*    pSslCtx = NULL;
    int         iSslRet = 0;
    PCSTR       pszCaller = NULL;
    DWORD       dwSize = 0;
    EC_KEY*     pEcdhKey = NULL;

    dwError = VMCAAllocateMemory( CRYPTO_num_locks() * sizeof(pthread_mutex_t),
                                   (PVOID*) &(gVMCAOpensslGlobals.pMutexBuf) );
    BAIL_ON_VMCA_ERROR(dwError);

    gVMCAOpensslGlobals.dwMutexBufSize = CRYPTO_num_locks();

    for (dwSize = 0; dwSize < gVMCAOpensslGlobals.dwMutexBufSize; dwSize++)
    {
        pthread_mutex_init( &(gVMCAOpensslGlobals.pMutexBuf[dwSize]), NULL);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // http://www.openssl.org/docs/crypto/threads.html
    // To make openssl functions thread safe, we need to register at least following two functions.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    CRYPTO_set_locking_callback(_VMCAOpensslLockingFunc);
    CRYPTO_set_id_callback(_VMCAOpensslIdFunc);

    // always return 1
    iSslRet = SSL_library_init();
    if ( iSslRet != 1 )
    {
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_library_init");
    }

    // load error strings for ssl and crypto API
    SSL_load_error_strings();

    // support SSLV23 and above context.  We disable SSLv23 optoin in _VMCACtxSetOptions later.
    pSslCtx = SSL_CTX_new(SSLv23_method());
    if (!pSslCtx)
    {
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_CTX_new");
    }

    (VOID)_VMCACtxSetOptions(pSslCtx);

    pEcdhKey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (NULL != pEcdhKey)
    {
        if (SSL_CTX_set_tmp_ecdh(pSslCtx, pEcdhKey) != 1)
        {
            VMCA_LOG_WARNING("SSL_CTX_set_tmp_ecdh call failed");
        }

        EC_KEY_free(pEcdhKey);
        pEcdhKey = NULL;
    }

    // let ssl layer handle possible retry
    SSL_CTX_set_mode(pSslCtx, SSL_MODE_AUTO_RETRY);

    dwError = _VMCAInitSslCtx(pSslCtx);
    if (dwError)
    {
        // soft fail - Lotus will not listen on LDAPs port.
        dwError = 0;
        goto error;
    }

    iSslRet = SSL_CTX_check_private_key(pSslCtx);
    if (iSslRet != 1)
    {
        dwError = VMCA_OPENSSL_ERROR;
        BAIL_ON_OPENSSL_ERROR(TRUE, "SSL_CTX_check_private_key");
    }

    gVMCAServerGlobals.gpVMCASslCtx = pSslCtx;
    gVMCAOpensslGlobals.bSSLInitialized = TRUE;

cleanup:
    return dwError;

error:
    if (pSslCtx)
    {
        SSL_CTX_free(pSslCtx);
    }

    if (gVMCAOpensslGlobals.bSSLInitialized == FALSE)
    {
        VMCA_LOG_WARNING("SSL port init failed.");
    }

    goto cleanup;

openssl_error:
    _VMCALogOpenSSLError(iSslRet, pszCaller);

    goto error;
}

/*
 * shutdown openssl during server shutdown
 */
VOID
VMCAOpensslShutdown(
    VOID
    )
{
    DWORD   dwSize = 0;

    if (gVMCAServerGlobals.gpVMCASslCtx)
    {
        SSL_CTX_free(gVMCAServerGlobals.gpVMCASslCtx);
    }

    ERR_remove_state(0);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());

    for (dwSize = 0; dwSize < gVMCAOpensslGlobals.dwMutexBufSize; dwSize++)
    {
        pthread_mutex_destroy( &(gVMCAOpensslGlobals.pMutexBuf[dwSize]));
    }
    gVMCAOpensslGlobals.dwMutexBufSize = 0;
    VMCA_SAFE_FREE_MEMORY( gVMCAOpensslGlobals.pMutexBuf );

    return;
}

#define MAX_SSL_ERROR_BUF_LEN   120

static
VOID
_VMCALogOpenSSLError(
    int     iSslErr,
    PCSTR   pszCaller
    )
{
    PCSTR           pszMsg = NULL;
    unsigned long   ulErr = 0;

    VMCALog(VMCA_LOG_LEVEL_ERROR, "Failed SSL function (%s), return value (%d)", VMCA_SAFE_STRING(pszCaller), iSslErr);

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

        VMCALog(VMCA_LOG_LEVEL_ERROR, "SSL error (%lu)(%s)(%s)", ulErr, pszMsg, errbuf);
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
_VMCAOpensslLockingFunc(
    int         mode,
    int         lockNum,
    const char *file,
    int         line
    )
{
    if (mode & CRYPTO_LOCK)
    {
        pthread_mutex_lock( &(gVMCAOpensslGlobals.pMutexBuf[lockNum]) );
    }
    else
    {
        pthread_mutex_unlock( &(gVMCAOpensslGlobals.pMutexBuf[lockNum]) );
    }
}

/**
 * OpenSSL uniq id function.
 *
 * @return    thread id
 */
static
unsigned long
_VMCAOpensslIdFunc(
    VOID
    )
{
#ifdef _WIN32
    return ((unsigned long) (pthread_self().p));
#else
    return ((unsigned long) pthread_self());
#endif
}

static
VOID
_VMCACtxSetOptions(
    SSL_CTX*    pSslCtx
    )
{
    CHAR    szSslDisabledProtocols[VMCA_SSL_DISABLED_PROTOCOL_LEN] = {0};
    long    sslOptions = SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3;
    PSTR    pProtocol = NULL;
    PSTR    pSep = " ,";
    PSTR    pRest = NULL;

    struct
    {
        PCSTR  pszProtocol;
        long   disableOpt;
    }
    optTable[] =
    {
        { "SSLv2",   SSL_OP_NO_SSLv2 },
        { "SSLv3",   SSL_OP_NO_SSLv3 },
        { "TLSv1",   SSL_OP_NO_TLSv1 },
        { "TLSv1.1", SSL_OP_NO_TLSv1_1 },
        { "TLSv1.2", SSL_OP_NO_TLSv1_2 },
    };

    // use registry key value if exists
    if ( VMCAGetRegKeyValue(VMCA_CONFIG_PARAMETER_KEY_PATH, // vmca/parameters
                            VMCA_REG_KEY_SSL_DISABLED_PROTOCOLS,
                            szSslDisabledProtocols,
                            VMCA_SSL_DISABLED_PROTOCOL_LEN - 1) == 0
       )
    {
        for ( pProtocol = VMCAStringTokA(szSslDisabledProtocols, pSep, &pRest);
              pProtocol;
              pProtocol = VMCAStringTokA(NULL, pSep, &pRest) )
        {
            DWORD iValue = 0;

            for (; iValue < sizeof(optTable)/sizeof(optTable[0]); iValue++)
            {
                if (VMCAStringCompareA(pProtocol, optTable[iValue].pszProtocol, FALSE) == 0)
                {
                    VMCA_LOG_INFO("Openssl Ctx disable protocol (%s)", pProtocol);
                    sslOptions |= optTable[iValue].disableOpt;
                    break;
                }
            }

            if (iValue >= sizeof(optTable)/sizeof(optTable[0]))
            {
                VMCA_LOG_ERROR("Invalid protocol set for SslDisabledProtocol: (%s)", VMCA_SAFE_STRING(pProtocol));
            }
        }
    }

    SSL_CTX_set_options(pSslCtx, sslOptions); // ignore return options bitmask

    return;
}
