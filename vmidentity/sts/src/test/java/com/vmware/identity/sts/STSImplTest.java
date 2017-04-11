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
package com.vmware.identity.sts;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.security.cert.X509Certificate;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import junit.framework.Assert;
import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.ConditionsType;

import org.junit.Before;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.DelegateToType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.LifetimeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.UseKeyType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ValidateTargetType;
import org.oasis_open.docs.ws_sx.ws_trust._200802.ActAsType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.AttributedDateTime;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.ObjectFactory;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0.TimestampType;
import org.w3._2000._09.xmldsig_.SignatureType;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SamlToken;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData;
import com.vmware.identity.saml.SamlTokenSpec.Builder;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec.DelegationHistory;
import com.vmware.identity.saml.SamlTokenSpec.RenewSpec;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SamlTokenDelegate;
import com.vmware.identity.saml.ServerValidatableSamlToken.Subject;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.Request.CertificateLocation;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;
import com.vmware.identity.sts.idm.InvalidPrincipalException;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.STSConfigExtractor;
import com.vmware.identity.sts.idm.STSConfiguration;
import com.vmware.identity.sts.idm.SsoStatisticsService;
import com.vmware.identity.sts.impl.DelegationParser;
import com.vmware.identity.sts.impl.STSImpl;
import com.vmware.identity.sts.util.PrincipalIdConvertor;
import com.vmware.identity.sts.ws.WsConstants;
import com.vmware.identity.util.TimePeriod;

public class STSImplTest {

   private static final String DEFAULT_TIMEZONE = "GMT";
   private static final String DATETIME_FORMAT = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'";

   private final static String UPN = "http://schemas.xmlsoap.org/claims/UPN";
   private final static String FIRST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
   private final static String LAST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
   private final static String GROUPS = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";
   private final static String SUBJECT_TYPE = "http://vmware.com/schemas/attr-names/2011/07/isSolution";

   private final Collection<String> attributeNames = Arrays
      .asList(new String[] { FIRST_NAME, LAST_NAME, GROUPS, SUBJECT_TYPE });

   private static final String actAsGroupName = "ActAsUsers";

   private STS sts;
   private TokenAuthority tokenAuthority;
   private TokenValidator tokenValidator;
   private TokenValidator authOnlyTokenValidator;
   private Authenticator authenticator;
   private DelegationParser delegationHandler;
   private PrincipalDiscovery principalDiscovery;
   private SsoStatisticsService ssoStatistics;
   private Date createdTime;
   private Date expiresTime;
   private Date authenticationTime;
   private STSConfigExtractor configExtractor;

   @Before
   public void init() {
      tokenAuthority = createMock(TokenAuthority.class);
      tokenValidator = createMock(TokenValidator.class);
      authOnlyTokenValidator = createMock(TokenValidator.class);
      authenticator = createMock(Authenticator.class);
      principalDiscovery = createMock(PrincipalDiscovery.class);
      delegationHandler = new DelegationParser(principalDiscovery);
      ssoStatistics = createMock(SsoStatisticsService.class);
      STSConfiguration config = new STSConfiguration(0);
      configExtractor = createMock(STSConfigExtractor.class);
      expect(configExtractor.getConfig()).andReturn(config).anyTimes();
      replay(configExtractor);
      sts = new STSImpl(tokenAuthority, tokenValidator, authOnlyTokenValidator, authenticator,
         delegationHandler, configExtractor, principalDiscovery, ssoStatistics);

      Calendar cal = Calendar.getInstance();
      createdTime = cal.getTime();
      cal.add(Calendar.MINUTE, 10);
      expiresTime = cal.getTime();
      authenticationTime = createdTime;
   }

   @Test
   public void testValidateOK_IncludingContextInResponseIfPresentInRequest() {
      testValidate("TestContext");
   }

   @Test
   public void testValidateNotOK_InvalidSignature() {
      testValidateFailsInvalidSignature();
   }

   @Test(expected = RequestExpiredException.class)
   public void testValidateNotOK_RequestLifetimeInThePast() {
      Calendar cal = Calendar.getInstance();
      cal.add(Calendar.HOUR, -10);
      createdTime = cal.getTime();
      cal.add(Calendar.MINUTE, 10);
      expiresTime = cal.getTime();
      authenticationTime = createdTime;

      testValidate(null);
   }

   @Test(expected = RequestExpiredException.class)
   public void testValidateNotOK_RequestLifetimeInTheFuture() {
      Calendar cal = Calendar.getInstance();
      cal.add(Calendar.HOUR, 10);
      createdTime = cal.getTime();
      cal.add(Calendar.MINUTE, 10);
      expiresTime = cal.getTime();
      authenticationTime = createdTime;

      testValidate(null);
   }

   @Test
   public void testValidateOK() {
      testValidate(null);
   }

   @Test
   public void testValidate_TokenNotValid() {
      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      setValidateTarget(rst);

      SecurityHeaderType header = new SecurityHeaderType();

      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createTimestamp(createTimestamp(createdTime, expiresTime)));

      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      Request req = new Request(header, rst, null, token, null);

      expect(tokenValidator.validate(eq(token))).andThrow(
         new InvalidTokenException("Invalid Token"));

      replay(tokenValidator, token);

      RequestSecurityTokenResponseType response = sts.validate(req);
      assertNotNull(response);
      assertEquals(WsConstants.VALIDATE_TOKEN_TYPE, response.getTokenType());
      assertEquals(WsConstants.VALIDATE_STATUS_CODE_INVALID, response
         .getStatus().getCode());
      verify(tokenValidator, token);
   }

   @Test
   public void testValidate_TokenNotValidSignature() {
      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      setValidateTarget(rst);

      SecurityHeaderType header = new SecurityHeaderType();

      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createTimestamp(createTimestamp(createdTime, expiresTime)));

      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      Request req = new Request(header, rst, null, token, null);

      expect(tokenValidator.validate(eq(token)))
         .andThrow(
            new com.vmware.identity.saml.InvalidSignatureException(
               "Invalid Token"));

      replay(tokenValidator, token);

      try {
         sts.validate(req);
      } catch (InvalidSignatureException e) {
         // expected
      }
      verify(tokenValidator, token);
   }

   @Test
   public void testIssueOK_NoSignatureAlgorithmInReq()
      throws InvalidPrincipalException {
      testIssueOK(null);
   }

   @Test
   public void testIssueOK_SignatureAlgorithmInRequest()
      throws InvalidPrincipalException {
      testIssueOK(SignatureAlgorithm.RSA_SHA512);
   }

   @Test
   public void testIssueAmbiguous() {
      SecurityHeaderType header = new SecurityHeaderType();

      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createTimestamp(createTimestamp(createdTime, expiresTime)));

      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      rst.setDelegateTo(new DelegateToType());
      rst.setActAs(new ActAsType());

      ServerValidatableSamlToken actAsToken = createMock(ServerValidatableSamlToken.class);
      Request req = new Request(header, rst, null, null, actAsToken);

      try {
         sts.issue(req);
         Assert.fail();
      } catch (RequestFailedException e) {
         // expected
      }
   }

   @Test
   public void testIssueActAs() throws InvalidPrincipalException {
      testIssueOK(null, true, true);
   }

   @Test(expected = InvalidRequestException.class)
   public void testIssueActAsNOK() throws InvalidPrincipalException {
      testIssueOK(null, true, false);
   }

   private void testIssueOK(SignatureAlgorithm desiredSigningAlgorithmInRequest)
      throws InvalidPrincipalException {
      testIssueOK(desiredSigningAlgorithmInRequest, false, false);
   }

   private void testIssueOK(
      SignatureAlgorithm desiredSigningAlgorithmInRequest, boolean actAsReq,
      boolean actAsPermission) throws InvalidPrincipalException {
      // arrange
      TimestampType timestamp = createTimestamp(createdTime, expiresTime);

      SecurityHeaderType header = new SecurityHeaderType();

      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createTimestamp(timestamp));

      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      rst.setLifetime(createLifetimeType(createdTime, expiresTime));

      String rst_context = "RST_Context";
      rst.setContext(rst_context);
      rst.setKeyType("http://docs.oasis-open.org/ws-sx/ws-trust/200512/PublicKey");
      UseKeyType useKey = new UseKeyType();
      String signatureId = "signatureId";
      useKey.setSig(signatureId);
      rst.setUseKey(useKey);
      if (desiredSigningAlgorithmInRequest != null) {
         rst.setSignatureAlgorithm(desiredSigningAlgorithmInRequest.toString());
      }

      final PrincipalId actAsPrincipalId = new PrincipalId(
         "actAsPrincipalName", "ActAsUserDomain");
      ServerValidatableSamlToken actAsToken = null;
      Subject actAsTokSubject = null;
      if (actAsReq) {
         final ActAsType actAsElement = new ActAsType();
         final AssertionType actAsAssertion = new AssertionType();
         actAsAssertion.setConditions(new ConditionsType());
         actAsElement.setAssertion(actAsAssertion);
         rst.setActAs(actAsElement);

         actAsToken = createMock(ServerValidatableSamlToken.class);

         actAsTokSubject = createMock(Subject.class);
         expect(actAsTokSubject.subjectUpn()).andReturn(actAsPrincipalId).anyTimes();
         expect(actAsToken.getSubject()).andReturn(actAsTokSubject).anyTimes();
         expect(actAsToken.getExpirationTime()).andReturn(new Date())
            .anyTimes();
         List<SamlTokenDelegate> dels = new ArrayList<SamlTokenDelegate>();
         expect(actAsToken.getDelegationChain()).andReturn(dels);
         expect(actAsToken.getAdvice()).andReturn(new ArrayList<Advice>());
         replay(actAsTokSubject, actAsToken);
      }

      SignatureType signatureInHeader = new SignatureType();
      signatureInHeader.setId(signatureId);

      org.w3._2000._09.xmldsig_.ObjectFactory objectFactorySig = new org.w3._2000._09.xmldsig_.ObjectFactory();
      header.getAny().add(objectFactorySig.createSignature(signatureInHeader));

      X509Certificate signingCertificate = createMock(X509Certificate.class);
      Signature signature = new Signature(signingCertificate,
         CertificateLocation.ASSERTION);

      Request req = new Request(header, rst, signature, null, actAsToken);

      final com.vmware.vim.sso.PrincipalId principalToBeAuth = new com.vmware.vim.sso.PrincipalId(
         "principalName", "domain.com");
      PrincipalId principalToBeAuthnIdm = PrincipalIdConvertor
         .toIdmPrincipalId(principalToBeAuth);

      Result authnResult = new Result(principalToBeAuthnIdm,
         authenticationTime, AuthnMethod.PASSWORD);
      expect(authenticator.authenticate(eq(req))).andReturn(authnResult);

      DelegationSpec delSpec = (actAsReq) ? new DelegationSpec(
         principalToBeAuthnIdm,
         false,
         new DelegationHistory(
            actAsPrincipalId,
            new ArrayList<com.vmware.identity.saml.SamlTokenSpec.TokenDelegate>(),
            0, actAsToken.getExpirationTime()))
         : new DelegationSpec(null, false);
      RenewSpec renewSpec = new RenewSpec(true);

      Builder builder = new SamlTokenSpec.Builder(
         new TimePeriod(createdTime, expiresTime),
         new Confirmation(signingCertificate),
         new AuthenticationData(
            authnResult.getPrincipalId(),
            authnResult.getAuthnInstant(),
            com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod.PASSWORD,
            UPN), attributeNames).setDelegationSpec(delSpec).setRenewSpec(
         renewSpec);
      if (desiredSigningAlgorithmInRequest != null) {
         builder.setSignatureAlgorithm(desiredSigningAlgorithmInRequest);
      }
      if (actAsReq) {
         expect(principalDiscovery.isMemberOfSystemGroup(principalToBeAuthnIdm, actAsGroupName))
            .andReturn(actAsPermission);
      }
      SamlTokenSpec spec = builder.createSpec();

      Document assertion = createMock(Document.class);
      TimePeriod validity = spec.getLifespan();
      SamlToken issuedToken = new SamlToken(
         assertion,
         (desiredSigningAlgorithmInRequest != null) ? desiredSigningAlgorithmInRequest
            : SignatureAlgorithm.RSA_SHA256, validity,
         ConfirmationType.HOLDER_OF_KEY, delSpec.isDelegable(),
         renewSpec.isRenewable());
      expect(tokenAuthority.issueToken(eq(spec))).andReturn(issuedToken);
      Element assertionElement = createMock(Element.class);
      expect(assertion.getDocumentElement()).andReturn(assertionElement);

      replay(authenticator, tokenAuthority, principalDiscovery,
         signingCertificate, assertion, assertionElement);

      // act
      RequestSecurityTokenResponseCollectionType issuedResponse = sts
         .issue(req);

      // assert
      RequestSecurityTokenResponseType rstr = issuedResponse
         .getRequestSecurityTokenResponse();
      assertEquals(rst_context, rstr.getContext());
      assertEquals(ConfirmationType.HOLDER_OF_KEY.toString(), rstr.getKeyType());
      assertEquals(
         (desiredSigningAlgorithmInRequest != null) ? desiredSigningAlgorithmInRequest.toString()
            : SignatureAlgorithm.RSA_SHA256.toString(),
         rstr.getSignatureAlgorithm());
      assertEquals("urn:oasis:names:tc:SAML:2.0:assertion", rstr.getTokenType());
      LifetimeType expectedRstrLifetime = createResponseLifetime(spec);
      assertEquals(expectedRstrLifetime.getCreated().getValue(), rstr
         .getLifetime().getCreated().getValue());
      assertEquals(expectedRstrLifetime.getExpires().getValue(), rstr
         .getLifetime().getExpires().getValue());
      assertFalse(rstr.isDelegatable());
      assertEquals(assertionElement, rstr.getRequestedSecurityToken().getAny());
      if (desiredSigningAlgorithmInRequest != null) {
         assertEquals(desiredSigningAlgorithmInRequest.toString(),
            rstr.getSignatureAlgorithm());
      }

      verify(authenticator, tokenAuthority, principalDiscovery,
         signingCertificate, assertion, assertionElement);
      if (actAsToken != null) {
         verify(actAsTokSubject, actAsToken);
      }
   }

   private void testValidate(String context) {
      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      rst.setContext(context);
      setValidateTarget(rst);

      SecurityHeaderType header = new SecurityHeaderType();

      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createTimestamp(createTimestamp(createdTime, expiresTime)));

      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      Request req = new Request(header, rst, null, token, null);

      ServerValidatableSamlToken validToken = createMock(ServerValidatableSamlToken.class);
      Subject tokSubject = createMock(Subject.class);
      expect(tokSubject.subjectValidation()).andReturn(SubjectValidation.Regular).anyTimes();
      expect(validToken.isExternal()).andReturn(false).anyTimes();
      expect(validToken.getSubject()).andReturn(tokSubject).anyTimes();
      expect(tokenValidator.validate(eq(token))).andReturn(validToken);

      replay(tokenValidator, token, tokSubject, validToken);

      RequestSecurityTokenResponseType response = sts.validate(req);

      assertNotNull(response);
      assertEquals(WsConstants.VALIDATE_TOKEN_TYPE, response.getTokenType());
      assertNotNull(response.getStatus());
      assertEquals(WsConstants.VALIDATE_STATUS_CODE_VALID, response.getStatus()
         .getCode());
      if (context != null) {
         assertNotNull(response.getContext());
         assertEquals(context, response.getContext());
      }

      verify(tokenValidator, token, tokSubject, validToken);
   }

   private void testValidateFailsInvalidSignature() {
      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      String ctx = "context";
      rst.setContext(ctx);
      setValidateTarget(rst);

      SecurityHeaderType header = new SecurityHeaderType();

      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createTimestamp(createTimestamp(createdTime, expiresTime)));

      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      Request req = new Request(header, rst, null, token, null);

      com.vmware.vim.sso.client.SamlToken validToken = createMock(com.vmware.vim.sso.client.SamlToken.class);
      expect(tokenValidator.validate(eq(token))).andThrow(
         new com.vmware.identity.saml.InvalidSignatureException(
            "invalid sicnature"));

      replay(tokenValidator, token, validToken);

      try {
         sts.validate(req);
         fail();
      } catch (InvalidSignatureException e) {
         // pass
      }

      verify(tokenValidator, token, validToken);
   }

   private void setValidateTarget(RequestSecurityTokenType rst) {
      ValidateTargetType validateTarget = new ValidateTargetType();
      validateTarget.setAny(createMock(Element.class));
      rst.setValidateTarget(validateTarget);
   }

   private AttributedDateTime convertDateToAttributedDateTime(Date dateTime) {
      DateFormat formatter = new SimpleDateFormat(DATETIME_FORMAT);
      formatter.setTimeZone(TimeZone.getTimeZone(DEFAULT_TIMEZONE));
      String formattedDate = formatter.format(dateTime);

      AttributedDateTime attributedDateTime = new AttributedDateTime();
      attributedDateTime.setValue(formattedDate);

      return attributedDateTime;
   }

   private LifetimeType createResponseLifetime(SamlTokenSpec spec) {
      LifetimeType lifetimeType = new LifetimeType();
      AttributedDateTime timeCreated = convertDateToAttributedDateTime(spec
         .getLifespan().getStartTime());
      AttributedDateTime timeExpires = convertDateToAttributedDateTime(spec
         .getLifespan().getEndTime());
      lifetimeType.setCreated(timeCreated);
      lifetimeType.setExpires(timeExpires);

      return lifetimeType;
   }

   private TimestampType createTimestamp(Date createdTime, Date expiresTime) {
      TimestampType timestamp = new TimestampType();
      AttributedDateTime created = convertDateToAttributedDateTime(createdTime);
      AttributedDateTime expires = convertDateToAttributedDateTime(expiresTime);
      timestamp.setCreated(created);
      timestamp.setExpires(expires);

      return timestamp;
   }

   private LifetimeType createLifetimeType(Date createdTime, Date expiresTime) {
      LifetimeType lifetime = new LifetimeType();
      AttributedDateTime created = convertDateToAttributedDateTime(createdTime);
      AttributedDateTime expires = convertDateToAttributedDateTime(expiresTime);
      lifetime.setCreated(created);
      lifetime.setExpires(expires);

      return lifetime;
   }

}
