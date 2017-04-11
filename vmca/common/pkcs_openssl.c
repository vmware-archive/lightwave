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



#include "includes.h"

static VMCA_OPENSSL_GLOBALS gVmcaOpenSslGlobals = {0};

static
VOID
VMCAFreeCA(
    PVMCA_X509_CA pCA
    );

static
DWORD
GetError(
    VOID
    );

static
BOOLEAN
IsWildcardString(
    PCSTR szString,
    size_t length
    )
{
    const char ch = '*';
    size_t pos = 0;

    while (pos < length &&
           szString[pos] != ch)
    {
        ++pos;
    }

    return (szString[pos] == ch);
}

static
DWORD
VMCASetAddDCToX509Name(
     X509_NAME *pCertName,
     PSTR pszDomainName
     );

VOID
VMCACleanupSslMutexes(pthread_mutex_t* pSslMutexes, int nSslMutexes)
{
    if (pSslMutexes)
    {
        int i = 0;
        int dwNumMutexes = nSslMutexes;
        for (i = 0; i < dwNumMutexes; ++i)
        {
            pthread_mutex_destroy( &(pSslMutexes[i]));
        }

        VMCAFreeMemory(pSslMutexes);
    }
}

static void
VMCAOpenSSLLockingCallback(
    int mode,
    int index,
    const char * file,
    int line
    )
{
    if (gVmcaOpenSslGlobals.pSslMutexes && gVmcaOpenSslGlobals.nSslMutexes > index)
    {
        if (mode & CRYPTO_LOCK)
        {
             pthread_mutex_lock(&gVmcaOpenSslGlobals.pSslMutexes[index]);
        }
        else
        {
             pthread_mutex_unlock(&gVmcaOpenSslGlobals.pSslMutexes[index]);
        }
    }
}

DWORD
VMCAIntializeOpenSSL()
{
    DWORD dwError = 0;
    pthread_mutex_t* pSslMutexes = NULL;
    int nSslMutexes = 0;

    if (gVmcaOpenSslGlobals.pSslMutexes == NULL)
    {
        int i = 0;
        int dwNumMutexes = CRYPTO_num_locks();

        nSslMutexes = 0;

        dwError = VMCAAllocateMemory(dwNumMutexes * sizeof(pthread_mutex_t), (PVOID*)&pSslMutexes);
        BAIL_ON_ERROR(dwError);

        for (i = 0; i < dwNumMutexes; ++i)
        {
            if ( 0 != pthread_mutex_init( &(pSslMutexes[i]), NULL))
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                BAIL_ON_ERROR(dwError);
            }
            ++nSslMutexes;
        }

        gVmcaOpenSslGlobals.pSslMutexes = pSslMutexes;
        gVmcaOpenSslGlobals.nSslMutexes = nSslMutexes;
        pSslMutexes = NULL;

        CRYPTO_set_locking_callback(VMCAOpenSSLLockingCallback);
    }

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

cleanup:
    return dwError;

error:
    if (pSslMutexes)
    {
        VMCACleanupSslMutexes(pSslMutexes, nSslMutexes);
    }
    goto cleanup;
}

DWORD
VMCACleanupOpenSSL()
{
    EVP_cleanup();
    ERR_free_strings();
    CRYPTO_set_locking_callback(NULL);

    VMCACleanupSslMutexes(gVmcaOpenSslGlobals.pSslMutexes, gVmcaOpenSslGlobals.nSslMutexes);

    gVmcaOpenSslGlobals.pSslMutexes = NULL;
    gVmcaOpenSslGlobals.nSslMutexes = 0;

    return ERROR_SUCCESS;
}

DWORD
VMCAPrivateKeyToPEM(
    EVP_PKEY* pPrivateKey,
    PSTR* ppPrivateKey
)
// This function creates takes a Private Key and returns the Private Part
// as PEM Encoded String
//
// Arguments :
//          pPrivateKey     : OpenSSL EVP_PKEY which is Private Key
//          ppPrivateKey    : PEM encoded Private Key String.
//
// Returns :
//      Error Code
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;
	PKCS8_PRIV_KEY_INFO *pPkcs8FormatKey = NULL;

    if(pPrivateKey == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

	pPkcs8FormatKey = EVP_PKEY2PKCS8_broken(pPrivateKey,PKCS8_OK);
	if ( pPkcs8FormatKey == NULL) {
		dwError = VMCA_KEY_IO_FAILURE;
		BAIL_ON_ERROR(dwError);
	}

	dwError = PEM_write_bio_PKCS8_PRIV_KEY_INFO(pBioMem,pPkcs8FormatKey);
    BAIL_ON_SSL_ERROR(dwError, VMCA_KEY_IO_FAILURE);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VMCAAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pszBuffer);
    BAIL_ON_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    *ppPrivateKey = pszBuffer;

cleanup:
    if ( pBioMem != NULL ) {
        BIO_free_all(pBioMem);
    }
    PKCS8_PRIV_KEY_INFO_free(pPkcs8FormatKey);

    return dwError;
error :

    VMCA_SAFE_FREE_MEMORY(pszBuffer);
    goto cleanup;
}

DWORD
VMCAPublicKeyToPEM(
    EVP_PKEY* pPubKey,
    PSTR* ppPublicKey
)
// This function creates takes a Private Key and returns the public part
// as a PEM Encoded String.
//
// Arguments :
//          pPubKey         : OpenSSL EVP_PKEY which is Private Key
//          ppPublicKey     : PEM encoded Public Key String.
//
// Returns :
//      Error Code
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;

    if(pPubKey == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = PEM_write_bio_PUBKEY(
                  pBioMem,
                  pPubKey);
    BAIL_ON_SSL_ERROR(dwError, VMCA_KEY_IO_FAILURE);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VMCAAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pszBuffer);
    BAIL_ON_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    *ppPublicKey = pszBuffer;
cleanup:
     if ( pBioMem != NULL) {
        BIO_free(pBioMem);
    }

    return dwError;


error :

    VMCA_SAFE_FREE_MEMORY(pszBuffer);
    goto cleanup;
}


DWORD
VMCACRLToPEM(
    X509_CRL* pCrl,
    PSTR* ppCrl
)
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;

    if ( pCrl == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = PEM_write_bio_X509_CRL(
                  pBioMem,
                  pCrl);

    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VMCAAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pszBuffer);
    BAIL_ON_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    *ppCrl = pszBuffer;

cleanup:
    if ( pBioMem != NULL) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
    if (ppCrl)
    {
        *ppCrl = NULL;
    }
     VMCA_SAFE_FREE_MEMORY(pszBuffer);
     goto cleanup;
}


DWORD
VMCACertToPEM(
    X509* pCertificate,
    PSTR* ppCertificate
)
// This function takes a Certificate and returns a PEM Encoded String.
//
// Arguments :
//          pCertificate      : OpenSSL X509 Certificate
//          ppCertificate     : PEM encoded Certificate String.
//
// Returns :
//      Error Code
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;

    if ( pCertificate == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = PEM_write_bio_X509(
                  pBioMem,
                  pCertificate);

    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VMCAAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pszBuffer);
    BAIL_ON_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    *ppCertificate = pszBuffer;

cleanup:
    if ( pBioMem != NULL) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
     VMCA_SAFE_FREE_MEMORY(pszBuffer);
     goto cleanup;
}

DWORD
VMCACertStackToPEM(
    STACK_OF(X509) *skX509certs,
    PSTR* ppszCertificate
    )
{
    DWORD dwError = 0;
    PSTR pszBuffer = NULL;
    DWORD dwIndex = 0;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    X509 *certificate = NULL;
    DWORD dwNumCerts = 0;

    if (!skX509certs ||
        !ppszCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwNumCerts = sk_X509_num(skX509certs);

    for (; dwIndex <dwNumCerts; dwIndex++)
    {
        certificate = sk_X509_value(skX509certs, dwIndex);
        if (!certificate)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMCA_ERROR (dwError);
        }

        if (!PEM_write_bio_X509(pBioMem, certificate))
        {
            dwError = ERROR_BAD_FORMAT;
            BAIL_ON_VMCA_ERROR (dwError);
        }
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VMCAAllocateMemory(
                        (DWORD)pBuffMem->length +1,
                        (PVOID*)&pszBuffer
                        );
    BAIL_ON_VMCA_ERROR (dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length-1);

    pszBuffer[pBuffMem->length-1] = '\n';

    *ppszCertificate = pszBuffer;


cleanup:
    if ( pBioMem != NULL) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
     if (ppszCertificate)
     {
        *ppszCertificate = NULL;
     }
     VMCA_SAFE_FREE_MEMORY (pszBuffer);
     goto cleanup;

}


DWORD
VMCACSRToPEM(
        X509_REQ* pCSR,
        PSTR* ppCSR
)
// This function takes a CSR and returns a PEM Encoded String.
//
// Arguments :
//          pCertificate      : OpenSSL CSR
//          ppCertificate     : PEM encoded CSR String.
//
// Returns :
//      Error Code
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;

    if ( pCSR == NULL ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = PEM_write_bio_X509_REQ(
                  pBioMem,
                  pCSR);

    BAIL_ON_SSL_ERROR(dwError, VMCA_REQUEST_ERROR);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VMCAAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pszBuffer);
    BAIL_ON_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);
    *ppCSR = pszBuffer;

cleanup:
    if ( pBioMem != NULL) {
        BIO_free(pBioMem);
    }
    return dwError;
error :
    VMCA_SAFE_FREE_MEMORY(pszBuffer);
    goto cleanup;
}

DWORD
VMCACSRToFile(
        X509_REQ* pREQ,
        PSTR pszFileName
        )
{

    FILE * fp = NULL;
    DWORD dwError = 0;

    if (!pREQ ||
        IsNullOrEmptyString (pszFileName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "w", &fp);
    BAIL_ON_ERROR(dwError);

    dwError = PEM_write_X509_REQ(
                  fp,
                  pREQ);
    BAIL_ON_SSL_ERROR(dwError, VMCA_REQUEST_ERROR);

cleanup:
    if(fp)
    {
        fclose(fp);
    }
    return dwError;

error :
    goto cleanup;
}


DWORD
VMCACRLToFile(
        X509_CRL* pCRL,
        PSTR pszFileName
)
// This function takes a CSR and returns a PEM Encoded String.
//
// Arguments :
//          pCRL      : CRL
//          pszFileName : File Name to write the CRL to
// Returns :
//      Error Code
{

    FILE * fp = NULL;
    DWORD dwError = 0;

    if ( pCRL == NULL ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "w", &fp);
    BAIL_ON_ERROR(dwError);

    dwError = PEM_write_X509_CRL(
                  fp,
                  pCRL);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CRL_ERROR);

cleanup:
    if( fp != NULL){
        fclose(fp);
    }
    return dwError;

error :
    goto cleanup;
}

DWORD
VMCAPEMToX509Stack(
    PCSTR pCertificate,
    STACK_OF(X509) **pskX509certs
    )
{
    DWORD dwError = 0;
    STACK_OF(X509) *skX509certs = sk_X509_new_null();
    BIO *pBioMem = NULL;
    X509 *pCert = NULL;

    if (skX509certs == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    pBioMem = BIO_new_mem_buf((PVOID) pCertificate, -1);
    if (pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    while ((pCert = PEM_read_bio_X509( pBioMem, NULL, NULL, NULL)) != NULL)
    {
      sk_X509_push(skX509certs, pCert);
    }

    if (!sk_X509_num(skX509certs))
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    *pskX509certs = skX509certs;

cleanup:
    if (pBioMem)
    {
        BIO_free (pBioMem);
    }

    return dwError;

error:
    if (pskX509certs)
    {
        *pskX509certs = NULL;
    }

    sk_X509_pop_free(skX509certs, X509_free);

    goto cleanup;
}


DWORD
VMCAPEMToX509(
    PSTR pCertificate,
    X509 **ppX509Cert
)
{
    X509 *pCert = NULL;
    BIO *pBioMem = NULL;
    DWORD dwError = 0;

    pBioMem = BIO_new_mem_buf((PVOID) pCertificate, -1);
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pCert  = PEM_read_bio_X509(pBioMem, NULL, NULL, NULL);
    if (pCert  == NULL){
        dwError = VMCA_CERT_IO_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    *ppX509Cert = pCert;
cleanup :
    if(pBioMem) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
    goto cleanup;
}

DWORD
VMCAPEMToPrivateKey(
    PSTR pKey,
    RSA **ppPrivateKey
)
{

    BIO *pBioMem = NULL;
    RSA *pRsa = NULL;
    DWORD dwError = 0;
    pBioMem = BIO_new_mem_buf((PVOID) pKey, -1);
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pRsa  = PEM_read_bio_RSAPrivateKey(pBioMem, NULL, NULL, NULL);
    if (pRsa  == NULL){
        dwError = VMCA_KEY_IO_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    *ppPrivateKey = pRsa;

cleanup :
    if(pBioMem) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
    goto cleanup;

}

DWORD
VMCAPEMToPublicKey(
    PSTR pKey,
    RSA **ppPublicKey
)
{
    BIO *pBioMem = NULL;
    RSA *pRsa = NULL;
    DWORD dwError = 0;

    pBioMem = BIO_new_mem_buf((PVOID) pKey, -1);
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pRsa  = PEM_read_bio_RSA_PUBKEY(pBioMem, NULL, NULL, NULL);
    if (pRsa  == NULL){
        dwError = VMCA_KEY_IO_FAILURE;
        BAIL_ON_ERROR(dwError);
    }
    *ppPublicKey = pRsa;

cleanup :
    if(pBioMem) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
    goto cleanup;

}


DWORD
VMCAPEMToCSR(
    PCSTR pCSR,
    X509_REQ **ppReq
)
{
    BIO *pBioMem = NULL;
    X509_REQ *pRequest = NULL;
    DWORD dwError = 0;

    pBioMem = BIO_new_mem_buf((PVOID) pCSR,-1);
    if ( pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pRequest  = PEM_read_bio_X509_REQ(pBioMem, NULL, NULL, NULL);
    if (pRequest  == NULL){
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_ERROR(dwError);
    }
    *ppReq = pRequest;

cleanup :
    if(pBioMem) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
    goto cleanup;

}


DWORD
VMCAFileToCRL(
    PSTR pszFileName,
    X509_CRL **ppCRL
)
{
    X509_CRL *pCRL = NULL;
    DWORD dwError = 0;
    FILE *fp =  NULL;

    dwError = VMCAOpenFilePath(pszFileName, "r", &fp);
    BAIL_ON_ERROR(dwError);

    if(fp == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pCRL  = PEM_read_X509_CRL(fp, NULL, NULL, NULL);
    if (pCRL  == NULL){
        dwError = VMCA_CRL_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    *ppCRL = pCRL;

cleanup :
    if( fp != NULL) {
        fclose(fp);
    }
    return dwError;
error :
    goto cleanup;
}


DWORD
VMCAAllocatePrivateKeyPrivate(
    PWSTR pszPassPhrase,
    size_t uiKeyLength,
    PSTR* ppPrivateKey,
    PSTR* ppPublicKey
)

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
{
    #define EXPONENT 65537
    DWORD dwError = 0;
    EVP_PKEY *pPrivateKey = NULL;
    RSA *rsa = NULL;
    BIGNUM *pBigNum = NULL;
    PSTR pszPrivString = NULL;
    PSTR pszPubString = NULL;

    if( ( uiKeyLength < 1024 ) ||
        ( uiKeyLength > 16 * 1024 ) ) {
        dwError = VMCA_ERROR_INVALID_KEY_LENGTH;
        BAIL_ON_ERROR(dwError);
    }

    if ( ( ppPrivateKey == NULL ) ||
         ( ppPublicKey == NULL ) ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if ( ( pPrivateKey=EVP_PKEY_new() ) == NULL ) {
        dwError = VMCA_KEY_CREATION_FAILURE;
        BAIL_ON_ERROR( dwError );
    }

    rsa = RSA_new();
    if ( rsa == NULL ) {
        dwError = VMCA_KEY_CREATION_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    EVP_PKEY_assign_RSA(pPrivateKey, rsa);

    pBigNum = BN_new();
    if ( pBigNum == NULL) {
        dwError = VMCA_KEY_CREATION_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    BN_set_word(pBigNum, EXPONENT);

    dwError = RSA_generate_key_ex(rsa,(int) uiKeyLength, pBigNum, NULL);
    if ( dwError == -1) {
        dwError = VMCA_KEY_CREATION_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPrivateKeyToPEM(pPrivateKey, &pszPrivString);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAPublicKeyToPEM(pPrivateKey, &pszPubString);
    BAIL_ON_ERROR(dwError);

    *ppPrivateKey = pszPrivString;
    *ppPublicKey = pszPubString;


cleanup:
    if ( pBigNum != NULL ) {
        BN_free(pBigNum);
    }

    // This will free rsa too.
    if ( pPrivateKey != NULL ) {
        EVP_PKEY_free(pPrivateKey);
    }

    return dwError;

error :
    VMCA_SAFE_FREE_MEMORY(pszPrivString);
    VMCA_SAFE_FREE_MEMORY(pszPrivString);
    goto cleanup;
}



DWORD
VMCAReadCertificateFromFilePrivate(
    PSTR pszFileName,
    PSTR* ppszCertificate
)
// VMCAReadCertificateFromFile reads a Certificate from a file
//
// Arguments :
//  pszFilename : Full file Path to the certificate File
//  ppszCertificate : The Certificate String, Caller to Free this string after use
// Returns :
// Error Code
{
    DWORD dwError =0;
    X509 *pCert = NULL;
    FILE *fpCert = NULL;

    if (ppszCertificate == NULL) 
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "r", &fpCert);
    BAIL_ON_ERROR(dwError);

    pCert = PEM_read_X509(fpCert, NULL, NULL, NULL);
    if ( pCert == NULL ) {
         dwError = VMCA_CERT_IO_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACertToPEM(pCert,(PSTR*) ppszCertificate);
    BAIL_ON_ERROR(dwError);

error :
    if ( pCert ) {
        X509_free(pCert);
    }
    if ( fpCert ) {
        fclose(fpCert);
    }
    return dwError;
}

DWORD
VMCAReadCertificateChainFromFile(
    PSTR pszFileName,
    PSTR* ppszCertificate
)
{
    DWORD dwError = 0;
    FILE *fpCert = NULL;
    PSTR pszContent = NULL;
    DWORD dwFileSize = 0;
    PSTR pszCertificate = NULL;
    size_t dwBytesRead = 0;

    if (IsNullOrEmptyString(pszFileName) ||
        !ppszCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "r", &fpCert);
    BAIL_ON_ERROR(dwError);

    fseek(fpCert, 0L, SEEK_END);
    dwFileSize = ftell(fpCert);
    fseek(fpCert, 0L, SEEK_SET);

    dwError = VMCAAllocateMemory (
                          dwFileSize+1,
                          (PVOID *)&pszContent
                          );
    BAIL_ON_VMCA_ERROR (dwError);

    dwBytesRead = fread(
                    pszContent,
                    sizeof(CHAR),
                    dwFileSize,
                    fpCert
                    );
    if (!dwBytesRead)
    {
        if (ferror(fpCert))
        {
            dwError = GetError();
            BAIL_ON_VMCA_ERROR (dwError);
        }
    }

    pszContent[dwBytesRead] = '\0';


    dwError = VMCAValidateAndFormatCert(
                                        pszContent,
                                        &pszCertificate
                                       );
    BAIL_ON_VMCA_ERROR (dwError);

    *ppszCertificate = pszCertificate;

cleanup:

    if (fpCert)
    {
        fclose(fpCert);
    }

    VMCA_SAFE_FREE_MEMORY (pszContent);

    return dwError;

error:
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }

    VMCA_SAFE_FREE_MEMORY (pszCertificate);

    goto cleanup;
}


DWORD
VMCAReadPrivateKeyFromFilePrivate(
    PSTR pszFileName,
    PSTR pszPassPhrase,
    PSTR* ppszPrivateKey
)
// VMCAReadPrivateKeyFromFile reads a Private from a file
//
// Arguments :
//  pszFilename : Full file Path to the certificate File
//  pszPassPhrase : Optional password to protect the Private Key
//  ppszPrivateKey : The Private Key String, Caller to Free this string after use
// Returns :
// Error Code
{
    DWORD dwError =0;
    EVP_PKEY *pKey = NULL;
    FILE *fpKey = NULL;

    if (ppszPrivateKey == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "r", &fpKey);
    BAIL_ON_ERROR(dwError);

    pKey = PEM_read_PrivateKey(fpKey, NULL, NULL, NULL);
    if ( pKey == NULL ) {
        dwError = VMCA_KEY_IO_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPrivateKeyToPEM(pKey,(PSTR*)ppszPrivateKey);
    BAIL_ON_ERROR(dwError);

error :
    if ( pKey ) {
        EVP_PKEY_free(pKey);
    }

    if ( fpKey ) {
        fclose(fpKey);
    }
    return dwError;
}

DWORD
VMCAGetCertificateAsStringA(
    PVMCA_CERTIFICATE pCertificate,
    PSTR *ppCertString
    )
// VMCAGetCertificateAsString returns certificate in human readable
// format.
//
// pCertificate : PEM encoded certificate
// ppCertString : Human readable certificate String
{
    DWORD dwError = 0;
    BIO *pBioCert = NULL;
    X509 *pCert = NULL;
    BUF_MEM *pBuffMem = NULL;
    PSTR pTempCert = NULL;

    if ( IsNullOrEmptyString(pCertificate) ||
         ( ppCertString == NULL ) ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPEMToX509(pCertificate, &pCert);
    BAIL_ON_ERROR(dwError);

    pBioCert = BIO_new(BIO_s_mem());
    if ( pBioCert == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = X509_print(pBioCert, pCert);
    BIO_get_mem_ptr(pBioCert, &pBuffMem);

    dwError = VMCAAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pTempCert);
    BAIL_ON_ERROR(dwError);

    memcpy(pTempCert, pBuffMem->data, pBuffMem->length - 1);

    dwError = VMCAAllocateStringA(pTempCert, ppCertString);
    BAIL_ON_ERROR(dwError);

error :
    if (pCert) {
        X509_free(pCert);
    }

    if (pBioCert) {
        BIO_free(pBioCert);
    }

    if (pTempCert) {
        VMCA_SAFE_FREE_MEMORY(pTempCert);
    }
    return dwError;
}

DWORD
VMCAGetCertificateAsStringW(
    PVMCA_CERTIFICATE pCertificate,
    PWSTR *ppCertString
    )
{
    DWORD dwError = 0;
    PSTR pTemp = NULL;

    dwError = VMCAGetCertificateAsStringA(
                    pCertificate,
                    &pTemp);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(
                    pTemp,
                    ppCertString);

error:

    VMCA_SAFE_FREE_STRINGA(pTemp);
    return dwError;
}

DWORD
VMCAWritePublicKeyToFile(
    PSTR pszPublicKeyFileName,
    PSTR pszPublicKey
)
// VMCAWritePublicKeyToFile reads a Private from a file
//
// Arguments :
//  pszPublicKeyFileName : Full file Path to the where private key will be written to
//  pszPublicKey : Public Key
// Returns :
// Error Code
{

    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    RSA *pRSA = NULL;
    FILE *fpPubKey = NULL;

    if (IsNullOrEmptyString(pszPublicKey) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszPublicKeyFileName, "w", &fpPubKey);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAPEMToPublicKey(pszPublicKey, &pRSA);
    BAIL_ON_ERROR(dwError);

    dwError = PEM_write_RSAPublicKey(fpPubKey, pRSA);
    BAIL_ON_SSL_ERROR(dwError, VMCA_KEY_IO_FAILURE);

error :
    if ( pRSA ){
        RSA_free(pRSA);
    }

    if ( pBioMem ) {
        BIO_free(pBioMem);
    }

    if( fpPubKey ) {
        fclose(fpPubKey);
    }

    return dwError;
}


DWORD
VMCAWritePrivateKeyToFile(
    PSTR pszPrivateKeyFileName,
    PSTR pszPrivateKey,
    PSTR pszPassPhraseFileName,
    PWSTR pszPassPhrase
)
// VMCAWritePrivateKeyToFile reads a Private from a file
//
// Arguments :
//  pszPrivateKeyFileName : Full file Path to the where private key will be written to
//  pszPassPhrase : Optional password to protect the Private Key
//  pszPassPhraseFileName : Optional Password File Name, if specified the Password will also be stored
//  pszPassPhrase : Password that protects the PrivateKey
// Returns :
// Error Code
{

    DWORD dwError = 0;
    RSA *pRSA = NULL;
    FILE *fpPrivateKey = NULL;

    if (IsNullOrEmptyString(pszPrivateKey))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszPrivateKeyFileName, "w", &fpPrivateKey);
    BAIL_ON_ERROR(dwError);

    fwrite(pszPrivateKey, VMCAStringLenA(pszPrivateKey),1, fpPrivateKey);

error :
    if ( pRSA ){
        RSA_free(pRSA);
    }

    if(fpPrivateKey){
        fclose(fpPrivateKey);
    }

    return dwError;
}

DWORD
VMCAWriteCertificateChainToFile(
    PSTR pszFileName,
    PSTR pszCertificate
    )
{
    DWORD dwError = 0;
    STACK_OF(X509) *skX509Certs = NULL;
    FILE *fpCertificate = NULL;
    PSTR pszCanonicalCert = NULL;

    if (IsNullOrEmptyString(pszFileName) ||
        IsNullOrEmptyString(pszCertificate)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR (dwError);
    }

    dwError = VMCAPEMToX509Stack(
                                 pszCertificate,
                                 &skX509Certs
                                );
    BAIL_ON_ERROR (dwError);

    dwError = VMCACertStackToPEM(
                                 skX509Certs,
                                 &pszCanonicalCert
                                );
    BAIL_ON_ERROR (dwError);

    dwError = VMCAOpenFilePath(pszFileName, "w", &fpCertificate);
    BAIL_ON_ERROR(dwError);

    fprintf (
             fpCertificate,
             "%s\n",
             pszCanonicalCert
            );

cleanup:
    VMCA_SAFE_FREE_MEMORY (pszCanonicalCert);

    if (skX509Certs)
    {
        sk_X509_pop_free(skX509Certs, X509_free);
    }

    if( fpCertificate ) {
        fclose(fpCertificate);
    }

    return dwError;

error:
    goto cleanup;
}


DWORD
VMCAWriteCertificateToFile(
    PSTR pszFileName,
    PSTR pszCertificate
)
// VMCAWriteCertficateToFile writes a certificate to a file
//
// Arguments :
//    pszFileName : Full Path to the File where Certificate gets written to
//    pszCertificate : The Certificate that gets written
// Returns :
//  Error code
{
    DWORD dwError = 0;
    X509 *pCert  = NULL;
    FILE *fpCertificate = NULL;

    if (IsNullOrEmptyString(pszCertificate))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "w", &fpCertificate);
    BAIL_ON_ERROR(dwError);

   dwError = VMCAPEMToX509(pszCertificate, &pCert);
   BAIL_ON_ERROR(dwError);

    dwError = PEM_write_X509(fpCertificate, pCert);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

error :
    if ( pCert ){
        X509_free(pCert);
    }

    if( fpCertificate ) {
        fclose(fpCertificate);
    }

    return dwError;

}

DWORD
VMCAWriteCSRToFile(
    PSTR pszFileName,
    PVMCA_CSR pszCSR
    )
{
    DWORD dwError = 0;
    X509_REQ *pCSR  = NULL;

    if (IsNullOrEmptyString(pszCSR) ||
        IsNullOrEmptyString(pszFileName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPEMToCSR(pszCSR, &pCSR);
    BAIL_ON_ERROR(dwError);

    dwError = VMCACSRToFile(
                          pCSR,
                          pszFileName
                          );
    BAIL_ON_ERROR (dwError);

cleanup:

    if (pCSR)
    {
        X509_REQ_free(pCSR);
    }
    return dwError;

error:
    goto cleanup;
}



DWORD
VMCAValidateCACertificatePrivate(
    PSTR pszCertificate,
	PSTR pszPassPhrase,
	PSTR pszPrivateKey
)
//  VMCAValidateCACertificate checks if the given Certificate has
//  the capability to be a CA Cert
//  Arguments:
//  pszCertificate : Pointer to a PEM encoded Certificate String
//  Returns :
//  Error code , 0 means the certificate can be used as CA Cert
{
    X509 *pCert = NULL;
	EVP_PKEY *pKey = NULL;
	RSA *pRsa = NULL;
    DWORD dwError = 0;

    if ( IsNullOrEmptyString(pszCertificate)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPEMToX509(pszCertificate, &pCert);
    BAIL_ON_ERROR(dwError);

    dwError = X509_check_ca(pCert);
    if (dwError == 1) // it is  a CA
    {
        dwError = 0;
    } else { // 3 , 4 means it is a CA, but we don't care
        dwError = VMCA_NOT_CA_CERT;
    }
    BAIL_ON_ERROR(dwError);

    if ( pszPrivateKey != NULL) {
        pKey = EVP_PKEY_new();
        if ( pKey == NULL) {
            dwError = VMCA_OUT_MEMORY_ERR;
            BAIL_ON_ERROR(dwError);
        }

        dwError = VMCAPEMToPrivateKey(pszPrivateKey, &pRsa);
        BAIL_ON_ERROR(dwError);

        EVP_PKEY_assign_RSA(pKey, pRsa);

        dwError = X509_check_private_key(pCert, pKey);
        BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_PRIVATE_KEY_MISMATCH);
    }

error :
    if(pCert){
        X509_free(pCert);
    }

	// This will free RSA too.
	if(pKey){
		EVP_PKEY_free(pKey);
	}

    return dwError;
}

DWORD
VMCACreateCertificateName(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    X509_NAME *pCertName
)
// VMCACreateCertificateName Creates a X509_NAME
// for the certificate.
{
    LPSTR pszName = NULL;
    LPSTR pszDomainName = NULL;
    LPSTR pszCountry = NULL;
    LPSTR pszState = NULL;
    LPSTR pszLocality = NULL;
    LPSTR pszOrganization = NULL;
    LPSTR pszOU = NULL;

    DWORD dwError = 0;

    if ((pCertRequest == NULL) || (pCertName == NULL)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (pCertRequest->pszName)  {

            dwError = VMCAAllocateStringAFromW(pCertRequest->pszName, &pszName);
            BAIL_ON_ERROR(dwError);

            dwError = X509_NAME_add_entry_by_txt(pCertName,
                    "CN", MBSTRING_UTF8,
                    pszName, -1, -1, 0);
            ERR_print_errors_fp(stdout);
            BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
    }

    if (pCertRequest->pszDomainName) {

            dwError = VMCAAllocateStringAFromW(pCertRequest->pszDomainName, &pszDomainName);
            BAIL_ON_ERROR(dwError);

            dwError = VMCASetAddDCToX509Name(
                                        pCertName,
                                        pszDomainName
                                        );
            BAIL_ON_ERROR (dwError);

    }

    if (pCertRequest->pszCountry) {

            dwError = VMCAAllocateStringAFromW(pCertRequest->pszCountry, &pszCountry);
            BAIL_ON_ERROR(dwError);

            dwError = X509_NAME_add_entry_by_txt(pCertName,
                "C", MBSTRING_UTF8,
                pszCountry, -1, -1, 0);
            BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
    }

    if (pCertRequest->pszState) {

            dwError = VMCAAllocateStringAFromW(pCertRequest->pszState, &pszState);
            BAIL_ON_ERROR(dwError);

            dwError = X509_NAME_add_entry_by_txt(pCertName,
                "ST", MBSTRING_UTF8,
                pszState, -1, -1, 0);
            BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
    }

    if (pCertRequest->pszLocality) {

            dwError = VMCAAllocateStringAFromW(pCertRequest->pszLocality, &pszLocality);
            BAIL_ON_ERROR(dwError);

            dwError = X509_NAME_add_entry_by_txt(pCertName,
                "L", MBSTRING_UTF8,
                pszLocality, -1, -1, 0);
            BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
    }

    if (pCertRequest->pszOrganization) {

            dwError = VMCAAllocateStringAFromW(pCertRequest->pszOrganization, &pszOrganization);
            BAIL_ON_ERROR(dwError);

            dwError = X509_NAME_add_entry_by_txt(pCertName,
                "O", MBSTRING_UTF8,
                pszOrganization, -1, -1, 0);
            BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
    }

    if (pCertRequest->pszOU) {

            dwError = VMCAAllocateStringAFromW(pCertRequest->pszOU, &pszOU);
            BAIL_ON_ERROR(dwError);

            dwError = X509_NAME_add_entry_by_txt(pCertName,
                "OU", MBSTRING_UTF8,
                pszOU, -1, -1, 0);
            BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
    }

error :

    VMCA_SAFE_FREE_STRINGA(pszName);
    VMCA_SAFE_FREE_STRINGA(pszDomainName);
    VMCA_SAFE_FREE_STRINGA(pszCountry);
    VMCA_SAFE_FREE_STRINGA(pszState);
    VMCA_SAFE_FREE_STRINGA(pszLocality);
    VMCA_SAFE_FREE_STRINGA(pszOrganization);
    VMCA_SAFE_FREE_STRINGA(pszOU);

    return dwError;
}

static DWORD
VMCASetSubjectKeyIdentifier(
    STACK_OF(X509_EXTENSION) *pStack,
    X509 *pCert
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;
    X509_EXTENSION *pExtension = NULL;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, NULL, pCert, NULL, NULL, 0);

    pExtension = X509V3_EXT_conf_nid(NULL, &ctx, NID_subject_key_identifier, "hash");
    if (pExtension == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);
error:
    return dwError;
}

static DWORD
VMCASetCSRSubjectKeyIdentifier(
    STACK_OF(X509_EXTENSION) *pStack,
    X509_REQ *pReq
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;
    X509_EXTENSION *pExtension = NULL;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, NULL, NULL, pReq, NULL, 0);

    pExtension = X509V3_EXT_conf_nid(
                                NULL,
                                &ctx,
                                NID_subject_key_identifier,
                                (char*)"hash");
    if (pExtension == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);
error:
    return dwError;
}

static DWORD
VMCASetCSRAuthorityInfoAccess(
    STACK_OF(X509_EXTENSION) *pStack,
    X509 *pCert,
    X509 *pIssuer
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;
    X509_EXTENSION *pExtension = NULL;
    PSTR pszIPAddress = NULL;
    PSTR pszAIAString = NULL;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, pIssuer, pCert, NULL, NULL, 0);

    dwError = VmAfdGetPNIDA(NULL, &pszIPAddress);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                                &pszAIAString,
                                "caIssuers;URI:https://%s/afd/vecs/ssl",
                                pszIPAddress);
    BAIL_ON_ERROR(dwError);

    pExtension = X509V3_EXT_conf_nid(
                                NULL,
                                &ctx,
                                NID_info_access,
                                (char*)pszAIAString);
    if (pExtension == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);
error:
    VMCA_SAFE_FREE_MEMORY(pszIPAddress);
    VMCA_SAFE_FREE_MEMORY(pszAIAString);
    return dwError;
}

static DWORD
VMCASetAuthorityKeyIdentifier(
    STACK_OF(X509_EXTENSION) *pStack,
    X509 *pCert,
    X509 *pIssuer
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;
    X509_EXTENSION *pExtension = NULL;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, pIssuer, pCert, NULL, NULL, 0);

    pExtension = X509V3_EXT_conf_nid(
                                NULL,
                                &ctx,
                                NID_authority_key_identifier,
                                "keyid");
    if (pExtension == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);
error:
    return dwError;
}


DWORD
VMCAAddExtension(
    STACK_OF(X509_EXTENSION) *pStack,
    int NID,
    PSTR pszValue
)
{
    DWORD dwError = 0;
    X509_EXTENSION *pExtension = NULL;

    // Not so sure about the UTF8 handling here ... need to test
    pExtension = X509V3_EXT_conf_nid(NULL, NULL, NID, pszValue);
    if (pExtension == NULL) {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);

error:
    return dwError;
}

DWORD
VMCAAppendAlternateNameString(
    PSTR szDestinationString,
    size_t cDestinationString,
    PWSTR szSourceName,
    PSTR szSourceNameType
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR szNameElement = NULL;
    PSTR szNextToken = NULL;
    PSTR ptmpString = NULL;
    size_t nCurrentSize = VMCAStringLenA(szDestinationString);
    size_t sizeStr =  0;

    if (IsNullOrEmptyString(szSourceName) ||
        IsNullOrEmptyString(szSourceNameType))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateStringAFromW(szSourceName, &ptmpString);
    BAIL_ON_ERROR(dwError);

    szNameElement = VMCAStringTokA(ptmpString, ",", &szNextToken);
    while(szNameElement)
    {
        if (nCurrentSize != 0 )
        {
            dwError = VMCAStringCatA(szDestinationString, cDestinationString, ", ");
            BAIL_ON_ERROR(dwError);
            nCurrentSize += 2;
        }

        sizeStr = VMCAStringLenA(szNameElement) + VMCAStringLenA(szSourceNameType);
        if ( (cDestinationString - nCurrentSize) > sizeStr)
        {
            dwError = VMCAStringCatA(szDestinationString, cDestinationString, szSourceNameType);
            BAIL_ON_ERROR(dwError);

            dwError = VMCAStringCatA(szDestinationString, cDestinationString, szNameElement);
            BAIL_ON_ERROR(dwError);

            nCurrentSize += sizeStr;
        }
        else
        {
            break;
        }

        szNameElement = VMCAStringTokA(NULL, ",", &szNextToken);
    }

cleanup:
    if (ptmpString)
    {
        VMCAFreeStringA(ptmpString);
    }
    return dwError;
error:
    goto cleanup;
}

DWORD
VMCACreateExtensions(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    STACK_OF(X509_EXTENSION) *pStack
    )
{
    DWORD dwError = 0;
    char extensionString[1024] = { 0 };
    char subAltName[2048] = { 0 };

    /* Current behaviour for VMCA Clients
    // if( pCertRequest->dwKeyUsageConstraints == 0 ) {
    //             dwError = VMCAAddExtension(pStack,
    //                 NID_basic_constraints,
    //                 "critical,           \
    //                 digitalSignature,    \
    //                 nonRepudiation,      \
    //                 keyEncipherment,     \
    //                 dataEncipherment");
    //             BAIL_ON_ERROR(dwError);
    // }
    */

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_DIGITAL_SIGNATURE))
    {
        VMCAStringCatA(extensionString, sizeof(extensionString), "digitalSignature");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_NON_REPUDIATION))
    {
        if(VMCAStringLenA(extensionString) > 0)
        {
            VMCAStringCatA(extensionString, sizeof(extensionString), ", ");
        }
        VMCAStringCatA(extensionString, sizeof(extensionString), "nonRepudiation");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_KEY_ENCIPHERMENT))
    {
        if(VMCAStringLenA(extensionString) > 0)
        {
            VMCAStringCatA(extensionString, sizeof(extensionString), ", ");
        }
        VMCAStringCatA(extensionString, sizeof(extensionString), "keyEncipherment");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_DATA_ENCIPHERMENT))
    {
        if(VMCAStringLenA(extensionString) > 0)
        {
            VMCAStringCatA(extensionString, sizeof(extensionString), ", ");
        }
        VMCAStringCatA(extensionString, sizeof(extensionString), "dataEncipherment");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_KEY_AGREEMENT))
    {
        if(VMCAStringLenA(extensionString) > 0)
        {
            VMCAStringCatA(extensionString, sizeof(extensionString), ", ");
        }
        VMCAStringCatA(extensionString, sizeof(extensionString), "keyAgreement");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_KEY_CERT_SIGN))
    {
        VMCAStringCatA(extensionString, sizeof(extensionString), "critical, keyCertSign");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_KEY_CRL_SIGN))
    {
        if(VMCAStringLenA(extensionString) > 0)
        {
            VMCAStringCatA(extensionString, sizeof(extensionString), ", ");
        }
        VMCAStringCatA(extensionString, sizeof(extensionString), "cRLSign");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_ENCIPHER_ONLY))
    {
        if(VMCAStringLenA(extensionString) > 0)
        {
            VMCAStringCatA(extensionString, sizeof(extensionString), ", ");
        }
        VMCAStringCatA(extensionString, sizeof(extensionString), "encipherOnly");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints,VMCA_DECIPHER_ONLY))
    {
        if(VMCAStringLenA(extensionString) > 0)
        {
            VMCAStringCatA(extensionString, sizeof(extensionString), ", ");
        }
        VMCAStringCatA(extensionString, sizeof(extensionString), "decipherOnly");
    }

    if (VMCAisBitSet(pCertRequest->dwKeyUsageConstraints , VMCA_KEY_CERT_SIGN))
    {
        dwError = VMCAAddExtension(pStack,
                            NID_basic_constraints,
                            "critical,CA:TRUE, pathlen:0"
                            );
        BAIL_ON_ERROR(dwError);
    }

    if (VMCAStringLenA(extensionString) > 0)
    {
        dwError = VMCAAddExtension(pStack,
                            NID_key_usage,
                            extensionString
                            );
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertRequest->pszEmail))
    {
        dwError = VMCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            pCertRequest->pszEmail,
                            "email:"
                            );
    }

    if (!IsNullOrEmptyString(pCertRequest->pszIPAddress))
    {
        dwError = VMCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            pCertRequest->pszIPAddress,
                            "IP:"
                            );
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertRequest->pszURIName))
    {
        dwError = VMCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            pCertRequest->pszURIName,
                            "URI:"
                            );
        BAIL_ON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pCertRequest->pszDNSName))
    {
        dwError = VMCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            pCertRequest->pszDNSName,
                            "DNS:"
                            );
        BAIL_ON_ERROR(dwError);
    }

    if (VMCAStringLenA(subAltName) > 0)
    {
        dwError = VMCAAddExtension(pStack,
                        NID_subject_alt_name,
                        subAltName
                        );
        BAIL_ON_ERROR(dwError);
    }

error :
    return dwError;
}

DWORD
VMCACreateSigningRequestPrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PSTR pszPrivateKey,
    PWSTR pszPassPhrase,
    PSTR* ppAllocatedCSR
)
// VMCACreateSigningRequestPrivate creates a CSR
// from the VMCA_REQUEST_DATA
{

    RSA *pRsa = NULL;
    DWORD dwError = 0;
    X509_REQ  *pReq = NULL;
    X509_NAME *pCertName = NULL;
    STACK_OF(X509_EXTENSION) *pStack = NULL;
    EVP_PKEY *pKey = NULL;
    PSTR ptmpCSR = NULL;

    if ((pCertRequest == NULL) ||
        IsNullOrEmptyString(pszPrivateKey) ||
        (ppAllocatedCSR == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPEMToPrivateKey(pszPrivateKey, &pRsa);
    BAIL_ON_ERROR(dwError);

    pReq = X509_REQ_new();
    if(pReq == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }


    pCertName = X509_REQ_get_subject_name(pReq);
    if (pCertName == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACreateCertificateName(pCertRequest, pCertName);
    BAIL_ON_ERROR(dwError);

    pStack = sk_X509_EXTENSION_new_null();
    if( pStack == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACreateExtensions(pCertRequest, pStack);
    BAIL_ON_ERROR(dwError);

    pKey = EVP_PKEY_new();
    if ( pKey == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    EVP_PKEY_assign_RSA(pKey, pRsa);

    dwError = X509_REQ_set_pubkey(pReq, pKey);
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_PUBKEY_ERR);

    dwError = VMCASetCSRSubjectKeyIdentifier(pStack, pReq);
    BAIL_ON_ERROR(dwError);

//    dwError = VMCASetCSRAuthorityInfoAccess(pStack, pReq, pCertRequest->pszIPAddress);
//    BAIL_ON_ERROR(dwError);

    dwError = X509_REQ_add_extensions(pReq, pStack);
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_ADD_EXTENSION);

    dwError = X509_REQ_sign(pReq, pKey, EVP_sha256());
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_REQ_SIGN_ERR);

    dwError = VMCACSRToPEM(pReq, &ptmpCSR);
    BAIL_ON_ERROR(dwError);

    *ppAllocatedCSR = ptmpCSR;


cleanup :
    if(pKey){
        EVP_PKEY_free(pKey); // will free RSA too
    }

    if(pReq) {
        X509_REQ_free(pReq);
    }

    if ( pStack ) {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    return dwError;

error :
    if ( ptmpCSR ) {
        VMCAFreeStringA(ptmpCSR);
    }
    goto cleanup;
}

/////////////////////////////////////////////////////////////
DWORD
VMCAGenerateX509Serial(
   ASN1_INTEGER *pSerial
)
// VMCA generates random Serial Numbers
// This function creates the random serial numbers for certificates
{
    #define RAND_BITS_SIZE 64
    BIGNUM *bn = NULL;
    DWORD dwError = 0;

    bn = BN_new();
    if ( bn == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

	dwError = BN_pseudo_rand(bn, RAND_BITS_SIZE, 1, 0);
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_RAND_ERR);

    BN_to_ASN1_INTEGER(bn, pSerial);

error:
    if (bn != NULL){
        BN_free(bn);
    }

    return dwError;
}



//////////////////////////////////////////////////////////////////////////////////////////////////

DWORD
VMCASetCertExtensions(
    X509 *pCert,
    STACK_OF(X509_EXTENSION) *pStack
)
{
    DWORD dwError = 0;
    int nIndex = 0;
    X509_EXTENSION *extension = NULL;
    int nCount = X509v3_get_ext_count((const STACK_OF(X509_EXTENSION) *)pStack);

    for ( nIndex = 0; nIndex < nCount ; nIndex++) {
        extension = sk_X509_EXTENSION_pop(pStack);
        dwError = X509_add_ext(pCert,extension,-1);
        BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_EXT_ERR);
        X509_EXTENSION_free(extension);
    }
error :
    return dwError;
}

DWORD
VMCASelfSignedCertificatePrivate(
    PVMCA_PKCS_10_REQ_DATAW pCertRequest,
    PSTR pszPrivateKey,
    PWSTR pszPassPhrase,
    time_t tmNotBefore,
    time_t tmNotAfter,
    PSTR* ppszCertificate
)
// VMCASelfSignedCertificatePrivate creates
// a self Signed Certificate
{

    RSA *pRsa = NULL;
    DWORD dwError = 0;
    X509  *pCert = NULL;
    X509_NAME *pCertName = NULL;
    STACK_OF(X509_EXTENSION) *pStack = NULL;
    EVP_PKEY *pKey = NULL;
    PSTR  pszCertString = NULL;
	ASN1_INTEGER *aiSerial = NULL;

    if ((pCertRequest == NULL) ||
        IsNullOrEmptyString(pszPrivateKey) ||
        (ppszCertificate == NULL) ||
         (tmNotAfter < tmNotBefore) ||
         (tmNotBefore == 0 )  ||
         (tmNotAfter == 0) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    dwError = VMCAPEMToPrivateKey(pszPrivateKey, &pRsa);
    BAIL_ON_ERROR(dwError);

    pCert = X509_new();
    if(pCert == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pCertName = X509_get_subject_name(pCert);
    if (pCertName == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }
    dwError = VMCACreateCertificateName(pCertRequest, pCertName);
    BAIL_ON_ERROR(dwError);

    pStack = sk_X509_EXTENSION_new_null();
    if( pStack == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pKey = EVP_PKEY_new();
    if ( pKey == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    EVP_PKEY_assign_RSA(pKey, pRsa);
    X509_set_version(pCert,2);

    aiSerial = ASN1_INTEGER_new();
    if( aiSerial == NULL){
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAGenerateX509Serial(aiSerial);
	BAIL_ON_ERROR(dwError);

    X509_set_serialNumber(pCert,aiSerial);

	if (!ASN1_TIME_set(X509_get_notBefore(pCert), tmNotBefore)){
		dwError = 0;
		BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_START_TIME);
	}

	if(!ASN1_TIME_set(X509_get_notAfter(pCert), tmNotAfter)) {
		dwError = 0;
		BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_END_TIME);
	}

    dwError = X509_set_pubkey(pCert,pKey);
    BAIL_ON_SSL_ERROR( dwError, VMCA_SSL_SET_PUBKEY_ERR);

    dwError = X509_set_issuer_name(pCert, pCertName);
    BAIL_ON_SSL_ERROR( dwError, VMCA_SSL_SET_ISSUER_NAME);

    dwError = VMCACreateExtensions(pCertRequest, pStack);
    BAIL_ON_ERROR(dwError);

    dwError = VMCASetSubjectKeyIdentifier(pStack, pCert);
    BAIL_ON_ERROR(dwError);

    dwError = VMCASetCertExtensions(pCert, pStack);
    BAIL_ON_ERROR(dwError);

    dwError = X509_sign(pCert,pKey,EVP_sha256());
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_CERT_SIGN_ERR);

    dwError = VMCACertToPEM(pCert,&pszCertString);
    BAIL_ON_ERROR(dwError);

    *ppszCertificate = pszCertString;

cleanup :
    if(pKey){
        EVP_PKEY_free(pKey); // will free RSA too
    }

    if(pCert) {
        X509_free(pCert);
    }

    if ( pStack != NULL) {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }

    if( aiSerial != NULL){
        ASN1_INTEGER_free(aiSerial);
    }
    return dwError;

error :
    if(pszCertString != NULL) {
        VMCAFreeStringA(pszCertString);
    }
    goto cleanup;
}

DWORD
VMCACreateCA(
    PSTR pszCACertificate,
    PSTR pszPrivateKey,
    PSTR pszPassPhrase,
    PVMCA_X509_CA *pCA
)
// VMCACreate CA allocates a Certificate Authority based on
// the Certificate and Private Key that is passed on by the user.
//
// Arguments :
//      pszCACertificate : The CA's Root Certificate
//      pszPrivateKey    : The Private Key for the Certificate
//      pszPassPhrase    : Pass Phrase that protects the Private Key
//      pCA              : returns a pointer to the allocated CA
// Returns :
//      Error Code
{

    DWORD dwError = 0;
    RSA *pRSA = NULL;
    X509 *pSSCert = NULL;

    PVMCA_X509_CA pVCA= NULL;

    if ((pszCACertificate == NULL) || (pszPrivateKey == NULL) || (pCA == NULL))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(sizeof(VMCA_X509_CA), (PVOID*) &pVCA);
    BAIL_ON_ERROR(dwError);

    pVCA->refCount = 1;

    pVCA->pKey = EVP_PKEY_new();

    if(pVCA->pKey == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }
    dwError =  VMCAPEMToX509( pszCACertificate, &pVCA->pCertificate);
    BAIL_ON_ERROR(dwError);

    pRSA = RSA_new();
    if(pRSA == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPEMToPrivateKey( pszPrivateKey, &pRSA);
    BAIL_ON_ERROR(dwError);

    // Releasing EVP will release rsa
    EVP_PKEY_assign_RSA(pVCA->pKey, pRSA);

    dwError = VMCAPEMToX509Stack(
                                 pszCACertificate,
                                 &(pVCA->skCAChain)
                                );

    BAIL_ON_ERROR (dwError);

    if (sk_X509_num(pVCA->skCAChain) > 1)
    {
        pSSCert = sk_X509_pop(pVCA->skCAChain);
    }

    dwError = VMCACertStackToPEM(
                                 pVCA->skCAChain,
                                 &pVCA->pszCertificate
                                );
    BAIL_ON_ERROR (dwError);

    if (pSSCert)
    {
      sk_X509_push(pVCA->skCAChain, pSSCert);
      pSSCert = NULL;
    }

    *pCA = pVCA;

cleanup:

    if (pSSCert)
    {
        X509_free(pSSCert);
    }

    return dwError;

error :

    if ( pVCA != NULL)
    {
        VMCAReleaseCA(pVCA);
    }

    goto cleanup;
}

PVMCA_X509_CA
VMCAAcquireCA(
	PVMCA_X509_CA pCA
	)
{
	if (pCA)
	{
		InterlockedIncrement(&pCA->refCount);
	}

	return pCA;
}

VOID
VMCAReleaseCA(
    PVMCA_X509_CA pCA
    )
{
    if ( (pCA != NULL) && (InterlockedDecrement(&pCA->refCount) == 0))
    {
    	VMCAFreeCA(pCA);
    }
}

static
VOID
VMCAFreeCA(
	PVMCA_X509_CA pCA
	)
{
    if (pCA->pCertificate != NULL) {
        X509_free(pCA->pCertificate);
    }
    if(pCA->pKey != NULL) {
        EVP_PKEY_free(pCA->pKey);
    }
    VMCA_SAFE_FREE_STRINGA (pCA->pszCertificate);
    if (pCA->skCAChain)
    {
        sk_X509_pop_free(pCA->skCAChain, X509_free);
    }
    VMCAFreeMemory(pCA);
}

DWORD
VMCAGetCertificateName(
    X509 *pCert,
    PSTR *ppszCertName
)
{
    DWORD dwError = 0;
    X509_NAME *pCertName = NULL;
    char *pName = NULL;

    if( pCert == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pCertName = X509_get_subject_name(pCert);
    if ( pCertName == NULL) {
        dwError = VMCA_CERT_DECODE_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    pName = X509_NAME_oneline(pCertName, NULL , 0);
    if( pName != NULL ) {
        dwError = VMCAAllocateStringA(pName, ppszCertName);
        BAIL_ON_ERROR(dwError);
    }

// TODO : it is technically possible for a Certificate to have no name
// Should we fail on that behavior ?


error :
    if(pName != NULL) {
        free(pName);
    }
    // if(pCertName != NULL) {
    //     X509_NAME_free(pCertName);
    // }
    return dwError;
}

DWORD
VMCAGetIssuerName(
    X509 *pCert,
    PSTR *ppszIssuerName
)
{
    DWORD dwError = 0;
    X509_NAME *pIssuerName = NULL;
    char *pName = NULL;
    PSTR pszIssuerName = NULL;

    if( pCert == NULL ||
        !ppszIssuerName
      )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pIssuerName = X509_get_issuer_name(pCert);
    if ( pIssuerName == NULL) {
        dwError = VMCA_CERT_DECODE_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    pName = X509_NAME_oneline(pIssuerName, NULL , 0);
    if( pName != NULL ) {
        dwError = VMCAAllocateStringA(pName, &pszIssuerName);
        BAIL_ON_ERROR(dwError);
    }

    *ppszIssuerName = pszIssuerName;

cleanup:
    if(pName != NULL) {
        free(pName);
    }
    return dwError;

error:
    if (ppszIssuerName)
    {
      *ppszIssuerName = NULL;
    }

    VMCA_SAFE_FREE_STRINGA(pszIssuerName);

    goto cleanup;
}

DWORD
VMCAGetCertificateSerial(
    X509 *pCert,
    PSTR *ppszSerialNumber
)
{
    DWORD dwError = 0;
    ASN1_INTEGER *pSerialNumber = NULL;
    BIO *pBioMem = NULL;
    int written = 0;
    #define SERIAL_LEN 256
    char szSerialNumber[SERIAL_LEN] = { 0 };

    if (pCert == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    pSerialNumber = X509_get_serialNumber(pCert);
    i2a_ASN1_INTEGER(pBioMem, pSerialNumber);

    written = BIO_read(pBioMem, szSerialNumber, SERIAL_LEN-1);
    dwError = VMCAAllocateStringA(szSerialNumber, ppszSerialNumber);
    BAIL_ON_ERROR(dwError);

error :
    if (pBioMem != NULL) {
        BIO_free(pBioMem);
    }
    return dwError;
}

DWORD
VMCAGetCertificateTime(
    X509 *pCert,
    PSTR *ppszNotBefore,
    PSTR *ppszNotAfter
)
{

    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    int written = 0;
    #define BUF_LEN 32
    char buff[BUF_LEN]= {0};
    PSTR tmpNotBefore = NULL;
    PSTR tmpNotAfter = NULL;

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL) {
            dwError = VMCA_OUT_MEMORY_ERR;
            BAIL_ON_ERROR(dwError);
    }

    dwError = ASN1_TIME_print(pBioMem, X509_get_notBefore(pCert));
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_TIME_ERROR);

    written = BIO_read(pBioMem, buff, BUF_LEN-1);

    dwError = VMCAAllocateStringA(buff,&tmpNotBefore);
    BAIL_ON_ERROR(dwError);

    dwError = ASN1_TIME_print(pBioMem, X509_get_notAfter(pCert));
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_TIME_ERROR);

    written = BIO_read(pBioMem, buff, BUF_LEN-1);

    dwError = VMCAAllocateStringA(buff, &tmpNotAfter);
    BAIL_ON_ERROR(dwError);

    *ppszNotBefore = tmpNotBefore;
    *ppszNotAfter = tmpNotAfter;

cleanup :
    if(pBioMem) {
        BIO_free(pBioMem);
    }
    return dwError;

error :
    if (tmpNotBefore != NULL) {
        VMCAFreeStringA(tmpNotBefore);
    }

    if (tmpNotAfter != NULL) {
        VMCAFreeStringA(tmpNotAfter);
    }
    goto cleanup;
}



// TODO : Implement proper verification for extensions
DWORD
VMCAVerifyExtensions(
    STACK_OF(X509_EXTENSION) *pExtension
)
{
    return 0;
}

DWORD
VMCACopyExtensions(
    X509 *pCertificate,
    X509 *pCACertificate,
    X509_REQ *pRequest
    )
{
    DWORD dwError = 0;
    STACK_OF(X509_EXTENSION) *pStack  = NULL;
    X509_EXTENSION *pExtension = NULL;
    int extCount = 0;
    int Counter = 0;

    pStack = X509_REQ_get_extensions(pRequest);
    if(pStack == NULL) {
        goto error; // nothing to do here, just get out
    }

    // Copy AuthorityKeyId from CA certificate
    dwError = VMCASetAuthorityKeyIdentifier(pStack, pCertificate, pCACertificate);
    BAIL_ON_ERROR(dwError);

    dwError = VMCASetCSRAuthorityInfoAccess(pStack, pCertificate, pCACertificate);
    BAIL_ON_ERROR(dwError);

    extCount = sk_X509_EXTENSION_num(pStack);
    for(Counter = 0; Counter < extCount; Counter ++)
    {
        pExtension = sk_X509_EXTENSION_value(pStack, Counter);

        // TODO : Clean up the Extensions, and have
        // Policy on duplicate extension ext.
        // We should probably log all this information before
        // returning the certificate too.

        dwError = X509_add_ext(pCertificate, pExtension, -1);
        BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_EXT_ERR);
    }

error:
    if(pStack) {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    return dwError;
}

DWORD
VMCAVerifyCertificateName(
    X509* pCert
)
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwCNPos = -1;
    X509_NAME_ENTRY *pCNEntry = NULL;
    ASN1_STRING *pCNAsn1 = NULL;
    PSTR pszCNString = NULL;
    size_t length = 0;

    for(;;)
    {
        dwCNPos = X509_NAME_get_index_by_NID(X509_get_subject_name(pCert), NID_commonName, dwCNPos);
        if (dwCNPos == -1)
        {
            break;
        }

        pCNEntry = X509_NAME_get_entry(X509_get_subject_name(pCert), dwCNPos);
        if (pCNEntry == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        pCNAsn1 = X509_NAME_ENTRY_get_data(pCNEntry);
        if (pCNAsn1 == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        length = ASN1_STRING_to_UTF8((unsigned char **)&pszCNString, pCNAsn1);

        if (!pszCNString || length != strlen(pszCNString))
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        if (IsWildcardString(pszCNString, length))
        {
            dwError = VMCA_ERROR_INVALID_SN;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        if (pszCNString)
        {
             OPENSSL_free(pszCNString);
             pszCNString = NULL;
        }
    }

cleanup:
    if (pszCNString)
    {
         OPENSSL_free(pszCNString);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VMCAVerifySubjectAltNames(
    X509* pCert
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD i;
    DWORD dwCountSanNames = 0;
    STACK_OF(GENERAL_NAME)* pSANNames = NULL;
    DWORD dwDNSCount = 0;
    PSTR pszSANName = NULL;

    pSANNames = X509_get_ext_d2i(pCert, NID_subject_alt_name, NULL, NULL);
    if (pSANNames == NULL)
    {
        dwError = ERROR_SUCCESS;
        goto error;
    }

    dwCountSanNames = sk_GENERAL_NAME_num(pSANNames);

    for (i = 0; i < dwCountSanNames; i++)
    {
        const GENERAL_NAME *pCurrentName = sk_GENERAL_NAME_value(pSANNames, i);
        if (pCurrentName->type == GEN_DNS)
        {
            size_t length = ASN1_STRING_to_UTF8(
                                    (unsigned char **)&pszSANName,
                                    (ASN1_IA5STRING*)pCurrentName->d.ptr);

            if (!pszSANName || length != strlen(pszSANName))
            {
                dwError = VMCA_CERT_DECODE_FAILURE;
                BAIL_ON_VMCA_ERROR(dwError);
            }

            if (IsWildcardString(pszSANName, length))
            {
                dwError = VMCA_ERROR_INVALID_SAN;
                BAIL_ON_ERROR(dwError);
            }

            if (pszSANName)
            {
                OPENSSL_free(pszSANName);
                pszSANName = NULL;
            }
            ++dwDNSCount;
        }
    }

    if (dwDNSCount > 1 &&
        !VMCAConfigIsServerOptionEnabled(VMCA_SERVER_OPT_ALLOW_MULTIPLE_SAN))
    {
        dwError = VMCA_ERROR_INVALID_SAN;
        BAIL_ON_ERROR(dwError);
    }

cleanup:
    if (pszSANName)
    {
        OPENSSL_free(pszSANName);
    }

    if (pSANNames)
    {
        sk_GENERAL_NAME_pop_free(pSANNames, GENERAL_NAME_free);
    }

    return dwError;

error:
    goto cleanup;
}



DWORD
VMCASignedRequestPrivate(
    PVMCA_X509_CA pCA,
    PSTR pszPKCS10Request,
    PSTR *ppszCertificate,
    time_t tmNotBefore,
    time_t tmNotAfter
)
// VMCASignedRequestPrivate takes and CSR and signs the request
//
//Arguments :
//      pCA : The CA class that can sign the request
//      pszPKCS19Request : The Request that needs to be signed
//      ppszCertificate : Points to a PEM encoded Signed Cert
//      tmNotBefore : A Valid Time String that indicates when the Certificate is Valid From
//      tmNotAfter : The End of certificates validity
// Returns :
//  Error Code
{

    DWORD dwError = 0;
    X509_REQ *pRequest = NULL;
    EVP_PKEY *pPublicKey = NULL;
    X509 *pCertificate = NULL;
    X509_NAME *pSubjName = NULL;
    X509_NAME *pCAName = NULL;
    PSTR pszStartTime = NULL;
    PSTR pszEndTime = NULL;
    PSTR pTempCertString = NULL;
    PSTR pTempCertChainString = NULL;
    const EVP_MD *digest = EVP_sha256();
    ASN1_INTEGER *aiSerial = NULL;
    time_t tmNow = 0;

    if  ( (pCA == NULL) ||
          ( pszPKCS10Request == NULL ) ||
          (ppszCertificate == NULL)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAPEMToCSR(pszPKCS10Request, &pRequest);
    BAIL_ON_ERROR(dwError);

    if ((pPublicKey = X509_REQ_get_pubkey(pRequest)) == NULL )
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: CSR does not have a public key");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    //
    // Verify the Public Key is good and the signature is
    // indeed for that key.
    //
    if (pPublicKey->type != EVP_PKEY_RSA ||
        BN_num_bits(pPublicKey->pkey.rsa->n) < VMCA_MIN_CERT_PRIV_KEY_LENGTH)
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Key length not supported");
        dwError = VMCA_ERROR_INVALID_KEY_LENGTH;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    time(&tmNow);
    if (tmNotBefore < (tmNow - VMCA_VALIDITY_SYNC_BACK_DATE))
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Invalid start date");
        dwError = VMCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if ((tmNotAfter - tmNotBefore) > VMCA_MAX_CERT_DURATION)      // 10. year
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Invalid validity period requested");
        dwError = VMCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = X509_REQ_verify(pRequest, pPublicKey);
    BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);

    pSubjName = X509_REQ_get_subject_name(pRequest);
    if( pSubjName == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if ( X509_NAME_entry_count(pSubjName) == 0 )
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pCertificate = X509_new();
    if(pCertificate == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // VMCA Supports only X509V3 only
    dwError = X509_set_version(pCertificate, 2);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    aiSerial = ASN1_INTEGER_new();
    if (aiSerial == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCAGenerateX509Serial(aiSerial);
    X509_set_serialNumber(pCertificate,aiSerial);

    dwError = X509_set_subject_name(pCertificate, pSubjName);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    pCAName = X509_get_subject_name(pCA->pCertificate);
    if ( pCAName == NULL) {
        dwError = VMCA_CERT_IO_FAILURE;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = X509_set_issuer_name(pCertificate, pCAName);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    dwError = X509_set_pubkey(pCertificate, pPublicKey);
    BAIL_ON_SSL_ERROR(dwError, VMCA_CERT_IO_FAILURE);

    if (X509_cmp_time(X509_get_notBefore(pCA->pCertificate), &tmNotBefore) >= 0)
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Invalid validity period requested");
        dwError = VMCA_SSL_SET_START_TIME;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!ASN1_TIME_set(X509_get_notBefore(pCertificate), tmNotBefore)){
        dwError = 0;
        BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_START_TIME);
    }

    // If the request is beyond CA cert validity use CA cert validity
    if (X509_cmp_time(X509_get_notAfter(pCA->pCertificate), &tmNotAfter) <= 0)
    {
        VMCA_LOG_INFO("VMCASignedRequestPrivate: Using CA certs not after field");
        if(!ASN1_TIME_set_string(X509_get_notAfter(pCertificate),
                                 X509_get_notAfter(pCA->pCertificate)->data))
        {
            dwError = 0;
            BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_END_TIME);
        }
    }
    else
    {
        if(!ASN1_TIME_set(X509_get_notAfter(pCertificate), tmNotAfter))
        {
            dwError = 0;
            BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SET_END_TIME);
        }
    }

    dwError = VMCACopyExtensions(pCertificate, pCA->pCertificate, pRequest);
    BAIL_ON_ERROR(dwError);

    if (X509_check_ca(pCertificate))
    {
        VMCA_LOG_INFO("Request for a CA certificate is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (((pCertificate->ex_flags & EXFLAG_KUSAGE) &&
         (pCertificate->ex_kusage & KU_KEY_CERT_SIGN)))
    {
        VMCA_LOG_INFO("Request for a certificate signing cert is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (((pCertificate->ex_flags & EXFLAG_KUSAGE) &&
         (pCertificate->ex_kusage & KU_CRL_SIGN)))
    {
        VMCA_LOG_INFO("Request for a CRL signing cert is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (((pCertificate->ex_flags & EXFLAG_KUSAGE) &&
         (pCertificate->ex_kusage & KU_DATA_ENCIPHERMENT)))
    {
        VMCA_LOG_INFO("Request for a cert with data encryption key usage is not allowed");
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAVerifyCertificateName(pCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAVerifySubjectAltNames(pCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = X509_sign (pCertificate, pCA->pKey, digest);
    BAIL_ON_SSL_ERROR(dwError, VMCA_SSL_SIGN_FAIL);

    dwError = VMCACertToPEM(pCertificate, &pTempCertString);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!VMCAIsSelfSignedCert(pCA->pCertificate))
    {
        dwError = VMCAAllocateStringPrintfA(
                                            &pTempCertChainString,
                                            "%s\n%s",
                                            pTempCertString,
                                            pCA->pszCertificate
                                           );
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppszCertificate = pTempCertChainString?pTempCertChainString:pTempCertString;
cleanup:

    if(pPublicKey != NULL) {
        EVP_PKEY_free(pPublicKey);
    }

    if (pRequest != NULL) {
        X509_REQ_free(pRequest);
    }

    if( pszStartTime != NULL) {
        VMCAFreeStringA(pszStartTime);
    }

    if(pszEndTime != NULL) {
        VMCAFreeStringA(pszEndTime);
    }

    if(pCertificate != NULL) {
        X509_free(pCertificate);
    }
    if(aiSerial != NULL){
        ASN1_INTEGER_free(aiSerial);
    }
    if (pTempCertChainString)
    {
        VMCA_SAFE_FREE_STRINGA (pTempCertString);
    }

    return dwError;
error :
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }
    if(pTempCertString != NULL){
        VMCAFreeStringA(pTempCertString);
        pTempCertString = NULL;
    }
    VMCA_SAFE_FREE_MEMORY (pTempCertChainString);
    goto cleanup;
}

DWORD
VMCAWIntegerToASN1Integer (
                            PWSTR pwszInteger,
                            ASN1_INTEGER *asnInteger
                          )
{
    DWORD dwError = 0;
    DWORD dwBytesWritten = 0;
    PSTR pszInteger = NULL;
    BIGNUM *bn = NULL;

    if (IsNullOrEmptyString (pwszInteger))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VMCAAllocateStringAFromW(
                                        pwszInteger,
                                        &pszInteger
                                      );
    BAIL_ON_VMCA_ERROR (dwError);

    dwBytesWritten = BN_hex2bn(&bn,pszInteger);

    if (!dwBytesWritten)
    {
        dwError = VMCA_CRL_SET_SERIAL_FAIL;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    BN_to_ASN1_INTEGER (bn,asnInteger);

cleanup:

   VMCA_SAFE_FREE_STRINGA (pszInteger);

   if (bn)
   {
      BN_free (bn);
   }

   return dwError;

error:
   goto cleanup;
}

BOOL
VMCAIsSelfSignedCert(
                     X509* pCertificate
                    )
{
    BOOL bIsSelfSigned = FALSE;
    DWORD dwError = 0;

    if (!pCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    if (X509_check_issued(pCertificate, pCertificate) == X509_V_OK)
    {
        bIsSelfSigned = TRUE;
    }

error:
    if (dwError)
    {
        bIsSelfSigned = FALSE;
    }
    return bIsSelfSigned;
}

DWORD
VMCAValidateAndFormatCert(
    PCSTR pszCertificate,
    PSTR *ppszPEMCertificate
    )
{
    DWORD dwError = 0;
    PSTR pszPEMCertificate = NULL;
    STACK_OF(X509) *x509Certs = NULL;

    if (IsNullOrEmptyString(pszCertificate) ||
        !ppszPEMCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VMCAPEMToX509Stack(
                              pszCertificate,
                              &x509Certs
                              );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCAVerifyCertificateChain(
                             x509Certs
                             );
    BAIL_ON_VMCA_ERROR (dwError);

    dwError = VMCACertStackToPEM(
                              x509Certs,
                              &pszPEMCertificate
                              );
    BAIL_ON_VMCA_ERROR (dwError);

    *ppszPEMCertificate = pszPEMCertificate;

cleanup:
    if (x509Certs)
    {
        sk_X509_pop_free(x509Certs, X509_free);
    }

    return dwError;

error:
    if (ppszPEMCertificate)
    {
        *ppszPEMCertificate = NULL;
    }

    VMCA_SAFE_FREE_MEMORY (pszPEMCertificate);

    goto cleanup;
}

DWORD
VMCAVerifyCertificateChain(
          STACK_OF(X509) *skX509Certs
          )
{
    DWORD dwError = 0;

    X509 *pCurrCertificate = NULL;
    X509 *pPrevCertificate = NULL;
    DWORD dwIndex = 0;
    DWORD dwNumCerts = 0;


    dwNumCerts = sk_X509_num(skX509Certs);

    if (!dwNumCerts)
    {
        dwError = VMCA_ERROR_INVALID_CHAIN;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    for (;dwIndex < dwNumCerts; dwIndex++)
    {
        pCurrCertificate = sk_X509_value(skX509Certs, dwIndex);

        if (!pCurrCertificate)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMCA_ERROR (dwError);
        }

        if (
            pPrevCertificate &&
            (X509_check_issued(pCurrCertificate, pPrevCertificate) != X509_V_OK)
           )
        {
            dwError = VMCA_ERROR_INVALID_CHAIN;
            BAIL_ON_VMCA_ERROR (dwError);
        }

        pPrevCertificate = pCurrCertificate;

    }

    if (!VMCAIsSelfSignedCert(pCurrCertificate))
    {
        dwError = VMCA_ERROR_INCOMPLETE_CHAIN;
        BAIL_ON_VMCA_ERROR (dwError);
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VMCAParseCertChain(
    PSTR pszCertChainString,
    PSTR **pppszCertChainArray,
    PDWORD pdwCount,
    STACK_OF(X509) **pskX509certs //optional
    )
{
    DWORD dwError = 0;
    PSTR *ppszCertChainArray = NULL;
    DWORD dwCount = 0;
    STACK_OF(X509) *x509Certs = NULL;
    DWORD dwIndex = 0;
    X509 *pCert = NULL;

    if (IsNullOrEmptyString(pszCertChainString) ||
        !pppszCertChainArray ||
        !pdwCount
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR (dwError);
    }

    dwError = VMCAPEMToX509Stack(
                                 pszCertChainString,
                                 &x509Certs
                                );
    BAIL_ON_VMCA_ERROR (dwError);

    dwCount = sk_X509_num(x509Certs);

    if (dwCount)
    {
        dwError = VMCAAllocateMemory(
                                     sizeof(PSTR)*dwCount,
                                     (PVOID *)&ppszCertChainArray
                                    );
        BAIL_ON_VMCA_ERROR (dwError);

        for (; dwIndex<dwCount; dwIndex++)
        {
            pCert = sk_X509_value(x509Certs, dwIndex);

            if (!pCert)
            {
                dwError = ERROR_INVALID_STATE;
                BAIL_ON_VMCA_ERROR(dwError);
            }

            dwError = VMCACertToPEM(
                                    pCert,
                                    &ppszCertChainArray[dwIndex]
                                   );
            BAIL_ON_VMCA_ERROR (dwError);
        }
    }

    if (pskX509certs)
    {
        *pskX509certs = x509Certs;
        x509Certs = NULL;
    }

    *pdwCount = dwCount;
    *pppszCertChainArray = ppszCertChainArray;

cleanup:

    if(x509Certs)
    {
        sk_X509_pop_free(x509Certs, X509_free);
    }
    return dwError;

error:
    if (pppszCertChainArray)
    {
        *pppszCertChainArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (ppszCertChainArray)
    {
        VMCAFreeStringArrayA(ppszCertChainArray, dwCount);
    }

    goto cleanup;
}

DWORD
VMCAGetCSRFromCert(
    X509 *pCertificate,
    EVP_PKEY *pKey,
    X509_REQ  **ppREQ
    )
{
    DWORD dwError = 0;
    EVP_MD const *digest = EVP_sha256();
    X509_REQ *pX509Req = NULL;
    STACK_OF(X509_EXTENSION) *pExtensions = NULL;
    int subAltNameLocation = 0;
    int keyUsageLocation = 0;
    DWORD isCACert = 0;

    if (!pCertificate ||
        !pKey ||
        !ppREQ
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pX509Req = X509_to_X509_REQ(pCertificate,pKey,digest);

    if (!pX509Req)
    {
        dwError = VMCA_ERROR_CANNOT_FORM_REQUEST;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = X509_REQ_set_pubkey(pX509Req, pKey);
    BAIL_ON_SSL_ERROR (dwError, VMCA_SSL_SET_PUBKEY_ERR);

    /* NOTE: This method only adds the following extensions from the certificate.
     * 1.Subject Alternative Name
     * 2.Key Usage
     */

    pExtensions = sk_X509_EXTENSION_new_null();
    if( pExtensions == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    subAltNameLocation = X509_get_ext_by_NID(pCertificate,NID_subject_alt_name,-1);

    if (subAltNameLocation != -1)
    {
        sk_X509_EXTENSION_push(pExtensions, X509_get_ext(pCertificate,subAltNameLocation));
    }

    keyUsageLocation = X509_get_ext_by_NID(pCertificate, NID_key_usage,-1);

    if (keyUsageLocation != -1)
    {
        sk_X509_EXTENSION_push(pExtensions, X509_get_ext(pCertificate,keyUsageLocation));
    }

    isCACert = X509_check_ca(pCertificate);

    if (isCACert == 1)
    {
        dwError = VMCAAddExtension(pExtensions,
                            NID_basic_constraints,
                            "critical,CA:TRUE"
                            );
        BAIL_ON_ERROR(dwError);
    }

    dwError = X509_REQ_add_extensions(pX509Req, pExtensions);
    BAIL_ON_SSL_ERROR (dwError, VMCA_SSL_ADD_EXTENSION);
    pExtensions = NULL;

    dwError = X509_REQ_sign(pX509Req, pKey, digest);
    BAIL_ON_SSL_ERROR (dwError, VMCA_SSL_REQ_SIGN_ERR);

    *ppREQ = pX509Req;

cleanup:

    return dwError;

error:
    if (ppREQ)
    {
        *ppREQ = NULL;
    }
    if (pX509Req)
    {
        X509_REQ_free(pX509Req);
    }
    if (pExtensions)
    {
        sk_X509_EXTENSION_pop_free(pExtensions,X509_EXTENSION_free);
    }

    goto cleanup;
}


DWORD
VMCAReadX509FromFile(
    PSTR pszFileName,
    X509 ** ppCertificate
    )
{
    DWORD dwError =0;
    X509 *pCert = NULL;
    FILE *fpCert = NULL;

    if (ppCertificate == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "r", &fpCert);
    BAIL_ON_ERROR(dwError);

    pCert = PEM_read_X509(fpCert, NULL, NULL, NULL);
    if (pCert == NULL)
    {
        dwError = VMCA_CERT_DECODE_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    *ppCertificate = pCert;

cleanup:
    if (fpCert)
    {
        fclose(fpCert);
    }
    return dwError;

error:
    if (ppCertificate)
    {
        *ppCertificate = NULL;
    }
    if (pCert)
    {
        X509_free(pCert);
    }
    goto cleanup;
}

DWORD
VMCAReadPKEYFromFile(
    PSTR pszFileName,
    PSTR pszPassPhrase,
    EVP_PKEY ** ppPrivateKey
    )
{
    DWORD dwError =0;
    EVP_PKEY *pKey = NULL;
    FILE *fpKey = NULL;

    if (ppPrivateKey == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "r", &fpKey);
    BAIL_ON_ERROR(dwError);

    pKey = PEM_read_PrivateKey(fpKey, NULL, NULL, NULL);
    if (pKey == NULL)
    {
        dwError = VMCA_KEY_DECODE_FAILURE;
        BAIL_ON_ERROR(dwError);
    }

    *ppPrivateKey = pKey;

cleanup:
    if (fpKey)
    {
        fclose(fpKey);
    }
    return dwError;

error:
    if (ppPrivateKey)
    {
        *ppPrivateKey = NULL;
    }
    if (pKey)
    {
        EVP_PKEY_free(pKey);
    }

    goto cleanup;
}


static
DWORD
GetError(
    VOID
    )
{
#if defined _WIN32
        return GetLastError();
#else
        return LwErrnoToWin32Error(errno);
#endif
}

static
DWORD
VMCASetAddDCToX509Name(
     X509_NAME *pCertName,
     PSTR pszDomainName
     )
{
    DWORD dwError = 0;

    PSTR pToken = strtok(pszDomainName, ".");

    while (pToken)
    {
            if (!(strncmp(pToken, "dc=", 3)))
            {
                dwError = X509_NAME_add_entry_by_txt(pCertName,
                    "DC", MBSTRING_UTF8,
                    &pToken[3], -1, -1, 0);
                BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
            }
            else
            {
                dwError = X509_NAME_add_entry_by_txt(pCertName,
                    "DC", MBSTRING_UTF8,
                    pToken, -1, -1, 0);
                BAIL_ON_SSL_ERROR(dwError, VMCA_INVALID_CSR_FIELD);
            }
           pToken = strtok(NULL, ",");
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VMCAVerifyHostNameInCN(
    X509_REQ* pCSR,
    PCSTR szHostName
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwCNPos = -1;
    X509_NAME_ENTRY *pCNEntry = NULL;
    ASN1_STRING *pCNAsn1 = NULL;
    PSTR pszCNString = NULL;
    size_t length = 0;

    for(;;)
    {
        dwCNPos = X509_NAME_get_index_by_NID(
                        X509_REQ_get_subject_name(pCSR),
                        NID_commonName,
                        dwCNPos);
        if (dwCNPos == -1)
        {
             break;
        }

        pCNEntry = X509_NAME_get_entry(
                        X509_REQ_get_subject_name(pCSR),
                        dwCNPos);
        if (pCNEntry == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        pCNAsn1 = X509_NAME_ENTRY_get_data(pCNEntry);
        if (pCNAsn1 == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        length = ASN1_STRING_to_UTF8(
                        (unsigned char **)&pszCNString,
                        pCNAsn1);

        if (!pszCNString || length != strlen(pszCNString))
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        if (!IsNullOrEmptyString(szHostName))
        {
            if (VMCAStringCompareA(
                        pszCNString,
                        szHostName,
                        FALSE))
            {
                dwError = VMCA_ERROR_CN_HOSTNAME_MISMATCH;
                BAIL_ON_VMCA_ERROR(dwError);
            }
        }
        else
        {
            dwError = VMCA_ERROR_CN_HOSTNAME_MISMATCH;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        if (pszCNString)
        {
             OPENSSL_free(pszCNString);
             pszCNString = NULL;
        }
    }

cleanup:
    if (pszCNString)
    {
         OPENSSL_free(pszCNString);
    }

    return dwError;

error:
    goto cleanup;
}


DWORD
VMCAVerifyHostNameInSAN(
    X509_REQ* pCSR,
    PCSTR szHostName,
    PBOOLEAN pIsSANPresent
    )
{
    DWORD dwError = ERROR_SUCCESS;
    GENERAL_NAMES *pSANNames = NULL;
    STACK_OF(X509_EXTENSION) *pExts = NULL;
    PSTR pszSANName = NULL;
    DWORD i, dwCountSanNames = 0;

    pExts = X509_REQ_get_extensions(pCSR);
    if (pExts == NULL)
    {
        dwError = ERROR_SUCCESS;
        goto error;
    }

    pSANNames = X509V3_get_d2i(pExts, NID_subject_alt_name, NULL, NULL);
    if (pSANNames == NULL)
    {
        dwError = ERROR_SUCCESS;
        goto error;
    }

    dwCountSanNames = sk_GENERAL_NAME_num(pSANNames);

    for (i = 0; i < dwCountSanNames; i++)
    {
        const GENERAL_NAME *pCurrentName = sk_GENERAL_NAME_value(pSANNames, i);

        if (pCurrentName->type == GEN_DNS)
        {
            size_t length = ASN1_STRING_to_UTF8(
                                    (unsigned char **)&pszSANName,
                                    (ASN1_IA5STRING*)pCurrentName->d.dNSName);

            if (!pszSANName || length != strlen(pszSANName))
            {
                dwError = VMCA_CERT_DECODE_FAILURE;
                BAIL_ON_VMCA_ERROR(dwError);
            }

            if (!IsNullOrEmptyString(szHostName))
            {
                if (VMCAStringCompareA(
                                pszSANName,
                                szHostName,
                                FALSE))
                {
                    dwError = VMCA_ERROR_SAN_HOSTNAME_MISMATCH;
                    BAIL_ON_ERROR(dwError);
                }
            }
            else
            {
                dwError = VMCA_ERROR_SAN_HOSTNAME_MISMATCH;
                BAIL_ON_ERROR(dwError);
            }

            if (pszSANName)
            {
                OPENSSL_free(pszSANName);
                pszSANName = NULL;
            }
        }
    }

cleanup:

    if (pIsSANPresent)
    {
        *pIsSANPresent = (pSANNames != NULL);
    }

    if (pszSANName)
    {
        OPENSSL_free(pszSANName);
    }

    if (pSANNames)
    {
        sk_GENERAL_NAME_pop_free(pSANNames, GENERAL_NAME_free);
    }

    if (pExts)
    {
        sk_X509_EXTENSION_pop_free(pExts, X509_EXTENSION_free);
    }

    return dwError;

error:
    goto cleanup;
}


DWORD
VMCAVerifyIpAddressInSAN(
    X509_REQ* pCSR,
    ASN1_OCTET_STRING* pAnsHostIp
    )
{
    DWORD dwError = ERROR_SUCCESS;
    GENERAL_NAMES *pSANNames = NULL;
    STACK_OF(X509_EXTENSION) *pExts = NULL;
    DWORD i, dwCountSanNames = 0;

    pExts = X509_REQ_get_extensions(pCSR);
    if (pExts == NULL)
    {
        dwError = ERROR_SUCCESS;
        goto error;
    }

    pSANNames = X509V3_get_d2i(pExts, NID_subject_alt_name, NULL, NULL);
    if (pSANNames == NULL)
    {
        dwError = ERROR_SUCCESS;
        goto error;
    }

    dwCountSanNames = sk_GENERAL_NAME_num(pSANNames);

    for (i = 0; i < dwCountSanNames; i++)
    {
        const GENERAL_NAME *pCurrentName = sk_GENERAL_NAME_value(pSANNames, i);
        if (pCurrentName->type == GEN_IPADD)
        {
            if (pAnsHostIp)
            {
                if (pAnsHostIp->length != pCurrentName->d.ip->length)
                {
                    dwError = VMCA_ERROR_SAN_IPADDR_INVALID;
                    BAIL_ON_ERROR(dwError);
                }

                if (memcmp(
                       pCurrentName->d.iPAddress->data,
                       pAnsHostIp->data,
                       pAnsHostIp->length))
                {
                    dwError = VMCA_ERROR_SAN_IPADDR_INVALID;
                    BAIL_ON_ERROR(dwError);
                }
            }
            else
            {
                dwError = VMCA_ERROR_SAN_IPADDR_INVALID;
                BAIL_ON_ERROR(dwError);
            }
        }
    }

cleanup:

    if (pSANNames)
    {
        sk_GENERAL_NAME_pop_free(pSANNames, GENERAL_NAME_free);
    }

    if (pExts)
    {
        sk_X509_EXTENSION_pop_free(pExts, X509_EXTENSION_free);
    }


    return dwError;

error:
    goto cleanup;
}

DWORD
VMCAVerifyHostName(
    PCSTR pszHostName,
    PCSTR pszHostIp,
    PCSTR pszCSR
    )
{
    DWORD dwError = ERROR_SUCCESS;
    X509_REQ *pRequest = NULL;
    ASN1_OCTET_STRING* pAsnHostNameIp = NULL;
    ASN1_OCTET_STRING* pAnsHostIp = NULL;
    BOOLEAN bIsSANPresent = FALSE;

    if (IsNullOrEmptyString(pszCSR))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAPEMToCSR(pszCSR, &pRequest);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAVerifyHostNameInSAN(pRequest, pszHostName, &bIsSANPresent);
    BAIL_ON_ERROR(dwError);

    if (bIsSANPresent)
    {
        pAsnHostNameIp = a2i_IPADDRESS(pszHostName);
        if (pAsnHostNameIp)
        {
            dwError = VMCAVerifyIpAddressInSAN(pRequest, pAsnHostNameIp);
            BAIL_ON_ERROR(dwError);
        }

        if (!IsNullOrEmptyString(pszHostIp))
        {
            pAnsHostIp = a2i_IPADDRESS(pszHostIp);

            if (!pszHostIp)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_ERROR(dwError);
            }

            dwError = VMCAVerifyIpAddressInSAN(pRequest, pAnsHostIp);
            BAIL_ON_ERROR(dwError);
        }

        if (!pAsnHostNameIp && IsNullOrEmptyString(pszHostIp))
        {
            dwError = VMCAVerifyIpAddressInSAN(pRequest, NULL);
            BAIL_ON_ERROR(dwError);
        }
    }
    else
    {
        // If SAN is missing from CSR check CN for hostnane
        dwError = VMCAVerifyHostNameInCN(pRequest, pszHostName);
        BAIL_ON_ERROR(dwError);
    }

cleanup:

    if (pAsnHostNameIp)
    {
        ASN1_OCTET_STRING_free(pAsnHostNameIp);
    }

    if (pAnsHostIp)
    {
        ASN1_OCTET_STRING_free(pAnsHostIp);
    }

    if (pRequest)
    {
        X509_REQ_free(pRequest);
    }

    return dwError;

error:
    goto cleanup;
}

