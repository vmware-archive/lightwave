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

import junit.framework.Assert;

import org.easymock.EasyMock;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.AttributedString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.PasswordString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UsernameTokenType;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.idm.SystemException;

/**
 * Insert your comment for UNTAuthenticatorTest here
 */
public final class UNTAuthenticatorTest {

   private final static String UPN = "user@acme.com";
   private final static String PASS = "password";

   @Test
   public void testNoUNT() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      Assert.assertNull(authenticator
         .authenticate(newReq(new SecurityHeaderType())));
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testUNTNoUserName() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final PasswordString pass = new PasswordString();
      pass.setValue(PASS);
      unt.setPassword(pass);
      header.setUsernameToken(unt);

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testUNTNoUserNameValue() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      unt.setUsername(new AttributedString());
      final PasswordString pass = new PasswordString();
      pass.setValue(PASS);
      unt.setPassword(pass);
      header.setUsernameToken(unt);

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testUNTNoPassword() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(UPN);
      unt.setUsername(userName);
      header.setUsernameToken(unt);

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testUNTNullPassword() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(UPN);
      unt.setUsername(userName);
      unt.setPassword(new PasswordString());
      header.setUsernameToken(unt);

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testInvalidCredentials() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = invalidCredentialsIDMAuth(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildHeader();

      intTestInvalidCredExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testPasswordExpired() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = passwordExpiredIDMAuth(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildHeader();

      intTestInvalidCredExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testLockedUserAccount() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = lockedAccountIDMAuth(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildHeader();

      intTestInvalidCredExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testSystemError() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = systemExceptionIDMAuth(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildHeader();

      intTestRequestFailedExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testOK() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      final String name = "user";
      final String upn = name + "@ACMEC";
      final String password = "pass123";

      final PrincipalId principalIdc13d = new PrincipalId(name, "acme.com");

      EasyMock.expect(idmAuth.authenticate(upn, password)).andReturn(
         principalIdc13d);
      EasyMock.replay(idmAuth);

      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(upn);
      unt.setUsername(userName);
      final PasswordString pass = new PasswordString();
      pass.setValue(password);
      unt.setPassword(pass);
      header.setUsernameToken(unt);

      final Result authResult = authenticator.authenticate(newReq(header));
      Assert.assertNotNull(authResult);
      Assert.assertTrue(authResult.completed());
      Assert.assertEquals(principalIdc13d, authResult.getPrincipalId());
      Assert.assertEquals(Result.AuthnMethod.PASSWORD,
         authResult.getAuthnMethod());

      EasyMock.verify(idmAuth);
   }

   private void intTestInvalidCredExc(Authenticator authenticator,
      SecurityHeaderType header) {
      try {
         authenticator.authenticate(newReq(header));
         Assert.fail();
      } catch (InvalidCredentialsException e) {
         // expected
      }
   }

   private void intTestUnsupportedSecurityTokenExc(Authenticator authenticator,
      SecurityHeaderType header) {
      try {
         authenticator.authenticate(newReq(header));
         Assert.fail();
      } catch (UnsupportedSecurityTokenException e) {
         // expected
      }
   }

   private void intTestRequestFailedExc(Authenticator authenticator,
      SecurityHeaderType header) {
      try {
         authenticator.authenticate(newReq(header));
         Assert.fail();
      } catch (RequestFailedException e) {
         // expected
      }
   }

   private com.vmware.identity.sts.idm.Authenticator noCalledIDMAuth() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator invalidCredentialsIDMAuth(
      String userUPN, String password) {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.expect(idmAuth.authenticate(userUPN, password)).andThrow(
         new com.vmware.identity.sts.idm.InvalidCredentialsException());
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator passwordExpiredIDMAuth(
      String userUPN, String password) {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.expect(idmAuth.authenticate(userUPN, password)).andThrow(
         new com.vmware.identity.sts.idm.PasswordExpiredException(
            "password expired", new RuntimeException()));
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator lockedAccountIDMAuth(
      String userUPN, String password) {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.expect(idmAuth.authenticate(userUPN, password)).andThrow(
         new com.vmware.identity.sts.idm.LockedUserAccountException(
            "locked account", new RuntimeException()));
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator systemExceptionIDMAuth(
      String userUPN, String password) {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.expect(idmAuth.authenticate(userUPN, password)).andThrow(
         new SystemException());
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private Request newReq(SecurityHeaderType header) {
      return new Request(header, new RequestSecurityTokenType(), null, null,
         null);
   }

   private SecurityHeaderType buildHeader() {
      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(UPN);
      unt.setUsername(userName);
      final PasswordString password = new PasswordString();
      password.setValue(PASS);
      unt.setPassword(password);
      header.setUsernameToken(unt);
      return header;
   }

}
