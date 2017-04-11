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
import java.util.HashSet;
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
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.KnownSamlAttributes;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.performance.PerformanceMonitorFactory;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.LdapConnectionPool;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.activedirectory.ActiveDirectoryProvider;
import com.vmware.identity.idm.server.provider.ldap.LdapWithAdMappingsProvider;

public class LdapWithAdMappingsProviderTest
{
   // bugzilla#1173915 - unable to create NativeAd due to changes. Need to adjust the tests ....
   //private static ServerIdentityStoreData realADProviderstoreData = null;
   private static ServerIdentityStoreData ldapADProviderstoreData = null;
   private static ServerIdentityStoreData ldapSADProviderstoreData = null;

   private static final String TENANT_NAME = "vsphere.local";
   private static final String AD_DOMAIN_NAME = "ssolabs.eng.vmware.com";
   private static final String AD_DOMAIN_ALIAS = "SSOLABS";
   private static final String AD_DOMAIN_NAME_USER = AD_DOMAIN_NAME;
   private static final String AD_LDAPS_DOMAIN_NAME = "ssovcui.com";
   private static final String AD_LDAPS_DOMAIN_ALIAS = "ssovcui";

   private static String NESTED_GROUP_USER_NAME = "user_ngs";

   private static String NESTED_GROUP01 = AD_DOMAIN_NAME + "\\NG01";
   private static String NESTED_GROUP02 = AD_DOMAIN_NAME + "\\NG02";
   private static String NESTED_GROUP03 = AD_DOMAIN_NAME + "\\NG03";
   private static String NESTED_GROUP04 = AD_DOMAIN_NAME + "\\NG04";
   private static String NESTED_GROUP05 = AD_DOMAIN_NAME + "\\NG05";
   private static String NESTED_GROUP06 = AD_DOMAIN_NAME + "\\NG06";
   private static String NESTED_GROUP07 = AD_DOMAIN_NAME + "\\NG07";
   private static String NESTED_GROUP08 = AD_DOMAIN_NAME + "\\NG08";
   private static String NESTED_GROUP09 = AD_DOMAIN_NAME + "\\NG09";
   private static String NESTED_GROUP10 = AD_DOMAIN_NAME + "\\NG10";
   private static String NESTED_GROUP11 = AD_DOMAIN_NAME + "\\NG11";
   private static String NESTED_GROUP12 = AD_DOMAIN_NAME + "\\NG12";
   private static String NESTED_GROUP14 = AD_DOMAIN_NAME + "\\NG14";
   private static String NESTED_GROUP15 = AD_DOMAIN_NAME + "\\NG15";
   private static String NESTED_GROUP16 = AD_DOMAIN_NAME + "\\NG16";
   private static String NESTED_GROUP17 = AD_DOMAIN_NAME + "\\NG17";
   private static String NESTED_GROUP18 = AD_DOMAIN_NAME + "\\NG18";
   private static String DOMAIN_USERS_GROUP = AD_DOMAIN_NAME + "\\Domain Users";
   private static String USERS_GROUP = AD_DOMAIN_NAME + "\\Users";

   private static String BASE_GROUP_DN = "OU=GroupsOU,DC=ssolabs,DC=eng,DC=vmware,DC=com";

   // domain groups: NG01, NG02, NG03, NG04, NG05, NG06, NG14, NG15, NG16
   // OU groups:    NG07, NG08, NG09, NG10, NG11, NG12, NG17, NG18
   // membership:
   //     NG01: NG02
   //     NG02: NG06;
   //     NG03: user_ngs
   //     NG04: NG10
   //     NG05: user_ngs
   //     NG06: user_ngs
   //     NG07  NG08;
   //     NG08: NG12
   //     NG09: user_ngs
   //     NG10: user_ngs
   //     NG11: NG05;
   //     NG12: user_ngs
   //     NG14: user_ngs, NG16
   //     NG15: NG14
   //     NG16: NG15
   //     NG17: user_ngs, NG18
   //     NG18: NG17

   private static Set<String> userNestedDomainGroups;
   private static Set<String> userNestedDomainGroupsOU;
   private static Set<String> userNestedDomainGroupsOUMTC;
   private static Set<String> userDirectDomainGroups;
   private static Set<String> userDirectDomainGroupsOU;

   static
   {
      //bugzilla#1173915
      /*
      realADProviderstoreData = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, AD_DOMAIN_NAME);
      realADProviderstoreData.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY);
      realADProviderstoreData.setConnectionStrings(Arrays.asList("ldap://dc-1.ssolabs.eng.vmware.com"));
      realADProviderstoreData.setUserName("CN=Administrator,CN=Users,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      realADProviderstoreData.setPassword("ca$hc0w");
      realADProviderstoreData.setUserBaseDn( "DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      realADProviderstoreData.setGroupBaseDn("DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      realADProviderstoreData.setAlias(AD_DOMAIN_ALIAS);

      realADProviderstoreData.setAttributeMap(attrMap);
      */

      Map<String, String> attrMap = getAttributes();

      ldapADProviderstoreData = getLdapADData(attrMap);

      ldapSADProviderstoreData = getLdapsADData(attrMap);

      userNestedDomainGroups = new HashSet<String>();
      userNestedDomainGroups.addAll(
        Arrays.asList(
            new String[] { NESTED_GROUP01, NESTED_GROUP02, NESTED_GROUP03, NESTED_GROUP04, NESTED_GROUP05, NESTED_GROUP06,
            NESTED_GROUP07, NESTED_GROUP08, NESTED_GROUP09, NESTED_GROUP10, NESTED_GROUP11, NESTED_GROUP12,
            NESTED_GROUP14, NESTED_GROUP15, NESTED_GROUP16, NESTED_GROUP17, NESTED_GROUP18,
            DOMAIN_USERS_GROUP, USERS_GROUP} )
         );

      userNestedDomainGroupsOU = new HashSet<String>();
      userNestedDomainGroupsOU.addAll(
          Arrays.asList(
                new String[] { NESTED_GROUP07, NESTED_GROUP08, NESTED_GROUP09, NESTED_GROUP10, NESTED_GROUP12,
                NESTED_GROUP17, NESTED_GROUP18})
          );

      userNestedDomainGroupsOUMTC = new HashSet<String>();
      userNestedDomainGroupsOUMTC.addAll(
              Arrays.asList(
                    new String[] { NESTED_GROUP07, NESTED_GROUP08, NESTED_GROUP09, NESTED_GROUP10, NESTED_GROUP11, NESTED_GROUP12,
                    NESTED_GROUP17, NESTED_GROUP18})
              );

      userDirectDomainGroups = new HashSet<String>();
      userDirectDomainGroups.addAll(
          Arrays.asList(
               new String[] {
                   NESTED_GROUP03, NESTED_GROUP05, NESTED_GROUP06, NESTED_GROUP09, NESTED_GROUP10, NESTED_GROUP12, DOMAIN_USERS_GROUP,
                   NESTED_GROUP14, NESTED_GROUP17})
          );

      userDirectDomainGroupsOU = new HashSet<String>();
      userDirectDomainGroupsOU.addAll(
          Arrays.asList(
              new String[] { NESTED_GROUP09, NESTED_GROUP10, NESTED_GROUP12, NESTED_GROUP17 } )
          );

      PerformanceMonitorFactory.setPerformanceMonitor(new TestPerfMonitor());
      LdapConnectionPool.getInstance().createPool("vsphere.local");
   }
   // bugzilla#1173915
   //private static final IIdentityProvider realAdProvider = new ActiveDirectoryProvider(realADProviderstoreData);
   private static final IIdentityProvider ldapAdProvider = new LdapWithAdMappingsProvider(TENANT_NAME, ldapADProviderstoreData);
   private static final IIdentityProvider ldapsAdProvider = new LdapWithAdMappingsProvider(TENANT_NAME, ldapSADProviderstoreData);

   @BeforeClass
   public static void testSetup()
   {
   }

   @AfterClass
   public static void testTearDown()
   {
      //any cleanup here
   }

   @Test
   public void testAuthenticate()
   {
       testAuthenticate( ldapAdProvider );
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

      try
      {
          provider.checkUserAccountFlags(new PrincipalId("locked_user", AD_DOMAIN_NAME_USER));
          Assert.fail("checkUserAccountFlags for a locked user [locked_user] should throw.");
      }
      catch(UserAccountLockedException ex)
      {
          // expected exception
      }
      catch(Exception ex)
      {
          Assert.fail(String.format("Unexpected exception: [%s]. Expected UserAccountLockedException exception", ex.toString()));
      }
   }

   @Test
   public void testLdapsAuthenticate()
   {
      testAuthenticate(ldapsAdProvider);
   }

   @Ignore("bugzilla#1173915")
   @Test
   public void testGetAttributesMatches() throws Exception
   {
       /*
      List<Attribute> attrs = Arrays.asList(
              new Attribute(attrNameGivenName),
              new Attribute(attrNameSn),
              new Attribute(attrNameMemberOf),
              new Attribute(attrNameSubjectType),
              new Attribute(attrNameEmailAddress),
              new Attribute(attrNameUserPrincipalName)
              );

      PrincipalId id = new PrincipalId("lookup", AD_DOMAIN_NAME_USER);
      Collection<AttributeValuePair> attrValsUnmapped = realAdProvider.getAttributes(id, attrs);
      Collection<AttributeValuePair> attrValsMapped = ldapAdProvider.getAttributes(id, attrs);
      Assert.assertEquals( attrValsUnmapped.size(), attrValsMapped.size() );
      Iterator<AttributeValuePair> itr = attrValsUnmapped.iterator();
      Iterator<AttributeValuePair> itrRes = attrValsMapped.iterator();
      while (itr.hasNext() && itrRes.hasNext())
      {
         AttributeValuePair adResult = itr.next();
         AttributeValuePair ldapAdResult = itrRes.next();
         List<String> result = adResult.getValues();
         List<String> resultMapped = ldapAdResult.getValues();
         Assert.assertEquals(result, resultMapped);
      }
      */
   }

   @Test
   public void testGetAttributesInvalid()
   {
       testGetAttributesInvalid(ldapAdProvider);
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
       testFindUser(ldapAdProvider);
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

   @Ignore("bugzilla#1173915")
   @Test
   public void testFindMatches() throws Exception
   {
       /*
       Set<PersonUser> users = realAdProvider.findUsers("testuser", realADProviderstoreData.getName(), -1);
       Set<PersonUser> usersMapped = ldapAdProvider.findUsers("testuser", realADProviderstoreData.getName(), -1);

       Assert.assertEquals(users, usersMapped);
       for(PersonUser pu : users)
       {
           Boolean isActive = realAdProvider.IsActive(pu.getId());;
           Boolean isActiveMapped = ldapAdProvider.IsActive(pu.getId());

           Assert.assertEquals(isActive, isActiveMapped);

           Set<Group> groups = realAdProvider.findDirectParentGroups(pu.getId());
           Set<Group> groupsMapped = ldapAdProvider.findDirectParentGroups(pu.getId());

           Assert.assertEquals(groups, groupsMapped);
           groups = realAdProvider.findNestedParentGroups(pu.getId());
           groupsMapped = ldapAdProvider.findNestedParentGroups(pu.getId());

           Assert.assertEquals(groups, groupsMapped);

           for(Group g : groups)
           {
               Set<PersonUser> users1 = null;
               users1 = realAdProvider.findUsersInGroup(g.getId(), "", -1);

               Set<PersonUser> usersMapped1 = ldapAdProvider.findUsersInGroup(g.getId(), "", -1);

               Assert.assertEquals(users1, usersMapped1);
               Set<Group> group1 = realAdProvider.findGroupsInGroup(g.getId(), "", -1);
               Set<Group> groupMapped1 = ldapAdProvider.findGroupsInGroup(g.getId(), "", -1);

               Assert.assertEquals(group1, groupMapped1);
           }
       }

       users = realAdProvider.findDisabledUsers("testuser", -1);
       usersMapped = ldapAdProvider.findDisabledUsers("testuser", -1);
       Assert.assertEquals(users, usersMapped);

       users = realAdProvider.findLockedUsers("locked", -1);
       usersMapped = ldapAdProvider.findLockedUsers("locked", -1);
       Assert.assertEquals(users, usersMapped);
       */
   }

   @Ignore("bugzilla#1173915")
   @Test
   public void testFindGroupsMatches() throws Exception
   {
       /*
       Set<Group> groups = realAdProvider.findGroups("JohnGroup-482", realADProviderstoreData.getName(), -1);
       Set<Group> groupsMapped = ldapAdProvider.findGroups("JohnGroup-482", realADProviderstoreData.getName(), -1);

       Assert.assertEquals(groups, groupsMapped);
       for(Group g : groups)
       {
           Set<PersonUser> users1 = realAdProvider.findUsersInGroup(g.getId(), "", -1);
           Set<PersonUser> usersMapped1 = ldapAdProvider.findUsersInGroup(g.getId(), "", -1);

           Assert.assertEquals(users1, usersMapped1);
           Set<Group> group1 = realAdProvider.findGroupsInGroup(g.getId(), "", -1);
           Set<Group> groupMapped1 = ldapAdProvider.findGroupsInGroup(g.getId(), "", -1);

           Assert.assertEquals(group1, groupMapped1);
       }
       */
   }

   @Ignore("bugzilla#1173915")
   @Test
   public void testFindPersonUserMatches() throws Exception
   {
       /*
      PrincipalId testUserId = new PrincipalId("lookup", AD_DOMAIN_NAME_USER);
      PersonUser user = realAdProvider.findUser(testUserId);

      PersonUser userMapped = ldapAdProvider.findUser(testUserId);

      verifyResult(user, userMapped);
      */
   }

   @Test
   public void testFindGroupsByNameInGroup() throws Exception
   {
       testFindGroupsByNameInGroup(ldapAdProvider);
   }

   @Ignore("bugzilla#1173915")
   @Test
   public void testFindGroupsByNameInGroupAD() throws Exception
   {
       /*
       testFindGroupsByNameInGroup(realAdProvider);
       */
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
       testFindUsersByNameInGroup(ldapAdProvider);
   }

   @Ignore("bugzilla#1173915")
   @Test
   public void testFindUsersByNameInGroupAD() throws Exception
   {
       /*
       testFindUsersByNameInGroup(realAdProvider);
       */
   }

   @Test
   public void testNestedTokenGroups() throws Exception
   {
       ServerIdentityStoreData data = getLdapADData(getAttributes());
       LdapWithAdMappingsProvider adOverLdap = null;

       // recursive domain scope
       data.setFlags(LdapWithAdMappingsProvider.FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS);
       // should not take effect
       data.setGroupBaseDn(BASE_GROUP_DN);
       adOverLdap = new LdapWithAdMappingsProvider(TENANT_NAME, data);
       testGetGroupsAttribute( adOverLdap, userNestedDomainGroups );

       // recusrive base dn
       data.setFlags( 0 );
       data.setGroupBaseDn(BASE_GROUP_DN);
       adOverLdap = new LdapWithAdMappingsProvider(TENANT_NAME, data);
       testGetGroupsAttribute( adOverLdap, userNestedDomainGroupsOU );

       // matching rule domain scope
       data.setFlags( LdapWithAdMappingsProvider.FLAG_AD_MATCHING_RULE_IN_CHAIN | LdapWithAdMappingsProvider.FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS );
       // should not take effect
       data.setGroupBaseDn(BASE_GROUP_DN);
       adOverLdap = new LdapWithAdMappingsProvider(TENANT_NAME, data);
       testGetGroupsAttribute( adOverLdap, userNestedDomainGroups );

       // matching rule base dn
       data.setFlags( LdapWithAdMappingsProvider.FLAG_AD_MATCHING_RULE_IN_CHAIN );
       data.setGroupBaseDn(BASE_GROUP_DN);
       adOverLdap = new LdapWithAdMappingsProvider(TENANT_NAME, data);
       testGetGroupsAttribute( adOverLdap, userNestedDomainGroupsOUMTC );

       // direct group domain scope
       data.setFlags( LdapWithAdMappingsProvider.FLAG_DIRECT_GROUPS_ONLY | LdapWithAdMappingsProvider.FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS );
       // should not take effect
       data.setGroupBaseDn(BASE_GROUP_DN);
       adOverLdap = new LdapWithAdMappingsProvider(TENANT_NAME, data);
       testGetGroupsAttribute( adOverLdap, userDirectDomainGroups );

       // direct group base dn
       data.setFlags( LdapWithAdMappingsProvider.FLAG_DIRECT_GROUPS_ONLY );
       data.setGroupBaseDn(BASE_GROUP_DN);
       adOverLdap = new LdapWithAdMappingsProvider(TENANT_NAME, data);
       testGetGroupsAttribute( adOverLdap, userDirectDomainGroupsOU );
   }

   private static void testGetGroupsAttribute( IIdentityProvider provider, Set<String> expectedGroups )
       throws Exception
   {
       List<Attribute> attrs = Arrays.asList(
           new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_GROUPS)
       );

       PrincipalId id = new PrincipalId(NESTED_GROUP_USER_NAME, AD_DOMAIN_NAME);
       Collection<AttributeValuePair> attributeValues = provider.getAttributes(id, attrs);

       Assert.assertNotNull("attributeValues should not be null", attributeValues);
       Assert.assertTrue("attributeValues.size() > 0", attributeValues.size() > 0);

       HashSet<String> actual = new HashSet<String>();
       Iterator<AttributeValuePair> itr = attributeValues.iterator();
       while (itr.hasNext())
       {
           AttributeValuePair cur = itr.next();
           if ( KnownSamlAttributes.ATTRIBUTE_USER_GROUPS.equalsIgnoreCase(cur.getAttrDefinition().getName()) )
           {
              actual.addAll(cur.getValues());
              break;
           }
       }

       Assert.assertEquals("Group membership should match.", expectedGroups, actual);
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

   public static void verifyResult(PersonUser userA, PersonUser userB)
   {
      Assert.assertEquals(userA.getObjectId(), userB.getObjectId());
      Assert.assertEquals(userA.getAlias(), userB.getAlias());
      Assert.assertEquals(userA.getId(), userB.getId());
      Assert.assertEquals(userA.getDetail(), userB.getDetail());
      Assert.assertEquals(userA.isLocked(), userB.isLocked());
      Assert.assertEquals(userA.isDisabled(), userB.isDisabled());
   }

   private static Map<String, String> getAttributes()
   {
       Map<String, String> attrMap = new HashMap<String, String>();
       attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_FIRST_NAME, "givenName");
       attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_LAST_NAME, "sn");
       attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_GROUPS, "memberof");
       attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_SUBJECT_TYPE, "subjectType");
       attrMap.put("mail", "mail");
       attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME, "userPrincipalName");

       return attrMap;
   }

   private static ServerIdentityStoreData getLdapADData(Map<String, String> attributes)
   {
       ServerIdentityStoreData data = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, AD_DOMAIN_NAME);
       data.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING);
       data.setConnectionStrings(Arrays.asList("ldap://dc-1.ssolabs.eng.vmware.com"));
       data.setUserName("CN=Administrator,CN=Users,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
       data.setPassword("ca$hc0w");
       data.setUserBaseDn( "DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
       data.setGroupBaseDn("DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
       data.setAlias(AD_DOMAIN_ALIAS);
       data.setAttributeMap(attributes);

       return data;
}

   private static ServerIdentityStoreData getLdapsADData(Map<String, String> attributes)
   {
      ServerIdentityStoreData data = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, AD_DOMAIN_NAME);
      data.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING);
      data.setConnectionStrings(Arrays.asList("ldap://dc-1.ssolabs.eng.vmware.com"));
      data.setUserName("CN=Administrator,CN=Users,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      data.setPassword("ca$hc0w");
      data.setUserBaseDn( "DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      data.setGroupBaseDn("DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM");
      data.setAlias(AD_DOMAIN_ALIAS);
      data.setAttributeMap(attributes);

       return data;
   }
}
