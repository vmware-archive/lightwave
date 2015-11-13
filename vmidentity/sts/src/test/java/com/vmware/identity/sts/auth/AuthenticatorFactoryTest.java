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
package com.vmware.identity.sts.auth;

import static com.vmware.identity.sts.TestUtil.TENANT_NAME;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertNotNull;

import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.auth.impl.CompositeAuthenticatorFactory;
import com.vmware.identity.sts.idm.Authenticator;
import com.vmware.identity.sts.idm.IdmTenantServices;
import com.vmware.identity.sts.idm.PrincipalDiscovery;

public class AuthenticatorFactoryTest {

   private IdmTenantServices idm;
   private AuthenticatorFactory authenticatorFactory;

   @Before
   public void init() {
      idm = createMock(IdmTenantServices.class);
      authenticatorFactory = new CompositeAuthenticatorFactory(idm);
   }

   @Test
   public void testGetAuthenticator() {
      Authenticator idmAuthenticator = createMock(Authenticator.class);
      expect(idm.authService(eq(TENANT_NAME))).andReturn(idmAuthenticator);
      PrincipalDiscovery idmPrincipalDiscovery = createMock(PrincipalDiscovery.class);
      expect(idm.principalDiscovery(eq(TENANT_NAME))).andReturn(
         idmPrincipalDiscovery);
      TokenValidator tokenValidator = createMock(TokenValidator.class);
      replay(idm, idmAuthenticator, idmPrincipalDiscovery, tokenValidator);

      com.vmware.identity.sts.auth.Authenticator authenticator = authenticatorFactory
         .getAuthenticator(TENANT_NAME, tokenValidator);
      assertNotNull(authenticator);

      verify(idm, idmAuthenticator, idmPrincipalDiscovery, tokenValidator);
   }

}
