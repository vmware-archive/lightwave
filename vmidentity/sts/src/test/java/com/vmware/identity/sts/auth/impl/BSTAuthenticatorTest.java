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

import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.Date;

import javax.security.auth.x500.X500Principal;

import junit.framework.Assert;

import org.easymock.EasyMock;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.BinarySecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.sts.CertificateUtil;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.Request.CertificateLocation;
import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.SystemException;

/**
 * Insert your comment for BSTAuthenticatortest here
 */
public class BSTAuthenticatorTest {

   private static final int HALF_PERIOD_OF_CERT_VALIDITY = 2 * 60 * 1000;
   private static final PrincipalId VALID_PRINCIPAL = new PrincipalId("user",
      "acme.com");
   private static final String SUBJECT_DN = "C=BG,ST=Sofia,L=Sofia,O=VMWare,OU=R&D,CN=sts1";
   private final CertificateUtil certUtil = new CertificateUtil();

   @Test
   public void testNoBST() {
      PrincipalDiscovery principalDiscovery = noCalledPrincipalDiscovery();
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      final SecurityHeaderType header = new SecurityHeaderType();
      Assert.assertNull(authenticator.authenticate(newBSTRequest(header)));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createBinarySecurityToken(new BinarySecurityTokenType()));
      Assert.assertNull(authenticator.authenticate(newBSTRequest(header)));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testNoBSTSignature() {
      PrincipalDiscovery principalDiscovery = noCalledPrincipalDiscovery();
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      final SecurityHeaderType header = new SecurityHeaderType();
      X509Certificate cert = cert(SUBJECT_DN);
      Assert
         .assertNull(authenticator.authenticate(newBSTRequest(header, cert)));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTNoSignature() {
      PrincipalDiscovery principalDiscovery = noCalledPrincipalDiscovery();
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      intTestInvalidCredentialsExc(authenticator,
         newBSTRequest(newHeader(), (Signature) null));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTAssertionSignature() {
      PrincipalDiscovery principalDiscovery = noCalledPrincipalDiscovery();
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      X509Certificate cert = cert("C=BG");
      intTestInvalidCredentialsExc(
         authenticator,
         newBSTRequest(newHeader(), new Request.Signature(cert,
            CertificateLocation.ASSERTION)));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTNoSubjectDN() {
      PrincipalDiscovery principalDiscovery = noCalledPrincipalDiscovery();
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      X509Certificate cert = cert("");

      intTestInvalidCredentialsExc(authenticator,
         newBSTRequest(newHeader(), cert));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTFindSolutionUserThrowsSystemException() {
      PrincipalDiscovery principalDiscovery = principalDiscovery(SUBJECT_DN,
         new SystemException());
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      X509Certificate cert = cert(SUBJECT_DN);

      intTestRequestFailedExc(authenticator, newBSTRequest(newHeader(), cert));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTPrincipalNotFound() {
      PrincipalDiscovery principalDiscovery = principalDiscovery(SUBJECT_DN,
         (SolutionUser) null);
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      X509Certificate cert = cert(SUBJECT_DN);

      Assert.assertNull(authenticator.authenticate(newBSTRequest(newHeader(),
         cert)));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTInvalidPrincipalFound2() {
      final X509Certificate cert = cert(SUBJECT_DN);
      PrincipalDiscovery principalDiscovery = principalDiscovery(SUBJECT_DN,
         solutionUser(VALID_PRINCIPAL, null, false));
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      intTestInvalidCredentialsExc(authenticator,
         newBSTRequest(newHeader(), cert));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTPrincipalIsDisabled() {
      final X509Certificate cert = certUtil
         .loadCert(CertificateUtil.STS_CERT_ALIAS);
      final X509Certificate certExp = certUtil
         .loadCert(CertificateUtil.STS_CERT_ALIAS);

      PrincipalDiscovery principalDiscovery = principalDiscovery(SUBJECT_DN,
         solutionUser(VALID_PRINCIPAL, certExp, true));
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      intTestInvalidCredentialsExc(authenticator,
         newBSTRequest(newHeader(), cert));
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTPrincipalCertNotValidYet() {
      Date now = new Date();

      Date notBefore = new Date(now.getTime() + HALF_PERIOD_OF_CERT_VALIDITY);
      Date notAfter = new Date(now.getTime() + 2 * HALF_PERIOD_OF_CERT_VALIDITY);

      testBSTPrincipalCert(notBefore, notAfter, null);
   }

   @Test
   public void testBSTPrincipalCertNotValidAnymore() {
      Date now = new Date();

      Date notBefore = new Date(now.getTime() - 2
         * HALF_PERIOD_OF_CERT_VALIDITY);
      Date notAfter = new Date(now.getTime() - HALF_PERIOD_OF_CERT_VALIDITY);

      testBSTPrincipalCert(notBefore, notAfter, null);
   }

   @Test
   public void testBSTPrincipalCertNotEqual() {
      Date now = new Date();

      Date notBefore = new Date(now.getTime() - HALF_PERIOD_OF_CERT_VALIDITY);
      Date notAfter = new Date(now.getTime() + HALF_PERIOD_OF_CERT_VALIDITY);

      testBSTPrincipalCert(notBefore, notAfter, new byte[] { 3 });
   }

   private void testBSTPrincipalCert(Date notBefore, Date notAfter,
      byte[] content) {
      final X509Certificate certExp = (content != null) ? cert(SUBJECT_DN,
         notBefore, notAfter, content) : cert(SUBJECT_DN, notBefore, notAfter);

      final X509Certificate cert = (content == null) ? certExp : certUtil
         .loadCert(CertificateUtil.STS_CERT_ALIAS);

      PrincipalDiscovery principalDiscovery = principalDiscovery(SUBJECT_DN,
         solutionUser(VALID_PRINCIPAL, certExp, false));
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      Request req = newBSTRequest(newHeader(), cert);
      intTestInvalidCredentialsExc(authenticator, req);
      EasyMock.verify(principalDiscovery);
   }

   @Test
   public void testBSTOK() {
      final X509Certificate cert = certUtil
         .loadCert(CertificateUtil.STS_CERT_ALIAS);
      final X509Certificate certExp = certUtil
         .loadCert(CertificateUtil.STS_CERT_ALIAS);

      PrincipalDiscovery principalDiscovery = principalDiscovery(SUBJECT_DN,
         solutionUser(VALID_PRINCIPAL, certExp, false));
      final Authenticator authenticator = new BSTAuthenticator(
         principalDiscovery);

      final Result authResult = authenticator.authenticate(newBSTRequest(
         newHeader(), cert));
      Assert.assertNotNull(authResult);
      Assert.assertTrue(authResult.completed());
      Assert.assertEquals(VALID_PRINCIPAL, authResult.getPrincipalId());
      Assert.assertEquals(Result.AuthnMethod.DIG_SIG,
         authResult.getAuthnMethod());

      EasyMock.verify(principalDiscovery);
   }

   private Request newBSTRequest(SecurityHeaderType header) {

      return newBSTRequest(header, (Request.Signature) null);
   }

   private Request newBSTRequest(SecurityHeaderType header, X509Certificate cert) {
      final Request.Signature signature = (cert == null) ? null
         : new Request.Signature(cert, CertificateLocation.BST);

      return newBSTRequest(header, signature);
   }

   private Request newBSTRequest(SecurityHeaderType header,
      Request.Signature signature) {

      return new Request(header, new RequestSecurityTokenType(), signature,
         null, null);
   }

   private void intTestRequestFailedExc(Authenticator authenticator, Request req) {
      try {
         authenticator.authenticate(req);
         Assert.fail();
      } catch (RequestFailedException e) {
         // expected
      }
   }

   private void intTestInvalidCredentialsExc(Authenticator authenticator,
      Request req) {
      try {
         authenticator.authenticate(req);
         Assert.fail();
      } catch (InvalidCredentialsException e) {
         // expected
      }
   }

   private PrincipalDiscovery noCalledPrincipalDiscovery() {
      PrincipalDiscovery principalDiscovery = EasyMock
         .createMock(PrincipalDiscovery.class);
      EasyMock.replay(principalDiscovery);
      return principalDiscovery;
   }

   private PrincipalDiscovery principalDiscovery(String subjectDN,
      SolutionUser user) {
      PrincipalDiscovery principalDiscovery = EasyMock
         .createMock(PrincipalDiscovery.class);
      EasyMock.expect(principalDiscovery.findSolutionUser(subjectDN))
         .andReturn(user);
      EasyMock.replay(principalDiscovery);
      return principalDiscovery;
   }

   private PrincipalDiscovery principalDiscovery(String subjectDN,
      Exception excToBeThrown) {
      PrincipalDiscovery principalDiscovery = EasyMock
         .createMock(PrincipalDiscovery.class);
      EasyMock.expect(principalDiscovery.findSolutionUser(subjectDN)).andThrow(
         excToBeThrown);
      EasyMock.replay(principalDiscovery);
      return principalDiscovery;
   }

   private X509Certificate cert(String subjectDN) {
      X509Certificate cert = certInt(subjectDN);

      EasyMock.replay(cert);
      return cert;
   }

   private X509Certificate cert(String subjectDN, Date notBefore, Date notAfter) {
      X509Certificate cert = certInt(subjectDN, notBefore, notAfter);
      EasyMock.replay(cert);
      return cert;
   }

   private X509Certificate cert(String subjectDN, Date notBefore,
      Date notAfter, byte[] content) {
      X509Certificate cert = certInt(subjectDN, notBefore, notAfter);
      try {
         EasyMock.expect(cert.getEncoded()).andReturn(content);
      } catch (CertificateEncodingException e) {
         Assert.fail();
      }
      EasyMock.replay(cert);
      return cert;
   }

   private X509Certificate certInt(String subjectDN) {
      X509Certificate cert = EasyMock.createMock(X509Certificate.class);
      EasyMock.expect(cert.getSubjectX500Principal())
         .andReturn(new X500Principal(subjectDN)).anyTimes();
      return cert;
   }

   private X509Certificate certInt(String subjectDN, Date notBefore,
      Date notAfter) {
      X509Certificate cert = certInt(subjectDN);
      EasyMock.expect(cert.getNotBefore()).andReturn(notBefore);
      EasyMock.expect(cert.getNotAfter()).andReturn(notAfter);
      return cert;
   }

   private SecurityHeaderType newHeader() {
      final SecurityHeaderType header = new SecurityHeaderType();
      final BinarySecurityTokenType bst = new BinarySecurityTokenType();
      bst.setValue("base64certificatecontent");
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createBinarySecurityToken(bst));
      return header;
   }

   private SolutionUser solutionUser(PrincipalId principalId,
      X509Certificate cert, boolean disabled) {

      SolutionDetail detail = new SolutionDetail(cert, "description");
      final SolutionUser user = new SolutionUser(principalId, detail, disabled);
      return user;
   }
}
