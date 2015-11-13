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
package com.vmware.identity.sts.auth.impl;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SamlTokenDelegate;
import com.vmware.identity.saml.ServerValidatableSamlToken.Subject;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.InvalidSignatureException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.Request.CertificateLocation;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;
import com.vmware.identity.token.impl.ValidateUtil;

public class SamlTokenAuthenticatorTest {

   private static final PrincipalId SUBJECT = new PrincipalId("subject",
      "subjectDomain");
   private static final PrincipalId DELEGATE1 = new PrincipalId("delegate1",
      "domain1");
   private static final PrincipalId DELEGATE2 = new PrincipalId("delegate2",
      "domain2");

   private TokenValidator validator;
   public SamlTokenAuthenticator authenticator;

   @Before
   public void init() {
      validator = createMock(TokenValidator.class);
      authenticator = new SamlTokenAuthenticator(validator);
   }

   @Test
   public void testNoAssertionInRequest() {
      // no token in the request
      ServerValidatableSamlToken token = null;
      X509Certificate certInSignature = createMock(X509Certificate.class);
      replay(validator, certInSignature);

      Result result = authenticator.authenticate(newReq(new Signature(
         certInSignature, CertificateLocation.ASSERTION), token));

      assertNull(result);
      verify(validator, certInSignature);
   }

   @Test
   public void testNoSignatureInRequest() {
      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      X509Certificate cert = createMock(X509Certificate.class);
      Subject subject = createMock(Subject.class);
      expect(subject.subjectValidation()).andReturn(SubjectValidation.Regular).anyTimes();
      expect(token.isExternal()).andReturn(false).anyTimes();
      expect(token.getConfirmationCertificate()).andReturn(cert);
      expect(token.getSubject()).andReturn(subject).anyTimes();
      expect(validator.validate(eq(token))).andReturn(token);

      Request req = newReq(null, token);
      replay(cert, token, subject, validator);
      try {
         authenticator.authenticate(req);
         fail();
      } catch (InvalidCredentialsException e) {
         // pass
      }

      verify(cert, token, subject, validator);
   }

   @Test
   public void testAuthnWithBearerAssertion() {
      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      X509Certificate certInSignature = createMock(X509Certificate.class);
      Signature signature = new Signature(certInSignature,
         CertificateLocation.ASSERTION);
      Request req = newReq(signature, token);

      ServerValidatableSamlToken returnedToken = createMock(ServerValidatableSamlToken.class);
      Subject subject = createMock(Subject.class);
      expect(subject.subjectValidation()).andReturn(SubjectValidation.Regular).anyTimes();
      expect(returnedToken.isExternal()).andReturn(false).anyTimes();
      expect(returnedToken.getSubject()).andReturn(subject).anyTimes();
      expect(validator.validate(eq(token))).andReturn(returnedToken);
      expect(token.getConfirmationCertificate()).andReturn(null);

      replay(token, certInSignature, subject, returnedToken, validator);

      try {
         authenticator.authenticate(req);
         fail();
      } catch (UnsupportedSecurityTokenException e) {
         // pass
      }
      verify(token, certInSignature, subject, returnedToken, validator);
   }

   @Test
   public void testAssertionAndSignatureHaveDifferentOwners() {
      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      X509Certificate certInSignature = createMock(X509Certificate.class);
      Signature signature = new Signature(certInSignature,
         CertificateLocation.ASSERTION);
      Request req = newReq(signature, token);
      X509Certificate certInAssertion = createMock(X509Certificate.class);
      ServerValidatableSamlToken returnedToken = createMock(ServerValidatableSamlToken.class);
      Subject subject = createMock(Subject.class);
      expect(subject.subjectValidation()).andReturn(SubjectValidation.Regular).anyTimes();
      expect(returnedToken.isExternal()).andReturn(false).anyTimes();
      expect(returnedToken.getSubject()).andReturn(subject).anyTimes();
      expect(validator.validate(eq(token))).andReturn(returnedToken);
      expect(token.getConfirmationCertificate()).andReturn(certInAssertion);

      replay(token, certInSignature, certInAssertion, subject, returnedToken, validator);

      try {
         authenticator.authenticate(req);
         fail();
      } catch (InvalidCredentialsException e) {
         // pass
      }
      verify(token, certInSignature, certInAssertion, subject, returnedToken, validator);
   }

   @Test
   public void testAssertionNotValid() {
      // arrange
      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      X509Certificate certInSignature = createMock(X509Certificate.class);
      Signature signature = new Signature(certInSignature,
         CertificateLocation.ASSERTION);
      Request req = newReq(signature, token);

      expect(validator.validate(token)).andThrow(
         new InvalidTokenException("Cannot validate token"));

      replay(validator, token, certInSignature);

      // act
      // TODO uncomment this when the ssoClient is submitted
      // intTestInvalidCredentialsException(req);
      intTestInvalidSignatureException(req);

      verify(validator, token, certInSignature);
   }

   @Test
   public void testAssertionNotValid_InvalidSignature() {
      // arrange
      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      X509Certificate certInSignature = createMock(X509Certificate.class);
      Signature signature = new Signature(certInSignature,
         CertificateLocation.ASSERTION);
      Request req = newReq(signature, token);

      expect(validator.validate(token)).andThrow(
         new com.vmware.identity.saml.InvalidSignatureException(
            "Invalid signature"));

      replay(validator, token, certInSignature);

      // act
      intTestInvalidSignatureException(req);
      verify(validator, token, certInSignature);
   }

   @Test
   public void testOk_SignatureInAssertion() {
      testOk(CertificateLocation.ASSERTION);
   }

   @Test
   public void testOk_SignatureInBST() {
      testOk(CertificateLocation.BST);
   }

   @Test
   public void testNotDelegatedToken() {
      testOk(CertificateLocation.BST, SUBJECT, SUBJECT, null);
   }

   @Test
   public void testDelegatedTokenOneDelegate() {
      testOk(CertificateLocation.BST, DELEGATE1, SUBJECT,
         new PrincipalId[] { DELEGATE1 });
   }

   @Test
   public void testDelegatedTokenTwoDelegates() {
      testOk(CertificateLocation.BST, DELEGATE2, SUBJECT, new PrincipalId[] {
         DELEGATE1, DELEGATE2 });
   }

   @Test
   public void testOk_ExternalTokenBearer() {
      testOk(CertificateLocation.BST, SUBJECT, SUBJECT, null, true, true, false);
   }

   @Test
   public void testOk_ExternalTokenBearerNoSignature() {
      testOk(CertificateLocation.BST, SUBJECT, SUBJECT, null, true, true, true);
   }

   @Test
   public void testOk_ExternalTokenHok() {
      testOk(CertificateLocation.ASSERTION, SUBJECT, SUBJECT, null, true, false, false);
   }

   private void intTestInvalidCredentialsException(Request req) {
      try {
         authenticator.authenticate(req);
         fail();
      } catch (InvalidCredentialsException e) {
         // pass
      }
   }

   private void intTestInvalidSignatureException(Request req) {
      try {
         authenticator.authenticate(req);
         fail();
      } catch (InvalidSignatureException e) {
         // pass
      }
   }

   private void testOk(CertificateLocation signatureLocation) {
      testOk(signatureLocation, SUBJECT, SUBJECT, null);
   }
   private void testOk(CertificateLocation signatureLocation,
           PrincipalId authPrincipal, PrincipalId subject, PrincipalId[] delegates) {
       testOk(signatureLocation, authPrincipal, subject, delegates, false, false, false);
   }

   private void testOk(CertificateLocation signatureLocation,
      PrincipalId authPrincipal, PrincipalId subject, PrincipalId[] delegates,
      Boolean external, Boolean bearer, Boolean noSignature) {
       Assert.assertTrue("bearer should be set only for external", (external == true) || (bearer == false) );
       Assert.assertTrue("delegate can be specified only for own token", (delegates == null) || (external == false) );
       Assert.assertTrue("nosignature can be set only for external", (external == true) || (noSignature == false) );
      // arrange
      ServerValidatableSamlToken token = createMock(ServerValidatableSamlToken.class);
      X509Certificate certInSignature = createMock(X509Certificate.class);
      Signature signature = null;
      if (noSignature == false)
      {
          signature = new Signature(certInSignature, signatureLocation);
      }
      Request req = newReq(signature, token);

      X509Certificate certInAssertion = certInSignature;
      expect(token.getConfirmationCertificate()).andReturn((bearer == false) ? certInAssertion : null);

      ServerValidatableSamlToken returnedToken = createMock(ServerValidatableSamlToken.class);
      Subject tokSubject = createMock(Subject.class);
      expect(tokSubject.subjectValidation()).andReturn(SubjectValidation.Regular).anyTimes();
      expect(returnedToken.isExternal()).andReturn(external).anyTimes();
      expect(returnedToken.getSubject()).andReturn(tokSubject).anyTimes();
      expect(validator.validate(eq(token))).andReturn(returnedToken);
      if (delegates != null && delegates.length > 0) {
         final List<SamlTokenDelegate> tokenDelegates = new ArrayList<SamlTokenDelegate>();
         for (PrincipalId delegate : delegates) {
            tokenDelegates.add(new DummyTokenDelegate(delegate));
         }
         expect(returnedToken.getDelegationChain()).andReturn(tokenDelegates);
      } else {
         expect(returnedToken.getDelegationChain()).andReturn(
            new ArrayList<SamlTokenDelegate>());
         expect(tokSubject.subjectUpn()).andReturn(subject).anyTimes();
      }
      replay(token, certInSignature, tokSubject, returnedToken, validator);

      // act
      Date start = new Date();
      Result result = authenticator.authenticate(req);
      Date end = new Date();

      // assert
      if(external == false){
          assertEquals(AuthnMethod.ASSERTION, result.getAuthnMethod());
      } else {
          assertEquals(AuthnMethod.EXTERNAL_ASSERTION, result.getAuthnMethod());
      }

      Date authnInstantTime = result.getAuthnInstant();
      assertTrue(start.compareTo(authnInstantTime) <= 0);
      assertTrue(end.compareTo(authnInstantTime) >= 0);
      com.vmware.identity.idm.PrincipalId returnedPrincipalId = result
         .getPrincipalId();
      assertEquals(authPrincipal.getDomain(), returnedPrincipalId.getDomain());
      assertEquals(authPrincipal.getName(), returnedPrincipalId.getName());

      verify(token, certInSignature, tokSubject, returnedToken, validator);
   }

   private Request newReq(Signature signature, ServerValidatableSamlToken token) {
      return new Request(new SecurityHeaderType(),
         new RequestSecurityTokenType(), signature, token, null);
   }
}

final class DummyTokenDelegate implements SamlTokenDelegate {
   private final PrincipalId delegate;

   DummyTokenDelegate(PrincipalId delegate) {
      assert delegate != null;
      this.delegate = delegate;
   }

    @Override
    public Subject subject()
    {
        return new ServerValidatableSamlToken.SubjectImpl(
            delegate,
            new ServerValidatableSamlToken.NameIdImpl(
                this.delegate.getName() + "@" + this.delegate.getDomain(),
                "http://schemas.xmlsoap.org/claims/UPN"
            ),
            SubjectValidation.Regular
        );
    }

    @Override
    public Date delegationInstant()
    {
        return new Date();
    }
}

