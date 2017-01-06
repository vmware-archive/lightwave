/*
 *
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
 *
 */

package com.vmware.identity.idm.server;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.security.auth.login.LoginException;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping.ObjectIds;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.performance.IPerformanceMonitor;
import com.vmware.identity.idm.server.performance.IdmAuthStatCache;
import com.vmware.identity.idm.server.performance.PerformanceMonitorFactory;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.LdapConnectionPool;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.activedirectory.ActiveDirectoryProvider;
import com.vmware.identity.idm.server.provider.ldap.LdapWithAdMappingsProvider;


public class ADProviderTest
{
   private static ServerIdentityStoreData storeData = null;
   private static ServerIdentityStoreData storeDataSchemaMapped = null;
   private static final String attrNameGivenName = "givenName";
   private static final String attrSamAccountName = "sAMAccountName";
   private static final String attrNameSn = "sn";
   private static final String attrNameMemberOf = "memberof";
   private static final String attrNameSubjectType = "subjectType";
   private static final String attrNameEmailAddress = "mail";
   private static final String attrNameUserPrincipalName = "userPrincipalName";

   private static final String AD_DOMAIN_NAME = "ssolabs.eng.vmware.com";
   private static final String AD_DOMAIN_ALIAS = "SSOLABS";
   private static final String AD_DOMAIN_NAME_USER = AD_DOMAIN_NAME;

   static
   {
      storeData = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, AD_DOMAIN_NAME);
      // bugzilla#1173915 -- this test supposedly should test real ad provider
      // but was switched i think once the AD changes were done ....
      // we should ensure we test native AD too...
      storeData.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING);
      storeData.setConnectionStrings(Arrays.asList("ldap://dc-1.ssolabs.eng.vmware.com"));
      storeData.setUserName("CN=Administrator,CN=Users,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      storeData.setPassword("ca$hc0w");
      storeData.setUserBaseDn( "DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      storeData.setGroupBaseDn("DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      storeData.setAlias(AD_DOMAIN_ALIAS);
      storeData.setAccountLinkingUseUPN(false);
      storeData.setHintAttributeName(attrNameUserPrincipalName);

      Map<String, String> attrMap = new HashMap<String, String>();
      attrMap.put(attrNameGivenName, attrNameGivenName);
      attrMap.put(attrNameSn, attrNameSn);
      attrMap.put(attrNameMemberOf, attrNameMemberOf);
      attrMap.put(attrNameSubjectType, attrNameSubjectType);
      attrMap.put(attrNameEmailAddress, attrNameEmailAddress);
      attrMap.put(attrNameUserPrincipalName, attrNameUserPrincipalName);
      storeData.setAttributeMap(attrMap);

      storeDataSchemaMapped = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, AD_DOMAIN_NAME);
      storeDataSchemaMapped.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING);
      storeDataSchemaMapped.setConnectionStrings(Arrays.asList("ldap://dc-1.ssolabs.eng.vmware.com"));
      storeDataSchemaMapped.setUserName("CN=Administrator,CN=Users,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      storeDataSchemaMapped.setPassword("ca$hc0w");
      storeDataSchemaMapped.setUserBaseDn( "DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      storeDataSchemaMapped.setGroupBaseDn("DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      storeDataSchemaMapped.setAlias(AD_DOMAIN_ALIAS);
      storeDataSchemaMapped.setAttributeMap(attrMap);
      storeDataSchemaMapped.setSchemaMapping(getADSchemaMapping());

      PerformanceMonitorFactory.setPerformanceMonitor(new TestPerfMonitor());
      LdapConnectionPool.getInstance().createPool("vsphere.local");
   }
   private static final IIdentityProvider unMappedprovider = new LdapWithAdMappingsProvider("vsphere.local", storeData);
   private static final IIdentityProvider schemaMappedprovider = new LdapWithAdMappingsProvider("vsphere.local", storeDataSchemaMapped);

   @BeforeClass
   public static void testSetup()
   {
   }

   @AfterClass
   public static void testTearDown()
   {
      //any cleanup here
   }

   @Ignore("enable once ad provider can handle null perf data sync ...")
   @Test
   public void testAuthenticate()
   {
       testAuthenticate( unMappedprovider );
   }

   @Ignore("enable once ad provider can handle null perf data sync ...")
   @Test
   public void testAuthenticateSchemaMapped()
   {
       testAuthenticate( schemaMappedprovider );
   }

   private static void testAuthenticate(IIdentityProvider provider)
   {
      Map<PrincipalId, String> validCreds = new HashMap<PrincipalId, String>();
      validCreds.put(new PrincipalId("lookup", AD_DOMAIN_NAME_USER), "vmware123$");
      validCreds.put(new PrincipalId("lookup", AD_DOMAIN_NAME_USER), "vmware123$");

      for (PrincipalId user: validCreds.keySet())
      {
         try {
            PrincipalId res = provider.authenticate(user, validCreds.get(user));
            Assert.assertEquals(user, res);
         }
         catch (LoginException e)
         {
            Assert.fail(e.getMessage());
         }
      }

      Map<PrincipalId, String> invalidCreds = new HashMap<PrincipalId, String>();
      invalidCreds.put(new PrincipalId("nonExistant", AD_DOMAIN_NAME_USER), "invalidPwd");
      invalidCreds.put(new PrincipalId("lookup", AD_DOMAIN_NAME_USER), "1xxxxxxxxxxxxxx");
      for (PrincipalId invalidUser : invalidCreds.keySet())
      {
         try {
            provider.authenticate(invalidUser, invalidCreds.get(invalidUser));
         }
         catch (LoginException e)
         {
            //expected exception
            continue;
         }
         Assert.assertTrue("should not reach here", false);
      }
   }

   @Test
   public void testGetAttributesMatches() throws Exception
   {
      List<Attribute> attrs = Arrays.asList(
              new Attribute(attrNameGivenName),
              new Attribute(attrNameSn),
              new Attribute(attrNameMemberOf),
              new Attribute(attrNameSubjectType),
              new Attribute(attrNameEmailAddress),
              new Attribute(attrNameUserPrincipalName)
              );

      PrincipalId id = new PrincipalId("lookup", AD_DOMAIN_NAME_USER);
      Collection<AttributeValuePair> attrValsUnmapped = unMappedprovider.getAttributes(id, attrs);
      Collection<AttributeValuePair> attrValsMapped = schemaMappedprovider.getAttributes(id, attrs);
      Assert.assertEquals( attrValsUnmapped.size(), attrValsMapped.size() );
      Iterator<AttributeValuePair> itr = attrValsUnmapped.iterator();
      Iterator<AttributeValuePair> itrRes = attrValsMapped.iterator();
      while (itr.hasNext() && itrRes.hasNext())
      {
         List<String> result = itr.next().getValues();
         List<String> resultMapped = itrRes.next().getValues();
         Assert.assertEquals(result, resultMapped);
      }

   }

   @Test
   public void testGetAttributesInvalid()
   {
       testGetAttributesInvalid(unMappedprovider);
   }

   @Test
   public void testGetAttributesInvalidSchemaMapped()
   {
       testGetAttributesInvalid(schemaMappedprovider);
   }

   private static void testGetAttributesInvalid(IIdentityProvider provider)
   {
      PrincipalId id = new PrincipalId("lookup", AD_DOMAIN_NAME_USER);
      List<Attribute> invalidList = Arrays.asList(new Attribute("unknownAttrName"));
      try {
         provider.getAttributes(id, invalidList);
         Assert.fail("getting invalid attributes should fail.");
      }
      catch (IllegalArgumentException e)
      {
         Assert.assertTrue(e.getMessage().contains("No attribute mapping found"));
      }
      catch(Exception ex)
      {
          Assert.fail(String.format("Unexpected exception: [%s].", ex.toString()));
      }
   }

   @Test
   public void testFindUser() throws Exception
   {
       testFindUser(unMappedprovider);
   }

   @Test
   public void testFindUserSchemaMapped() throws Exception
   {
       testFindUser(schemaMappedprovider);
   }

   private static void testFindUser(IIdentityProvider provider) throws Exception
   {
       PrincipalId user = new PrincipalId("lookup", AD_DOMAIN_NAME_USER);
       PersonUser result = provider.findUser(user);

       Assert.assertNotNull(result);
       Assert.assertEquals("", user.getName(), result.getId().getName());

      PrincipalId nonExistant = new PrincipalId("nonExistant", AD_DOMAIN_NAME_USER);
      try {
         provider.findUser(nonExistant);
      }
      catch (InvalidPrincipalException e)
      {
         //expected exception
         return;
      }
      Assert.assertTrue("should not reach here", false);
   }

   @Test
   public void testFindActiveUser() throws Exception
   {
       PrincipalId user = new PrincipalId("lookup", AD_DOMAIN_NAME_USER);
       IIdentityProvider provider =  schemaMappedprovider;
       try {
           PrincipalId result = provider.findActiveUser(attrNameUserPrincipalName, user.getUPN());
           Assert.assertNotNull(result);
           Assert.assertEquals("", user.getName(), result.getName());
        }
        catch (Exception e)
        {
            Assert.fail(String.format("Failed to find the user by UPN", user.getUPN()) );
        }

       String accountName = "lookup";
       try {
           PrincipalId result = provider.findActiveUser(attrSamAccountName, accountName);
           Assert.assertNotNull(result);
           Assert.assertEquals("", user.getName(), result.getName());
        }
        catch (InvalidPrincipalException e)
        {
            Assert.fail(String.format("Failed to find the user by account name %s.", accountName) );
        }

       String givenName = "lookup";
       try {
           PrincipalId result = provider.findActiveUser(attrNameGivenName, givenName);
           Assert.assertNotNull(result);
           Assert.assertEquals("", user.getName(), result.getName());
        }
        catch (InvalidPrincipalException e)
        {
            Assert.fail(String.format("Failed to find the user by account name %s.", givenName) );
        }
        catch (Exception e) {
            Assert.fail("Unexpected search result.");
        }


   }

   @Test
   public void testFindMatches() throws Exception
   {
       Set<PersonUser> users = unMappedprovider.findUsers("testuser", storeData.getName(), -1);
       Set<PersonUser> usersMapped = schemaMappedprovider.findUsers("testuser", storeData.getName(), -1);

       Assert.assertEquals(users, usersMapped);
       for(PersonUser pu : users)
       {
           Boolean isActive = unMappedprovider.IsActive(pu.getId());;
           Boolean isActiveMapped = schemaMappedprovider.IsActive(pu.getId());

           Assert.assertEquals(isActive, isActiveMapped);

           PrincipalGroupLookupInfo groups = unMappedprovider.findDirectParentGroups(pu.getId());
           PrincipalGroupLookupInfo groupsMapped = schemaMappedprovider.findDirectParentGroups(pu.getId());

           Assert.assertEquals(groups, groupsMapped);
           groups = unMappedprovider.findNestedParentGroups(pu.getId());
           groupsMapped = schemaMappedprovider.findNestedParentGroups(pu.getId());

           Assert.assertEquals(groups, groupsMapped);
           if (groups != null)
              for(Group g :  groups.getGroups())
              {
                  Set<PersonUser> users1 = null;
                  users1 = unMappedprovider.findUsersInGroup(g.getId(), "", -1);

                  Set<PersonUser> usersMapped1 = schemaMappedprovider.findUsersInGroup(g.getId(), "", -1);

                  Assert.assertEquals(users1, usersMapped1);
                  Set<Group> group1 = unMappedprovider.findGroupsInGroup(g.getId(), "", -1);
                  Set<Group> groupMapped1 = schemaMappedprovider.findGroupsInGroup(g.getId(), "", -1);

                  Assert.assertEquals(group1, groupMapped1);
              }
       }

       users = unMappedprovider.findDisabledUsers("testuser", -1);
       usersMapped = schemaMappedprovider.findDisabledUsers("testuser", -1);
       Assert.assertEquals(users, usersMapped);

       users = unMappedprovider.findLockedUsers("locked", -1);
       usersMapped = schemaMappedprovider.findLockedUsers("locked", -1);
       Assert.assertEquals(users, usersMapped);
   }

   @Test
   public void testFindGroupsMatches() throws Exception
   {
       Set<Group> groups = unMappedprovider.findGroups("JohnGroup-482", storeData.getName(), -1);
       Set<Group> groupsMapped = schemaMappedprovider.findGroups("JohnGroup-482", storeData.getName(), -1);

       Assert.assertEquals(groups, groupsMapped);
       for(Group g : groups)
       {
           Set<PersonUser> users1 = unMappedprovider.findUsersInGroup(g.getId(), "", -1);
           Set<PersonUser> usersMapped1 = schemaMappedprovider.findUsersInGroup(g.getId(), "", -1);

           Assert.assertEquals(users1, usersMapped1);
           Set<Group> group1 = unMappedprovider.findGroupsInGroup(g.getId(), "", -1);
           Set<Group> groupMapped1 = schemaMappedprovider.findGroupsInGroup(g.getId(), "", -1);

           Assert.assertEquals(group1, groupMapped1);
       }
   }



    @Test
    public void testFindGroupsByNameInGroup() throws Exception
    {
        testFindGroupsByNameInGroup(unMappedprovider);
    }

    @Test
    public void testFindGroupsByNameInGroupSchemaMapped() throws Exception
    {
        testFindGroupsByNameInGroup(schemaMappedprovider);
    }

    private static void testFindGroupsByNameInGroup(IIdentityProvider provider) throws Exception
    {
        PrincipalId idGroup = new PrincipalId("Group1", AD_DOMAIN_NAME);

        Set<Group> findAnyGroup = provider.findGroupsByNameInGroup(idGroup, "", -1);
        Set<Group> findGroupByName = provider.findGroupsByNameInGroup(idGroup, "grpFgPwdPolicy", -1);
        Set<Group> findGroupByPrefix = provider.findGroupsByNameInGroup(idGroup, "grp", -1);

        Assert.assertTrue("Unexpected size", findAnyGroup.size() == 1);
        Assert.assertTrue("Unexpected size", findGroupByName.size() == 1);
        Assert.assertTrue("Unexpected size", findGroupByPrefix.size() == 1);

        Assert.assertArrayEquals(findAnyGroup.toArray(), findGroupByName.toArray());
        Assert.assertArrayEquals(findAnyGroup.toArray(), findGroupByPrefix.toArray());
    }

    @Test
    public void testFindUsersByNameInGroup() throws Exception
    {
        testFindUsersByNameInGroup(unMappedprovider);
    }

    @Test
    public void testFindUsersByNameInGroupSchemaMapped() throws Exception
    {
        testFindUsersByNameInGroup(schemaMappedprovider);
    }

    private static void testFindUsersByNameInGroup(IIdentityProvider provider) throws Exception
    {
        Map<PrincipalId, Integer> sizeFromGroupId = new HashMap<PrincipalId, Integer>();
        sizeFromGroupId.put(new PrincipalId("JohnGroup-482", AD_DOMAIN_NAME_USER), null);

        final String strJohn = "lookup";
        final String strGivenNamePrefix = "givenName-John";

        for (PrincipalId groupId : sizeFromGroupId.keySet()) {
            Set<PersonUser> resultByCN = provider.findUsersByNameInGroup(groupId, strJohn, -1);
            Set<PersonUser> resultByGivenNamePrefix = provider.findUsersByNameInGroup(groupId, strGivenNamePrefix, -1);

            Assert.assertArrayEquals(resultByCN.toArray(), resultByGivenNamePrefix.toArray());
        }
    }


   private static IdentityStoreSchemaMapping getADSchemaMapping() {
       IdentityStoreSchemaMapping.Builder schemaMapBuilder = new IdentityStoreSchemaMapping.Builder();
       IdentityStoreObjectMapping.Builder objectMapBuilder = null;
       objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdUser);
       objectMapBuilder.setObjectClass("user");
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "userAccountControl"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "sAMAccountName"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "description"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "displayname"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "mail"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "lockoutTime"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, "memberof"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "objectSid"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, "msDS-ResultantPSO"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, "primaryGroupID"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, "userPrincipalName"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, "pwdLastSet"));
       schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());


       objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdGroup);
       objectMapBuilder.setObjectClass("group");
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "sAMAccountName"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "description"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, "memberof"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "member"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "objectSid"));
       schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

       objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdPasswordSettings);
       objectMapBuilder.setObjectClass("msDS-PasswordSettings");
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, "msDS-MaximumPasswordAge"));
       schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

       objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdDomain);
       objectMapBuilder.setObjectClass("domain");
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, "maxPwdAge"));
       schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

       return schemaMapBuilder.buildSchemaMapping();
   }

}

class TestPerfMonitor implements IPerformanceMonitor {

    @Override
    public IdmAuthStatCache getCache(String tenantName) {
	return new IdmAuthStatCache(getDefaultCacheSize(), false);
    }

    @Override
    public void deleteCache(String tenantName) {
    }

    @Override
    public int getDefaultCacheSize() {
        return 0;
    }

    @Override
    public boolean summarizeLdapQueries() {
        return false;
    }
}
