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

import java.util.Properties;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.ActiveDirectoryJoinInfo;

public class VmafADDomainAPITest
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

   @Ignore("Disable --- we can run the SystemManagementServiceTest in dev environment for coverage")
   @Test
   public void testIdmVmafd() throws Exception
   {
      final String admin = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_ADMIN);
      final String password = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_ADMIN_PASSWORD);
      final String domain = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_3_DOMAIN_NAME);
      final String orgUnit = null;

      ActiveDirectoryJoinInfo joinInfo = null;
      try {
         client.getActiveDirectoryJoinStatus();
      } catch (Exception e)
      {
         Assert.fail(String.format("query domain failed, msg: [%s]", e.getMessage()));
      }

      try {
         client.joinActiveDirectory(admin, password, domain, orgUnit);
      } catch (Exception e)
      {
         Assert.fail(String.format("join domain failed, msg: [%s]", e.getMessage()));
      }

      try {
         joinInfo = client.getActiveDirectoryJoinStatus();
         Assert.assertNotNull(joinInfo);
         Assert.assertFalse(joinInfo.getName().isEmpty());
         Assert.assertFalse(joinInfo.getAlias().isEmpty());
      } catch(Exception e)
      {
         Assert.fail(String.format("couldn't query result after join, msg: [%d]", e.getMessage()));
      }

      try {
         client.leaveActiveDirectory(admin, password);
      } catch (Exception e)
      {
         Assert.fail(String.format("leave domain failed, msg: [%s]", e.getMessage()));
      }
      return;
   }
}
