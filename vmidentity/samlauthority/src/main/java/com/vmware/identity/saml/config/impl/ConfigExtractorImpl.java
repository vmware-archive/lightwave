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
package com.vmware.identity.saml.config.impl;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.NoSuchIdPException;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.SystemConfigurationException;
import com.vmware.identity.saml.config.TokenRestrictions;

public final class ConfigExtractorImpl implements ConfigExtractor {

   private final static IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(ConfigExtractorImpl.class);

   private final String tenantName;
   private final CasIdmClient client;

   public ConfigExtractorImpl(String tenantName, CasIdmClient client)
      throws NoSuchIdPException, SystemConfigurationException {
      Validate.notEmpty(tenantName);
      Validate.notNull(client);

      this.tenantName = tenantName;
      this.client = client;

      Tenant tenant = getTenant();
      assert tenant != null;
   }

   @Override
   public Config getConfig() throws NoSuchIdPException,
      SystemConfigurationException {
      final String issuer;
      final List<Certificate> signingCertificateChain;
      final PrivateKey authorityPrivateKey;
      final int delegationCount;
      final int renewCount;
      final long maximumBearerTokenLifetime;
      final long maximumHoKTokenLifetime;
      final Collection<List<Certificate>> validCertificateChains;
      final long clockTolerance;
      final String signatureAlgorithm;
      final ArrayList<List<Certificate>> allValidCertChains;
      final Collection<IDPConfig> extIdps;

      log.debug("Retrieving configuration for tenant {}", tenantName);
      try {
         allValidCertChains = new ArrayList<List<Certificate>>();
         issuer = client.getEntityID(tenantName);
         signingCertificateChain = client.getTenantCertificate(tenantName);
         validCertificateChains = client.getTenantCertificates(tenantName);
         authorityPrivateKey = client.getTenantPrivateKey(tenantName);
         delegationCount = client.getDelegationCount(tenantName);
         renewCount = client.getRenewCount(tenantName);
         maximumBearerTokenLifetime = client
            .getMaximumBearerTokenLifetime(tenantName);
         maximumHoKTokenLifetime = client
            .getMaximumHoKTokenLifetime(tenantName);
         clockTolerance = client.getClockTolerance(tenantName);
         signatureAlgorithm = client.getTenantSignatureAlgorithm(tenantName);
         extIdps = client.getAllExternalIdpConfig(tenantName);
         allValidCertChains.addAll(validCertificateChains);
         if( extIdps != null ) {
             for(IDPConfig conf : extIdps) {
                 if(conf != null) {
                     ArrayList<Certificate> certChain = new ArrayList<Certificate>();
                     for(X509Certificate aCert : conf.getSigningCertificateChain()) {
                        certChain.add(aCert);
                     }
                     allValidCertChains.add(certChain);
                 }
             }
         }
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemConfigurationException(e);
      } catch (Exception e) {
         throw new SystemConfigurationException(e);
      }

      final Config result = newConfig(issuer, signingCertificateChain,
         authorityPrivateKey, new TokenRestrictions(maximumBearerTokenLifetime,
            maximumHoKTokenLifetime, delegationCount, renewCount),
            allValidCertChains, clockTolerance, signatureAlgorithm, extIdps);

      assert result != null;
      return result;
   }

   private Tenant getTenant() throws NoSuchIdPException,
      SystemConfigurationException {
      final Tenant result;
      try {
         result = client.getTenant(tenantName);
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (Exception e) {
         throw new SystemConfigurationException(e);
      }
      if (result == null) {
         throw noSuchIdPExc(null);
      }

      return result;
   }

   private Config newConfig(String issuerName,
      List<Certificate> signingCertificateChain, PrivateKey tenantPrivateKey,
      TokenRestrictions tokenRestrictions,
      Collection<List<Certificate>> validCertificateChains,
      long clockTolerance, String signatureAlgorithm,
      Collection<IDPConfig> externalIdps) {
      assert issuerName != null;
      assert issuerName != "";
      assert signingCertificateChain != null;
      assert tenantPrivateKey != null;
      assert tokenRestrictions != null;
      assert validCertificateChains != null;
      assert clockTolerance > 0;

      return new Config(new SamlAuthorityConfiguration(issuerName,
         signingCertificateChain, tenantPrivateKey, signatureAlgorithm),
         tokenRestrictions, validCertificateChains, clockTolerance, externalIdps);
   }

   private NoSuchIdPException noSuchIdPExc(NoSuchTenantException e) {
      return new NoSuchIdPException("IdP '" + tenantName + "' not found.", e);
   }
}
