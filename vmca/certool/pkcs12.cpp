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
#include <vmca.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/x509.h>
#include <vmca_error.h>
#include <vmcacommon.h>
#include <macros.h>

#if _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif // _MSC_VER

extern "C"
DWORD
VMCAWritePKCS12(
    PSTR pszFileName,
    PSTR pszFriendlyName,
    PSTR pszPassword,
    PSTR pszCertFile,
    PSTR pszPrivateKeyFile,
    PSTR *ppCACerts,
    int uCACertCount)
{
    FILE *fp = NULL;
    EVP_PKEY *pkey = NULL;
    X509 *cert = NULL;
    PKCS12 *p12 = NULL;
    DWORD dwError = 0;
    STACK_OF(X509) *ca = NULL;
    BIO* BioCert = NULL;
    X509* caCert = NULL;
    int x =0;

    if (pszPassword == NULL)
    {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    SSLeay_add_all_algorithms();
    ERR_load_crypto_strings();

    dwError = VMCAOpenFilePath(pszCertFile, "r", &fp);
    BAIL_ON_ERROR(dwError);

    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    if(cert == NULL ) {
        ERR_print_errors_fp(stderr);
        dwError = VMCA_CERT_IO_FAILURE;
        BAIL_ON_ERROR(dwError);
    }
    fclose(fp);

    dwError = VMCAOpenFilePath(pszPrivateKeyFile, "r", &fp);
    BAIL_ON_ERROR(dwError);

    pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    if (pkey == NULL) {
        ERR_print_errors_fp(stderr);
        dwError = VMCA_KEY_IO_FAILURE;
        BAIL_ON_ERROR(dwError);
    }
    fclose(fp);

    ca = sk_X509_new_null();
    if (ca == NULL) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

    while (x < uCACertCount ) {
        if ( ppCACerts[x] != NULL) {
             BioCert = BIO_new_mem_buf(ppCACerts[x], -1);
             BIO_set_close(BioCert, BIO_NOCLOSE);
             PEM_read_bio_X509(BioCert, &caCert, 0, NULL);
             sk_X509_push(ca, caCert);
             BIO_free(BioCert);
        }
    x++;
    }


    p12 = PKCS12_create(pszPassword, pszFriendlyName, pkey, cert, ca, 0,0,0,0,0);
    if(!p12) {
        fprintf(stderr, "Error creating PKCS#12 structure\n");
        ERR_print_errors_fp(stderr);
        dwError = VMCA_PKCS12_CREAT_FAIL;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAOpenFilePath(pszFileName, "wb", &fp);
    BAIL_ON_ERROR(dwError);

    i2d_PKCS12_fp(fp, p12);

 error :
    if (p12) {
        PKCS12_free(p12);
    }

    if (fp) {
        fclose(fp);
    }

    if(ca) {
        sk_X509_free(ca);
    }
    return dwError;
}

