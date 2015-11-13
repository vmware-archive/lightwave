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

/*
 *
typedef struct x509_cinf_st
        {
        ASN1_INTEGER *version;          // [ 0 ] default of v1
        ASN1_INTEGER *serialNumber;
        X509_ALGOR *signature;
        X509_NAME *issuer;
        X509_VAL *validity;
        X509_NAME *subject;
        X509_PUBKEY *key;
        ASN1_BIT_STRING *issuerUID;             // [ 1 ] optional in v2
        ASN1_BIT_STRING *subjectUID;            // [ 2 ] optional in v2
        STACK_OF(X509_EXTENSION) *extensions;   // [ 3 ] optional in v3
        ASN1_ENCODING enc;
        } X509_CINF;

 */

public class X509CINFNative extends Structure {

   public Pointer version;
   public Pointer serialNumber;  /**{@link ASN1_String}*/
   public Pointer signature;
   public Pointer issuer;
   public Pointer validity;   /**{@link X509_VAL}*/
   public Pointer subject;
   public Pointer key;
   //Note: Omit the remaining mappings since they are not needed for the implementation.

   public X509CINFNative(Pointer p)
   {
      super();
      useMemory(p);
      read();
   }

   @Override
   protected List<String> getFieldOrder()
   {
      return Arrays.asList("version", "serialNumber", "signature", "issuer", "validity", "subject", "key");
   }
}
