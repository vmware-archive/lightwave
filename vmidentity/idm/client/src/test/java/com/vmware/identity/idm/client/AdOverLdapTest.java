/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.idm.client;

import java.util.LinkedList;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.idm.IdentityStoreData;

public class AdOverLdapTest {
   private final String CFG_KEY_IDM_HOSTNAME = "idm.server.hostname";

   private Properties testProps;
   private CasIdmClient idmClient;

   @Test
   public void testMultipleTenantsWithSharedProviders() throws Exception {
      CasIdmClient idmClient = getIdmClient();

      List<String> tenants = new LinkedList<String>();
      tenants.add("test1.com");
      tenants.add("test2.com");

      try {
         IdentityStoreData store = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);
         List<IdentityStoreData> providers = new LinkedList<IdentityStoreData>();
         providers.add(store);
         providers.add(IdentityStoreData.CreateExternalIdentityStoreData(
                        "test." + store.getName(),
                        "test." + store.getExtendedIdentityStoreData().getAlias(),
                        store.getExtendedIdentityStoreData().getProviderType(),
                        store.getExtendedIdentityStoreData().getAuthenticationType(),
                        store.getExtendedIdentityStoreData().getFriendlyName(),
                        store.getExtendedIdentityStoreData().getSearchTimeoutSeconds(),
                        store.getExtendedIdentityStoreData().getUserName(),
                        store.getExtendedIdentityStoreData().getPassword(),
                        store.getExtendedIdentityStoreData().getUserBaseDn(),
                        store.getExtendedIdentityStoreData().getGroupBaseDn(),
                        store.getExtendedIdentityStoreData().getConnectionStrings(),
                        store.getExtendedIdentityStoreData().getAttributeMap()));

         for (String tenant : tenants) {
            IdmClientTestUtil.ensureTenantExists(idmClient, tenant);

            for (IdentityStoreData provider : providers) {
               idmClient.addProvider(tenant, provider);
               Set<String> suffixes = idmClient.getUpnSuffixes(tenant, provider.getName());
               Assert.assertEquals("There is more than one UPN suffix for the provider '" + provider.getName() +"'" , 1, suffixes.size());
            }
         }

      } finally {
         for (String tenant : tenants) {
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenant);
         }
      }

   }

   private synchronized CasIdmClient getIdmClient() throws Exception {
      if (idmClient == null) {
         Properties props = getTestProperties();

         String hostname = props.getProperty(CFG_KEY_IDM_HOSTNAME);
         Assert.assertNotNull(hostname);

         idmClient = new CasIdmClient(hostname);
      }

      return idmClient;
   }

   private synchronized Properties getTestProperties() throws Exception {
      if (testProps == null) {
         testProps = new Properties();
         testProps.load(getClass().getResourceAsStream("/config.properties"));
      }

      return testProps;
   }

}
