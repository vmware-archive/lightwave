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

import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
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
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.performance.PerformanceMonitorFactory;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.LdapConnectionPool;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.ldap.LdapProvider;

public class LdapProviderTest
{
   private static ServerIdentityStoreData storeData = null;
   private static ServerIdentityStoreData storeDataSchemaMapped = null;
   private static ServerIdentityStoreData storeDataWithSsl = null;
   private static final String attrNameGivenName = "givenName";
   private static final String attrNameSn = "sn";
   private static final String attrNameMemberOf = "memberOf";
   private static final String attrNameSubjectType = "subjectType";
   private static final String attrNameEmailAddress = "mail";
   private static final String attrNameUserPrincipalName = "userPrincipalName";

   private static final String OPENLDAP_DOMAIN_NAME = "ssolabs-openldap.eng.vmware.com";
   private static final String OPENLDAP_DOMAIN_ALIAS = "ssolabs-openldap";
   private static final String OPENLDAP_DOMAIN_NAME_USER = OPENLDAP_DOMAIN_NAME;
   private static final String OPENLDAP_DOMAIN_NAME_GROUP = OPENLDAP_DOMAIN_NAME;
   private final static String CFG_KEY_STS_KEYSTORE = "sts-store.jks";
   private final static String CFG_KEY_STS_KEYSTORE_PASSWORD = "ca$hc0w";
   private static final String CFG_KEY_LDAPS_KEY_ALIAS = "/slapd.cer";
   private static final String TENANT_NAME = "TestTenant";

   static
   {
      storeData = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, OPENLDAP_DOMAIN_NAME);
      storeData.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP);
      storeData.setConnectionStrings(Arrays.asList("ldap://openldap-1.ssolabs.eng.vmware.com"));
      storeData.setUserName("cn=administrator,dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeData.setPassword("123");
      storeData.setUserBaseDn( "dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeData.setGroupBaseDn("dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeData.setAlias("ssolabs-openldap");
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

      storeDataSchemaMapped = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, OPENLDAP_DOMAIN_NAME);
      storeDataSchemaMapped.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP);
//    storeDataSchemaMapped.setConnectionStrings(Arrays.asList("ldap://192.168.103.144"));
      storeDataSchemaMapped.setConnectionStrings(Arrays.asList("ldap://openldap-1.ssolabs.eng.vmware.com"));
      storeDataSchemaMapped.setUserName("cn=administrator,dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeDataSchemaMapped.setPassword("123");
      storeDataSchemaMapped.setUserBaseDn( "dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeDataSchemaMapped.setGroupBaseDn("dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeDataSchemaMapped.setAlias("ssolabs-openldap");
      storeDataSchemaMapped.setAttributeMap(attrMap);
      storeDataSchemaMapped.setSchemaMapping(getOpenLdapSchemaMapping());

      storeDataWithSsl = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN,
                  OPENLDAP_DOMAIN_NAME);
      storeDataWithSsl.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP);
      storeDataWithSsl.setConnectionStrings(Arrays.asList("ldaps://openldap-1.ssolabs.eng.vmware.com"));
      storeDataWithSsl.setUserName("cn=administrator,dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeDataWithSsl.setPassword("123");
      storeDataWithSsl.setUserBaseDn("dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeDataWithSsl.setGroupBaseDn("dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com");
      storeDataWithSsl.setAlias("ssolabs-openldap");
      storeDataWithSsl.setAttributeMap(attrMap);
      storeDataWithSsl.setCertificates(getLdapsCertificate());

      PerformanceMonitorFactory.setPerformanceMonitor(new TestPerfMonitor());
      LdapConnectionPool.getInstance().createPool(TENANT_NAME);
   }
   private static final IIdentityProvider unMappedprovider = new LdapProvider(TENANT_NAME, storeData, null);
   private static final IIdentityProvider schemaMappedprovider = new LdapProvider(TENANT_NAME, storeDataSchemaMapped, null);
   private static final IIdentityProvider ldapsProvider = new LdapProvider(TENANT_NAME, storeDataWithSsl, null);

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
       testAuthenticate( unMappedprovider );
   }

   @Test
   public void testAuthenticateSchemaMapped()
   {
       testAuthenticate( schemaMappedprovider );
   }

   @Test
   public void testAuthenticateWithSsl()
   {
       testAuthenticate(ldapsProvider);
   }

   private static void testAuthenticate(IIdentityProvider provider)
   {
      Map<PrincipalId, String> validCreds = new HashMap<PrincipalId, String>();
      validCreds.put(new PrincipalId("John-1", OPENLDAP_DOMAIN_NAME_USER), "1");
      validCreds.put(new PrincipalId("John-2", OPENLDAP_DOMAIN_NAME_USER), "2");

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
      invalidCreds.put(new PrincipalId("nonExistant", OPENLDAP_DOMAIN_NAME_USER), "invalidPwd");
      invalidCreds.put(new PrincipalId("John-1", OPENLDAP_DOMAIN_NAME_USER), "1xxxxxxxxxxxxxx");
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
   public void testGetAttributes() throws Exception
   {
       testGetAttributes(unMappedprovider);
   }

   @Test
   public void testGetAttributesSchemaMapped() throws Exception
   {
       testGetAttributes(schemaMappedprovider);
   }

   @Test
   public void testGetAttributesSSL() throws Exception
   {
       testGetAttributes(ldapsProvider);
   }

   private static void testGetAttributes(IIdentityProvider provider) throws Exception
   {

      List<Attribute> attrs = Arrays.asList(
            new Attribute(attrNameGivenName),
            new Attribute(attrNameSn),
            new Attribute(attrNameMemberOf),
            new Attribute(attrNameSubjectType),
            new Attribute(attrNameEmailAddress),
            new Attribute(attrNameUserPrincipalName)
            );

      Map<PrincipalId, Map<String, String>> resultFromPrincipalId = new HashMap<PrincipalId, Map<String, String>>();
      Map<String, String> attributeValues = new HashMap<String, String>();
      attributeValues.put( attrNameGivenName, "[givenName-John-1, gn222, gn333]" );
      attributeValues.put( attrNameSn, "[Smith, sn333, sn222++]" );
      attributeValues.put( attrNameMemberOf, "[ssolabs-openldap.eng.vmware.com\\Group-2, ssolabs-openldap.eng.vmware.com\\Group-1, ssolabs-openldap.eng.vmware.com\\Group-3]");
      attributeValues.put( attrNameEmailAddress, "[John-1@vmware.com]");
      attributeValues.put( attrNameUserPrincipalName, "[John-1@ssolabs-openldap.eng.vmware.com]");
      attributeValues.put( attrNameSubjectType, "[false]" );
      resultFromPrincipalId.put(
              new PrincipalId("John-1", OPENLDAP_DOMAIN_NAME_USER), attributeValues);

      attributeValues = new HashMap<String, String>();
      attributeValues.put( attrNameGivenName, "[givenName-John-3]" );
      attributeValues.put( attrNameSn, "[Smith]" );
      attributeValues.put( attrNameMemberOf, "[ssolabs-openldap.eng.vmware.com\\Group-2]" );
      attributeValues.put( attrNameEmailAddress, "[John-3@vmware.com]" );
      attributeValues.put( attrNameUserPrincipalName, "[John-3@ssolabs-openldap.eng.vmware.com]" );
      attributeValues.put( attrNameSubjectType, "[false]" );
      resultFromPrincipalId.put(
            new PrincipalId("John-3", OPENLDAP_DOMAIN_NAME_USER), attributeValues);

       attributeValues = new HashMap<String, String>();
       attributeValues.put( attrNameGivenName, "[givenName-John-12]" );
       attributeValues.put( attrNameSn, "[Smith]" );
       attributeValues.put( attrNameMemberOf, "[ssolabs-openldap.eng.vmware.com\\Group-5]");
       attributeValues.put( attrNameEmailAddress, "[John-12@vmware.com]" );
       attributeValues.put( attrNameUserPrincipalName, "[John-12@ssolabs-openldap.eng.vmware.com]" );
       attributeValues.put( attrNameSubjectType, "[false]" );
       resultFromPrincipalId.put(
            new PrincipalId("John-12", OPENLDAP_DOMAIN_NAME_USER), attributeValues );

       attributeValues = new HashMap<String, String>();
       attributeValues.put( attrNameGivenName, "[givenName-John-11]" );
       attributeValues.put( attrNameSn, "[Smith]" );
       attributeValues.put( attrNameMemberOf, "[ssolabs-openldap.eng.vmware.com\\Group-2, ssolabs-openldap.eng.vmware.com\\Group-1, ssolabs-openldap.eng.vmware.com\\Group-3, ssolabs-openldap.eng.vmware.com\\Group-4]");
       attributeValues.put( attrNameEmailAddress, "[John-11@vmware.com]");
       attributeValues.put( attrNameUserPrincipalName, "[John-11@ssolabs-openldap.eng.vmware.com]" );
       attributeValues.put( attrNameSubjectType, "[false]" );
       resultFromPrincipalId.put(
          new PrincipalId("John-11", OPENLDAP_DOMAIN_NAME_USER), attributeValues);

      for (PrincipalId id : resultFromPrincipalId.keySet())
      {
         Collection<AttributeValuePair> attrVals = provider.getAttributes(id, attrs);
         Iterator<AttributeValuePair> itr = attrVals.iterator();
         while (itr.hasNext())
         {
            AttributeValuePair avPair = itr.next();
            if ( resultFromPrincipalId.get(id).containsKey(avPair.getAttrDefinition().getName()) )
            {
                List<String> result = avPair.getValues();
                String expected = resultFromPrincipalId.get(id).get(avPair.getAttrDefinition().getName());
                Assert.assertTrue(String.format("result mismatch: %s vs. %s", result, expected), expected.equals(result.toString()));
            }
         }
      }

      PrincipalId id = new PrincipalId("John-1", OPENLDAP_DOMAIN_NAME_USER);
      List<Attribute> invalidList = Arrays.asList(new Attribute("unknownAttrName"));
      try {
         provider.getAttributes(id, invalidList);
      }
      catch (IllegalArgumentException e)
      {
         Assert.assertTrue(e.getMessage().contains("No attribute mapping found"));
         return;
      }
      Assert.fail("should not reach here");
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

   @Test
   public void testFindUserSSL() throws Exception
   {
       testFindUser(ldapsProvider);
   }

   private static void testFindUser(IIdentityProvider provider) throws Exception
   {
      List<PrincipalId> users = Arrays.asList(
            new PrincipalId("John-1", OPENLDAP_DOMAIN_NAME_USER),
            new PrincipalId("John-2", OPENLDAP_DOMAIN_NAME_USER),
            new PrincipalId("John-3", OPENLDAP_DOMAIN_ALIAS)
            );
      final String givenNamePreFix = "givenName-";
      HashMap<String, String> expectedResult = new HashMap<String, String>(users.size());
      expectedResult.put(users.get(0).getUPN(), "gn333");
      expectedResult.put(users.get(1).getUPN(), givenNamePreFix + users.get(1).getName());
      expectedResult.put(users.get(2).getUPN(), givenNamePreFix + users.get(2).getName());
      for (PrincipalId user : users)
      {
         PersonUser result = provider.findUser(user);
         Assert.assertEquals( "unexpected result",
               givenNamePreFix+user.getName(),
               result.getDetail().getFirstName());
      }

      PrincipalId nonExistant = new PrincipalId("nonExistant", OPENLDAP_DOMAIN_NAME_USER);
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
   public void testFindUsers()
   {
       testFindUsers(unMappedprovider);
   }

   @Test
   public void testFindUsersSchemaMapped()
   {
       testFindUsers(schemaMappedprovider);
   }

   @Test
   public void testFindUsersSSL()
   {
       testFindUsers(ldapsProvider);
   }

   private static void testFindUsers(IIdentityProvider provider)
   {
      final String strAll = "";
      final String strAllBySnCis = "Cis";
      final String strAllBySnSmith = "Smith";
      final String strAllBySpecialChar = "specialChar";
      final String strAllByFspNoUUID = "fspnouuid";
      final String strAllByPartialGivenNameCis = "Cis-";
      final String strAllByPartialGivenName = "John-";
      final String strAllByFspSn = "FSP";
      final String strAllByPartialGivenFspName = "fsp";
      try {
         Set<PersonUser> setAll = provider.findUsers(strAll, storeData.getName(), -1);
         Set<PersonUser> setAllBySn = provider.findUsers(strAllBySnCis, storeData.getName(), -1);
         Set<PersonUser> setAllBySnSmith = provider.findUsers(strAllBySnSmith, storeData.getName(), -1);
         Set<PersonUser> setAllByFspSn = provider.findUsers(strAllByFspSn, storeData.getName(), -1);
         Set<PersonUser> setAllBySpecChar = provider.findUsers(strAllBySpecialChar, storeData.getName(), -1);
         Set<PersonUser> setAllByFspNoUUID = provider.findUsers(strAllByFspNoUUID, storeData.getName(), -1);
         Set<PersonUser> setAllByPartialGivenName = provider.findUsers(strAllByPartialGivenName, storeData.getName(), -1);
         Set<PersonUser> setAllByPartialGivenNameCis = provider.findUsers(strAllByPartialGivenNameCis, storeData.getName(), -1);
         Set<PersonUser> setAllByPartialFspGivenName = provider.findUsers(strAllByPartialGivenFspName, storeData.getName(), -1);

         setAllBySn.addAll(setAllByFspSn);
         setAllBySn.addAll(setAllBySnSmith);
         setAllBySn.addAll(setAllBySpecChar);
         Assert.assertTrue(setAllBySn.equals(setAll));
         setAllByPartialGivenName.addAll(setAllByPartialFspGivenName);
         setAllByPartialGivenName.addAll(setAllByPartialGivenNameCis);
         setAllByPartialGivenName.addAll(setAllBySpecChar);
         setAllByPartialGivenName.addAll(setAllByFspNoUUID);
         Assert.assertTrue(setAllByPartialGivenName.equals(setAll));
      }
      catch (Exception e)
      {
         Assert.fail(e.getMessage());
      }
   }

   @Test
   public void testFindUsersInGroup() throws Exception
   {
       testFindUsersInGroup(unMappedprovider);
   }

   @Test
   public void testFindUsersInGroupSchemaMapped() throws Exception
   {
       testFindUsersInGroup(schemaMappedprovider);
   }

   @Test
   public void testFindUsersInGroupSSL() throws Exception
   {
       testFindUsersInGroup(ldapsProvider);
   }

   private static void testFindUsersInGroup(IIdentityProvider provider) throws Exception
   {
      Map<PrincipalId, Integer> sizeFromGroupId = new HashMap<PrincipalId, Integer>();
      sizeFromGroupId.put(new PrincipalId("Group-1", OPENLDAP_DOMAIN_NAME_USER), 3);
      //not including users in nested group -- this is just a "shallow" find of
      // direct children in a group.
      // This is the common behavior for the other two plugins
      sizeFromGroupId.put(new PrincipalId("Group-2", OPENLDAP_DOMAIN_NAME_USER), 5);
      sizeFromGroupId.put(new PrincipalId("Group-3", OPENLDAP_DOMAIN_NAME_USER), 4);

      final String strJohn = "John";
      final String strDescPrefix = "VMware";

      for (PrincipalId groupId : sizeFromGroupId.keySet())
      {
         Set<PersonUser> resultByCN =
               provider.findUsersInGroup(groupId, strJohn, -1);
         Set<PersonUser> resultByDescPrefix =
               provider.findUsersInGroup(groupId, strDescPrefix, -1);

         Assert.assertEquals("unexpected size for CN search" + groupId,
               sizeFromGroupId.get(groupId).intValue(),
               resultByCN.size());
         Assert.assertEquals("unexpected size for DescPrefix search" + groupId,
               sizeFromGroupId.get(groupId).intValue(),
               resultByDescPrefix.size());
         Assert.assertArrayEquals(resultByCN.toArray(), resultByDescPrefix.toArray());
      }
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

   @Test
   public void testFindUsersByNameInGroupSSL() throws Exception
   {
       testFindUsersByNameInGroup(ldapsProvider);
   }

   private static void testFindUsersByNameInGroup(IIdentityProvider provider) throws Exception
   {
      Map<PrincipalId, Integer> sizeFromGroupId = new HashMap<PrincipalId, Integer>();
      sizeFromGroupId.put(new PrincipalId("Group-1", OPENLDAP_DOMAIN_NAME_USER), 3);
      //not including users in nested group -- this is just a "shallow" find of
      // direct children in a group.
      // This is the common behavior for the other two plugins
      sizeFromGroupId.put(new PrincipalId("Group-2", OPENLDAP_DOMAIN_NAME_USER), 5);
      sizeFromGroupId.put(new PrincipalId("Group-3", OPENLDAP_DOMAIN_NAME_USER), 4);

      final String strJohn = "John";
      final String strGivenNamePrefix = "givenName-John";

      for (PrincipalId groupId : sizeFromGroupId.keySet())
      {
         Set<PersonUser> resultByCN =
               provider.findUsersByNameInGroup(groupId, strJohn, -1);
         Set<PersonUser> resultByGivenNamePrefix =
               provider.findUsersByNameInGroup(groupId, strGivenNamePrefix, -1);

         Assert.assertEquals("unexpected size for CN search" + groupId,
               sizeFromGroupId.get(groupId).intValue(),
               resultByCN.size());
         Assert.assertEquals("unexpected size for givenNamePrefix search" + groupId,
               sizeFromGroupId.get(groupId).intValue(),
               resultByGivenNamePrefix.size());
         Assert.assertArrayEquals(resultByCN.toArray(), resultByGivenNamePrefix.toArray());
      }
   }

   @Test
   public void testFindDisabledUsers() throws Exception
   {
       testFindDisabledUsers(unMappedprovider);
   }

   @Test
   public void testFindDisabledUsersSchemaMapped() throws Exception
   {
       testFindDisabledUsers(schemaMappedprovider);
   }

   @Test
   public void testFindDisabledUsersSSL() throws Exception
   {
       testFindDisabledUsers(ldapsProvider);
   }

   private static void testFindDisabledUsers(IIdentityProvider provider) throws Exception
   {
      Set<PersonUser> resultFilteredByCN = provider.findDisabledUsers("John", -1);
      Set<PersonUser> resultFilteredByDesc = provider.findDisabledUsers("Employee of VMware", -1);

      Assert.assertTrue("unexpected result size", (resultFilteredByCN.size() == 6));
      Assert.assertArrayEquals(resultFilteredByCN.toArray(), resultFilteredByDesc.toArray());
   }

   @Test
   public void testFindLockedUsers() throws Exception
   {
       testFindLockedUsers(unMappedprovider);
   }

   @Test
   public void testFindLockedUsersSchemaMapped() throws Exception
   {
       testFindLockedUsers(schemaMappedprovider);
   }

   @Test
   public void testFindLockedUsersSSL() throws Exception
   {
       testFindLockedUsers(ldapsProvider);
   }

   private static void testFindLockedUsers(IIdentityProvider provider) throws Exception
   {
      Set<PersonUser> resultFilteredByCN = provider.findLockedUsers("John", -1);
      Set<PersonUser> resultFilteredByDesc = provider.findLockedUsers("Employee of VMware", -1);

      Assert.assertTrue("unexpected result size", (resultFilteredByCN.size() == 3));
      Assert.assertArrayEquals(resultFilteredByCN.toArray(), resultFilteredByDesc.toArray());
   }

   @Test
   public void testFindDirectParentGroups() throws Exception
   {
       testFindDirectParentGroups(unMappedprovider);
   }

   @Test
   public void testFindDirectParentGroupsSchemaMapped() throws Exception
   {
       testFindDirectParentGroups(schemaMappedprovider);
   }

   @Test
   public void testFindDirectParentGroupSSL() throws Exception
   {
       testFindDirectParentGroups(ldapsProvider);
   }

   private static void testFindDirectParentGroups(IIdentityProvider provider) throws Exception
   {
      Map<PrincipalId, Integer> sizeFromId = new HashMap<PrincipalId, Integer>();
      sizeFromId.put(new PrincipalId("John-1", OPENLDAP_DOMAIN_NAME_USER), 1);
      sizeFromId.put(new PrincipalId("John-10", OPENLDAP_DOMAIN_NAME_USER), 2);
      sizeFromId.put(new PrincipalId("Group-1", OPENLDAP_DOMAIN_NAME_USER), 2);
      sizeFromId.put(new PrincipalId("Group-2", OPENLDAP_DOMAIN_NAME_USER), 0);

      for (PrincipalId current : sizeFromId.keySet())
      {
         Assert.assertTrue("size does not match for query: " + current,
               sizeFromId.get(current)
               .equals(provider.findDirectParentGroups(current).getGroups().size()));
      }

      PrincipalId invalidId = new PrincipalId("nonExistant", OPENLDAP_DOMAIN_NAME_USER);

      try {
         provider.findDirectParentGroups(invalidId);
      }
      catch (InvalidPrincipalException e)
      {
         //expected exception caught here
         return;
      }
      Assert.fail("should not reached here");
   }

   @Test
   public void testFindNestedParentGroups() throws Exception
   {
       testFindNestedParentGroups(unMappedprovider);
   }

   @Test
   public void testFindNestedParentGroupsSchemaMapped() throws Exception
   {
       testFindNestedParentGroups(schemaMappedprovider);
   }

   @Test
   public void testFindNestedParentGroupsSSL() throws Exception
   {
       testFindNestedParentGroups(ldapsProvider);
   }

   private static void testFindNestedParentGroups(IIdentityProvider provider) throws Exception
   {
      Map<PrincipalId, Integer> sizeFromUserId = new HashMap<PrincipalId, Integer>();
      sizeFromUserId.put(new PrincipalId("John-11", OPENLDAP_DOMAIN_NAME_USER), 4);  //{1, 2, 3, 4, Everyone}
      sizeFromUserId.put(new PrincipalId("John-1",  OPENLDAP_DOMAIN_NAME_USER), 3);  //{1, 2, 3, Everyone}
      sizeFromUserId.put(new PrincipalId("John-10", OPENLDAP_DOMAIN_NAME_USER), 3);  //{1, 2, 3, Everyone}
      sizeFromUserId.put(new PrincipalId("John-12", OPENLDAP_DOMAIN_NAME_USER), 1);
      sizeFromUserId.put(new PrincipalId("John-6",  OPENLDAP_DOMAIN_NAME_USER), 1);
      sizeFromUserId.put(new PrincipalId("John-9",  OPENLDAP_DOMAIN_NAME_USER), 1);

      for (PrincipalId userId : sizeFromUserId.keySet())
      {
         PrincipalGroupLookupInfo nestedGroups = provider.findNestedParentGroups(userId);
         Assert.assertTrue("unexpected size", nestedGroups.getGroups().size() == sizeFromUserId.get(userId).intValue());
      }
   }

   @Test
   public void testFindGroup() throws Exception
   {
       testFindGroup(unMappedprovider);
   }

   @Test
   public void testFindGroupSchemaMapped() throws Exception
   {
       testFindGroup(schemaMappedprovider);
   }

   @Test
   public void testFindGroupSSL() throws Exception
   {
       testFindGroup(ldapsProvider);
   }

   private static void testFindGroup(IIdentityProvider provider) throws Exception
   {
      List<PrincipalId> groupIds = Arrays.asList(
            new PrincipalId("Group-1", OPENLDAP_DOMAIN_NAME_GROUP),
            new PrincipalId("Group-2", OPENLDAP_DOMAIN_NAME_GROUP)
            );

      final String descPrefix = "desc-";
      for (PrincipalId currentGroupId : groupIds)
      {
         try {
            Group result = provider.findGroup(currentGroupId);
            Assert.assertEquals("unexpected result",
                  descPrefix+currentGroupId.getName(),
                  result.getDetail().getDescription());
         }
         catch (Exception e)
         {
            Assert.fail(e.getMessage());
         }
      }

      PrincipalId nonExistant = new PrincipalId("nonExistantGroupCN", OPENLDAP_DOMAIN_NAME_GROUP);
      Group nonExistGroup = provider.findGroup(nonExistant);
      Assert.assertNull(nonExistGroup);
   }

   @Test
   public void testFindGroups() throws Exception
   {
       testFindGroups(unMappedprovider);
   }

   @Test
   public void testFindGroupsSchemaMapped() throws Exception
   {
       testFindGroups(schemaMappedprovider);
   }

   @Test
   public void testFindGroupsSSL() throws Exception
   {
       testFindGroups(ldapsProvider);
   }

   private static void testFindGroups(IIdentityProvider provider) throws Exception
   {
      final String strAllGroupsName = "Group-";
      final String strAllGroupsDesc = "desc-";
      final String strGroup2 = "Group-2";

      Set<Group> resultAllGroupsName = provider.findGroups(strAllGroupsName, storeData.getName(), -1);
      Set<Group> resultAllGroupsDesc = provider.findGroups(strAllGroupsDesc, storeData.getName(), -1);

      Iterator<Group> itrName = resultAllGroupsName.iterator();
      Iterator<Group> itrDesc = resultAllGroupsName.iterator();
      while (itrName.hasNext() && itrDesc.hasNext())
      {
         Assert.assertEquals(
               itrName.next().getAlias(),
               itrDesc.next().getAlias()
               );
      }
      Assert.assertTrue("unexpected result size",
            (resultAllGroupsDesc.size() == 5) &&
            (resultAllGroupsName.size() == 5));
      Assert.assertArrayEquals(resultAllGroupsDesc.toArray(), resultAllGroupsName.toArray());

      Set<Group> resultGroup2 = provider.findGroups(strGroup2, storeData.getName(), -1);
      Assert.assertTrue("set size of 0 is expeted", resultGroup2.size() == 1);

      Set<Group> resultEmpty = provider.findGroups("nonExistant", storeData.getName(), -1);
      Assert.assertTrue("set size of 0 is expeted", resultEmpty.size() == 0);
   }

   @Test
   public void testFindGroupsInGroup() throws Exception
   {
       testFindGroupsInGroup(unMappedprovider);
   }

   @Test
   public void testFindGroupsInGroupSchemaMapped() throws Exception
   {
       testFindGroupsInGroup(schemaMappedprovider);
   }

   @Test
   public void testFindGroupsInGroupSSL() throws Exception
   {
       testFindGroupsInGroup(ldapsProvider);
   }

   private static void testFindGroupsInGroup(IIdentityProvider provider) throws Exception
   {
      PrincipalId idGroup1 = new PrincipalId("Group-1", OPENLDAP_DOMAIN_NAME_GROUP);
      PrincipalId idGroup2 = new PrincipalId("Group-2", OPENLDAP_DOMAIN_NAME_GROUP);
      PrincipalId idGroup3 = new PrincipalId("Group-3", OPENLDAP_DOMAIN_NAME_GROUP);

      Set<Group> group1SubGroup = provider.findGroupsInGroup(idGroup1, "", -1);
      Set<Group> group2SubGroup = provider.findGroupsInGroup(idGroup2, "", -1);
      Set<Group> group3SubGroup = provider.findGroupsInGroup(idGroup3, "", -1);

      Assert.assertTrue("unexpected size", group1SubGroup.size() == 1);
      Assert.assertTrue("unexpected size", group2SubGroup.size() == 1);
      Assert.assertArrayEquals(group2SubGroup.toArray(), group3SubGroup.toArray());
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

   @Test
   public void testFindGroupsByNameInGroupSSL() throws Exception
   {
       testFindGroupsByNameInGroup(ldapsProvider);
   }

   private static void testFindGroupsByNameInGroup(IIdentityProvider provider) throws Exception
   {
       PrincipalId idGroup1 = new PrincipalId("Group-1", OPENLDAP_DOMAIN_NAME_GROUP);
       PrincipalId idGroup2 = new PrincipalId("Group-2", OPENLDAP_DOMAIN_NAME_GROUP);
       PrincipalId idGroup3 = new PrincipalId("Group-3", OPENLDAP_DOMAIN_NAME_GROUP);

       Set<Group> group1SubGroup = provider.findGroupsByNameInGroup(idGroup1, "", -1);
       Set<Group> group2SubGroup = provider.findGroupsByNameInGroup(idGroup2, "", -1);
       Set<Group> group3SubGroup = provider.findGroupsByNameInGroup(idGroup3, "", -1);

       Assert.assertTrue("unexpected size", group1SubGroup.size() == 1);
       Assert.assertTrue("unexpected size", group2SubGroup.size() == 1);
       Assert.assertArrayEquals(group2SubGroup.toArray(), group3SubGroup.toArray());
   }

   @Test
   public void testFind() throws Exception
   {
       testFind(unMappedprovider);
   }

   @Test
   public void testFindSchemaMapped() throws Exception
   {
       testFind(schemaMappedprovider);
   }

   @Test
   public void testFindSSL() throws Exception
   {
       testFind(ldapsProvider);
   }

   private static void testFind(IIdentityProvider provider) throws Exception
   {
      final String strUserCN = new String("John-");
      final String strGroupCN = new String("Group-");

      SearchResult searchUserCN = provider.find(strUserCN, storeData.getName(), -1);
      Assert.assertNull("value should be null", searchUserCN.getSolutionUsers());
      Assert.assertTrue("unexpected size", searchUserCN.getPersonUsers().size() == 13);
      Assert.assertTrue("unexpected size", searchUserCN.getGroups().size() == 0);

      SearchResult searchGroupCN = provider.find(strGroupCN, storeData.getName(), -1);
      Assert.assertNull("vaule should be null", searchGroupCN.getSolutionUsers());
      Assert.assertTrue("unexpected size", searchGroupCN.getPersonUsers().size() == 0);
      Assert.assertTrue("unexpected size", searchGroupCN.getGroups().size() == 5);
   }

   @Test
   public void testIsActive()
   {
       testIsActive(unMappedprovider);
   }

   @Test
   public void testIsActiveSchemaMapped()
   {
       testIsActive(schemaMappedprovider);
   }

   @Test
   public void testFindActiveUser() {
       testFindActiveUser(schemaMappedprovider);
   }
   private void testFindActiveUser(IIdentityProvider provider) {
       PrincipalId user = new PrincipalId("John-3", OPENLDAP_DOMAIN_NAME_USER);
/*
       //search by upn
       try {
           PrincipalId result = provider.findActiveUser(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, user.getUPN());
           Assert.assertNotNull(result);
           Assert.assertEquals("", user.getName(), result.getName());
       }
       catch (Exception e)
       {
           Assert.fail(String.format("Failed to find the user by UPN", user.getUPN()) );
       }
*/
       //by accuntname
       String accountName = "John-3";
       try {
           PrincipalId result = provider.findActiveUser(attrNameUserPrincipalName, accountName);
           Assert.assertNotNull(result);
           Assert.assertEquals("", user.getName(), result.getName());
        }
        catch (Exception e)
        {
            Assert.fail(String.format("Failed to find the user by account name %s.", accountName) );
        }

       //by email
       String emailValue = "John-3@vmware.com";
       try {
           PrincipalId result = provider.findActiveUser(attrNameEmailAddress, emailValue);
           Assert.assertNotNull(result);
           Assert.assertEquals("", user.getName(), result.getName());
        }
        catch (Exception e)
        {
            Assert.fail(String.format("Failed to find the user by account name %s.", accountName) );
        }

   }

@Test
   public void testIsActiveSSL()
   {
       testIsActive(ldapsProvider);
   }

   private static void testIsActive(IIdentityProvider provider)
   {
      Map<PrincipalId, Boolean> activeStatusFromPrincipalId = new HashMap<PrincipalId, Boolean>();
      activeStatusFromPrincipalId.put(new PrincipalId("John-1", OPENLDAP_DOMAIN_NAME_GROUP), Boolean.FALSE);  /* userAccountControl: 11 */
      activeStatusFromPrincipalId.put(new PrincipalId("John-2", OPENLDAP_DOMAIN_NAME_GROUP), Boolean.FALSE); /* userAccountControl: 22 */
      activeStatusFromPrincipalId.put(new PrincipalId("John-3", OPENLDAP_DOMAIN_NAME_GROUP), Boolean.TRUE);
      activeStatusFromPrincipalId.put(new PrincipalId("John-6", OPENLDAP_DOMAIN_NAME_GROUP), Boolean.FALSE); /* userAccountControl: 66 */
      activeStatusFromPrincipalId.put(new PrincipalId("John-7", OPENLDAP_DOMAIN_NAME_GROUP), Boolean.TRUE);
      activeStatusFromPrincipalId.put(new PrincipalId("John-8", OPENLDAP_DOMAIN_NAME_GROUP), Boolean.TRUE);

      try {
         for (PrincipalId user: activeStatusFromPrincipalId.keySet())
         {
            Boolean isActive = provider.IsActive(user);
            Assert.assertEquals("unexpected result: " + user.getName(),
                  activeStatusFromPrincipalId.get(user),
                  isActive);
         }
      }
      catch (Exception e)
      {
         Assert.fail(e.getMessage());
      }
   }

   private static IdentityStoreSchemaMapping getOpenLdapSchemaMapping() {
       IdentityStoreSchemaMapping.Builder schemaMapBuilder = new IdentityStoreSchemaMapping.Builder();
       IdentityStoreObjectMapping.Builder objectMapBuilder = null;
       objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdUser);
       objectMapBuilder.setObjectClass("inetOrgPerson");
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "userAccountControl"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "cn"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "description"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "displayName"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "mail"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, "memberOf"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "entryUUID"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, "userPrincipalName"));
       schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

       objectMapBuilder = new IdentityStoreObjectMapping.Builder(ObjectIds.ObjectIdGroup);
       objectMapBuilder.setObjectClass("groupOfUniqueNames");
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "cn"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "description"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "uniqueMember"));
       objectMapBuilder.addAttributeMapping(new IdentityStoreAttributeMapping(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "entryUUID"));
       schemaMapBuilder.addObjectMappings(objectMapBuilder.buildObjectMapping());

       return schemaMapBuilder.buildSchemaMapping();
   }

   private static Collection<X509Certificate> getLdapsCertificate() {
	   try {

	       CertificateFactory cf = CertificateFactory.getInstance("X.509");
	       X509Certificate slapdCert = (X509Certificate)cf.generateCertificate( LdapProviderTest.class.getResourceAsStream(
	               CFG_KEY_LDAPS_KEY_ALIAS));

	       if (slapdCert != null)
	            return java.util.Collections.singletonList(slapdCert);
	       else
	            return null;

	      } catch (Exception e) {
	         throw new IllegalStateException(e);
	      }
   }
}
