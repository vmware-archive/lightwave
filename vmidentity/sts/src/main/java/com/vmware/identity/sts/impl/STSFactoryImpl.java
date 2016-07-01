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

import java.util.Date;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.saml.SamlAuthorityFactory;
import com.vmware.identity.saml.SamlAuthorityFactory.TokenServices;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.STS;
import com.vmware.identity.sts.STSFactory;
import com.vmware.identity.sts.auth.AuthenticatorFactory;
import com.vmware.identity.sts.idm.IdmTenantServices;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.SsoStatisticsService;

public final class STSFactoryImpl implements STSFactory {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(STSFactoryImpl.class);

   // TODO [848560] extract this value as a STS module parameter
   private static final long INSTANCE_LIFETIME = 30 * 60 * 1000;

   private final Cache stsCache = new Cache(INSTANCE_LIFETIME);

   private final AuthenticatorFactory authenticatorFactory;
   private final SamlAuthorityFactory authorityFactory;
   private final IdmTenantServices idm;

   public STSFactoryImpl(AuthenticatorFactory authenticatorFactory,
      SamlAuthorityFactory authorityFactory, IdmTenantServices idm) {
      assert authenticatorFactory != null;
      assert authorityFactory != null;
      assert idm != null;

      this.authenticatorFactory = authenticatorFactory;
      this.authorityFactory = authorityFactory;
      this.idm = idm;
   }

   @Override
   public STS getSTS(String tenantName) throws NoSuchIdPException,
      RequestFailedException {
      assert tenantName != null;

      STS sts = stsCache.get(tenantName);
      if (sts == null) {
          log.debug("Will create STS for tenant: {}", tenantName);
          sts = createSTSInstance(tenantName);
          log.debug("Created STS for tenant: {} at {}", tenantName, new Date());
          stsCache.put(tenantName, sts);
          log.debug("Cached STS for tenant: {}", tenantName);
      }
      log.debug("Get STS for tenant: {}", tenantName);
      return sts;
   }

   private STS createSTSInstance(String tenantName) throws NoSuchIdPException,
      RequestFailedException {
      assert tenantName != null;

      final TokenServices tokenServices;
      try {
         tokenServices = authorityFactory.createTokenServices(tenantName);
      } catch (com.vmware.identity.saml.NoSuchIdPException e) {
         throw new NoSuchIdPException(e);
      } catch (SystemException e) {
         throw new RequestFailedException(e);
      }
      // TODO add here an appropriate exception to catch
      // (SystemConfigurationException e)
      final PrincipalDiscovery principalDiscovery = idm
         .principalDiscovery(tenantName);
      final SsoStatisticsService ssoStatisticsService = idm
         .ssoStatisticsService(tenantName);
      return new STSImpl(tokenServices.getAuthority(),
         tokenServices.getValidator(), tokenServices.getAuthnOnlyValidator(), authenticatorFactory.getAuthenticator(
            tenantName, tokenServices.getAuthnOnlyValidator()), new DelegationParser(
            principalDiscovery), idm.stsConfigService(tenantName), principalDiscovery, ssoStatisticsService);
   }

   private static final class Cache {
      private final Map<String, CachedSTS> stsCache = new ConcurrentHashMap<String, CachedSTS>();
      private final long invalidateInstanceTimeoutMillis;

      public Cache(long invalidateInstanceTimeoutMillis) {
         this.invalidateInstanceTimeoutMillis = invalidateInstanceTimeoutMillis;
      }

      public STS get(String tenantName) {
         CachedSTS cachedSTS = stsCache.get(tenantName);

         return cachedSTS == null || isExpired(cachedSTS) ? null : cachedSTS
            .getSTSInstance();
      }

      public void put(String tenantName, STS sts) {
         stsCache.put(tenantName, new CachedSTS(sts));
      }

      private boolean isExpired(CachedSTS cachedSTS) {
         return cachedSTS.getSTSAge() > invalidateInstanceTimeoutMillis;
      }

      private static class CachedSTS {
         private final STS sts;
         private final Date createdOn;

         public CachedSTS(STS sts) {
            assert sts != null;

            this.sts = sts;
            this.createdOn = new Date();
         }

         public STS getSTSInstance() {
            return sts;
         }

         public long getSTSAge() {
            return new Date().getTime() - createdOn.getTime();
         }
      }
   }

}
