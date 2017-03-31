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
package com.vmware.identity.sts.impl;

import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.ObjectFactory;

import org.easymock.EasyMock;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AdviceSetType;
import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AdviceType;
import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AttributeType;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.Advice.Attribute;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData;
import com.vmware.identity.saml.SamlTokenSpec.Builder;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.CertificateUtil;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.Request.CertificateLocation;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.util.TimePeriod;

/**
 * Unit tests for SamlTokenSpecBuilder
 */
public final class SamlTokenSpecBuilderTest {

   private static final String HTTP_SCHEMAS_XMLSOAP_ORG_CLAIMS_UPN = "http://schemas.xmlsoap.org/claims/UPN";

   private static final String FIRST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
   private static final String LAST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
   private static final String GROUPS = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";
   private static final String SUBJECT_TYPE = "http://vmware.com/schemas/attr-names/2011/07/isSolution";
   private final Collection<String> attrNames = Arrays.asList(FIRST_NAME,
      LAST_NAME, GROUPS, SUBJECT_TYPE);

   private static final PrincipalId SUBJECT = new PrincipalId("Solution",
      "xyz.com");
   private static final PrincipalId DELEGATE = new PrincipalId(
      "DelegateSolution", "abv.org");
   private static final PrincipalId DELEGATE2 = new PrincipalId(
      "OtherDelegateSolution", "abv.uk.com");
   private static final PrincipalId NO_DELEGATETO = null;

   private static final X509Certificate SUBJECT_CERT = new CertificateUtil(
      CertificateUtil.SOLUTION_STORE_JKS, CertificateUtil.PASSWORD)
      .loadCert(CertificateUtil.SOLUTION_CERT_ALIAS);
   private static final X509Certificate DELEGATE_CERT = new CertificateUtil(
      CertificateUtil.STS_STORE_JKS, CertificateUtil.PASSWORD)
      .loadCert(CertificateUtil.STS_CERT_ALIAS);

   private static final Collection<String> AUDIENCE = Arrays.asList("vim:sol1",
      "vim:sol2", "vim:sol3");
   private static final Collection<String> NO_AUDIENCE = Collections.emptySet();

   /* advice constants */
   private static final String ADVICE_SOURCE_URI1 = "advice_sourceURI1";
   private static final String ADVICE_SOURCE_URI2 = "advice_sourceURI2";
   private static final String ADVICE_ATTR_NAME1 = "advice_attr_name";
   private static final String ADVICE_ATTR_FRIENDLY_NAME1 = "attr1_friendly_name";
   private static final String ADVICE_ATTR_VALUE1 = "attr1_value";
   private static final String ADVICE_ATTR_VALUE2 = "attr2_value";

   private static final Attribute ATTRIBUTE1 = new Attribute(ADVICE_ATTR_NAME1,
      ADVICE_ATTR_FRIENDLY_NAME1, Arrays.asList(ADVICE_ATTR_VALUE1,
         ADVICE_ATTR_VALUE2));

   /* no advice constants */
   private static final AdviceSetType NO_RST_ADVICE = null;
   private static final List<Advice> NO_TOKEN_ADVICE = Collections
      .emptyList();
   private static final List<Advice> NO_ADVICE = Collections.emptyList();

   /* advice1 constants */
   private static final Advice ADVICE1 = new Advice(ADVICE_SOURCE_URI1,
      Arrays.asList(ATTRIBUTE1));
   private static final AdviceType ADVICE1_RST = new AdviceType();
   static {
      final Advice adviceOrig = ADVICE1;
      final AdviceType advice = ADVICE1_RST;
      advice.setAdviceSource(adviceOrig.sourceURI());
      {
         final Attribute advice1Attr = adviceOrig.attributes().get(0);
         final AttributeType attr = new AttributeType();
         attr.setName(advice1Attr.nameURI());
         attr.setFriendlyName(advice1Attr.friendlyName());
         attr.getAttributeValue().addAll(advice1Attr.values());
         advice.getAttribute().add(attr);
      }
   }
   private static final Advice ADVICE1_TOKEN = new Advice(
      ADVICE1.sourceURI(),
      Arrays.asList(new Advice.Attribute(
         ATTRIBUTE1.nameURI(), ATTRIBUTE1.friendlyName(), ATTRIBUTE1.values())));
   private static final Advice ADVICE2 = new Advice(ADVICE_SOURCE_URI2,
      Arrays.asList(ATTRIBUTE1));
   private static final Advice ADVICE2_TOKEN = new Advice(
      ADVICE2.sourceURI(),
      Arrays.asList(new Advice.Attribute(
         ATTRIBUTE1.nameURI(), ATTRIBUTE1.friendlyName(), ATTRIBUTE1.values())));

   private static final PrincipalDiscovery principalDiscovery = new PrincipalDiscoveryMock(
      DELEGATE, DELEGATE_CERT);

   private static final boolean RENEW = true;
   private static final boolean RENEWABLE = true;

   private static final int REMAINING_DELEGATIONS = 3;

   private TokenValidator validator;
   private Date reqStartTime = new Date();
   private Date authnTime = shift(reqStartTime, 1000);
   private Date reqEndTime = shift(reqStartTime, 1000000);
   private Date tokenExpiresOn = shift(reqEndTime, 100);
   private Date prevoiusDelegationAt = shift(reqStartTime, -1000);

   @Before
   public void setup() {
      validator = EasyMock.createMock(TokenValidator.class);
      reqStartTime = new Date();
      authnTime = shift(reqStartTime, 1000);
      reqEndTime = shift(reqStartTime, 1000000);
      tokenExpiresOn = shift(reqEndTime, 100);
      prevoiusDelegationAt = shift(reqStartTime, -1000);
   }

   @Test
   public void testRenewNotDelegatedTokenAssertionOK() {
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION);
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION,
         SignatureAlgorithm.RSA_SHA512);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION,
         SignatureAlgorithm.RSA_SHA512);
   }

   @Test
   public void testRenewNotDelegatedTokenBSTOK() {
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG);
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG, SignatureAlgorithm.RSA_SHA256);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG, SignatureAlgorithm.RSA_SHA256);
   }

   @Test
   public void testRenewNotDelegatedTokenUserCertOK() {
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.SMARTCARD,
         AuthenticationData.AuthnMethod.SMARTCARD);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.SMARTCARD,
         AuthenticationData.AuthnMethod.SMARTCARD);
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.SMARTCARD,
         AuthenticationData.AuthnMethod.SMARTCARD, SignatureAlgorithm.RSA_SHA256);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.SMARTCARD,
         AuthenticationData.AuthnMethod.SMARTCARD, SignatureAlgorithm.RSA_SHA256);
   }

   @Test
   public void testRenewNotDelegatedTokenSecurIDOK() {
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.TIMESYNCTOKEN,
         AuthenticationData.AuthnMethod.TIMESYNCTOKEN);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.TIMESYNCTOKEN,
         AuthenticationData.AuthnMethod.TIMESYNCTOKEN);
      testRenewOKInt(SUBJECT, SUBJECT_CERT, SUBJECT, AuthnMethod.TIMESYNCTOKEN,
         AuthenticationData.AuthnMethod.TIMESYNCTOKEN, SignatureAlgorithm.RSA_SHA256);
      testRenewOKInt(DELEGATE, DELEGATE_CERT, DELEGATE, AuthnMethod.TIMESYNCTOKEN,
         AuthenticationData.AuthnMethod.TIMESYNCTOKEN, SignatureAlgorithm.RSA_SHA256);
   }

   @Test
   public void testRenewDelegatedTokenAssertionOK() {
      testRenewOKInt(SUBJECT, DELEGATE_CERT, DELEGATE, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION);
      testRenewOKInt(DELEGATE, SUBJECT_CERT, SUBJECT, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION);
      testRenewOKInt(SUBJECT, DELEGATE_CERT, DELEGATE, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION,
         SignatureAlgorithm.RSA_SHA256);
      testRenewOKInt(DELEGATE, SUBJECT_CERT, SUBJECT, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION,
         SignatureAlgorithm.RSA_SHA256);
   }

   @Test
   public void testRenewDelegatedTokenBSTOK() {
      testRenewOKInt(SUBJECT, DELEGATE_CERT, DELEGATE, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG);
      testRenewOKInt(DELEGATE, SUBJECT_CERT, SUBJECT, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG);
      testRenewOKInt(SUBJECT, DELEGATE_CERT, DELEGATE, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG, SignatureAlgorithm.RSA_SHA256);
      testRenewOKInt(DELEGATE, SUBJECT_CERT, SUBJECT, AuthnMethod.DIG_SIG,
         AuthenticationData.AuthnMethod.XMLDSIG, SignatureAlgorithm.RSA_SHA256);
   }

   @Test
   public void testRenewTokenWithAudienceOK() {
      testRenewOKInt(DELEGATE, SUBJECT_CERT, SUBJECT, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION,
         SignatureAlgorithm.RSA_SHA256, AUDIENCE, NO_RST_ADVICE,
         NO_TOKEN_ADVICE, NO_ADVICE, NO_ADVICE);
   }

   @Test
   public void testRenewTokenAdviceOK() {
      // present advice is interpreted as requested advice
      final AdviceSetType adviceRstSet = new AdviceSetType();
      adviceRstSet.getAdvice().add(ADVICE1_RST);
      testRenewOKInt(DELEGATE, SUBJECT_CERT, SUBJECT, AuthnMethod.ASSERTION,
         AuthenticationData.AuthnMethod.ASSERTION,
         SignatureAlgorithm.RSA_SHA256, AUDIENCE, adviceRstSet,
         Arrays.asList(ADVICE2_TOKEN), Arrays.asList(ADVICE2),
         Arrays.asList(ADVICE2));
   }

   @Test
   public void testIssueNotDelegatedTokenOK() {
      testIssueOKInt(SUBJECT, SUBJECT_CERT, NO_DELEGATETO, SUBJECT);
      testIssueOKInt(SUBJECT, SUBJECT_CERT, NO_DELEGATETO, SUBJECT,
         SignatureAlgorithm.RSA_SHA256);
   }

   @Test
   public void testIssueDelegatedTokenOK() {
      testIssueOKInt(SUBJECT, DELEGATE_CERT, NO_DELEGATETO, DELEGATE);
      testIssueOKInt(SUBJECT, DELEGATE_CERT, NO_DELEGATETO, DELEGATE,
         SignatureAlgorithm.RSA_SHA512);
   }

   @Test
   public void testDelegateDelegatedTokenOK() {
      testIssueOKInt(SUBJECT, DELEGATE_CERT, DELEGATE, DELEGATE2);
      testIssueOKInt(SUBJECT, DELEGATE_CERT, DELEGATE, DELEGATE2,
         SignatureAlgorithm.RSA_SHA512);
   }

   @Test
   public void testIssueTokenWithAudienceOK() {
      testIssueOKInt(SUBJECT, SUBJECT_CERT, NO_DELEGATETO, SUBJECT,
         SignatureAlgorithm.RSA_SHA256, AUDIENCE, NO_RST_ADVICE,
         NO_TOKEN_ADVICE, NO_ADVICE, NO_ADVICE);
   }

   @Test
   public void testIssueTokenReqAdviceOK() {
      final AdviceSetType adviceRstSet = new AdviceSetType();
      adviceRstSet.getAdvice().add(ADVICE1_RST);
      testIssueOKInt(SUBJECT, SUBJECT_CERT, NO_DELEGATETO, SUBJECT,
         SignatureAlgorithm.RSA_SHA256, AUDIENCE, adviceRstSet,
         NO_TOKEN_ADVICE, Arrays.asList(ADVICE1), NO_ADVICE);
   }

   @Test
   public void testIssueTokenPresentAdviceOK() {
      testIssueOKInt(SUBJECT, SUBJECT_CERT, NO_DELEGATETO, SUBJECT,
         SignatureAlgorithm.RSA_SHA256, AUDIENCE, NO_RST_ADVICE,
         Arrays.asList(ADVICE1_TOKEN), NO_ADVICE, Arrays.asList(ADVICE1));
   }

   @Test
   public void testIssueTokenBothAdviceOK() {
      final AdviceSetType adviceRstSet = new AdviceSetType();
      adviceRstSet.getAdvice().add(ADVICE1_RST);
      testIssueOKInt(SUBJECT, SUBJECT_CERT, NO_DELEGATETO, SUBJECT,
         SignatureAlgorithm.RSA_SHA256, AUDIENCE, adviceRstSet,
         Arrays.asList(ADVICE2_TOKEN), Arrays.asList(ADVICE1),
         Arrays.asList(ADVICE2));
   }

   private void testIssueOKInt(PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId delegateTo,
      PrincipalId authPrincipal) {
      testIssueOKInt(authTokenSubject, confCert, delegateTo, authPrincipal,
         null);
   }

   private void testIssueOKInt(PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId delegateTo,
      PrincipalId authPrincipal, SignatureAlgorithm requestedSignatureAlgo) {
      testIssueOKInt(authTokenSubject, confCert, delegateTo, authPrincipal,
         requestedSignatureAlgo, NO_AUDIENCE, NO_RST_ADVICE, NO_TOKEN_ADVICE,
         NO_ADVICE, NO_ADVICE);
   }

   private void testIssueOKInt(PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId delegateTo,
      PrincipalId authPrincipal, SignatureAlgorithm requestedSignatureAlgo,
      Collection<String> audience, AdviceSetType rstAdvice,
      List<Advice> tokenAdvice,
      List<Advice> reqAdviceExp, List<Advice> presentAdviceExp) {
      assert authTokenSubject != null && authPrincipal != null
         && confCert != null && audience != null && tokenAdvice != null
         && reqAdviceExp != null && presentAdviceExp != null;

      final PrincipalId delegate = authTokenSubject.equals(authPrincipal) ? null
         : authPrincipal;
      final SamlTokenSpec expSpec = buildExpectedSpec(reqStartTime, reqEndTime,
         requestedSignatureAlgo, authPrincipal,
         AuthenticationData.AuthnMethod.ASSERTION, authnTime, confCert,
         authTokenSubject, tokenExpiresOn, delegate, prevoiusDelegationAt,
         delegateTo, false, audience, reqAdviceExp, presentAdviceExp);

      final Request req = buildIssueReq(reqStartTime, reqEndTime,
         requestedSignatureAlgo, authTokenSubject, confCert, delegate,
         tokenExpiresOn, prevoiusDelegationAt, delegateTo, audience, rstAdvice,
         tokenAdvice);
      final Result authResult = new Result(authPrincipal, authnTime,
         AuthnMethod.ASSERTION);
      final SamlTokenSpec spec = new SamlTokenSpecBuilder(new DelegationParser(
         principalDiscovery), validator).buildIssueTokenSpec(req, authResult);

      Assert.assertEquals(expSpec, spec);
   }

   private void testRenewOKInt(PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId authPrincipal,
      AuthnMethod authnMethod,
      SamlTokenSpec.AuthenticationData.AuthnMethod expAuthnMethod) {
      testRenewOKInt(authTokenSubject, confCert, authPrincipal, authnMethod,
         expAuthnMethod, null);
   }

   private void testRenewOKInt(PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId authPrincipal,
      AuthnMethod authnMethod,
      SamlTokenSpec.AuthenticationData.AuthnMethod expAuthnMethod,
      SignatureAlgorithm requestedSignatureAlgo) {
      testRenewOKInt(authTokenSubject, confCert, authPrincipal, authnMethod,
         expAuthnMethod, requestedSignatureAlgo, NO_AUDIENCE, NO_RST_ADVICE,
         NO_TOKEN_ADVICE, NO_ADVICE, NO_ADVICE);
   }

   private void testRenewOKInt(PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId authPrincipal,
      AuthnMethod authnMethod,
      SamlTokenSpec.AuthenticationData.AuthnMethod expAuthnMethod,
      SignatureAlgorithm requestedSignatureAlgo, Collection<String> audience,
      AdviceSetType rstAdvice,
      List<Advice> tokenAdvice,
      List<Advice> reqAdviceExp, List<Advice> presentAdviceExp) {

      assert authTokenSubject != null && authPrincipal != null
         && confCert != null && audience != null && tokenAdvice != null
         && reqAdviceExp != null && presentAdviceExp != null;

      final PrincipalId delegate = authTokenSubject.equals(authPrincipal) ? null
         : authPrincipal;
      final SamlTokenSpec expSpec = buildExpectedSpec(reqStartTime, reqEndTime,
         requestedSignatureAlgo, authPrincipal, expAuthnMethod, authnTime,
         confCert, authTokenSubject, tokenExpiresOn, delegate,
         prevoiusDelegationAt, null, RENEW, audience, reqAdviceExp,
         presentAdviceExp);

      final Request req = buildRenewReq(reqStartTime, reqEndTime,
         requestedSignatureAlgo, authTokenSubject, confCert, delegate,
         tokenExpiresOn, prevoiusDelegationAt, audience, rstAdvice, tokenAdvice);
      final Result authResult = new Result(authPrincipal, authnTime,
         authnMethod);
      final SamlTokenSpec spec = new SamlTokenSpecBuilder(new DelegationParser(
         principalDiscovery), validator).buildRenewTokenSpec(req, authResult);
      Assert.assertEquals(expSpec, spec);
   }

   private SamlTokenSpec buildExpectedSpec(Date reqStartTime, Date reqEndTime,
      SignatureAlgorithm requestedSignatureAlgo, PrincipalId authPrincipal,
      SamlTokenSpec.AuthenticationData.AuthnMethod authnMethod, Date authnTime,
      X509Certificate confCert, PrincipalId authTokenSubject,
      Date tokenExpiresOn, PrincipalId delegate, Date prevoiusDelegationAt,
      PrincipalId delegateTo, boolean renew, Collection<String> audience,
      List<Advice> reqAdvice, List<Advice> presentAdvice) {

      assert reqAdvice != null && presentAdvice != null;
      final AuthenticationData authN = new AuthenticationData(authPrincipal,
         authnTime, authnMethod, HTTP_SCHEMAS_XMLSOAP_ORG_CLAIMS_UPN);

      final SamlTokenSpec.DelegationSpec delegationSpec = new SamlTokenSpec.DelegationSpec(
         delegateTo,
         false,
         new SamlTokenSpec.DelegationSpec.DelegationHistory(
            authTokenSubject,
            Arrays
               .asList((delegate == null) ? new SamlTokenSpec.TokenDelegate[] {}
                  : new SamlTokenSpec.TokenDelegate[] { new SamlTokenSpec.TokenDelegate(
                     delegate, prevoiusDelegationAt) }), REMAINING_DELEGATIONS,
            tokenExpiresOn));

      final Builder builder = new SamlTokenSpec.Builder(new TimePeriod(
         reqStartTime, reqEndTime), new Confirmation(confCert), authN,
         attrNames).setDelegationSpec(delegationSpec).setRenewSpec(
         new SamlTokenSpec.RenewSpec(RENEWABLE, renew, 0));

      if (requestedSignatureAlgo != null) {
         builder.setSignatureAlgorithm(requestedSignatureAlgo);
      }

      for (String audienceParty : audience) {
         builder.addAudience(audienceParty);
      }

      for (Advice advice : reqAdvice) {
         builder.addRequestedAdvice(advice);
      }

      for (Advice advice : presentAdvice) {
         builder.addPresentAdvice(advice);
      }
      return builder.createSpec();
   }

   private Request buildIssueReq(Date reqStartTime, Date reqEndTime,
      SignatureAlgorithm requestedSignatureAlgo, PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId delegate, Date tokenExpiresOn,
      Date previosDelegationAt, PrincipalId delegateTo,
      Collection<String> audience, AdviceSetType rstAdvice,
      List<Advice> tokenAdvice) {

      assert tokenAdvice != null;
      final RequestSecurityTokenType rst = TestSetup.createRST(
         delegateTo == null ? null : delegateTo.getName(), null,
         requestedSignatureAlgo, audience);
      TestSetup.setLifetime(rst, reqStartTime, reqEndTime);
      rst.setAdviceSet(rstAdvice);
      final SecurityHeaderType header = new SecurityHeaderType();
      ObjectFactory ObjectFactory = new ObjectFactory();
      header.getAny().add(ObjectFactory.createAssertion(TestSetup.createAssertion(REMAINING_DELEGATIONS)));
      final ServerValidatableSamlToken token = newTokenMock(authTokenSubject,
         confCert, delegate, tokenExpiresOn, previosDelegationAt, audience,
         tokenAdvice);
      return new Request(header, rst, new Signature(confCert,
         CertificateLocation.ASSERTION), token, null);
   }

   private Request buildRenewReq(Date reqStartTime, Date reqEndTime,
      SignatureAlgorithm requestedSignatureAlgo, PrincipalId authTokenSubject,
      X509Certificate confCert, PrincipalId delegate, Date tokenExpiresOn,
      Date previousDelegationAt, Collection<String> audience,
      AdviceSetType rstAdvice,
      List<Advice> tokenAdvice) {

      assert tokenAdvice != null;
      final AssertionType renewAssertion = TestSetup
         .createAssertion(REMAINING_DELEGATIONS);
      final RequestSecurityTokenType rst = TestSetup.createRST(null,
         renewAssertion, requestedSignatureAlgo, NO_AUDIENCE);
      TestSetup.setLifetime(rst, reqStartTime, reqEndTime);
      rst.setAdviceSet(rstAdvice);
      final ServerValidatableSamlToken token = newTokenMock(authTokenSubject,
         confCert, delegate, tokenExpiresOn, previousDelegationAt, audience,
         tokenAdvice);
      return new Request(new SecurityHeaderType(), rst, new Signature(confCert,
         CertificateLocation.BST), token, null);
   }

   private ServerValidatableSamlToken newTokenMock(PrincipalId subject,
      X509Certificate confCert, PrincipalId delegate, Date expiresOn,
      Date prevoiusDelegationAt, Collection<String> audience,
      List<Advice> advice) {

      final SamlTokenMock token = (delegate == null) ? new SamlTokenMock(
         subject.getName(), subject.getDomain(), confCert, expiresOn)
         : new SamlTokenMock(subject.getName(), subject.getDomain(), confCert,
            expiresOn, delegate.getName(), delegate.getDomain(),
            prevoiusDelegationAt, 1);
      token.addAudience(audience);
      token.addAdvice(advice);
      return token;
   }

   private static Date shift(Date date, int shift) {
      return new Date(date.getTime() + shift);
   }
}
