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
import static org.junit.Assert.assertNotNull;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import junit.framework.Assert;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.saml.DefaultSamlAuthorityFactory;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.SamlAuthorityFactory;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.config.TokenRestrictions;

public class SamlAuthorityFactoryTest {

   private SamlAuthorityFactory factory;
   private PrivateKey authorityKey;
   private X509Certificate cert1;
   private X509Certificate cert2;
   private PrincipalAttributesExtractorFactory attributesExtractorFactory;
   private ConfigExtractorFactory configExtractorFactory;
   private ConfigExtractor configExtractor;

   @Before
   public void initSamlAuthorityFactory() throws Exception {
      authorityKey = createMock(PrivateKey.class);
      cert1 = createMock(X509Certificate.class);
      cert2 = createMock(X509Certificate.class);
      attributesExtractorFactory = createMock(PrincipalAttributesExtractorFactory.class);
      configExtractorFactory = createMock(ConfigExtractorFactory.class);

      // config factory
      configExtractor = createMock(ConfigExtractor.class);
      expect(configExtractorFactory.getConfigExtractor(eq(TENANT_NAME)))
         .andReturn(configExtractor);
      final List<Certificate> signingChain = new ArrayList<Certificate>();
      Collections.addAll(signingChain, cert1, cert2);
      Config config = initDefaultConfig(signingChain,
         SignatureAlgorithm.RSA_SHA512.toString());
      expect(configExtractor.getConfig()).andReturn(config);

      PrincipalAttributesExtractor extractor = new PrincipalAttributeExtractorImpl();
      expect(
         attributesExtractorFactory
            .getPrincipalAttributesExtractor(eq(TENANT_NAME))).andReturn(
         extractor);
      replay(attributesExtractorFactory, configExtractorFactory,
         configExtractor, authorityKey, cert1, cert2);

      factory = new DefaultSamlAuthorityFactory(SignatureAlgorithm.RSA_SHA256,
         attributesExtractorFactory, configExtractorFactory);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreatingFactoryWithoutSignatureAlgorithm() {
      new DefaultSamlAuthorityFactory(null, attributesExtractorFactory,
         configExtractorFactory);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreatingFactoryWithoutConfigExtractor() {
      new DefaultSamlAuthorityFactory(SignatureAlgorithm.RSA_SHA256,
         attributesExtractorFactory, null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreatingFactoryWithoutAttributesExtractorFactory() {
      new DefaultSamlAuthorityFactory(SignatureAlgorithm.RSA_SHA256, null,
         configExtractorFactory);
   }

   @Ignore
   public void testCreatingFactoryWithInvalidCertificateChain()
      throws Exception {
      // load keystore from : cert_chain_invalidVal
      // Provider securityProvider = null;
      // SignInfo signInfo = new SignInfo(privateKey, certPath,
      // securityProvider);

      factory.createTokenAuthority(TENANT_NAME);

      Assert.fail();
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateSamlAuthorityWithNoTenant() {
      factory.createTokenAuthority(null);
   }

   @Test
   public void testCreateTokenAuthority() {
      assertNotNull(factory.createTokenAuthority(TENANT_NAME));
      verify(configExtractorFactory, attributesExtractorFactory,
         authorityKey, cert1, cert2);
   }

   @Test
   public void testCreateTokenValidatorOK() {
      assertNotNull(factory.createTokenValidator(TENANT_NAME));
      verify(configExtractorFactory, attributesExtractorFactory,
         authorityKey, cert1, cert2);
   }

   private Config initDefaultConfig(List<Certificate> signingChain,
      String signatureAlgorithm) {
      Collection<List<Certificate>> validCerts = initValidCerts(signingChain);
      Config config = new Config(new SamlAuthorityConfiguration("issuer",
         signingChain, authorityKey, signatureAlgorithm),
         new TokenRestrictions(DEFAULT_MAXIMUM_TOKEN_LIFETIME,
            DEFAULT_MAXIMUM_TOKEN_LIFETIME, TOKEN_DELEGATION_COUNT,
            TOKEN_RENEW_COUNT), validCerts, DEFAULT_CLOCK_TOLERANCE,
            Arrays.<IDPConfig>asList(new IDPConfig(TestConstants.EXTERNAL_ISSUER)));
      return config;
   }

   private Collection<List<Certificate>> initValidCerts(
      List<Certificate> signingChain) {
      Collection<List<Certificate>> validCerts = new HashSet<List<Certificate>>();
      validCerts.add(signingChain);
      return validCerts;
   }

}
