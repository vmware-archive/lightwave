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
 * Filename: openssl.c
 *
 * Abstract:
 *
 * Open SSL transport support
 *
 */

#include "includes.h"

#define BAIL_ON_OPENSSL_ERROR(dwError, callerFunction) \
    if (dwError)                                                   \
    {                                                              \
        pszCaller = (callerFunction);                              \
        VmDirLog( LDAP_DEBUG_TRACE, "[%s,%d]",__FILE__, __LINE__); \
        goto openssl_error;                                        \
    }
/*
 * https://wiki.eng.vmware.com/Security_Best_Practices#Encryption
 */
#define VMDIR_SSL_STRONG_CIPHER_SUITE "HIGH:!ADH:!EXP:!MD5:!3DES:!CAMELLIA:!PSK:!SRP:@STRENGTH"

static
ber_slen_t
opensslSbRead(
    Sockbuf_IO_Desc*    pSbiod,
    void*               pBuf,
    ber_len_t           len
    );

static
ber_slen_t
opensslSbWrite(
    Sockbuf_IO_Desc*    pSbiod,
    void*               pBuf,
    ber_len_t           len
    );

static
int
opensslSbClose(
    Sockbuf_IO_Desc*    pSbiod
    );

static
int
opensslSbSetup(
    Sockbuf_IO_Desc*    pSbiod,
    void*               pArg
    );

static
int
opensslSbCtrl(
    Sockbuf_IO_Desc*    pSbiod,
    int                 opt,
    void*               pArg
    );

static
VOID
_VmDirLogOpenSSLError(
    int     iSslErr,
    PCSTR   pszCaller
    );

static
VOID
_VmDirOpensslLockingFunc(
    int         mode,
    int         lockNum,
    const char *file,
    int         line
    );

static
unsigned long
_VmDirOpensslIdFunc(
    VOID
    );

/*
static
int
opensslSbRemove(
    Sockbuf_IO_Desc*    pSbiod
    );
*/

// call back function for lber (see sockbuf.c and io.c)
static
Sockbuf_IO opensslBerSockbufIO = {
    opensslSbSetup,    /* sbi_setup: called via NewConnection ber_sockbuf_add_io*/
    NULL,              // opensslSbRemove,   /* sbi_remove */
    opensslSbCtrl,     /* sbi_ctrl: called via NewConnection ber_sockbuf_ctrl */
    opensslSbRead,     /* sbi_read: called via ber_int_sb_read */
    opensslSbWrite,    /* sbi_write: called via ber_int_sb_write */
    opensslSbClose     /* sbi_close: called via DeleteConnection ber_sockbuf_free */
};

///////////////////////////////////////////////////////////////////////////////
// ****************** shared static variables begin ********************
///////////////////////////////////////////////////////////////////////////////
Sockbuf_IO*         gpVdirBerSockbufIOOpenssl = &opensslBerSockbufIO;

// initialized in VmDirOpensslInit during startup and used to create SSL*
static SSL_CTX*     gpVdirSslCtx = NULL;
///////////////////////////////////////////////////////////////////////////////
// ****************** shared static variables end ********************
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

static  _TCHAR  RSA_SERVER_CERT[MAX_PATH];
static  _TCHAR  RSA_SERVER_KEY[MAX_PATH];

#endif

/*
 * Initialize openssl libraries and create a default SSL_CTX - gpVdirSslCtx
 */
DWORD
VmDirOpensslInit(
    VOID
    )
{
    DWORD       dwError = 0;
    SSL_CTX*    pSslCtx = NULL;
    int         iSslRet = 0;
    PCSTR       pszCaller = NULL;
    DWORD       dwSize = 0;

    dwError = VmDirAllocateMemory( CRYPTO_num_locks() * sizeof(pthread_mutex_t),
                                   (PVOID*) &(gVmdirOpensslGlobals.pMutexBuf) );
    BAIL_ON_VMDIR_ERROR(dwError);
    gVmdirOpensslGlobals.dwMutexBufSize = CRYPTO_num_locks();

    for (dwSize = 0; dwSize < gVmdirOpensslGlobals.dwMutexBufSize; dwSize++)
    {
        pthread_mutex_init( &(gVmdirOpensslGlobals.pMutexBuf[dwSize]), NULL);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // http://www.openssl.org/docs/crypto/threads.html
    // To make openssl functions thread safe, we need to register at least following two functions.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    CRYPTO_set_locking_callback( _VmDirOpensslLockingFunc );
    CRYPTO_set_id_callback( _VmDirOpensslIdFunc );

    // always return 1
    iSslRet = SSL_library_init();
    if ( iSslRet != 1 )
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_OPENSSL_ERROR( TRUE, "SSL_library_init");
    }

    // load error strings for ssl and crypto API
    SSL_load_error_strings();

    // support TLS v1 compatible context
    pSslCtx = SSL_CTX_new(TLSv1_method());
    if (!pSslCtx)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_OPENSSL_ERROR( TRUE, "SSL_CTX_new");
    }

    // only allow strong cipher suite
    iSslRet = SSL_CTX_set_cipher_list( pSslCtx, VMDIR_SSL_STRONG_CIPHER_SUITE );
    if (iSslRet != 1)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SSL_Ctx_set_cipher_list failed. (%d)", iSslRet );
        dwError = VMDIR_ERROR_INVALID_CONFIGURATION;
        BAIL_ON_OPENSSL_ERROR( TRUE, "SSL_CTX_set_cipher_list");
    }

    // let ssl layer handle possible retry
    SSL_CTX_set_mode(pSslCtx, SSL_MODE_AUTO_RETRY);

#ifdef _WIN32
    dwError = VmDirOpensslSetServerCertPath(RSA_SERVER_CERT);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirOpensslSetServerKeyPath(RSA_SERVER_KEY);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    // set server certificate
    iSslRet = SSL_CTX_use_certificate_file(
                pSslCtx,
                RSA_SERVER_CERT,
                SSL_FILETYPE_PEM);
    if (iSslRet != 1)
    {
        VmDirLog( LDAP_DEBUG_ANY, "SSL_Ctx_use_certificate_file failed. "   \
                                  "Certificate file (%s) "                  \
                                  "error code (%d)",
                                  RSA_SERVER_CERT, iSslRet );
        // soft fail if cert file loading failed - Lotus will not listen on LDAPs port.
        dwError = 0;
        goto error;
    }

    // set server private key
    iSslRet = SSL_CTX_use_PrivateKey_file(
                pSslCtx,
                RSA_SERVER_KEY,
                SSL_FILETYPE_PEM);
    if (iSslRet != 1)
    {
        VmDirLog( LDAP_DEBUG_ANY, "SSL_CTX_use_PrivateKey_file failed. "    \
                                  "Key file (%s) "                          \
                                  "error code (%d)",
                                  RSA_SERVER_KEY, iSslRet );
        // soft fail if key file loading failed - Lotus will not listen on LDAPs port.
        dwError = 0;
        goto error;
    }

    iSslRet = SSL_CTX_check_private_key(pSslCtx);
    if (iSslRet != 1)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_OPENSSL_ERROR( TRUE, "SSL_CTX_check_private_key");
    }

    gpVdirSslCtx = pSslCtx;
    gVmdirOpensslGlobals.bSSLInitialized = TRUE;

cleanup:
    return dwError;

error:
    if (pSslCtx)
    {
        SSL_CTX_free(pSslCtx);
    }

    goto cleanup;

openssl_error:
    _VmDirLogOpenSSLError( iSslRet, pszCaller);

    goto error;
}

/*
 * shutdown openssl during server shutdown
 */
VOID
VmDirOpensslShutdown(
    VOID
    )
{
    DWORD   dwSize = 0;

    if (gpVdirSslCtx)
    {
        SSL_CTX_free(gpVdirSslCtx);
    }

    ERR_remove_state(0);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());

    for (dwSize = 0; dwSize < gVmdirOpensslGlobals.dwMutexBufSize; dwSize++)
    {
        pthread_mutex_destroy( &(gVmdirOpensslGlobals.pMutexBuf[dwSize]));
    }
    gVmdirOpensslGlobals.dwMutexBufSize = 0;
    VMDIR_SAFE_FREE_MEMORY( gVmdirOpensslGlobals.pMutexBuf );

    return;
}

/*
 * called via sockbuf.c:ber_int_sb_read
 * return length of bytes read
 */
static
ber_slen_t
opensslSbRead(
    Sockbuf_IO_Desc*    pSbiod,
    void*               pBuf,
    ber_len_t           len
    )
{
    SSL*    pSsl = NULL;
    int     iSslRet = 0;
    PCSTR   pszCaller = NULL;

    assert( pSbiod != NULL );
    assert( SOCKBUF_VALID( pSbiod->sbiod_sb ) );

    assert( pSbiod->sbiod_pvt );
    pSsl = (SSL*)pSbiod->sbiod_pvt;

    iSslRet =  SSL_read( pSsl, pBuf, (int)len );
    BAIL_ON_OPENSSL_ERROR( (iSslRet <= 0), "SSL_read" ); // we use blocking IO, iSslRet > 0 is success;

cleanup:

    return iSslRet;

openssl_error:

    _VmDirLogOpenSSLError( iSslRet, pszCaller);
    goto cleanup;
}

/*
 * called via sockbuf.c:ber_int_sb_write
 * return length of bytes written
 */
static
ber_slen_t
opensslSbWrite(
    Sockbuf_IO_Desc*    pSbiod,
    void*               pBuf,
    ber_len_t           len
    )
{
    SSL*    pSsl = NULL;
    int     iSslRet = 0;
    PCSTR   pszCaller = NULL;

    assert( pSbiod != NULL );
    assert( SOCKBUF_VALID( pSbiod->sbiod_sb ) );

    assert( pSbiod->sbiod_pvt );
    pSsl = (SSL*)pSbiod->sbiod_pvt;

    iSslRet = SSL_write( pSsl, pBuf, (int)len );
    BAIL_ON_OPENSSL_ERROR( (iSslRet <= 0), "SSL_write"); // we use blocking IO, iSslRet > 0 is success;

cleanup:

    return iSslRet;

openssl_error:

    _VmDirLogOpenSSLError( iSslRet, pszCaller);
    goto cleanup;
}

/*
 * terminate SSL connection
 * called via sockbuf.c:ber_int_sb_close
 */
static
int
opensslSbClose(
    Sockbuf_IO_Desc*    pSbiod
    )
{
    SSL*    pSsl = NULL;
    int     iSslRet = 0;

    assert( pSbiod != NULL );
    assert( SOCKBUF_VALID( pSbiod->sbiod_sb ) );

    pSsl = (SSL*)pSbiod->sbiod_pvt;

    // clean up SSL*
    if (pSsl)
    {
        iSslRet = SSL_shutdown(pSsl);
        if (iSslRet == 0)
        {
            // From http://www.openssl.org/docs/ssl/SSL_shutdown.html#
            // The shutdown is not yet finished. Call SSL_shutdown() for a second time, if a bidirectional shutdown shall be performed.
            iSslRet = SSL_shutdown( pSsl );
        }
    }

    if (pSsl)
    {
        SSL_free(pSsl);
        pSbiod->sbiod_pvt = NULL;
    }

    // close socket
    if (pSbiod->sbiod_sb->sb_fd > 0)
    {
        tcp_close( pSbiod->sbiod_sb->sb_fd );
    }

   return 0;
}

/*
 *  For each new connection, setup Sockbuf_IO_Desc
 *  1. assign sockfd
 *  2. initiate openssl handshake (SSL_accept)
 *
 *  return  0 if success.
 *  return -1 if fail.
 *
 *  TODO: currently, SSL connection setup is called per new client side connection
 *  request.  To scale better, we should support SSL session caching in the future.
 */
static
int
opensslSbSetup(
    Sockbuf_IO_Desc*    pSbiod,
    void*               pArg     // ber_socket_t*
    )
{
    DWORD       dwError = 0;
    SSL*        pSsl  = NULL;
    int         iSslRet = 0;
    PCSTR       pszCaller = NULL;

    assert( pSbiod && pArg);

    pSbiod->sbiod_sb->sb_fd = *((int *)pArg);

    pSsl = SSL_new(gpVdirSslCtx);
    BAIL_ON_OPENSSL_ERROR( (pSsl == NULL), "SSL_new" );

#ifdef _WIN32
    #pragma warning( push )
    #pragma warning( disable : 4244 )
#endif
    iSslRet = SSL_set_fd(pSsl, pSbiod->sbiod_sb->sb_fd);
    BAIL_ON_OPENSSL_ERROR( (iSslRet != 1), "SSL_set_fd" );
#ifdef _WIN32
    #pragma warning( pop )
#endif

    iSslRet = SSL_accept(pSsl);  // SSL_accept return 1 if success
    BAIL_ON_OPENSSL_ERROR( (iSslRet != 1), "SSL_accept");

    // SSL/TLS connection established, let pSbiod owns it.
    // Use Sockbuf_IO_Desc.sbiod_pvt, a PVOID, to hold SSL*
    pSbiod->sbiod_pvt = pSsl;
    dwError = 0;    //reset dwError to 0

cleanup:

    return dwError;

error:

    if (pSsl)
    {
        SSL_shutdown(pSsl);
        SSL_free(pSsl);
        pSbiod->sbiod_pvt = NULL;
    }

    // always return -1 error code (as caller ber_sockbuf_add_io expect <0 for error case)
    dwError = -1;

    goto cleanup;

openssl_error:

    _VmDirLogOpenSSLError( iSslRet, pszCaller);

    goto error;
}

static
int
opensslSbCtrl(
    Sockbuf_IO_Desc*    pSbiod,
    int                 opt,
    void*               pArg
    )
{
    /* This is an end IO descriptor */
    return 0;
}

/*
static
int
opensslSbRemove(
    Sockbuf_IO_Desc*    pSbiod
    )
{
    return 0;
}
*/

#define MAX_SSL_ERROR_BUF_LEN   120

static
VOID
_VmDirLogOpenSSLError(
    int     iSslErr,
    PCSTR   pszCaller
    )
{
    PCSTR           pszMsg = NULL;
    unsigned long   ulErr = 0;

    VmDirLog( LDAP_DEBUG_ANY, "Failed SSL function (%s), return value (%d)", VDIR_SAFE_STRING(pszCaller), iSslErr);

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

        VmDirLog( LDAP_DEBUG_ANY, "SSL error (%lu)(%s)(%s)", ulErr, pszMsg, errbuf);
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
_VmDirOpensslLockingFunc(
    int         mode,
    int         lockNum,
    const char *file,
    int         line
    )
{
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock( &(gVmdirOpensslGlobals.pMutexBuf[lockNum]) );
    } else {
        pthread_mutex_unlock( &(gVmdirOpensslGlobals.pMutexBuf[lockNum]) );
    }
}

/**
 * OpenSSL uniq id function.
 *
 * @return    thread id
 */
static
unsigned long
_VmDirOpensslIdFunc(
    VOID
    )
{
#ifdef _WIN32
    return ((unsigned long) (pthread_self().p));
#else
    return ((unsigned long) pthread_self());
#endif
}
