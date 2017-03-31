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
package com.vmware.identity.sts.idm.impl;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.fail;

import java.security.cert.X509Certificate;

import junit.framework.Assert;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.CertificateRevocationCheckException;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.sts.idm.Authenticator;
import com.vmware.identity.sts.idm.IdmSecureIDNewPinException;
import com.vmware.identity.sts.idm.LockedUserAccountException;
import com.vmware.identity.sts.idm.PasswordExpiredException;
import com.vmware.identity.sts.idm.STSConfigExtractor;
import com.vmware.identity.sts.idm.STSConfiguration;
import com.vmware.identity.sts.idm.UserCertificateValidateException;

/**
 * Insert your comment for AuthenticatorImpl here
 */
public final class AuthenticatorImplTest {

   private static final String TENANT_NAME = "tenant1";
   private static final String ISSUER = "issuer";
   private static final long DEFAULT_CLOCK_TOLERANCE = 1000;
   private static final String USER_UPN = "user@domain.com";
   private static final String PASSWORD = "password";
   private static final X509Certificate[] X509CERTIFICATE_CHAIN = new X509Certificate[]{};
   private static final String PASSCODE = "passcode";
   private static final String CONTEXT_ID = "context";

   private Tenant tenant;

   @Before
   public void setup() {
      tenant = new Tenant(TENANT_NAME);
      tenant._issuerName = ISSUER;
   }

   @Test
   public void test() throws Exception {

      final CasIdmClient identityManager = mockIdentityManager();

      STSConfigExtractor configExtractor = new AuthenticatorImpl(
         identityManager, TENANT_NAME);

      STSConfiguration config = configExtractor.getConfig();

      Assert.assertNotNull(config);
      Assert
         .assertEquals(new STSConfiguration(DEFAULT_CLOCK_TOLERANCE), config);
      EasyMock.verify(identityManager);
   }

   @Test
   public void testFailAuthentication_PasswordExpired() throws Exception {
      final CasIdmClient client = createMock(CasIdmClient.class);

      expect(client.authenticate(eq(TENANT_NAME), eq(USER_UPN), eq(PASSWORD)))
         .andThrow(
            new com.vmware.identity.idm.PasswordExpiredException(
               "password expired"));

      replay(client);

      Authenticator authenticator = new AuthenticatorImpl(client, TENANT_NAME);
      try {
         authenticator.authenticate(USER_UPN, PASSWORD);
         fail("Password Expired exception should be thrown from authentication.");
      } catch (PasswordExpiredException e) {
         // expected
      }

      verify(client);
   }

   @Test
   public void testFailAuthentication_LockedUser() throws Exception {
      final CasIdmClient client = createMock(CasIdmClient.class);

      expect(client.authenticate(eq(TENANT_NAME), eq(USER_UPN), eq(PASSWORD)))
         .andThrow(new UserAccountLockedException("account locked"));

      replay(client);

      Authenticator authenticator = new AuthenticatorImpl(client, TENANT_NAME);
      try {
         authenticator.authenticate(USER_UPN, PASSWORD);
         fail("User account locked exception should be thrown from authentication.");
      } catch (LockedUserAccountException e) {
         // expected
      }

      verify(client);
   }

   @Test
   public void testFailAuthentication_CertificateRevocation() throws Exception {
      final CasIdmClient client = createMock(CasIdmClient.class);

      expect(client.authenticate(eq(TENANT_NAME), eq(X509CERTIFICATE_CHAIN),null))
         .andThrow(new CertificateRevocationCheckException("Certificate revoked."));

      replay(client);

      Authenticator authenticator = new AuthenticatorImpl(client, TENANT_NAME);
      try {
         authenticator.authenticate(X509CERTIFICATE_CHAIN);
         fail("User certificate validate exception should be thrown from authentication.");
      } catch (UserCertificateValidateException e) {
         // expected
      }

      verify(client);
   }

   @Test
   public void testFailAuthentication_IDMSecureIDNewPin() throws Exception {
      final CasIdmClient client = createMock(CasIdmClient.class);

      expect(client.authenticateRsaSecurId(eq(TENANT_NAME), eq(CONTEXT_ID), eq(USER_UPN), eq(PASSCODE)))
         .andThrow(new IDMSecureIDNewPinException("New Pin required for SecurID."));

      replay(client);

      Authenticator authenticator = new AuthenticatorImpl(client, TENANT_NAME);
      try {
         authenticator.authenticate(USER_UPN, CONTEXT_ID, PASSCODE);
         fail("Idm SecureID new pin exception should be thrown from authentication.");
      } catch (IdmSecureIDNewPinException e) {
         // expected
      }

      verify(client);
   }

   /*
    * TODO: Enable it again once IDM defines corresponding exception on
    * getClockTolerance
    *
    * @Test(expected = NoSuchIdPException.class)
    *
    * public void testNullTenant() throws Exception { doTestGetConfigFailure();
    * }
    */
   /*
    * private void doTestGetConfigFailure() throws Exception {
    *
    * final CasIdmClient identityManager = mockIdentityManager();
    *
    * new AuthenticatorImpl(identityManager, TENANT_NAME).getConfig();
    *
    * EasyMock.verify(identityManager); }
    */
   private CasIdmClient mockIdentityManager() throws Exception {

      final CasIdmClient identityManagerClient = EasyMock
         .createMock(CasIdmClient.class);

      EasyMock.expect(identityManagerClient.getClockTolerance(TENANT_NAME))
         .andReturn(DEFAULT_CLOCK_TOLERANCE);

      EasyMock.replay(identityManagerClient);
      return identityManagerClient;
   }
}
