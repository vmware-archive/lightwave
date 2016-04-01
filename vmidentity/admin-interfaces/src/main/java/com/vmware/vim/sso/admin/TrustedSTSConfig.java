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

package com.vmware.vim.sso.admin;

import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Class holding information about trusted STS configuration used for STS token conversion.
 * <p>
 * Attributes includes:
 * <ul>
 *      <li> issuerName -- the STS's unique identifier</li>
 *      <li> certPath   -- the signing certificate chain</li>
 * </ul>
 * </p>
 * This class is immutable.
 */
public final class TrustedSTSConfig {

   private final String issuerName;
   private final CertPath certPath;
   private final AttributeConfig[] subjectFormatMappings;
   private Map<TokenClaimAttribute,List<String>> tokenClaimGroupMappings;

   /**
    * Construct the trusted IDP configuration by issuer's name and its certChain
    *
    * @param issuerName
    *           cannot be empty
    * @param certPath
    *           cannot be null, it can contains only X509Certificate.
    * @param subjectFormatMappings
    *           configuration for the token subject format mapping to store attribute.
    *           can be null or empty.
    * @param tokenClaimGroupMappings
    *           mappings of token claims to identity store group sids
    *
    */
   public TrustedSTSConfig(String issuerName, CertPath certPath,
         AttributeConfig[] subjectFormatMappings, Map<TokenClaimAttribute,List<String>> tokenClaimGroupMappings) {
      ValidateUtil.validateNotEmpty(issuerName, "issuerName");
      ValidateUtil.validateNotNull(certPath, "certPath");
      for (Certificate cert : certPath.getCertificates()) {
         if (!(cert instanceof X509Certificate)) {
            throw new IllegalArgumentException(
                  "only X509Certificates can be imported");
         }
      }
      this.issuerName = issuerName;
      this.certPath = certPath;
      this.subjectFormatMappings = subjectFormatMappings;
      if (tokenClaimGroupMappings == null) {
          tokenClaimGroupMappings = new HashMap<TokenClaimAttribute,List<String>>();
      }
      if (!tokenClaimGroupMappings.isEmpty()) {
          if (tokenClaimGroupMappings.values().contains(null)) {
              throw new IllegalArgumentException("One or more group lists are null in claim group mappings.");
          }
      }
      this.tokenClaimGroupMappings = tokenClaimGroupMappings;
   }

   /**
    * Constructor without the subjectFormatMappings configuration
    * TODO: this is just to provide backward compatibility, remove if this is not needed
    */
   public TrustedSTSConfig(String issuerName, CertPath certPath){

      this(issuerName, certPath, null, null);
   }

   /**
    * @return the issuerName
    */
   public String getIssuerName() {
      return issuerName;
   }

   /**
    * @return the certPath
    */
   public CertPath getCertPath() {
      return certPath;
   }

   /**
    * @return the subjectFormatMappings
    */
   public AttributeConfig[] getSubjectFormatMappings() {
      return subjectFormatMappings;
   }

   /**
    * @return the tokenClaimGroupMappings
    */
   public Map<TokenClaimAttribute,List<String>> getTokenClaimGroupMappings() {
      return tokenClaimGroupMappings;
   }
}
