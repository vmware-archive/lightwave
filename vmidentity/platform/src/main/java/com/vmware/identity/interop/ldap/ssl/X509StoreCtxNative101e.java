/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap.ssl;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

/**
 * X509StoreCtxNative object mapping for OpenSSL 0.9.8r library used by lotus-2013 branch.
 *
 */

/*
 *
               struct x509_store_ctx_st       //X509_STORE_CTX
               {
                   X509_STORE *ctx;
                   int current_method;     // used when looking up certs

                   // The following are set by the caller
                   X509 *cert;             // The cert to check
                   STACK_OF(X509) *untrusted;      // chain of X509s - untrusted - passed in
                   STACK_OF(X509_CRL) *crls;       // set of CRLs passed in

                   X509_VERIFY_PARAM *param;
                   void *other_ctx;        // Other info for use with get_issuer()

                   // Callbacks for various operations
                   int (*verify)(X509_STORE_CTX *ctx);     // called to verify a certificate
                   int (*verify_cb)(int ok,X509_STORE_CTX *ctx);           // error callback
                   int (*get_issuer)(X509 **issuer, X509_STORE_CTX *ctx, X509 *x); // get issuers cert from ctx
                   int (*check_issued)(X509_STORE_CTX *ctx, X509 *x, X509 *issuer); // check issued
                   int (*check_revocation)(X509_STORE_CTX *ctx); // Check revocation status of chain
                   int (*get_crl)(X509_STORE_CTX *ctx, X509_CRL **crl, X509 *x); // retrieve CRL
                   int (*check_crl)(X509_STORE_CTX *ctx, X509_CRL *crl); // Check CRL validity
                   int (*cert_crl)(X509_STORE_CTX *ctx, X509_CRL *crl, X509 *x); // Check certificate against CRL
                   int (*check_policy)(X509_STORE_CTX *ctx);
                   STACK_OF(X509) * (*lookup_certs)(X509_STORE_CTX *ctx, X509_NAME *nm);
                   STACK_OF(X509_CRL) * (*lookup_crls)(X509_STORE_CTX *ctx, X509_NAME *nm);
                   int (*cleanup)(X509_STORE_CTX *ctx);

                   // The following is built up
                   int valid;              // if 0, rebuild chain
                   int last_untrusted;     // index of last untrusted cert
                   STACK_OF(X509) *chain;          // chain of X509s - built up and trusted
                   X509_POLICY_TREE *tree; // Valid policy tree

                   int explicit_policy;    // Require explicit policy value

                   // When something goes wrong, this is why
                   int error_depth;
                   int error;
                   X509 *current_cert;
                   X509 *current_issuer;   // cert currently being tested as valid issuer
                   X509_CRL *current_crl;  // current CRL

                   int current_crl_score;  // score of current CRL
                   unsigned int current_reasons;  // Reason mask

                   X509_STORE_CTX *parent; // For CRL path validation: parent context

                   CRYPTO_EX_DATA ex_data;
               }; // X509_STORE_CTX
 *
 */
public class X509StoreCtxNative101e extends Structure
{
   public Pointer ctx;
   public int current_method;
   public Pointer cert;  /**{@link X509Native}*/
   public Pointer untrusted;
   public Pointer crls;
   public Pointer param;
   public Pointer other_ctx;

   public Pointer verify;
   public Pointer verify_cb;
   public Pointer get_issuer;
   public Pointer check_issued;
   public Pointer check_revocation;
   public Pointer get_crl;
   public Pointer check_crl;
   public Pointer cert_crl;
   public Pointer check_policy;
   public Pointer lookup_certs;   //OpenSSL 1.0.1e
   public Pointer lookup_crls;    //OpenSSL 1.0.1e
   public Pointer cleanup;

   public int valid;
   public int last_untrusted;
   public Pointer chain;
   public Pointer tree;
   public int explicit_policy;
   public int error_depth;
   public int error;
   public Pointer current_cert;
   public Pointer current_issuer;
   public Pointer current_crl;
   public int current_crl_score;  //OpenSSL 1.0.1e
   public int current_reasons;    //OpenSSL 1.0.1e
   public Pointer parent;
//   public CrypoExDataNative ex_data;   //not used

   public X509StoreCtxNative101e(Pointer p)
   {
      super();
      useMemory(p);
      read();
   }

   @Override
   protected List<String> getFieldOrder()
   {
       return Arrays.asList(
               "ctx", "current_method", "cert", "untrusted", "crls",
               "param", "other_ctx", "verify", "verify_cb", "get_issuer",
               "check_issued", "check_revocation","get_crl","check_crl",
               "cert_crl","check_policy",
               "lookup_certs","lookup_crls",
               "cleanup",
               "valid","last_untrusted","chain","tree","explicit_policy",
               "error_depth","error","current_cert","current_issuer",
               "current_crl",
               "current_crl_score","current_reasons",
               "parent"//,"ex_data"
       );
   }
}
