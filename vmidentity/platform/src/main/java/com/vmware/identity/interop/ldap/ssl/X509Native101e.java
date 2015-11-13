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
 * X509Native object mapping for OpenSSL 0.9.8r library used by lotus-2013 branch.
 *
 */

/*
 *
struct x509_st
    {
    X509_CINF *cert_info;
    X509_ALGOR *sig_alg;
    ASN1_BIT_STRING *signature;
    int valid;
    int references;
    char *name;
    CRYPTO_EX_DATA ex_data;
    //These contain copies of various extension values
    long ex_pathlen;
    long ex_pcpathlen;
    unsigned long ex_flags;
    unsigned long ex_kusage;
    unsigned long ex_xkusage;
    unsigned long ex_nscert;
    ASN1_OCTET_STRING *skid;
    AUTHORITY_KEYID *akid;
    X509_POLICY_CACHE *policy_cache;
    STACK_OF(DIST_POINT) *crldp;       //OpenSSL 1.0.1e
    STACK_OF(GENERAL_NAME) *altname;   //OpenSSL 1.0.1e
    NAME_CONSTRAINTS *nc;              //OpenSSL 1.0.1e
#ifndef OPENSSL_NO_RFC3779
    STACK_OF(IPAddressFamily) *rfc3779_addr;
    struct ASIdentifiers_st *rfc3779_asid;
#endif
#ifndef OPENSSL_NO_SHA
    unsigned char sha1_hash[SHA_DIGEST_LENGTH];
#endif
    X509_CERT_AUX *aux;
    }; // X509;
*/

public class X509Native101e extends Structure
{

   public Pointer cert_info;  /**{@link X509CINFNative}*/
   //Note: Omit the remaining mappings since they are not needed for the implementation.

   public X509Native101e(Pointer p)
   {
      super();
      useMemory(p);
      read();
   }

   @Override
   protected List<String> getFieldOrder()
   {
      return Arrays.asList("cert_info");
   }
}
