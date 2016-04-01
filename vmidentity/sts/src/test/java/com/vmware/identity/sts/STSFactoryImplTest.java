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

import static com.vmware.identity.sts.TestUtil.TENANT_NAME;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.saml.SamlAuthorityFactory;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.AuthenticatorFactory;
import com.vmware.identity.sts.idm.IdmTenantServices;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.STSConfigExtractor;
import com.vmware.identity.sts.idm.SsoStatisticsService;
import com.vmware.identity.sts.impl.STSFactoryImpl;

public class STSFactoryImplTest {

   private AuthenticatorFactory authenticatorFactory;
   private SamlAuthorityFactory samlFactory;
   private STSFactory stsFactory;
   private IdmTenantServices idm;

   @Before
   public void init() {
      authenticatorFactory = createMock(AuthenticatorFactory.class);
      samlFactory = createMock(SamlAuthorityFactory.class);
      idm = createMock(IdmTenantServices.class);
      stsFactory = new STSFactoryImpl(authenticatorFactory, samlFactory, idm);
   }

   @Test
   public void testGetSTS_CreateAndSaveInCache() throws Exception {
      testCreateAndGetSTS(false);
   }

   @Test
   public void testGetSTS_CreateAndSaveInCacheThenGet() throws Exception {
      testCreateAndGetSTS(true);
   }

   private void testCreateAndGetSTS(boolean getSecondTime) {
      TokenAuthority tokenAuthority = createMock(TokenAuthority.class);
      TokenValidator tokenValidator = createMock(TokenValidator.class);
      TokenValidator authOnlyTokenValidator = createMock(TokenValidator.class);
      Authenticator authenticator = createMock(Authenticator.class);
      PrincipalDiscovery principalDiscovery = createMock(PrincipalDiscovery.class);
      STSConfigExtractor stsConfig = createMock(STSConfigExtractor.class);
      SsoStatisticsService ssoStatisticsService = createMock(SsoStatisticsService.class);

      expect(samlFactory.createTokenServices(eq(TENANT_NAME)))
         .andReturn(
            new SamlAuthorityFactory.TokenServices(tokenAuthority,
               tokenValidator, authOnlyTokenValidator));
      expect(
         authenticatorFactory.getAuthenticator(eq(TENANT_NAME),
            eq(authOnlyTokenValidator))).andReturn(authenticator);

      expect(idm.principalDiscovery(eq(TENANT_NAME))).andReturn(
         principalDiscovery);
      expect(idm.stsConfigService(eq(TENANT_NAME))).andReturn(stsConfig);
      expect(idm.ssoStatisticsService(eq(TENANT_NAME))).andReturn(ssoStatisticsService);

      replay(samlFactory, authenticatorFactory, idm, tokenAuthority,
         principalDiscovery, tokenValidator, authOnlyTokenValidator, authenticator, stsConfig, ssoStatisticsService);

      STS createdInCache = stsFactory.getSTS(TENANT_NAME);
      if (getSecondTime) {
         STS getFromCache = stsFactory.getSTS(TENANT_NAME);
         Assert.assertSame(createdInCache, getFromCache);
      }

      verify(samlFactory, authenticatorFactory, idm, tokenAuthority,
         principalDiscovery, tokenValidator, authOnlyTokenValidator, authenticator, stsConfig, ssoStatisticsService);
   }

}