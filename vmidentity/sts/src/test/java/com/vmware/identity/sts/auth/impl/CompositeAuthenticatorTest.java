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

import java.util.Date;
import java.util.HashSet;
import java.util.Set;

import junit.framework.Assert;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;

/**
 * Insert your comment for CompositeAuthenticatorTest here
 */
public final class CompositeAuthenticatorTest {

   private static final Result expResult = new Result(new PrincipalId("user",
      "acme.com"), new Date(), AuthnMethod.DIG_SIG);

   private static final Result incompleteResult = new Result(new byte[] { 4, 6,
      9, -78 });

   private Request req;

   @Before
   public void init() {
      req = new Request(new SecurityHeaderType(),
         new RequestSecurityTokenType(), null, null, null);
   }

   @Test
   public void testSuccess() {
      testSuccessInt(expResult, new Result[] { expResult });
   }

   @Test
   public void testDoubleSuccessSamePrincipal() {
      final Result baseResult2 = new Result(expResult.getPrincipalId(),
         new Date(expResult.getAuthnInstant().getTime() + 1),
         AuthnMethod.PASSWORD);
      // we expect the latest (by auth instant) result
      testSuccessInt(baseResult2, new Result[] { expResult, baseResult2 });
      testSuccessInt(baseResult2, new Result[] { baseResult2, expResult });
   }

   @Test
   public void testDoubleSuccessDifferentPrincipal() {
      final PrincipalId otherPrincipalId = new PrincipalId(expResult
         .getPrincipalId().getName() + '!', expResult.getPrincipalId()
         .getDomain());

      testFailInt(new Authenticator[] {
         successingAuthenticator(expResult),
         successingAuthenticator(new Result(otherPrincipalId, new Date(),
            AuthnMethod.KERBEROS)) });
   }

   // tests about unsuccessful authentication
   @Test
   public void testInvalidCredentials() {
      testFailInt(new Authenticator[] { failingAuthenticator(new InvalidCredentialsException(
         "testInvalidCredentials")) });
   }

   @Test
   public void testInvalidCridentialsAfterSuccess() {
      testFailInt(new Authenticator[] {
         successingAuthenticatorRelaxed(expResult),
         failingAuthenticator(new InvalidCredentialsException(
            "testInvalidCredentialsAfterSuccess")) });
   }

   // tests about incomplete authentication
   @Test
   public void testCompleteIncompleteAuth() {
      testSuccessInt(expResult, new Result[] { expResult, incompleteResult });
   }

   @Test
   public void testIncompleteCompleteAuth() {
      testSuccessInt(expResult, new Result[] { incompleteResult, expResult });
   }

   // tests about no authentication
   @Test
   public void testNoNoAuth() {
      testSuccessInt(null, new Result[] { null, null });
   }

   @Test
   public void testNoCompleteAuth() {
      testSuccessInt(expResult, new Result[] { null, expResult });
   }

   @Test
   public void testNoIncompleteAuth() {
      testSuccessInt(incompleteResult, new Result[] { null, incompleteResult });
   }

   @Test
   public void testCompleteNoIncomplete() {
      testSuccessInt(expResult, new Result[] { expResult, null,
         incompleteResult });
   }

   @Test
   public void testCompleteIncompleteNo() {
      testSuccessInt(expResult, new Result[] { expResult, incompleteResult,
         null });
   }

   @Test
   public void testNoIncompleteComplete() {
      testSuccessInt(expResult, new Result[] { null, incompleteResult,
         expResult });
   }

   @Test
   public void testNoCompleteIncomplete() {
      testSuccessInt(expResult, new Result[] { null, expResult,
         incompleteResult });
   }

   @Test
   public void testIncompleteCompleteNo() {
      testSuccessInt(expResult, new Result[] { incompleteResult, expResult,
         null });
   }

   @Test
   public void testIncompleteNoComplete() {
      testSuccessInt(expResult, new Result[] { incompleteResult, null,
         expResult });
   }

   private void testSuccessInt(Result expResult, Result[] baseResults) {
      final Set<Authenticator> authenticators = new HashSet<Authenticator>();
      for (Result baseResult : baseResults) {
         authenticators.add(successingAuthenticator(baseResult));
      }
      Authenticator authenticator = new CompositeAuthenticator(authenticators);

      final Result result = authenticator.authenticate(req);
      Assert.assertEquals(expResult, result);
      for (Authenticator baseAuthenticator : authenticators) {
         EasyMock.verify(baseAuthenticator);
      }
   }

   private void testFailInt(Authenticator[] baseAuthenticators) {
      final Set<Authenticator> authenticators = new HashSet<Authenticator>();
      for (Authenticator baseAuthenticator : baseAuthenticators) {
         authenticators.add(baseAuthenticator);
      }
      Authenticator authenticator = new CompositeAuthenticator(authenticators);

      try {
         authenticator.authenticate(req);
         Assert.fail();
      } catch (InvalidCredentialsException e) {
         // expected
      }
      for (Authenticator baseAuthenticator : authenticators) {
         EasyMock.verify(baseAuthenticator);
      }
   }

   private Authenticator successingAuthenticator(Result expResult) {
      Authenticator baseAuth = EasyMock.createMock(Authenticator.class);
      EasyMock.expect(baseAuth.authenticate(req)).andReturn(expResult);
      EasyMock.replay(baseAuth);
      return baseAuth;
   }

   private Authenticator successingAuthenticatorRelaxed(Result expResult) {
      Authenticator baseAuth = EasyMock.createMock(Authenticator.class);
      EasyMock.expect(baseAuth.authenticate(req)).andReturn(expResult)
         .anyTimes();
      EasyMock.replay(baseAuth);
      return baseAuth;
   }

   private Authenticator failingAuthenticator(Exception ex) {
      Authenticator baseAuth = EasyMock.createMock(Authenticator.class);
      EasyMock.expect(baseAuth.authenticate(req)).andThrow(ex);
      EasyMock.replay(baseAuth);
      return baseAuth;
   }
}
