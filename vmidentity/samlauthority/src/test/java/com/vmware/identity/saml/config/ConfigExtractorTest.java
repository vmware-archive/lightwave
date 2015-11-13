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

import static com.vmware.identity.saml.TestUtil.DEFAULT_CLOCK_TOLERANCE;
import static com.vmware.identity.saml.TestUtil.DEFAULT_MAXIMUM_TOKEN_LIFETIME;
import static com.vmware.identity.saml.TestUtil.TENANT_NAME;
import static com.vmware.identity.saml.TestUtil.TOKEN_DELEGATION_COUNT;
import static com.vmware.identity.saml.TestUtil.TOKEN_RENEW_COUNT;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.NoSuchIdPException;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.impl.ConfigExtractorImpl;

public class ConfigExtractorTest {

   private static final String ISSUER = "issuer";
   private static final String EXTERNAL_ISSUER = "externalIssuer";
   private CasIdmClient client;
   private List<Certificate> signingCertChain;
   private PrivateKey authorityKey;

   private ConfigExtractor configExtractor;
   private X509Certificate leafCert;
   private X509Certificate rootCert;

   @Before
   public void init() throws Exception {
      authorityKey = createMock(PrivateKey.class);

      final String keyStoreName = "sts-store.jks";
      final String keyalias = "stskey";
      final String storePass = "ca$hc0w";
      KeyStore ks = loadKeyStore(keyStoreName, storePass);
      Certificate[] certChain = ks.getCertificateChain(keyalias);
      List<X509Certificate> x509Certs = new ArrayList<X509Certificate>();
      for (Certificate cert : certChain) {
         x509Certs.add((X509Certificate) cert);
      }
      Assert.assertTrue(x509Certs.size() == 2);
      leafCert = x509Certs.get(0);
      rootCert = x509Certs.get(1);

      signingCertChain = new ArrayList<Certificate>();
      signingCertChain.add(leafCert);
      signingCertChain.add(rootCert);

      client =
            mockIdentityManager(ISSUER, signingCertChain, authorityKey,
                  SignatureAlgorithm.RSA_SHA512.toString());

      configExtractor = new ConfigExtractorImpl(TENANT_NAME, client);
   }

   @Test
   public void testGetConfig() {
//      replay(leafCert, rootCert, authorityKey);
      replay(authorityKey);
      Config config = configExtractor.getConfig();

      assertEquals(DEFAULT_CLOCK_TOLERANCE, config.getClockTolerance());

      SamlAuthorityConfiguration samlAuthorityConfig = config
         .getSamlAuthorityConfig();
      samlAuthorityConfig.getAuthorityKey();
      assertEquals(ISSUER, samlAuthorityConfig.getIssuer());
      assertEquals(signingCertChain,
         samlAuthorityConfig.getSigningCertificateChain());

      TokenRestrictions tokenRestrictions = config.getTokenRestrictions();
      assertEquals(TOKEN_DELEGATION_COUNT,
         tokenRestrictions.getDelegationCount());
      assertEquals(TOKEN_RENEW_COUNT, tokenRestrictions.getRenewCount());
      assertEquals(DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         tokenRestrictions.getMaximumBearerTokenLifetime());
      assertEquals(DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         tokenRestrictions.getMaximumHoKTokenLifetime());

      Collection<List<Certificate>> validCerts = config.getValidCerts();
      assertEquals(2, validCerts.size());
      assertEquals(signingCertChain, validCerts.iterator().next());

      verify(client, authorityKey);
   }

   @Test(expected = NoSuchIdPException.class)
   public void testGetConfigForNotExistingTenant() throws Exception {
      final CasIdmClient identityManagerClient = createMock(CasIdmClient.class);
      expect(identityManagerClient.getTenant(eq(TENANT_NAME))).andReturn(null);
      replay(identityManagerClient);
      try {
         configExtractor = new ConfigExtractorImpl(TENANT_NAME,
            identityManagerClient);
      } finally {
         verify(identityManagerClient);
      }
   }

   @Test
   public void testGetConfig_NoSignatureAlgorithm() throws Exception {
      CasIdmClient idmClient = mockIdentityManager(ISSUER, signingCertChain,
         authorityKey, null);
      ConfigExtractor configExtractor = new ConfigExtractorImpl(TENANT_NAME,
         idmClient);
      replay(authorityKey);

      Config config = configExtractor.getConfig();

      assertNull(config.getSamlAuthorityConfig().getSignatureAlgorithm());
      verify(idmClient, authorityKey);
   }

   private CasIdmClient mockIdentityManager(String issuer,
      List<Certificate> stsCertChain, PrivateKey mockKey,
      String signatureAlgorithm) throws Exception {

      final CasIdmClient identityManagerClient = createMock(CasIdmClient.class);

      expect(identityManagerClient.getTenant(TENANT_NAME)).andReturn(
         new Tenant(TENANT_NAME));
      expect(identityManagerClient.getEntityID(eq(TENANT_NAME))).andReturn(
         issuer).anyTimes();

      expect(identityManagerClient.getTenantCertificate(eq(TENANT_NAME)))
         .andReturn(stsCertChain == null ? null : stsCertChain);

      Collection<List<Certificate>> validCerts = new HashSet<List<Certificate>>();
      validCerts.add(stsCertChain);

      expect(identityManagerClient.getTenantCertificates(eq(TENANT_NAME)))
         .andReturn(stsCertChain == null ? null : validCerts);

      expect(identityManagerClient.getTenantPrivateKey(eq(TENANT_NAME)))
         .andReturn(mockKey);

      expect(identityManagerClient.getDelegationCount(TENANT_NAME)).andReturn(
         TOKEN_DELEGATION_COUNT);
      expect(identityManagerClient.getRenewCount(TENANT_NAME)).andReturn(
         TOKEN_RENEW_COUNT);
      expect(identityManagerClient.getMaximumBearerTokenLifetime(TENANT_NAME))
         .andReturn(DEFAULT_MAXIMUM_TOKEN_LIFETIME);
      expect(identityManagerClient.getMaximumHoKTokenLifetime(TENANT_NAME))
         .andReturn(DEFAULT_MAXIMUM_TOKEN_LIFETIME);
      expect(identityManagerClient.getClockTolerance(TENANT_NAME)).andReturn(
         DEFAULT_CLOCK_TOLERANCE);
      expect(identityManagerClient.getTenantSignatureAlgorithm(TENANT_NAME))
         .andReturn(signatureAlgorithm);

      ArrayList<X509Certificate> signingCertChain = new ArrayList<X509Certificate>();
      for(Certificate c: stsCertChain){
          signingCertChain.add((X509Certificate)c);
      }
      IDPConfig config = new IDPConfig(EXTERNAL_ISSUER);
      config.setSigningCertificateChain(signingCertChain);
      ArrayList<IDPConfig> extIdps = new ArrayList<IDPConfig>();
      extIdps.add(config);

      expect(identityManagerClient.getAllExternalIdpConfig(TENANT_NAME))
      .andReturn(extIdps);

      replay(identityManagerClient);
      return identityManagerClient;
   }

   private KeyStore loadKeyStore(String keyStoreFile, String pass) {
      KeyStore ks = null;
      try {
         ks = KeyStore.getInstance("JKS");
         ks.load(getClass().getClassLoader().getResourceAsStream(keyStoreFile),
               pass.toCharArray());
      } catch (FileNotFoundException fnfe) {
         throw new IllegalArgumentException(String.format(
               "keystore file [%s] not found", keyStoreFile), fnfe);
      } catch (IOException ioe) {
         String errMsg =
               ioe.getCause() instanceof UnrecoverableKeyException ? "Wrong keystore password"
                     : "";
         throw new IllegalArgumentException(errMsg, ioe);
      } catch (Exception e) {
         throw new IllegalStateException(e);
      }
      return ks;
   }
}
