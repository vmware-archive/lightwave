/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.identity.saml;
/**
 * Digest method for encrypting each reference.
 */
public enum DigestMethod {
   /**
    * RSA SHA1 algorithm for encryption
    */
   RSA_SHA1("http://www.w3.org/2000/09/xmldsig#sha1"),
   /**
    * RSA SHA256 algorithm for encryption
    */
   RSA_SHA256("http://www.w3.org/2001/04/xmlenc#sha256"),
   /**
    * RSA SHA512 algorithm for encryption
    */
   RSA_SHA512("http://www.w3.org/2001/04/xmlenc#sha512");

   private final String digestMethodURI;

   private DigestMethod(String digestMethodURI) {
      assert digestMethodURI != null;

      this.digestMethodURI = digestMethodURI;
   }

   @Override
   public String toString() {
      return digestMethodURI;
   }
}