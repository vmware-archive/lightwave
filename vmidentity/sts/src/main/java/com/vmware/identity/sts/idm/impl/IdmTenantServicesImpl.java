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

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContext;
import com.vmware.identity.idm.IIdmServiceContext;
import com.vmware.identity.idm.IdmServiceContextFactory;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.idm.client.IServiceContextProvider;
import com.vmware.identity.sts.idm.Authenticator;
import com.vmware.identity.sts.idm.IdmTenantServices;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.STSConfigExtractor;
import com.vmware.identity.sts.idm.SsoStatisticsService;
import com.vmware.identity.sts.idm.SystemException;

/**
 * Insert your comment for IdmTenantServicesImpl here
 */
final class IdmTenantServicesImpl implements IdmTenantServices {

   private final CasIdmClient idmClient;

   /**
    *
    */
   IdmTenantServicesImpl(String idmHostName) {
      assert idmHostName != null;
      this.idmClient = new CasIdmClient(idmHostName, new IdmServiceContextProvider());
   }

   /**
    * {@inheritDoc}
    *
    * @throws SystemException
    */
   @Override
   public Authenticator authService(String tenantName) throws SystemException {
      return newImpl(tenantName);
   }

   /**
    * {@inheritDoc}
    *
    * @throws SystemException
    */
   @Override
   public STSConfigExtractor stsConfigService(String tenantName)
      throws SystemException {

      return newImpl(tenantName);
   }

   /**
    * {@inheritDoc}
    * 
    * @throws SystemException
    */
   @Override
   public PrincipalDiscovery principalDiscovery(String tenantName)
      throws SystemException {

      return newImpl(tenantName);
   }

   @Override
   public SsoStatisticsService ssoStatisticsService(String tenantName)
      throws SystemException {

      return newImpl(tenantName);
   }

   private AuthenticatorImpl newImpl(String tenantName) {
      return new AuthenticatorImpl(idmClient, tenantName);
   }

    private static class IdmServiceContextProvider extends IServiceContextProvider
    {
        public IdmServiceContextProvider() {}

        @Override
        public IIdmServiceContext getServiceContext()
        {
            IIdmServiceContext serviceContext = null;
            IDiagnosticsContext context = DiagnosticsContextFactory.getCurrentDiagnosticsContext();
            if( context != null )
            {
                serviceContext = IdmServiceContextFactory.getIdmServiceContext(context.getCorrelationId());
            }
            return serviceContext;
        }
    }
}
