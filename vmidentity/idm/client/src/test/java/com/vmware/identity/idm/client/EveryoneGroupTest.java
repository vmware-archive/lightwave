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

import java.util.Arrays;
import java.util.Collection;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SSOImplicitGroupNames;

public class EveryoneGroupTest
{

   private static Properties props = null;
   private static CasIdmClient client = null;

   @BeforeClass
   public static void init() throws Exception
   {
       props = IdmClientTestUtil.getProps();
       String hostname = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_HOSTNAME);
       Assert.assertNotNull(hostname);
       client = new CasIdmClient(hostname);
   }

   @AfterClass
   public static void tearDown() throws Exception
   {
      client = null;
   }


   @Test
   public void testGetEveryoneGroup() throws Exception
   {
      String tenant1 = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);

      IdmClientTestUtil.ensureTenantExists(client, tenant1);

      IdmClientTestUtil.ensureADIdentityStoreExistForTenant(client, tenant1);
      IdmClientTestUtil.ensureOLIdentityStoreExistForTenant(client, tenant1);

      //attributes set that includes group information
      List<Attribute> attrs = Arrays.asList(
                  new Attribute(IdmClientTestUtil.SAMLATTR_GIVEN_NAME),
                  new Attribute(IdmClientTestUtil.SAMLATTR_GROUP_IDENTITY),
                  new Attribute(IdmClientTestUtil.SAMLATTR_UPN));
      Collection<IIdentityStoreData> stores =
            client.getProviders(tenant1, EnumSet.of(DomainType.SYSTEM_DOMAIN));
      String systemDomainName = stores.iterator().next().getName();
      try {
         for (IIdentityStoreData storeData : client.getProviders(props
               .getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME)))         {
            String domainName = storeData.getName();
            Collection<AttributeValuePair> adminAttrs =
                 client.getAttributeValues(tenant1, new PrincipalId(
                       "Administrator", domainName), attrs);
            verifyEveryoneGroupsCount(adminAttrs, systemDomainName);
         }
      }
      finally
      {
         // clean up tenant
         IdmClientTestUtil.ensureTenantDoesNotExist(client, tenant1);
      }
   }

   @Test
   public void testGetEveryoneGroupForFspUser() throws Exception
   {
       String userTenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
       Assert.assertNotNull(userTenantName);

       IdmClientTestUtil.ensureTenantExists(client, userTenantName);
       IdmClientTestUtil.ensureADIdentityStoreExistForTenant(client, userTenantName);

      final String adUserName =
            props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_FSPTEST_ID);
      final String adDomainName =
            props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
       PrincipalId adUserId = new PrincipalId(adUserName, adDomainName);
       PersonUser adUser = client.findPersonUser(userTenantName, adUserId);
       Assert.assertNotNull(adUser);

       Collection<IIdentityStoreData> stores =
             client.getProviders(userTenantName, EnumSet.of(DomainType.SYSTEM_DOMAIN));
       String systemDomainName = stores.iterator().next().getName();

       try {
          // Add the FSP user to administrators group of system provider
          client.addUserToGroup(userTenantName, adUserId, "Administrators");

          //attributes set that includes group information
          List<Attribute> attrs = Arrays.asList(
                new Attribute(IdmClientTestUtil.SAMLATTR_GIVEN_NAME),
                new Attribute(IdmClientTestUtil.SAMLATTR_GROUP_IDENTITY),
                new Attribute(IdmClientTestUtil.SAMLATTR_UPN));

          Collection<AttributeValuePair> fspUserAttrs =
              client.getAttributeValues(userTenantName, adUserId, attrs);

          verifyEveryoneGroupsCount(fspUserAttrs, systemDomainName);
       }
       finally
       {
          client.removeFromLocalGroup(userTenantName, adUserId, "Administrators");
          IdmClientTestUtil.ensureTenantDoesNotExist(client, userTenantName);
       }
   }

   private void verifyEveryoneGroupsCount(Collection<AttributeValuePair> attrs, String systemDomainName)
   {
      int everyoneGroupsCount = 0;
      for (AttributeValuePair pair : attrs)
      {
         if (pair.getAttrDefinition().getName().equals(IdmClientTestUtil.SAMLATTR_GROUP_IDENTITY))
         {
            for (String groupName : pair.getValues())
            {
               String groupCN = groupName.substring(groupName.indexOf("\\")+1);
               if (groupCN.equals(SSOImplicitGroupNames.getEveryoneGroupName()))
               {
                  everyoneGroupsCount++;
                  String groupDomainName = groupName.substring(0, groupName.indexOf("\\"));
                  Assert.assertEquals(systemDomainName, groupDomainName);
               }
            }
         }
      }
      Assert.assertEquals(1, everyoneGroupsCount);
   }

}
