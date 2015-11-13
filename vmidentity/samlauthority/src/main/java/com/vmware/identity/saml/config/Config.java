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
package com.vmware.identity.saml.config;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.Validate;

import com.vmware.identity.idm.IDPConfig;

/**
 * Class holding all the configuration needed from SAML authority to issue
 * tokens.
 */
public final class Config {

   private final SamlAuthorityConfiguration samlAuthorityConfig;
   private final TokenRestrictions tokenRestrictions;
   private final Collection<List<Certificate>> validCerts;
   private final long clockTolerance;
   private final Map<String, IDPConfig> externalIdps;

   /**
    *
    * Creates a new configuration
    *
    * @param samlAuthorityConfig
    *           Saml authority configuration properties, required
    * @param tokenRestrictions
    *           token issuance configuration properties, required
    * @param validCerts
    *           not empty array of SAML authority certificates, required. No
    *           assumptions should be made for the order of the certificate
    *           chains. However, within certificate chains, the certificates are
    *           ordered from leaf to root CA. Entire chains can be kept even if
    *           some of them is no longer valid, because when certificate is
    *           checked if it is signed with a certificate among those chains,
    *           this certificate will be no longer valid itself. required
    * @param clockTolerance
    *           Maximum allowed clock skew between token sender and consumer, in
    *           milliseconds, required.
    * @param inExternalIdps a list of external idps registered.
    * @throws IllegalArgumentException
    *            when some of the arguments are invalid
    */
   public Config(SamlAuthorityConfiguration samlAuthorityConfig,
      TokenRestrictions tokenRestrictions,
      Collection<List<Certificate>> validCerts, long clockTolerance,
      Collection<IDPConfig> inExternalIdps) {

      Validate.notNull(samlAuthorityConfig);
      Validate.notNull(tokenRestrictions);
      Validate.notEmpty(validCerts);

      List<Certificate> authorityCert = samlAuthorityConfig
         .getSigningCertificateChain();
      boolean authorityCertInValidCerts = false;

      for (List<Certificate> currentChain : validCerts) {
         Validate.notEmpty(currentChain);
         Validate.noNullElements(currentChain);
         if (!authorityCertInValidCerts && currentChain.equals(authorityCert)) {
            authorityCertInValidCerts = true;
         }
      }
      Validate.isTrue(authorityCertInValidCerts,
         "signing certificate chain is not in valid chains.");
      Validate.isTrue(clockTolerance >= 0);

      this.samlAuthorityConfig = samlAuthorityConfig;
      this.validCerts = validCerts;
      this.clockTolerance = clockTolerance;
      this.tokenRestrictions = tokenRestrictions;
      HashMap<String, IDPConfig> idpsSet = new HashMap<String, IDPConfig>();
      if(inExternalIdps != null) {
          for( IDPConfig conf : inExternalIdps )
          {
              if(conf != null)
              {
                  idpsSet.put(conf.getEntityID(), conf);
              }
          }
      }
      this.externalIdps = Collections.unmodifiableMap(idpsSet);
   }

   /**
    * @return the samlAuthorityConfig
    */
   public SamlAuthorityConfiguration getSamlAuthorityConfig() {
      return samlAuthorityConfig;
   }

   /**
    * @return the validCerts
    */
   public Collection<List<Certificate>> getValidCerts() {
      return validCerts;
   }

   /**
    * @return the clockTolerance
    */
   public long getClockTolerance() {
      return clockTolerance;
   }

   /**
    * @return the token restrictions
    */
   public TokenRestrictions getTokenRestrictions() {
      return tokenRestrictions;
   }

   /**
    * A list of external idps.
    * @return
    */
   public Map<String, IDPConfig> getExternalIdps() {
       return this.externalIdps;
   }

   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result
         + (int) (clockTolerance ^ (clockTolerance >>> 32));
      result = prime * result + tokenRestrictions.hashCode();
      result = prime * result + validCerts.hashCode();
      result = prime * result + samlAuthorityConfig.hashCode();
      result = prime * result + externalIdps.hashCode();
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || getClass() != obj.getClass()) {
         return false;
      }

      Config other = (Config) obj;
      return (clockTolerance == other.clockTolerance)
         && samlAuthorityConfig.equals(other.samlAuthorityConfig)
         && tokenRestrictions.equals(other.tokenRestrictions)
         && validCerts.equals(other.validCerts)
         && externalIdps.equals(other.externalIdps);
   }

   /**
    * Class representing a configuration properties of Saml authority.
    */
   public static final class SamlAuthorityConfiguration {
      private final String issuer;
      private final List<Certificate> signingCertificateChain;
      private final PrivateKey authorityKey;
      private final String signatureAlgorithm;

      /**
       * @param issuer
       *           not empty issuer name, required
       * @param signingCertificateChain
       *           not empty chain of SAML authority certificates, required. The
       *           chain starts from the leaf and ends to the root CA.
       * @param authorityKey
       *           SAML authority private key, required
       * @param signatureAlgorithm
       *           which should be used for signing issued tokens. Can be null,
       *           which means no such algorithm is configured.
       */
      public SamlAuthorityConfiguration(String issuer,
         List<Certificate> signingCertificateChain, PrivateKey authorityKey,
         String signatureAlgorithm) {
         Validate.notEmpty(issuer);
         Validate.notEmpty(signingCertificateChain);
         Validate.notNull(authorityKey);

         this.issuer = issuer;
         this.signingCertificateChain = signingCertificateChain;
         this.authorityKey = authorityKey;
         this.signatureAlgorithm = signatureAlgorithm;
      }

      /**
       * @return the issuer
       */
      public String getIssuer() {
         return issuer;
      }

      /**
       * @return the authorityCert
       */
      public List<Certificate> getSigningCertificateChain() {
         return signingCertificateChain;
      }

      /**
       * @return the authorityKey
       */
      public PrivateKey getAuthorityKey() {
         return authorityKey;
      }

      /**
       * @return signature algorithm used or null if there is no such
       *         configured.
       */
      public String getSignatureAlgorithm() {
         return signatureAlgorithm;
      }

      @Override
      public int hashCode() {
         final int prime = 31;
         int result = 1;
         result = prime * result + signingCertificateChain.hashCode();
         result = prime * result + authorityKey.hashCode();
         result = prime * result + issuer.hashCode();
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

         SamlAuthorityConfiguration other = (SamlAuthorityConfiguration) obj;
         return signingCertificateChain.equals(other.signingCertificateChain)
            && authorityKey.equals(other.authorityKey)
            && issuer.equals(other.issuer);
      }
   }

}
