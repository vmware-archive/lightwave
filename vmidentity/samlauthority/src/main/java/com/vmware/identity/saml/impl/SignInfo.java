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

package com.vmware.identity.saml.impl;

import java.security.PrivateKey;
import java.security.Provider;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

/**
 * Signature info
 */
public final class SignInfo {
   private PrivateKey privateKey;
   private CertPath certificationPath;
   private Provider securityProvider;

   /**
    * @param privateKey
    *           The private key used to sign tokens. This key should pair with
    *           the certificate parameter. Cannot be null.
    * @param certificationPath
    *           The certificates that will be embedded into the SAML token and
    *           should be used for signing assertions. Should match the private
    *           key. It contains multiple certificates, only the leaf is used
    *           for signing if it is valid(means not expired). This
    *           certification chain contains all the certificates from the leaf
    *           until the root certificate, exclusive. Cannot be null.
    * @param securityProvider
    *           The security provider used for signing the tokens. If this
    *           parameter is null the default provider will be used.
    */
   public SignInfo(PrivateKey privateKey, CertPath certificationPath,
      Provider securityProvider) {
      assert privateKey != null;
      assert certificationPath != null;
      assert certificationPath.getCertificates().size() > 0;

      this.privateKey = privateKey;
      this.certificationPath = certificationPath;
      this.securityProvider = securityProvider;
   }

   /**
    * @return private key part for signing the signature. Cannot be null.
    */
   public PrivateKey getPrivateKey() {
      return privateKey;
   }

   /**
    * @return certificate chain of STS. Cannot be null.
    */
   public CertPath getCertificationPath() {
      return certificationPath;
   }

   /**
    * @return security provider part of signature. Cannot be null.
    */
   public Provider getSecurityProvider() {
      return securityProvider;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + certificationPath.hashCode();
      result = prime * result + privateKey.hashCode();
      result = prime * result
         + ((securityProvider == null) ? 0 : securityProvider.hashCode());
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || this.getClass() != obj.getClass()) {
         return false;
      }

      SignInfo other = (SignInfo) obj; // TODO check for default provider
      boolean equalsWhenNoDefaultProvider = certificationPath
         .equals(other.certificationPath)
         && privateKey.equals(other.privateKey);
      return (securityProvider == null || other.securityProvider == null) ? equalsWhenNoDefaultProvider
         : equalsWhenNoDefaultProvider
            && securityProvider.equals(other.securityProvider);
   }

   /**
    * @return the signing certificate (first certificate) in chain.
    *
    */
   public X509Certificate getSigningCertificate() {
      Certificate firstCertificateInCertChain = getCertificationPath()
         .getCertificates().get(0);
      assert firstCertificateInCertChain != null;

      return (X509Certificate) firstCertificateInCertChain;
   }

}