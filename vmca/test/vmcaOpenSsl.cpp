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
// This file impliments certificate manipulation based on
// OpenSSL. This is part of tests for VMCA.
// These functions cross checks the functionality with VMCA.
// Since the whole world uses OpenSSL making sure that OpenSSL
// Can read and write the certificate using OpenSSL is a good idea
// author : aengineer@vmware.com



#include <stdio.h>
#include <stdlib.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <openssl/pem.h>

#include <iostream>
#include <string>


typedef  unsigned int DWORD;
typedef char * PSTR;

static void callback(int p, int n, void *arg);
int mkreq(X509_REQ **req, EVP_PKEY **pkeyp, int bits, int serial, int days);
int add_ext(STACK_OF(X509_EXTENSION) *sk, int nid, char *value);


X509 *
SSLOpenCert(std::string FileName)
{

 FILE *certFile = fopen(FileName.c_str(), "r");
 X509 *cert = NULL;
 if (certFile != NULL) {
    return PEM_read_X509(certFile, NULL, NULL, NULL);
  }
  return NULL;
}

DWORD
SSLGetSubjectNameFromCertificate(std::string CertFileName, PSTR* ppSubjectName)
{
    DWORD dwError = 0;
    X509_NAME *subject = NULL;
    X509 *cert = SSLOpenCert(CertFileName);

    if( cert == NULL) {
        dwError = -1;
        goto error ;
    }

    subject = X509_get_subject_name(cert);
    *ppSubjectName = X509_NAME_oneline(subject,NULL,0);
    std::cout << " Name : " << *ppSubjectName << std::endl;

error:
    if( cert != NULL) {
        X509_free(cert);
    }
    return dwError;
}

DWORD
SSLCreateCSR(PSTR* CSRdata)
{

    BIO *bio_err;
    X509_REQ *req=NULL;
    EVP_PKEY *pkey=NULL;
    BIO *ReqBio = NULL;

    CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
    bio_err=BIO_new_fp(stderr, BIO_NOCLOSE);
    mkreq(&req,&pkey,512,0,365);
    ReqBio = BIO_new(BIO_s_mem());
//    RSA_print_fp(stdout,pkey->pkey.rsa,0);
    PEM_write_bio_X509_REQ_NEW(ReqBio, req);
    char *dataStart = NULL;
    char *subjectString = NULL;
    long nameLength = BIO_get_mem_data(ReqBio,&dataStart);
    subjectString = new char[nameLength + 1];
    memset(subjectString,0, nameLength + 1);
    memcpy(subjectString, dataStart, nameLength);
    *CSRdata = subjectString;

    X509_REQ_free(req);
    EVP_PKEY_free(pkey);

#ifndef OPENSSL_NO_ENGINE
    ENGINE_cleanup();
#endif
    CRYPTO_cleanup_all_ex_data();

    CRYPTO_mem_leaks(bio_err);
    BIO_free(bio_err);
    return(0);

}



//Copied from OpenSSL Demo Sources

static void callback(int p, int n, void *arg)
    {
    char c='B';

    if (p == 0) c='.';
    if (p == 1) c='+';
    if (p == 2) c='*';
    if (p == 3) c='\n';
    fputc(c,stderr);
    }

int mkreq(X509_REQ **req, EVP_PKEY **pkeyp, int bits, int serial, int days)
    {
    X509_REQ *x;
    EVP_PKEY *pk;
    RSA *rsa;
    X509_NAME *name=NULL;
    STACK_OF(X509_EXTENSION) *exts = NULL;

    if ((pk=EVP_PKEY_new()) == NULL)
        goto err;

    if ((x=X509_REQ_new()) == NULL)
        goto err;

    rsa=RSA_generate_key(bits,RSA_F4,callback,NULL);
    if (!EVP_PKEY_assign_RSA(pk,rsa))
        goto err;

    rsa=NULL;

    X509_REQ_set_pubkey(x,pk);

    name=X509_REQ_get_subject_name(x);

    /* This function creates and adds the entry, working out the
     * correct string type and performing checks on its length.
     * Normally we'd check the return value for errors...
     */
    X509_NAME_add_entry_by_txt(name,"C",
                MBSTRING_ASC, (const unsigned char*) "UK", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name,"CN",
                MBSTRING_ASC, (const unsigned char*) "OpenSSL Group", -1, -1, 0);

#ifdef REQUEST_EXTENSIONS
    /* Certificate requests can contain extensions, which can be used
     * to indicate the extensions the requestor would like added to
     * their certificate. CAs might ignore them however or even choke
     * if they are present.
     */

    /* For request extensions they are all packed in a single attribute.
     * We save them in a STACK and add them all at once later...
     */

    exts = sk_X509_EXTENSION_new_null();
    /* Standard extenions */

    add_ext(exts, NID_key_usage, "critical,digitalSignature,keyEncipherment");

    /* This is a typical use for request extensions: requesting a value for
     * subject alternative name.
     */

    add_ext(exts, NID_subject_alt_name, "email:steve@openssl.org");

    /* Some Netscape specific extensions */
    add_ext(exts, NID_netscape_cert_type, "client,email");



#ifdef CUSTOM_EXT
    /* Maybe even add our own extension based on existing */
    {
        int nid;
        nid = OBJ_create("1.2.3.4", "MyAlias", "My Test Alias Extension");
        X509V3_EXT_add_alias(nid, NID_netscape_comment);
        add_ext(x, nid, "example comment alias");
    }
#endif

    /* Now we've created the extensions we add them to the request */

    X509_REQ_add_extensions(x, exts);

    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);

#endif

    if (!X509_REQ_sign(x,pk,EVP_sha1()))
        goto err;

    *req=x;
    *pkeyp=pk;
    return(1);
err:
    return(0);
    }

/* Add extension using V3 code: we can set the config file as NULL
 * because we wont reference any other sections.
 */

int add_ext(STACK_OF(X509_EXTENSION) *sk, int nid, char *value)
    {
    X509_EXTENSION *ex;
    ex = X509V3_EXT_conf_nid(NULL, NULL, nid, value);
    if (!ex)
        return 0;
    sk_X509_EXTENSION_push(sk, ex);

    return 1;
    }



//End of OpenSSL Demo Sources
