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
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.PasscodeString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.PasswordString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UsernameTokenType;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMResult;
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
   private final static String CONTEXT_ID = "context";

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
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPassword(pass));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

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
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPassword(pass));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testUNTNoPasswordAndPassCode() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(UPN);
      unt.setUsername(userName);
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testUNTBothPasswordAndPassCode() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(UPN);
      unt.setUsername(userName);
      final PasswordString password = new PasswordString();
      final PasscodeString passcode = new PasscodeString();
      password.setValue(PASS);
      passcode.setValue(PASS);
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPassword(password));
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPasscode(passcode));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

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
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPassword(new PasswordString()));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testUNTNullPasscode() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(UPN);
      unt.setUsername(userName);
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPasscode(new PasscodeString()));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

      intTestUnsupportedSecurityTokenExc(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testInvalidCredentialsPassword() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = invalidCredentialsPasswordIDMAuth(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasswordHeader();

      intTestInvalidCredExcPassword(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testInvalidCredentialsPasscode() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = invalidCredentialsPasscodeIDMAuth(
         UPN, CONTEXT_ID, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasscodeHeader();

      intTestInvalidCredExcPasscode(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testIdmSecureIDNewPinException() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = idmSecureIDNewPinIDMAuth(
         UPN, CONTEXT_ID, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasscodeHeader();

      intTestInvalidCredExcPasscode(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testNullRsaAMResult() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = nullRsaAMResultIDMAuth(
         UPN, CONTEXT_ID, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasscodeHeader();

      intTestInvalidCredExcPasscode(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testPasswordExpired() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = passwordExpiredIDMAuth(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasswordHeader();

      intTestInvalidCredExcPassword(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testLockedUserAccount() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = lockedAccountIDMAuth(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasswordHeader();

      intTestInvalidCredExcPassword(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testSystemErrorPassword() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = systemExceptionIDMAuthPassword(
         UPN, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasswordHeader();

      intTestRequestFailedExcPassword(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testSystemErrorPasscode() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = systemExceptionIDMAuthPasscode(
         UPN, CONTEXT_ID, PASS);
      final Authenticator authenticator = new UNTAuthenticator(idmAuth);
      final SecurityHeaderType header = buildPasscodeHeader();

      intTestRequestFailedExcPasscode(authenticator, header);
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testOKPassword() {
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
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPassword(pass));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

      final Result authResult = authenticator.authenticate(newReq(header));
      Assert.assertNotNull(authResult);
      Assert.assertTrue(authResult.completed());
      Assert.assertEquals(principalIdc13d, authResult.getPrincipalId());
      Assert.assertEquals(Result.AuthnMethod.PASSWORD,
         authResult.getAuthnMethod());

      EasyMock.verify(idmAuth);
   }

   @Test
   public void testOKPasscode() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      final String name = "user";
      final String upn = name + "@ACMEC";
      final String passcode = "pass123";

      final PrincipalId principalIdc13d = new PrincipalId(name, "acme.com");
      final RSAAMResult rsaAMResult = new RSAAMResult(principalIdc13d);

      EasyMock.expect(idmAuth.authenticate(upn, CONTEXT_ID, passcode)).andReturn(
              rsaAMResult);
      EasyMock.replay(idmAuth);

      final Authenticator authenticator = new UNTAuthenticator(idmAuth);

      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(upn);
      unt.setUsername(userName);
      final PasscodeString pass = new PasscodeString();
      pass.setValue(passcode);
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPasscode(pass));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));

      final Result authResult = authenticator.authenticate(newReqWithContextId(header));
      Assert.assertNotNull(authResult);
      Assert.assertTrue(authResult.completed());
      Assert.assertEquals(principalIdc13d, authResult.getPrincipalId());
      Assert.assertEquals(Result.AuthnMethod.TIMESYNCTOKEN,
         authResult.getAuthnMethod());

      EasyMock.verify(idmAuth);
   }

   private void intTestInvalidCredExcPassword(Authenticator authenticator,
      SecurityHeaderType header) {
      try {
         authenticator.authenticate(newReq(header));
         Assert.fail();
      } catch (InvalidCredentialsException e) {
         // expected
      }
   }

   private void intTestInvalidCredExcPasscode(Authenticator authenticator,
           SecurityHeaderType header) {
           try {
              authenticator.authenticate(newReqWithContextId(header));
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

   private void intTestRequestFailedExcPassword(Authenticator authenticator,
      SecurityHeaderType header) {
      try {
         authenticator.authenticate(newReq(header));
         Assert.fail();
      } catch (RequestFailedException e) {
         // expected
      }
   }

   private void intTestRequestFailedExcPasscode(Authenticator authenticator,
           SecurityHeaderType header) {
           try {
              authenticator.authenticate(newReqWithContextId(header));
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

   private com.vmware.identity.sts.idm.Authenticator invalidCredentialsPasswordIDMAuth(
      String userUPN, String password) {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.expect(idmAuth.authenticate(userUPN, password)).andThrow(
         new com.vmware.identity.sts.idm.InvalidCredentialsException());
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator invalidCredentialsPasscodeIDMAuth(
           String userUPN, String contextId, String passcode) {
           com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
              .createMock(com.vmware.identity.sts.idm.Authenticator.class);
           EasyMock.expect(idmAuth.authenticate(userUPN, contextId, passcode)).andThrow(
              new com.vmware.identity.sts.idm.InvalidCredentialsException());
           EasyMock.replay(idmAuth);
           return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator idmSecureIDNewPinIDMAuth(
           String userUPN, String contextId, String passcode) {
           com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
              .createMock(com.vmware.identity.sts.idm.Authenticator.class);
           EasyMock.expect(idmAuth.authenticate(userUPN, contextId, passcode)).andThrow(
              new com.vmware.identity.sts.idm.IdmSecureIDNewPinException("SecurId requires a new pin.", new RuntimeException()));
           EasyMock.replay(idmAuth);
           return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator nullRsaAMResultIDMAuth(
           String userUPN, String contextId, String passcode) {
           com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
              .createMock(com.vmware.identity.sts.idm.Authenticator.class);
           EasyMock.expect(idmAuth.authenticate(userUPN, contextId, passcode)).andReturn(null);
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

   private com.vmware.identity.sts.idm.Authenticator systemExceptionIDMAuthPassword(
      String userUPN, String password) {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.expect(idmAuth.authenticate(userUPN, password)).andThrow(
         new SystemException());
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private com.vmware.identity.sts.idm.Authenticator systemExceptionIDMAuthPasscode(
           String userUPN, String contextId, String passcode) {
           com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
              .createMock(com.vmware.identity.sts.idm.Authenticator.class);
           EasyMock.expect(idmAuth.authenticate(userUPN, contextId, passcode)).andThrow(
              new SystemException());
           EasyMock.replay(idmAuth);
           return idmAuth;
        }

   private Request newReq(SecurityHeaderType header) {
      return new Request(header, new RequestSecurityTokenType(), null, null,
         null);
   }

   private SecurityHeaderType buildPasswordHeader() {
      final SecurityHeaderType header = new SecurityHeaderType();
      final UsernameTokenType unt = new UsernameTokenType();
      final AttributedString userName = new AttributedString();
      userName.setValue(UPN);
      unt.setUsername(userName);
      final PasswordString password = new PasswordString();
      password.setValue(PASS);
      unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPassword(password));
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createUsernameToken(unt));
      return header;
   }

   private SecurityHeaderType buildPasscodeHeader() {
       final SecurityHeaderType header = new SecurityHeaderType();
       final UsernameTokenType unt = new UsernameTokenType();
       final AttributedString userName = new AttributedString();
       userName.setValue(UPN);
       unt.setUsername(userName);
       final PasscodeString passcode = new PasscodeString();
       passcode.setValue(PASS);
       unt.getAny().add(new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory().createPasscode(passcode));
       ObjectFactory objectFactory = new ObjectFactory();
       header.getAny().add(objectFactory.createUsernameToken(unt));
       return header;
    }

   private Request newReqWithContextId(SecurityHeaderType header) {
       RequestSecurityTokenType rst = new RequestSecurityTokenType();
       rst.setContext(CONTEXT_ID);
       return new Request(header, rst, null, null,
          null);
    }
}
