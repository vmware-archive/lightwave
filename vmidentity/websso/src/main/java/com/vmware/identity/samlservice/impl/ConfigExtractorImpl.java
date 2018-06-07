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
package com.vmware.identity.samlservice.impl;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.SystemConfigurationException;
import com.vmware.identity.saml.config.TokenRestrictions;
import com.vmware.identity.samlservice.DefaultIdmAccessorFactory;
import com.vmware.identity.samlservice.IdmAccessor;

public class ConfigExtractorImpl implements ConfigExtractor {

   private String tenantName;

   public ConfigExtractorImpl(String tenantName) {
      this.tenantName = tenantName;
   }

   public Config getConfig() {
      IdmAccessor idmAccessor = new DefaultIdmAccessorFactory().getIdmAccessor();
      idmAccessor.setTenant(tenantName);

      String issuer = idmAccessor.getIdpEntityId();
      List<Certificate> signingCertificateChain = idmAccessor.getSAMLAuthorityChain();
      PrivateKey authorityKey = idmAccessor.getSAMLAuthorityPrivateKey();
      TokenRestrictions tokenRestrictions = new TokenRestrictions(
         idmAccessor.getMaximumBearerTokenLifetime(),
         idmAccessor.getMaximumHoKTokenLifetime(),
         idmAccessor.getDelegationCount(),
         idmAccessor.getRenewCount());
      Collection<List<Certificate>> validCerts = idmAccessor.getSAMLAuthorityChains();
      long clockTolerance = idmAccessor.getClockTolerance();

      ArrayList<List<Certificate>> allChains = new ArrayList<List<Certificate>>();
      allChains.addAll(validCerts);
      Collection<IDPConfig> externalIdps = idmAccessor.getExternalIdps(IDPConfig.IDP_PROTOCOL_SAML_2_0);
      if( externalIdps != null ) {
          for(IDPConfig c : externalIdps){
              ArrayList<Certificate> chain = new ArrayList<Certificate>();
              for(Certificate cert : c.getSigningCertificateChain()){
                  chain.add(cert);
              }
              allChains.add(chain);
          }
      }

      Set<String> blackListedDomains = this.extractBlackListedDomains(tenantName, idmAccessor);
      return new Config(
         new Config.SamlAuthorityConfiguration(issuer,
            signingCertificateChain, authorityKey, idmAccessor.getTenantSignatureAlgorithm()),
            tokenRestrictions, allChains, clockTolerance, externalIdps, blackListedDomains);
   }

    private Set<String> extractBlackListedDomains(String tenantName, IdmAccessor idmAccessor) {
        // this keeps orginal logic form AuthnOnlyTokenValidator.java
        // it might need to be adjusted
        HashSet<String> blacklistedDomainsForExtIdp = new HashSet<>();

        String systemTenant = idmAccessor.getSystemTenant();
        EnumSet<DomainType> systemDomains = EnumSet.of(DomainType.SYSTEM_DOMAIN, DomainType.LOCAL_OS_DOMAIN);
        Collection<IIdentityStoreData> providers = idmAccessor
            .getProviders(systemTenant, systemDomains);
        if (providers != null) {
            for( IIdentityStoreData prov : providers ) {
                if ( prov != null ) {
                    blacklistedDomainsForExtIdp.add(prov.getName());
                }
            }
        }

        return blacklistedDomainsForExtIdp;
    }

}
