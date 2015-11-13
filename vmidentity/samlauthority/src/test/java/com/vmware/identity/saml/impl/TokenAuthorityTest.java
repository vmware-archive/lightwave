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
import static com.vmware.identity.saml.TestUtil.TOKEN_DELEGATION_COUNT;
import static com.vmware.identity.saml.TestUtil.TOKEN_RENEW_COUNT;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.security.PrivateKey;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice.Attribute;
import com.vmware.identity.saml.CertificateUtil;
import com.vmware.identity.saml.DefaultSamlAuthorityFactory;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.RenewException;
import com.vmware.identity.saml.SamlAuthorityFactory;
import com.vmware.identity.saml.SamlToken;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.Builder;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec;
import com.vmware.identity.saml.SamlTokenSpec.RenewSpec;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.TestUtil;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.saml.UnsupportedTokenLifetimeException;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.config.TokenRestrictions;
import com.vmware.identity.util.TimePeriod;
import com.vmware.vim.sso.client.Advice;
import com.vmware.vim.sso.client.Advice.AdviceAttribute;
import com.vmware.vim.sso.client.ConfirmationType;
import com.vmware.vim.sso.client.DefaultTokenFactory;
import com.vmware.vim.sso.client.exception.InvalidTokenException;

public class TokenAuthorityTest {

   private static final String SYSTEM_DOMAIN = "system-domain";
   private static final String ISSUER = "http://sts.example.com/service";
   private static final String GROUP = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";
   private static final PrincipalId TOKEN_OWNER = new PrincipalId("tokenOwner",
      SYSTEM_DOMAIN);
   private static final PrincipalId TOKEN_PRESENTER = new PrincipalId(
      "tokenPresenter", SYSTEM_DOMAIN);

   private final SamlAuthorityFactory factory;
   private final List<Certificate> certPathList;
   private final SignInfo signInfo;
   private final X509Certificate trustedRootCA;
   private final X509Certificate hokCertificate;
   private final TokenRestrictions defaultTokenRestrictions;
   private Map<String, String[]> attributes;

   private TokenAuthority authority;

   private Date startTime;
   private Date authnTime;
   private Date endTime;
   private ConfigExtractorFactory configExtractorFactory;
   private ConfigExtractor configExtractor;
   private PrincipalAttributesExtractorFactory attributeExtractorFactory;
   private final String DEFAULT_TENANT_NAME = "DefaultTenantName";
   private final SignatureAlgorithm defaultSignatureAlgorithm = SignatureAlgorithm.RSA_SHA256;

   public TokenAuthorityTest() throws Exception {
      CertificateUtil cu = new CertificateUtil();

      Certificate[] certificates = cu
         .loadCertPath(CertificateUtil.STS_CERT_ALIAS);
      trustedRootCA = (X509Certificate) certificates[certificates.length - 1];
      hokCertificate = (X509Certificate) certificates[certificates.length - 1];

      CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
      certPathList = Arrays.asList(certificates);
      CertPath certPath = certFactory.generateCertPath(certPathList);

      PrivateKey privateKey = cu.loadPrivateKey(CertificateUtil.STS_CERT_ALIAS);
      signInfo = new SignInfo(privateKey, certPath, null);

      assertEquals(((X509Certificate) certificates[0]).getNotAfter(),
         signInfo.getSigningCertificate().getNotAfter());

      defaultTokenRestrictions = TokenCreationUtil
         .createDefaultTokenRestrictions();

      configExtractorFactory = initializeConfigExtractor();

      attributeExtractorFactory = initializeAttributeExtractor(new PrincipalAttributeExtractorImpl(
         null));

      factory = new DefaultSamlAuthorityFactory(defaultSignatureAlgorithm,
         attributeExtractorFactory, configExtractorFactory);
   }

   @Before
   public void init() {
      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      Calendar calendar = new GregorianCalendar();
      startTime = calendar.getTime();
      calendar.add(Calendar.MINUTE, 1);
      authnTime = calendar.getTime();
      calendar.add(Calendar.MINUTE, 9);
      endTime = calendar.getTime();

      initTokenAttributes();
   }

   @After
   public void verifyMockObjects() {
      verify(configExtractor, configExtractorFactory, attributeExtractorFactory);
   }

   @Test
   public void testIssueBearerTokenWithGroups_SpecEndTime()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime));

      final String groupName1 = "groupAA";
      final String groupName2 = "groupBB";
      final String domain = "Domain";
      String[] groups = new String[] { toNetbios(groupName1, domain),
         toNetbios(groupName2, domain) };

      attributes.put(GROUP, groups);
      try {
         PrincipalAttributeExtractorImpl principalAttributeExtractor = new PrincipalAttributeExtractorImpl(
            groups);
         attributeExtractorFactory = initializeAttributeExtractor(principalAttributeExtractor);
         configExtractorFactory = initializeConfigExtractor();

         SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
            SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
            configExtractorFactory);

         authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

         Date issueInstantStartTime = new Date();
         SamlToken token = authority.issueToken(spec);
         Date issueInstantEndTime = new Date();

         Date expectedTokenStartTime = startTime;
         Date expectedTokenEndTime = endTime;

         checkToken(token, spec, issueInstantStartTime, issueInstantEndTime,
            expectedTokenStartTime, expectedTokenEndTime,
            new com.vmware.vim.sso.PrincipalId[] {
               new com.vmware.vim.sso.PrincipalId(groupName1, domain),
               new com.vmware.vim.sso.PrincipalId(groupName2, domain) }, false);
      } finally {
         attributes.remove(GROUP);
      }
   }

   @Test
   public void testIssueTokenWithGroups_TrailingSpaces()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime));

      final String groupName1 = "groupAA ";
      final String groupName2 = "groupAA  ";
      final String groupName3 = " groupBB";
      final String groupName4 = "  groupBB";
      final String domain = "Domain";
      String[] groups = new String[] {
         toNetbios(groupName1, domain),
         toNetbios(groupName2, domain),
         toNetbios(groupName3, domain),
         toNetbios(groupName4, domain) };

      attributes.put(GROUP, groups);
      try {
         PrincipalAttributeExtractorImpl principalAttributeExtractor = new PrincipalAttributeExtractorImpl(
            groups);
         attributeExtractorFactory = initializeAttributeExtractor(principalAttributeExtractor);
         configExtractorFactory = initializeConfigExtractor();

         SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
            SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
            configExtractorFactory);

         authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

         Date issueInstantStartTime = new Date();
         SamlToken token = authority.issueToken(spec);
         Date issueInstantEndTime = new Date();

         Date expectedTokenStartTime = startTime;
         Date expectedTokenEndTime = endTime;

         checkToken(token, spec, issueInstantStartTime, issueInstantEndTime,
            expectedTokenStartTime, expectedTokenEndTime,
            new com.vmware.vim.sso.PrincipalId[] {
               new com.vmware.vim.sso.PrincipalId(groupName1, domain),
               new com.vmware.vim.sso.PrincipalId(groupName2, domain),
               new com.vmware.vim.sso.PrincipalId(groupName3, domain),
               new com.vmware.vim.sso.PrincipalId(groupName4, domain)}, false);
      } finally {
         attributes.remove(GROUP);
      }
   }

   @Test
   public void testIssueBearerToken_SpecEndTime() throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime));
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueBearerToken_SpecEndTime_RecipientAndInResponseToAdded()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpecBuilder(null, authnTime,
         new TimePeriod(startTime, endTime), "recipient", "inResponseTo")
         .createSpec();

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueBearerToken_SpecEndTime_SignatureAlgorithmInConfig()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime));
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime,
         initializeConfigExtractor(SignatureAlgorithm.RSA_SHA1));
   }

   @Test
   public void testIssueBearerToken_SpecEndTime_WithSignatureAlgorithmInSpec()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), SignatureAlgorithm.RSA_SHA512);
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueBearerToken_SpecEndTime_WithSignatureAlgorithmInSpecAndDefaultOneInConfig()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), SignatureAlgorithm.RSA_SHA512);
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime,
         initializeConfigExtractor(SignatureAlgorithm.RSA_SHA1));
   }

   @Test
   public void testIssueBearerToken_SpecEndTime_OtherAttributeAsTokenSubject()
      throws InvalidTokenException {

      String identityAttribute = "SomeAttribute";
      String identityAttributeValue = "testAttribute@vmware.com";

      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), identityAttribute);

      PrincipalAttributeExtractorImpl principalAttributesExtractor = new PrincipalAttributeExtractorImpl(
         identityAttribute, new String[] { identityAttributeValue });

      attributeExtractorFactory = initializeAttributeExtractor(principalAttributesExtractor);
      configExtractorFactory = initializeConfigExtractor();

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      Document token = authority.issueToken(spec).getDocument();

      checkIdentityAttributeValue(token, identityAttribute,
         identityAttributeValue);
   }

   @Test
   public void testIssueBearerToken_SpecEndTime_TokenSubjectIncludedInTokenAttributes()
      throws InvalidTokenException {

      String identityAttribute = "SomeAttribute";
      String identityAttributeValue = "testAttribute@vmware.com";
      attributes
         .put(identityAttribute, new String[] { identityAttributeValue });

      Collection<String> attributeNames = Arrays.asList(new String[] {
         "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress",
         identityAttribute });

      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), identityAttribute, attributeNames);

      PrincipalAttributeExtractorImpl principalAttributesExtractor = new PrincipalAttributeExtractorImpl(
         identityAttribute, new String[] { identityAttributeValue });

      attributeExtractorFactory = initializeAttributeExtractor(principalAttributesExtractor);
      configExtractorFactory = initializeConfigExtractor();

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      SamlToken token = authority.issueToken(spec);

      checkIdentityAttributeValue(token.getDocument(), identityAttribute,
         identityAttributeValue);

      checkTokenAttributes(token.getDocument(), attributeNames);
   }

   @Test
   public void testIssueBearerToken_RemediatedEndTime_MaxLifetime()
      throws InvalidTokenException {
      TokenRestrictions tokenRestrictions = TokenCreationUtil
         .createTokenRestrictions(7 * 60 * 1000, 1, TOKEN_DELEGATION_COUNT,
            TOKEN_RENEW_COUNT);

      attributeExtractorFactory = initializeAttributeExtractor(new PrincipalAttributeExtractorImpl(
         null));
      configExtractorFactory = initializeConfigExtractor(tokenRestrictions);

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime));

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = new Date(startTime.getTime()
         + tokenRestrictions.getMaximumBearerTokenLifetime());
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueBearerToken_RemediatedEndTime_CertTime()
      throws Exception {
      Calendar cal = new GregorianCalendar();
      cal.add(Calendar.YEAR, 1000);
      endTime = cal.getTime();
      long maxBearerLifetimeMillis = endTime.getTime();
      // life span end time and max time are the same

      TokenRestrictions tokenRestrictions = TokenCreationUtil
         .createTokenRestrictions(maxBearerLifetimeMillis, 1,
            TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT);

      attributeExtractorFactory = initializeAttributeExtractor(new PrincipalAttributeExtractorImpl(
         null));
      configExtractorFactory = initializeConfigExtractor(tokenRestrictions);

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime));

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = TestUtil.shiftDate(signInfo
         .getSigningCertificate().getNotAfter(), 1);
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueBearerTokenNoLifetime_MaxLifeTime()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime, null);
      Date timeAroundExpectedTokenStartTime = startTime;
      Date timeAroundExpectedTokenEndTime = endTime;
      testIssueToken(spec, timeAroundExpectedTokenStartTime,
         timeAroundExpectedTokenEndTime);
   }

   @Test
   public void testIssueBearerTokenEmptyLifetime_MaxLifeTime()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(null, null));
      Date timeAroundExpectedTokenStartTime = startTime;
      Date timeAroundExpectedTokenEndTime = endTime;
      testIssueToken(spec, timeAroundExpectedTokenStartTime,
         timeAroundExpectedTokenEndTime);
   }

   @Test
   public void testIssueBearerTokenOnlyWithStartTimeInDesiredLifetime_MaxLifeTime()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, null));
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = new Date(expectedTokenStartTime.getTime()
         + defaultTokenRestrictions.getMaximumBearerTokenLifetime());
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueBearerTokenOnlyWithEndTimeInDesiredLifetime_SpecEndTime()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(null, endTime));
      Date timeAroundExpectedTokenStartTime = startTime;
      Date timeAroundExpectedTokenEndTime = endTime;
      testIssueToken(spec, timeAroundExpectedTokenStartTime,
         timeAroundExpectedTokenEndTime);
   }

   @Test(expected = UnsupportedTokenLifetimeException.class)
   public void testIssueBearerTokenNotValidStartTime_StartTimeBeforeIssueInstantMoreThanTolerance()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(new Date(startTime.getTime() - 10 * 60 * 1000), null));
      authority.issueToken(spec);
   }

   @Test(expected = IllegalStateException.class)
   public void testIssueBearerToken_NoIdentityAttributeExtractedFromIDM()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), "NotExistingAttributeInIDM");
      authority.issueToken(spec);
   }

   @Test(expected = IllegalStateException.class)
   public void testIssueBearerToken_IdentityAttributeExtractedHasNoValue()
      throws InvalidTokenException {
      String identityAttribute = "homeTown";
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), identityAttribute);

      String[] identityHasNoValue = null;
      PrincipalAttributeExtractorImpl principalAttributeExtractor = new PrincipalAttributeExtractorImpl(
         identityAttribute, identityHasNoValue);

      PrincipalAttributesExtractorFactory attributeExtractorFactory = initializeAttributeExtractor(principalAttributeExtractor);
      ConfigExtractorFactory configExtractorFactory = initializeConfigExtractor();

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      TokenAuthority authority = factory
         .createTokenAuthority(DEFAULT_TENANT_NAME);

      authority.issueToken(spec);
   }

   @Test(expected = IllegalStateException.class)
   public void testIssueBearerToken_IdentityAttributeExtractedHasEmptyValues()
      throws InvalidTokenException {
      String identityAttribute = "homeTown";
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), identityAttribute);

      String[] identityHasEmptyValues = new String[] {};
      PrincipalAttributeExtractorImpl principalAttributeExtractor = new PrincipalAttributeExtractorImpl(
         identityAttribute, identityHasEmptyValues);
      PrincipalAttributesExtractorFactory attributeExtractorFactory = initializeAttributeExtractor(principalAttributeExtractor);
      ConfigExtractorFactory configExtractorFactory = initializeConfigExtractor();

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      TokenAuthority authority = factory
         .createTokenAuthority(DEFAULT_TENANT_NAME);

      authority.issueToken(spec);
   }

   @Test(expected = IllegalStateException.class)
   public void testIssueBearerToken_IdentityAttributeExtractedHasMoreThanOneValue()
      throws InvalidTokenException {
      String identityAttribute = "homeTown";
      SamlTokenSpec spec = TokenCreationUtil.createSpec(null, authnTime,
         new TimePeriod(startTime, endTime), identityAttribute);

      PrincipalAttributeExtractorImpl principalAttributeExtractor = new PrincipalAttributeExtractorImpl(
         identityAttribute, new String[] { "Seattle", "San Francisco",
            "New York" });
      PrincipalAttributesExtractorFactory attributeExtractorFactory = initializeAttributeExtractor(principalAttributeExtractor);
      ConfigExtractorFactory configExtractorFactory = initializeConfigExtractor();

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      TokenAuthority authority = factory
         .createTokenAuthority(DEFAULT_TENANT_NAME);

      authority.issueToken(spec);
   }

   @Test
   public void testIssueBearerTokenAudience() throws InvalidTokenException {
      final Builder specBuilder = TokenCreationUtil.createSpecBuilder(null,
         authnTime, new TimePeriod(startTime, endTime));
      specBuilder.addAudience("cdvge:ege:fev");
      specBuilder.addAudience("cdvge:ege:fev1");
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(specBuilder.createSpec(), expectedTokenStartTime,
         expectedTokenEndTime);
   }

   @Test
   public void testIssueBearerTokenAdvicesByTokenOwner()
      throws InvalidTokenException {
      final Builder specBuilder = TokenCreationUtil.createSpecBuilder(null,
         authnTime, new TimePeriod(startTime, endTime));
      specBuilder.addRequestedAdvice(TestConstants.ADVICE);
      specBuilder.addRequestedAdvice(TestConstants.ADVICE_OTHER_VALUE);
      testIssueToken(specBuilder.createSpec(), startTime, endTime);
   }

   @Test
   public void testIssueBearerTokenOverlappedAdvicesByTokenPresenter()
      throws InvalidTokenException {

      initTokenAttributes(TOKEN_OWNER);
      final Builder specBuilder = TokenCreationUtil.createSpecBuilder(null,
         authnTime, new TimePeriod(startTime, endTime));
      TokenCreationUtil.setNoDelegateSpec(specBuilder, TOKEN_OWNER, endTime);
      specBuilder.addPresentAdvice(TestConstants.ADVICE);
      specBuilder.addRequestedAdvice(TestConstants.ADVICE_OTHER_VALUE);

      testIssueToken(specBuilder.createSpec(), startTime, endTime);
   }

   @Test
   public void testIssueHoKToken_SpecEndTime() throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueHoKToken_RemediatedEndTime_MaxLifetime()
      throws InvalidTokenException {
      SamlTokenSpec spec = TokenCreationUtil.createSpec(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      int initialTokenDelegationCount = 0;
      TokenRestrictions tokenRestrictions = TokenCreationUtil
         .createTokenRestrictions(1, 7 * 60 * 1000,
            initialTokenDelegationCount, TOKEN_RENEW_COUNT);

      attributeExtractorFactory = initializeAttributeExtractor(new PrincipalAttributeExtractorImpl(
         null));
      configExtractorFactory = initializeConfigExtractor(tokenRestrictions);

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = new Date(expectedTokenStartTime.getTime()
         + tokenRestrictions.getMaximumHoKTokenLifetime());
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   // TODO test returning hok cert end time

   @Test(expected = IllegalArgumentException.class)
   public void testIssueTokenWithNoSpec() {
      authority.issueToken(null);
   }

   @Test
   public void testCreateDelegableToken_SpecEndTime()
      throws InvalidTokenException {
      Builder builder = TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      builder.setDelegationSpec(new SamlTokenSpec.DelegationSpec(null, true));
      SamlTokenSpec spec = builder.createSpec();
      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testTokenOwnerIssueHoKTokenWithDelegationChain_SpecEndTime()
      throws InvalidTokenException {

      testIssueHoKTokenWithDelegationChain_SpecEndTime_Int(TOKEN_OWNER,
         TOKEN_OWNER);
   }

   @Test
   public void testTokenPresenterIssueHoKTokenWithDelegationChain_SpecEndTime()
      throws InvalidTokenException {

      testIssueHoKTokenWithDelegationChain_SpecEndTime_Int(TOKEN_OWNER,
         TOKEN_PRESENTER);
   }

   @Test
   public void testTokenOwnerIssueHoKTokenWithDelegationChain_DelegateAssertionEndTime()
      throws InvalidTokenException {

      testIssueHoKTokenWithDelegationChain_DelegateAssertionEndTime_Int(
         TOKEN_OWNER, TOKEN_OWNER);
   }

   @Test
   public void testTokenPresenterIssueHoKTokenWithDelegationChain_DelegateAssertionEndTime()
      throws InvalidTokenException {

      testIssueHoKTokenWithDelegationChain_DelegateAssertionEndTime_Int(
         TOKEN_OWNER, TOKEN_PRESENTER);
   }

   @Test
   public void testIssueRenewableTokenZeroInitialCount_SpecEndTime()
      throws InvalidTokenException {
      PrincipalAttributesExtractorFactory attributeExtractorFactory = initializeAttributeExtractor(new PrincipalAttributeExtractorImpl(
         null));
      ConfigExtractorFactory configExtractorFactory = initializeConfigExtractor(new TokenRestrictions(
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         TOKEN_DELEGATION_COUNT, 0));

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      TokenAuthority authority = factory
         .createTokenAuthority(DEFAULT_TENANT_NAME);

      Builder builder = TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      final boolean renewable = true;
      builder.setRenewSpec(new RenewSpec(renewable));
      SamlTokenSpec spec = builder.createSpec();

      Date issueInstantStartTime = new Date();
      SamlToken token = authority.issueToken(spec);
      Date issueInstantEndTime = new Date();

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;

      checkToken(token, spec, issueInstantStartTime, issueInstantEndTime,
         expectedTokenStartTime, expectedTokenEndTime,
         new com.vmware.vim.sso.PrincipalId[] {}, !renewable);
   }

   @Test
   public void testIssueRenewableToken_SpecEndTime()
      throws InvalidTokenException {
      int initialTokenDelegationCount = 0;
      PrincipalAttributesExtractorFactory attributeExtractorFactory = initializeAttributeExtractor(new PrincipalAttributeExtractorImpl(
         null));
      ConfigExtractorFactory configExtractorFactory = initializeConfigExtractor(new TokenRestrictions(
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         initialTokenDelegationCount, TOKEN_RENEW_COUNT));

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      Builder builder = TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      builder.setRenewSpec(new RenewSpec(true));
      SamlTokenSpec spec = builder.createSpec();

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueRenewableDelegatedToken_SpecEndTime()
      throws InvalidTokenException {
      Builder builder = TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      builder.setRenewSpec(new RenewSpec(true));

      PrincipalId delegate = new PrincipalId("solution2", SYSTEM_DOMAIN);
      builder.setDelegationSpec(new SamlTokenSpec.DelegationSpec(delegate,
         false));
      final SamlTokenSpec spec = builder.createSpec();

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime);
   }

   @Test
   public void testIssueFailRenewableToken() throws InvalidTokenException {
      Builder builder = TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      final boolean renewable = true;
      final boolean renew = true;
      final int noMoreRenewables = 0;
      builder.setRenewSpec(new RenewSpec(renewable, renew, noMoreRenewables));
      TokenCreationUtil.createDelegationChain(builder, TOKEN_OWNER,
         signInfo.getSigningCertificate().getNotAfter());
      final SamlTokenSpec spec = builder.createSpec();

      try {
         authority.issueToken(spec);
         fail();
      } catch (RenewException e) {
         // expected
      }
   }

   @Test
   public void testIssueRenewableTokenNotRespected_SpecEndTime()
      throws InvalidTokenException {

      initTokenAttributes(TOKEN_OWNER);
      Builder builder = TokenCreationUtil.createSpecBuilder(hokCertificate,
         authnTime, new TimePeriod(startTime, endTime));
      final boolean renewable = true;
      final boolean renew = false;
      final int noMoreRenewables = 0;
      builder.setRenewSpec(new RenewSpec(renewable, renew, noMoreRenewables));
      TokenCreationUtil.createDelegationChain(builder, TOKEN_OWNER,
         signInfo.getSigningCertificate().getNotAfter());
      final SamlTokenSpec spec = builder.createSpec();

      Date issueInstantStartTime = new Date();
      SamlToken token = authority.issueToken(spec);
      Date issueInstantEndTime = new Date();

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;

      checkToken(token, spec, issueInstantStartTime, issueInstantEndTime,
         expectedTokenStartTime, expectedTokenEndTime,
         new com.vmware.vim.sso.PrincipalId[] {}, !renewable);
   }

   private void testIssueHoKTokenWithDelegationChain_SpecEndTime_Int(
      PrincipalId tokenOwner, PrincipalId tokenPresenter)
      throws InvalidTokenException {

      initTokenAttributes(tokenOwner);
      Date delegateAssertionEndTime = new Date(endTime.getTime() + 60 * 1000);
      Builder builder = TokenCreationUtil.createSpecBuilder(tokenPresenter,
         hokCertificate, authnTime, new TimePeriod(startTime, endTime));
      TokenCreationUtil.createDelegationChain(builder, tokenOwner,
         delegateAssertionEndTime);

      SamlTokenSpec spec = builder.createSpec();

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = endTime;
      com.vmware.vim.sso.client.SamlToken samlToken = testIssueToken(spec,
         expectedTokenStartTime, expectedTokenEndTime);
      List<TokenDelegate> chain = spec.getDelegationSpec()
         .getDelegationHistory().getCurrentDelegateList();
      assertEquals(chain.size() + 1, samlToken.getDelegationChain().size());
   }

   private void testIssueHoKTokenWithDelegationChain_DelegateAssertionEndTime_Int(
      PrincipalId tokenOwner, PrincipalId tokenPresenter)
      throws InvalidTokenException {

      initTokenAttributes(tokenOwner);
      Date delegateAssertionEndTime = new Date(endTime.getTime() - 60 * 1000);
      Builder builder = TokenCreationUtil.createSpecBuilder(tokenPresenter,
         hokCertificate, authnTime, new TimePeriod(startTime, endTime));
      TokenCreationUtil.createDelegationChain(builder, tokenOwner,
         delegateAssertionEndTime);

      SamlTokenSpec spec = builder.createSpec();

      Date expectedTokenStartTime = startTime;
      Date expectedTokenEndTime = tokenOwner.equals(tokenPresenter) ? endTime
         : delegateAssertionEndTime;
      com.vmware.vim.sso.client.SamlToken samlToken = testIssueToken(spec,
         expectedTokenStartTime, expectedTokenEndTime);
      List<TokenDelegate> chain = spec.getDelegationSpec()
         .getDelegationHistory().getCurrentDelegateList();
      assertEquals(chain.size() + 1, samlToken.getDelegationChain().size());
   }

   private com.vmware.vim.sso.client.SamlToken testIssueToken(
      SamlTokenSpec spec, Date expectedTokenStartTime, Date expectedTokenEndTime)
      throws InvalidTokenException {

      return testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime,
         spec.getRenewSpec().isRenewable());
   }

   private com.vmware.vim.sso.client.SamlToken testIssueToken(
      SamlTokenSpec spec, Date expectedTokenStartTime,
      Date expectedTokenEndTime, ConfigExtractorFactory configExtractorFactory)
      throws InvalidTokenException {

      return testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime,
         spec.getRenewSpec().isRenewable(), configExtractorFactory);
   }

   private com.vmware.vim.sso.client.SamlToken testIssueToken(
      SamlTokenSpec spec, Date expectedTokenStartTime,
      Date expectedTokenEndTime, boolean renewable)
      throws InvalidTokenException {
      return testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime,
         renewable, null, null, null);
   }

   private com.vmware.vim.sso.client.SamlToken testIssueToken(
      SamlTokenSpec spec, Date expectedTokenStartTime,
      Date expectedTokenEndTime, boolean renewable,
      ConfigExtractorFactory configExtractorFactory)
      throws InvalidTokenException {
      return testIssueToken(spec, expectedTokenStartTime, expectedTokenEndTime,
         renewable, null, null, configExtractorFactory);
   }

   private com.vmware.vim.sso.client.SamlToken testIssueToken(
      SamlTokenSpec spec, Date expectedTokenStartTime,
      Date expectedTokenEndTime, boolean renewable, String identityAttribute,
      String identityAttributeValue,
      ConfigExtractorFactory configExtractorFactory)
      throws InvalidTokenException {

      if (configExtractorFactory == null) {
         configExtractorFactory = this.configExtractorFactory;
      }

      SamlAuthorityFactory factory = new DefaultSamlAuthorityFactory(
         SignatureAlgorithm.RSA_SHA256, attributeExtractorFactory,
         configExtractorFactory);

      authority = factory.createTokenAuthority(DEFAULT_TENANT_NAME);

      Date issueInstantStartTime = new Date();
      SamlToken token = authority.issueToken(spec);
      Date issueInstantEndTime = new Date();

      return checkToken(token, spec, issueInstantStartTime,
         issueInstantEndTime, expectedTokenStartTime, expectedTokenEndTime,
         new com.vmware.vim.sso.PrincipalId[] {}, renewable, identityAttribute,
         identityAttributeValue);
   }

   private void checkIdentityAttributeValue(Document token,
      String identityAttribute, String identityAttributeValue) {
      NodeList nameIdElements = token.getElementsByTagName("saml2:NameID");
      assertEquals(1, nameIdElements.getLength());
      Node nameId = nameIdElements.item(0);

      NamedNodeMap attributes = nameId.getAttributes(); // Format of attribute
      assertEquals(1, attributes.getLength());
      assertEquals(identityAttribute, attributes.item(0).getTextContent());

      assertEquals(1, nameIdElements.getLength());
      assertEquals(identityAttributeValue, nameId.getTextContent());
   }

   private void checkTokenAttributes(Document token,
      Collection<String> attributeNamesInSpec) {
      NodeList nameIdElements = token
         .getElementsByTagName("saml2:AttributeStatement");
      assertEquals(1, nameIdElements.getLength());
      Node attributeStatements = nameIdElements.item(0);

      NodeList attributes = attributeStatements.getChildNodes();

      assertTrue(attributes.getLength() > 0);
      Map<String, String[]> tokenAttributeValues = new HashMap<String, String[]>();

      for (int i = 0; i < attributes.getLength(); i++) {
         Node attribute = attributes.item(i);
         NamedNodeMap attributesOfCurrentAttributeNode = attribute
            .getAttributes();
         assertTrue(attributesOfCurrentAttributeNode.getLength() > 0);

         Node nameNode = attributesOfCurrentAttributeNode.getNamedItem("Name");
         String attributeName = nameNode.getTextContent();

         NodeList currentAttributeChildNodes = attribute.getChildNodes();
         if (currentAttributeChildNodes.getLength() == 1) {
            tokenAttributeValues.put(attributeName,
               new String[] { attribute.getTextContent() });
         } else {
            Set<String> values = new HashSet<String>();
            for (int j = 0; j < currentAttributeChildNodes.getLength(); j++) {
               Node attributeValue = currentAttributeChildNodes.item(j);
               values.add(attributeValue.getTextContent());
            }
            String[] valuesArr = values.toArray(new String[] {});
            tokenAttributeValues.put(attributeName, valuesArr);
         }
      }

      assertEquals(new HashSet<String>(attributeNamesInSpec),
         tokenAttributeValues.keySet());

      checkAttributeValues(tokenAttributeValues);
   }

   private void checkAttributeValues(Map<String, String[]> tokenAttributeValues) {
      // check attribute values
      for (String attributeName : tokenAttributeValues.keySet()) {
         String[] attributeValue = tokenAttributeValues.get(attributeName);
         String[] defaultAttributeValue = this.attributes.get(attributeName);
         Set<String> expected = new HashSet<String>(Arrays.asList(defaultAttributeValue));
         Set<String> actual = new HashSet<String>(Arrays.asList(attributeValue));
         boolean defaultValuesMatches = attributeValue != null
            && defaultAttributeValue != null
            && expected.equals(actual);

         if (!defaultValuesMatches) {
            fail("Attribute name: " + attributeName + " "
               + Arrays.toString(attributeValue)
               + " is attribute value, but default is "
               + Arrays.toString(defaultAttributeValue));
         }
      }
   }

   private com.vmware.vim.sso.client.SamlToken checkToken(SamlToken token,
      SamlTokenSpec spec, Date issueInstantStartTime, Date issueInstantEndTime,
      Date expectedTokenStartTime, Date expectedTokenEndTime,
      com.vmware.vim.sso.PrincipalId[] groups, boolean renewable)
      throws InvalidTokenException {
      return checkToken(token, spec, issueInstantStartTime,
         issueInstantEndTime, expectedTokenStartTime, expectedTokenEndTime,
         groups, renewable, null, null);
   }

   private com.vmware.vim.sso.client.SamlToken checkToken(SamlToken token,
      SamlTokenSpec spec, Date issueInstantStartTime, Date issueInstantEndTime,
      Date expectedTokenStartTime, Date expectedTokenEndTime,
      com.vmware.vim.sso.PrincipalId[] groups, boolean renewable,
      String identityAttribute, String identityAttributeValue)
      throws InvalidTokenException {
      assertNotNull(token);

      com.vmware.vim.sso.client.SamlToken samlToken = DefaultTokenFactory
         .createTokenFromDom(token.getDocument().getDocumentElement(),
            trustedRootCA);

      assertEquals(
         AdviceFilter.processRequest(spec.requesterIsTokenOwner(),
            spec.getRequestedAdvice(), spec.getPresentAdvice()),
         toSamlAuthorityAdvices(samlToken.getAdvice()));
      assertEquals(spec.getAudience(), samlToken.getAudience());

      if (spec.getConfirmationData().getType() == SamlTokenSpec.ConfirmationType.BEARER) {
         assertEquals(ConfirmationType.BEARER, samlToken.getConfirmationType());
         assertNull(samlToken.getConfirmationCertificate());
      } else {
         assertEquals(ConfirmationType.HOLDER_OF_KEY,
            samlToken.getConfirmationType());
         X509Certificate hokCert = spec.getConfirmationData().getCertificate();
         assertEquals(hokCert, samlToken.getConfirmationCertificate());
      }

      checkTokenTime(spec, issueInstantStartTime, issueInstantEndTime,
         samlToken, expectedTokenStartTime, expectedTokenEndTime);

      assertEquals(groups.length, samlToken.getGroupList().size());
      assertTrue(samlToken.getGroupList().containsAll(Arrays.asList(groups)));

      boolean isExpectedTokenSubjectIdentity;
      if (identityAttribute == null) {
         isExpectedTokenSubjectIdentity = samePrincipal(
            spec.requesterIsTokenOwner() ? spec.getAuthenticationData()
               .getPrincipalId() : spec.getDelegationSpec()
               .getDelegationHistory().getTokenSubject(),
            samlToken.getSubject());
      } else {
         isExpectedTokenSubjectIdentity = sameSubject(identityAttributeValue,
            samlToken.getSubject());
      }

      assertTrue(isExpectedTokenSubjectIdentity);

      checkDelegation(spec.getDelegationSpec(), samlToken);
      assertEquals(renewable, samlToken.isRenewable());
      assertEquals(samlToken.isDelegable(), token.isDelegable());

      checkTokenAttributes(token.getDocument(), spec.getAttributeNames());

      checkSignatureAlgorithm(spec, samlToken);

      assertEquals(samlToken.getConfirmationType().name(), token
         .getConfirmationType().name());

      return samlToken;
   }

   private void checkDelegation(DelegationSpec delegationSpec,
      com.vmware.vim.sso.client.SamlToken samlToken) {

      assertEquals(delegationSpec.isDelegable(), samlToken.isDelegable());
      List<com.vmware.vim.sso.client.SamlToken.TokenDelegate> delegationChain = samlToken
         .getDelegationChain();

      List<TokenDelegate> expDelegateList = new ArrayList<TokenDelegate>();
      if (delegationSpec.getDelegationHistory() != null) {
         expDelegateList.addAll(delegationSpec.getDelegationHistory()
            .getCurrentDelegateList());
      }
      if (delegationSpec.getDelegate() != null) {
         expDelegateList.add(new TokenDelegate(delegationSpec.getDelegate(),
            new Date()));
      }
      assertEquals(expDelegateList.size(), delegationChain.size());
      for (int i = 0; i < delegationChain.size(); i++) {
         final com.vmware.vim.sso.client.SamlToken.TokenDelegate delegate = delegationChain
            .get(i);
         final TokenDelegate expDelegate = expDelegateList.get(i);
         assertNotNull(delegate);
         assertNotNull(expDelegate);
         samePrincipal(expDelegate.getSubject(), delegate.getSubject());
      }
   }

   private void checkTokenTime(SamlTokenSpec spec, Date issueInstantStartTime,
      Date issueInstantEndTime, com.vmware.vim.sso.client.SamlToken samlToken,
      Date expectedStartTime, Date expectedEndTime) {

      String errMsg = "If this fails, issueing of token is faster than getting to the point of issue in samlauthority";
      long issueRangeMidTimeMillis = (issueInstantStartTime.getTime() + issueInstantEndTime
         .getTime()) / 2;

      checkTokenEndTime(spec, samlToken, expectedEndTime,
         issueRangeMidTimeMillis, errMsg);

      checkTokenStartTime(spec, samlToken, expectedStartTime,
         issueRangeMidTimeMillis, errMsg);
   }

   private void checkTokenEndTime(SamlTokenSpec spec,
      com.vmware.vim.sso.client.SamlToken samlToken, Date expectedEndTime,
      long issueRangeMidTimeMillis, String errMsg) {
      boolean noDesiredEndTimeInSpec = spec.getLifespan().getEndTime() == null;

      if (noDesiredEndTimeInSpec) {
         TimePeriod expectedEndTimePlusTolerance = new TimePeriod(
            expectedEndTime, issueRangeMidTimeMillis);

         assertTrue(errMsg, expectedEndTimePlusTolerance.contains(samlToken
            .getExpirationTime()));
      } else {
         assertEquals(expectedEndTime, samlToken.getExpirationTime());
      }
   }

   private void checkTokenStartTime(SamlTokenSpec spec,
      com.vmware.vim.sso.client.SamlToken samlToken, Date expectedStartTime,
      long issueRangeMidTimeMillis, String errMsg) {
      boolean noDesiredStartTimeInSpec = spec.getLifespan().getStartTime() == null;

      if (noDesiredStartTimeInSpec) {
         TimePeriod expectedStartTimePlusTolerance = new TimePeriod(
            expectedStartTime, issueRangeMidTimeMillis);

         assertTrue(errMsg,
            expectedStartTimePlusTolerance.contains(samlToken.getStartTime()));
      } else {
         assertEquals(expectedStartTime, samlToken.getStartTime());
      }
   }

   private void checkSignatureAlgorithm(SamlTokenSpec spec,
      com.vmware.vim.sso.client.SamlToken token) {
      SignatureAlgorithm signatureAlgorithm = null;
      String signatureAlgorithmInConfig = configExtractor.getConfig()
         .getSamlAuthorityConfig().getSignatureAlgorithm();
      if (signatureAlgorithmInConfig != null) {
         signatureAlgorithm = SignatureAlgorithm
            .getSignatureAlgorithmForURI(signatureAlgorithmInConfig);
      }
      if (signatureAlgorithm == null) {
         signatureAlgorithm = defaultSignatureAlgorithm;
      }
      if (spec.getSignatureAlgorithm() != null) {
         signatureAlgorithm = spec.getSignatureAlgorithm();
      }

      assertTrue(token.toXml().contains(signatureAlgorithm.toString()));
   }

   private ConfigExtractorFactory initializeConfigExtractor() {
      return initializeConfigExtractor(null, signInfo, certPathList);
   }

   private ConfigExtractorFactory initializeConfigExtractor(
      TokenRestrictions tokenRestrictions) {
      return initializeConfigExtractor(tokenRestrictions, signInfo,
         certPathList);
   }

   private ConfigExtractorFactory initializeConfigExtractor(
      SignatureAlgorithm signatureAlgorithm) {
      return initializeConfigExtractor(null, signInfo, certPathList,
         signatureAlgorithm.toString());
   }

   private ConfigExtractorFactory initializeConfigExtractor(
      TokenRestrictions tokenRestrictions, SignInfo signInfo,
      List<Certificate> certPathList) {
      return initializeConfigExtractor(tokenRestrictions, signInfo,
         certPathList, null);
   }

   private ConfigExtractorFactory initializeConfigExtractor(
      TokenRestrictions tokenRestrictions, SignInfo signInfo,
      List<Certificate> certPathList, String signatureAlgorithm) {
      ConfigExtractorFactory configExtractorFactory = createMock(ConfigExtractorFactory.class);
      configExtractor = createMock(ConfigExtractor.class);
      expect(configExtractorFactory.getConfigExtractor(eq(DEFAULT_TENANT_NAME)))
         .andReturn(configExtractor).anyTimes();

      Collection<List<Certificate>> validCerts = new HashSet<List<Certificate>>();
      validCerts.add(certPathList);

      @SuppressWarnings("unchecked")
      Config config = new Config(new SamlAuthorityConfiguration(ISSUER,
         (List<Certificate>) signInfo.getCertificationPath().getCertificates(),
         signInfo.getPrivateKey(), signatureAlgorithm),
         tokenRestrictions != null ? tokenRestrictions
            : defaultTokenRestrictions, validCerts, DEFAULT_CLOCK_TOLERANCE,
            Arrays.<IDPConfig>asList(new IDPConfig(TestConstants.EXTERNAL_ISSUER)));
      expect(configExtractor.getConfig()).andReturn(config).anyTimes();

      replay(configExtractor, configExtractorFactory);

      return configExtractorFactory;
   }

   private PrincipalAttributesExtractorFactory initializeAttributeExtractor(
      PrincipalAttributesExtractor extractor) {
      PrincipalAttributesExtractorFactory attributeExtractorFactory = createMock(PrincipalAttributesExtractorFactory.class);
      expect(
         attributeExtractorFactory
            .getPrincipalAttributesExtractor(eq(DEFAULT_TENANT_NAME)))
         .andReturn(extractor).anyTimes();
      replay(attributeExtractorFactory);

      return attributeExtractorFactory;
   }

   private boolean samePrincipal(PrincipalId first,
      com.vmware.vim.sso.PrincipalId second) {
      return first.getName().equals(second.getName())
         && first.getDomain().equals(second.getDomain());
   }

   private boolean sameSubject(String identityAttributeValue,
      com.vmware.vim.sso.PrincipalId subject) {
      return (subject.getName() + "@" + subject.getDomain())
         .equals(identityAttributeValue);

   }

   private List<com.vmware.identity.saml.Advice> toSamlAuthorityAdvices(
      List<Advice> advices) {
      assert advices != null;
      final List<com.vmware.identity.saml.Advice> result = new ArrayList<com.vmware.identity.saml.Advice>();
      for (Advice advice : advices) {
         result.add(new com.vmware.identity.saml.Advice(advice.getSource(),
            toSamlAuthorityAttributes(advice.getAttributes())));
      }
      return result;
   }

   private List<Attribute> toSamlAuthorityAttributes(
      List<AdviceAttribute> attributes) {
      assert attributes != null;
      final List<Attribute> result = new ArrayList<Attribute>();
      for (AdviceAttribute attribute : attributes) {
         result.add(new Attribute(attribute.getName(), null, attribute
            .getValue()));
      }
      return result;
   }

   private String toNetbios(final String name, final String domain) {
      return domain + '\\' + name;
   }

   private void initTokenAttributes(String upn, String email) {
      attributes = new HashMap<String, String[]>();

      String FIRST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
      String LAST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
      String SUBJECT_TYPE = "http://vmware.com/schemas/attr-names/2011/07/isSolution";
      String UPN = "http://schemas.xmlsoap.org/claims/UPN";
      String EMAIL = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress";
      String GROUP = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";

      String subjectTypeValue = "false";
      String firstNameValue = "Admin";
      String lastNameValue = "Admin";

      attributes.put(FIRST_NAME, new String[] { firstNameValue });
      attributes.put(LAST_NAME, new String[] { lastNameValue });
      attributes.put(SUBJECT_TYPE, new String[] { subjectTypeValue });
      attributes.put(UPN, new String[] { upn });
      attributes.put(EMAIL, new String[] { email });
      attributes.put(GROUP, new String[] {});

   }

   private void initTokenAttributes(PrincipalId tokenSubject) {
      initTokenAttributes(
         tokenSubject.getName() + '@' + tokenSubject.getDomain(), "AdminEmail@"
            + tokenSubject.getDomain());
   }

   private void initTokenAttributes() {
      initTokenAttributes(new PrincipalId("user", "example.com"));
   }

}
