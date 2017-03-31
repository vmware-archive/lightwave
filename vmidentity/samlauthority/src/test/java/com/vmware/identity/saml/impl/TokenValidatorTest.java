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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.easymock.EasyMock.reset;
import static org.junit.Assert.fail;

import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.TestUtil;
import com.vmware.identity.saml.CertificateUtil;
import com.vmware.identity.saml.InvalidSignatureException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.TokenRestrictions;

public class TokenValidatorTest {

   private static Set<X509Certificate> trustedRootCertificates = new HashSet<X509Certificate>();
   private static List<X509Certificate> signingCert = new ArrayList<X509Certificate>();

   private final String GROUP_DOMAIN = "vmware.com";
   private final String GROUP_NAME1 = "group1";
   private final String GROUP_NAME2 = "group2";
   private final String[] currentGroupList = new String[] {
      GROUP_DOMAIN + "\\" + GROUP_NAME1, GROUP_DOMAIN + "\\" + GROUP_NAME2 };
   private final PrincipalId[] tokenGroupList = new PrincipalId[] {
      new PrincipalId(GROUP_NAME1, GROUP_DOMAIN),
      new PrincipalId(GROUP_NAME2, GROUP_DOMAIN) };

   private static ConfigExtractor configExtractor;

   @BeforeClass
   public static void init() {
      CertificateUtil cu = new CertificateUtil();
      trustedRootCertificates.add(cu.loadCert(CertificateUtil.STS_CERT_ALIAS));
      Certificate[] certs = cu.loadCertPath(CertificateUtil.STS_CERT_ALIAS);
      for(int i = 0; i < certs.length; i++)
      {
          signingCert.add((X509Certificate)certs[i]);
      }
      configExtractor = mockConfigExtractor(trustedRootCertificates, 0);
   }

   @Test
   public void testValidToken() {
      PrincipalAttributeExtractorImpl attributesExtractor = new PrincipalAttributeExtractorImpl(
                currentGroupList, true);
      TokenValidatorImpl validator = new TokenValidatorImpl(
              new AuthnOnlyTokenValidator(configExtractor, attributesExtractor), attributesExtractor );
      validator.validate(new ValidatableTokenMock(true, Arrays
         .asList(tokenGroupList)));
   }

   @Test
   public void testExternalToken() {

      PrincipalAttributeExtractorImpl attributesExtractor1 = new PrincipalAttributeExtractorImpl(
          currentGroupList, true);

      PrincipalId principalId = new PrincipalId("user", "domain");
      com.vmware.identity.idm.PrincipalId idmPrincipalId = new com.vmware.identity.idm.PrincipalId(principalId.getName(), principalId.getDomain());

      PrincipalAttributesExtractor attributesExtractor = createMock(PrincipalAttributesExtractor.class);
      expect(attributesExtractor.isActive(idmPrincipalId)).andReturn(true);
      replay(attributesExtractor);

      AuthnOnlyTokenValidator authValidator =
          new AuthnOnlyTokenValidator(configExtractor, attributesExtractor);

      ServerValidatableSamlToken validatedToken = authValidator.validate(new ValidatableTokenMock(principalId, true, Arrays
         .asList(tokenGroupList), TestConstants.EXTERNAL_ISSUER));
      Assert.assertEquals("Token is issued by external idp", true, validatedToken.isExternal());
      Assert.assertEquals("User found", SubjectValidation.Regular, validatedToken.getSubject().subjectValidation());
      verify(attributesExtractor);

      reset(attributesExtractor);
      expect(attributesExtractor.isActive(idmPrincipalId)).andReturn(false);
      replay(attributesExtractor);

      validatedToken = authValidator.validate(new ValidatableTokenMock(principalId, true, Arrays
              .asList(tokenGroupList), TestConstants.EXTERNAL_ISSUER));
           Assert.assertEquals("Token is issued by external idp", true, validatedToken.isExternal());
           Assert.assertEquals("User not found", SubjectValidation.None, validatedToken.getSubject().subjectValidation());

      verify(attributesExtractor);
      reset(attributesExtractor);
      replay(attributesExtractor);

      try
      {
          authValidator.validate(new ValidatableTokenMock(false, Arrays
                  .asList(tokenGroupList), TestConstants.EXTERNAL_ISSUER));
          Assert.fail("verifying invalid token should fail.");
      }
      catch(InvalidTokenException ex) // expected
      {}

      verify(attributesExtractor);

      reset(attributesExtractor);
      expect(attributesExtractor.getAllAttributeDefinitions()).andReturn(attributesExtractor1.getAllAttributeDefinitions());
      expect(attributesExtractor.isActive(idmPrincipalId)).andReturn(true);
      replay(attributesExtractor);

      TokenValidatorImpl validator =
              new TokenValidatorImpl(new AuthnOnlyTokenValidator(configExtractor, attributesExtractor), attributesExtractor);

      try
      {
          validator.validate(new ValidatableTokenMock(principalId, true, Arrays
                  .asList(tokenGroupList), TestConstants.EXTERNAL_ISSUER));
          Assert.fail("verifying invalid token should fail.");
      }
      catch(InvalidTokenException ex) // expected
      {}

      verify(attributesExtractor);

      reset(attributesExtractor);
      expect(attributesExtractor.isActive(idmPrincipalId)).andReturn(false);
      replay(attributesExtractor);

      try
      {
          validator.validate(new ValidatableTokenMock(principalId, true, Arrays
                  .asList(tokenGroupList), TestConstants.EXTERNAL_ISSUER));
          Assert.fail("verifying invalid token should fail.");
      }
      catch(InvalidTokenException ex) // expected
      {}

      verify(attributesExtractor);
      reset(attributesExtractor);
      replay(attributesExtractor);

      try
      {
          authValidator.validate(new ValidatableTokenMock(false, Arrays
                  .asList(tokenGroupList), TestConstants.EXTERNAL_ISSUER));
          Assert.fail("verifying invalid token should fail.");
      }
      catch(InvalidTokenException ex) // expected
      {}

      verify(attributesExtractor);

   }

   @Test(expected = InvalidTokenException.class)
   public void testInvalidToken() {
      PrincipalAttributeExtractorImpl attributesExtractor =
          new PrincipalAttributeExtractorImpl();

      TokenValidatorImpl validator = new TokenValidatorImpl(
              new AuthnOnlyTokenValidator(configExtractor, attributesExtractor), attributesExtractor);
      validator.validate(new ValidatableTokenMock(false, Arrays
         .asList(tokenGroupList)));
   }

   @Test(expected = InvalidTokenException.class)
   public void testInvalidGroupsToken() {
          PrincipalAttributeExtractorImpl attributesExtractor = new PrincipalAttributeExtractorImpl();
      TokenValidatorImpl validator = new TokenValidatorImpl(
         new AuthnOnlyTokenValidator(configExtractor, attributesExtractor), attributesExtractor);
      validator.validate(new ValidatableTokenMock(true, Arrays
         .asList(tokenGroupList)));
   }

   @Test(expected = InvalidTokenException.class)
   public void testInactiveSubjectToken() {
          PrincipalAttributeExtractorImpl attributesExtractor = new PrincipalAttributeExtractorImpl(
            currentGroupList, false);
      TokenValidatorImpl validator = new TokenValidatorImpl(
              new AuthnOnlyTokenValidator( configExtractor, attributesExtractor), attributesExtractor);
      validator.validate(new ValidatableTokenMock(true, Arrays
         .asList(tokenGroupList)));
   }

   @Test
   public void testInvalidTokenSignature() {
          PrincipalAttributeExtractorImpl attributesExtractor = new PrincipalAttributeExtractorImpl(
            currentGroupList, false);
      TokenValidatorImpl validator = new TokenValidatorImpl(
              new AuthnOnlyTokenValidator( configExtractor, attributesExtractor), attributesExtractor);
      boolean invalidSignature = true;
      try {
         validator.validate(new ValidatableTokenMock(invalidSignature));
         fail();
      } catch (InvalidSignatureException e) {
         // expected
      }
   }

   @Test
   public void testInactiveDelegate() {
      final String subjectName = "userDancho";
      final String delegateName = "userPencho";
      final String domain = "serdika";
      final ValidatableTokenMock token = new ValidatableTokenMock(
         new PrincipalId(subjectName, domain), true,
         Arrays.asList(tokenGroupList));
      token.addDelegate(new PrincipalId(delegateName, domain));

      final PrincipalAttributeExtractorImpl principalAttributesExtractor = new PrincipalAttributeExtractorImpl(
         currentGroupList, false);
      principalAttributesExtractor
         .activePrincipal(new com.vmware.identity.idm.PrincipalId(subjectName,
            domain));
      TokenValidatorImpl validator = new TokenValidatorImpl(
              new AuthnOnlyTokenValidator( configExtractor, principalAttributesExtractor), principalAttributesExtractor);
      try {
         validator.validate(token);
         Assert.fail();
      } catch (InvalidTokenException e) {
         // expected
      } catch (Exception e){
    	  e.printStackTrace();
      }

      principalAttributesExtractor
         .activePrincipal(new com.vmware.identity.idm.PrincipalId(delegateName,
            domain));
      validator.validate(token);
   }

   private static ConfigExtractor mockConfigExtractor(
       Set<X509Certificate> trustedRootCertificates,
       long clockTolerance) {

       CertificateUtil cu = new CertificateUtil();

       final ArrayList<List<Certificate>> validCerts = new ArrayList<List<Certificate>>(trustedRootCertificates.size());
       for( X509Certificate cert : trustedRootCertificates ) {
           ArrayList<Certificate> chain = new ArrayList<Certificate>(1);
           chain.add(cert);
           validCerts.add(chain);
       }

       final TokenRestrictions tokenRestrictions =
           new TokenRestrictions(
               TestUtil.DEFAULT_MAXIMUM_TOKEN_LIFETIME,
               TestUtil.DEFAULT_MAXIMUM_TOKEN_LIFETIME,
               TestUtil.TOKEN_DELEGATION_COUNT,
               TestUtil.TOKEN_RENEW_COUNT);

       final SamlAuthorityConfiguration samlAuthorityConfig =
           new SamlAuthorityConfiguration(
               TestConstants.ISSUER,
               Collections.singletonList((Certificate)cu.loadCert(CertificateUtil.STS_CERT_ALIAS)),
               cu.loadPrivateKey(CertificateUtil.STS_CERT_ALIAS), "RSA_SHA256"
       );

       final Config config = new Config(
           samlAuthorityConfig, tokenRestrictions, validCerts, clockTolerance,
           Arrays.<IDPConfig>asList(new IDPConfig(TestConstants.EXTERNAL_ISSUER)));

       final ConfigExtractor configExtractor = createMock(ConfigExtractor.class);

       expect(configExtractor.getConfig()).andReturn(config).anyTimes();

       replay(configExtractor);

       return configExtractor;
    }
}
