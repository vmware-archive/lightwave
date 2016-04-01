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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

import javax.security.auth.login.LoginException;

import junit.framework.Assert;

import org.apache.commons.lang.SystemUtils;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.provider.IIdentityProvider;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.ProviderFactory;
import com.vmware.identity.idm.server.provider.localos.LocalOsIdentityProvider;

public class LocalOsIdentityProviderTest
{
    private final static String domainName = "localhost";
    private final static String domainAlias = "localos";
    private static IIdentityProvider localOsProvider;

    @Test
    public void TestUsers() throws Exception
    {
        for( UserInfo userInfo : _users.values() )
        {
            // ---------------------
            // findUser
            // ---------------------
            PersonUser user = localOsProvider.findUser( new PrincipalId( userInfo.getName(), domainName ) );
            Assert.assertNotNull(String.format("User '%s' should exist.", userInfo.getName()), user);
            validateUser(user, userInfo);

            // ---------------------
            // IsActive
            // ---------------------
            boolean isActive = SystemUtils.IS_OS_LINUX ? true : !userInfo.isDisabled();
            Assert.assertEquals(
                    isActive,
                    localOsProvider.IsActive( new PrincipalId( userInfo.getName(), domainName ) )
            );

            // ---------------------
            // getAttributes
            // ---------------------
            Collection<AttributeValuePair> attributes =
                localOsProvider.getAttributes(
                    new PrincipalId( userInfo.getName(), domainName ),
                    getAttributes()
                );

            Assert.assertNotNull(
                    String.format("Should be able to retrieve attributes for User '%s'.", userInfo.getName() ),
                    attributes
            );

            for(AttributeValuePair attr : attributes)
            {
                Assert.assertNotNull( attr );
                Assert.assertNotNull( attr.getAttrDefinition() );
                // new sids attributes comes without the friendly name.
                // Assert.assertNotNull( attr.getAttrDefinition().getFriendlyName() );

                if( GROUPS_FRIENDLY_NAME.equalsIgnoreCase(attr.getAttrDefinition().getFriendlyName()) )
                {
                    Set<GroupInfo> groups = _usersToGroups.get( userInfo.getName() );
                    if( groups != null && groups.isEmpty() == false )
                    {
                        Assert.assertNotNull( attr.getValues() );
                        Assert.assertTrue(groups.size() <= attr.getValues().size());
                        HashSet<String> attrGroups = new HashSet<String>();
                        for( String attributeValue : attr.getValues() )
                        {
                            Assert.assertNotNull( attributeValue );
                            Assert.assertTrue(
                                attributeValue.startsWith( domainName ) ||
                                ( providerHasAlias() && attributeValue.startsWith( domainAlias ) )
                            );
                            Assert.assertTrue( attributeValue.contains("\\") );

                            String groupName = attributeValue;

                            Assert.assertFalse(groupName.isEmpty());

                            attrGroups.add(groupName);
                        }

                        for(GroupInfo info : groups)
                        {
                            Assert.assertTrue(
                                    String.format("group '%s' is expected to be present", domainName + "\\" + info.getName()),
                                    attrGroups.contains( domainName + "\\" + info.getName() )
                            );
                            if ( providerHasAlias() )
                            {
                                Assert.assertTrue(
                                    String.format("group '%s' is expected to be present", domainAlias + "\\" + info.getName()),
                                    attrGroups.contains( domainAlias + "\\" + info.getName() )
                                );
                            }
                        }
                    }

                }
                else if ( LAST_NAME_FRIENDLY_NAME.equalsIgnoreCase(attr.getAttrDefinition().getFriendlyName()) )
                {
                    Assert.assertNotNull( attr.getValues() );
                    Assert.assertEquals( 1,  attr.getValues().size() );
                    assertEqualsString( userInfo.getLastName(), attr.getValues().get(0) );
                }
                else if ( FIRST_NAME_FRIENDLY_NAME.equalsIgnoreCase(attr.getAttrDefinition().getFriendlyName()) )
                {
                    Assert.assertNotNull(  attr.getValues() );
                    Assert.assertEquals( 1, attr.getValues().size() );
                    assertEqualsString( userInfo.getFirstName(), attr.getValues().get(0) );
                }
                else if ( SUBJECT_TYPE_FRIENDLY_NAME.equalsIgnoreCase(attr.getAttrDefinition().getFriendlyName()))
                {
                    Assert.assertNotNull(  attr.getValues() );
                    Assert.assertEquals( 1, attr.getValues().size() );
                    assertEqualsString( "false", attr.getValues().get(0) );
                }
                else if ( USER_PRINCIPAL_NAME_FRIENDLY_NAME.equalsIgnoreCase(attr.getAttrDefinition().getFriendlyName()))
                {
                    Assert.assertNotNull(  attr.getValues() );
                    Assert.assertEquals( 1, attr.getValues().size() );
                    assertEqualsString( userInfo.getName() + "@" + domainName, attr.getValues().get(0) );
                }
            }

            // ---------------------
            // findDirectParentGroups, findNestedParentGroups
            // ---------------------
            Set<GroupInfo> groups = _usersToGroups.get( userInfo.getName() );

            PrincipalGroupLookupInfo directParentGroups = localOsProvider.findDirectParentGroups(
                    new PrincipalId( userInfo.getName(), domainName )
            );

            validateGroupsSubset( groups, ((directParentGroups== null) ? null : directParentGroups.getGroups()), domainName, domainAlias );

            PrincipalGroupLookupInfo userGroups = localOsProvider.findNestedParentGroups(
                    new PrincipalId( userInfo.getName(), domainName )
            );

            validateGroupsSubset( groups, ((userGroups == null)? null:userGroups.getGroups()), domainName, domainAlias);
        }
    }

    @Test
    public void TestGroups() throws Exception
    {
        for( GroupInfo groupInfo : _groups.values() )
        {
            // ---------------------
            // findGroup
            // ---------------------
            Group group = localOsProvider.findGroup( new PrincipalId( groupInfo.getName(), domainName ) );
            Assert.assertNotNull(
                    String.format("Group should be found '%s'", groupInfo.getName()),
                    group
            );
            assertEqualsString( groupInfo.getName(), group.getName() );
            assertEqualsString( domainName, group.getDomain() );
            Assert.assertNotNull( group.getDetail() );
            // do not support group description yet
            //assertEqualsString( groupInfo.getDescription(), group.getDetail().getDescription() );

            // ---------------------
            // findUsersInGroup
            // ---------------------
            Set<PersonUser> usersInGroupList = localOsProvider.findUsersInGroup(
                    new PrincipalId( groupInfo.getName(), domainName ),
                    "",
                    -1
            );

            Set<UserInfo> usersInGroup = _groupsToUsers.get(groupInfo.getName());
            String userName = null;
            if(usersInGroup != null)
            {
                if(usersInGroup.isEmpty())
                {
                    Assert.assertTrue( ( usersInGroupList == null )||(usersInGroupList.isEmpty()) );
                }
                else
                {
                    Assert.assertEquals( usersInGroup.size(), usersInGroupList.size() );
                    for(PersonUser usr : usersInGroupList )
                    {
                        Assert.assertNotNull( usr.getId() );
                        Assert.assertTrue( usersInGroup.contains( _users.get(usr.getId().getName()) ) );
                        if(userName == null)
                        {
                            userName = usr.getId().getName();
                        }
                    }
                }

                if(userName != null)
                {
                    usersInGroupList = localOsProvider.findUsersInGroup(
                            new PrincipalId( groupInfo.getName(), domainName ),
                            userName, -1
                    );

                    Assert.assertNotNull(  usersInGroupList );
                    Assert.assertEquals( 1, usersInGroupList.size() );
                }
            }

            // ---------------------
            // findGroupsInGroup
            // ---------------------
            Set<Group> groupsInGroup = localOsProvider.findGroupsInGroup(
                    new PrincipalId( groupInfo.getName(), domainName ),
                    "", -1
            );
            Assert.assertTrue( (groupsInGroup == null) || (groupsInGroup.size() == 0) );

            // ---------------------
            // findDirectParentGroups
            // ---------------------
            PrincipalGroupLookupInfo directParentGroups = localOsProvider.findDirectParentGroups(
                    new PrincipalId( groupInfo.getName(), domainName )
            );
            Assert.assertTrue(
                (directParentGroups == null) ||
                (directParentGroups.getGroups() == null) ||
                (directParentGroups.getGroups().size() == 0) );
        }
    }

    @Test
    public void TestFind() throws Exception
    {
        // ---------------------
        // findUsers
        // ---------------------
        Set<PersonUser> users = localOsProvider.findUsers( usersSearchCriteria, domainName, -1 );

        Assert.assertNotNull( users );
        Assert.assertEquals( _found_os_users.size(), users.size() );
        for(PersonUser usr : users)
        {
            Assert.assertNotNull( usr );
            Assert.assertNotNull( usr.getId() );
            UserInfo userInfo = _users.get( usr.getId().getName() );

            validateUser( usr, userInfo);
        }

        // ---------------------
        // findDisabledUsers
        // ---------------------
        users = localOsProvider.findDisabledUsers(
                usersSearchCriteria, -1);
        Assert.assertNotNull( users );
        for(PersonUser usr : users)
        {
            Assert.assertNotNull( usr );
            Assert.assertNotNull( usr.getId() );
            UserInfo userInfo = _users.get( usr.getId().getName() );

            Assert.assertTrue( userInfo.isDisabled() );
            validateUser( usr, userInfo);
        }


        // ---------------------
        // findLockedUsers
        // ---------------------
        users = localOsProvider.findLockedUsers(
                usersSearchCriteria, -1);
        Assert.assertTrue( users == null || users.isEmpty() );
        // adjust when/if we are able to create locked accounts
        //Assert.assertNotNull( users );
        //for(PersonUser usr : users)
        //{
        //    Assert.assertNotNull( usr );
        //    Assert.assertNotNull( usr.getId() );
        //    UserInfo userData = this._users.get( usr.getId().getName() );
        //
        //    Assert.assertTrue( userData.isLocked() );
        //    validateUser( usr, userData );
        //}

        // ---------------------
        // findGroups
        // ---------------------
        Set<Group> groups = localOsProvider.findGroups( groupsSearchCriteria, domainName, -1);

        Assert.assertNotNull( groups );
        Assert.assertEquals( _found_os_groups.size(), groups.size() );
        for(Group g : groups)
        {
            Assert.assertNotNull( g );
            GroupInfo groupInfo = _found_os_groups.get(g.getName());
            Assert.assertNotNull(groupInfo);

            assertEqualsString( groupInfo.getName(), g.getName() );
            assertEqualsString( domainName, g.getDomain() );

            // do not support group description yet
            // Assert.assertNotNull( g.getDetail() );
            // assertEqualsString( groupInfo.getDescription(), g.getDetail().getDescription() );
        }
    }

    @Test
    public void TestAuth() throws Exception
    {
        // ---------------------
        // authenticate
        // ---------------------
        UserInfo user = _users.values().iterator().next();
        localOsProvider.authenticate(
                new PrincipalId(
                        user.getName(), domainName ),
                user.getPassword()
        );
        if ( providerHasAlias() )
        {
            localOsProvider.authenticate(
                    new PrincipalId(
                            user.getName(), domainAlias ),
                    user.getPassword()
            );
        }

        try
        {
            localOsProvider.authenticate(
                    new PrincipalId(
                            user.getName(), domainName ),
                    "def"
            );
            Assert.fail( "Authenticate call expected to fail." );
        }
        catch(LoginException ex) // expected
        {}

        try
        {
            localOsProvider.authenticate(
                    new PrincipalId(
                            UUID.randomUUID().toString(), domainName ),
                    "def"
            );
            Assert.fail( "Authenticate call expected to fail." );
        }
        catch(LoginException ex) // expected
        {}
    }

    private static boolean providerHasAlias()
    {
        return providerHasAlias(domainAlias);
    }

    private static boolean providerHasAlias(String inDomainAlias)
    {
        return (inDomainAlias != null) && (inDomainAlias.length() > 0);
    }

    private static Map<String, GroupInfo> _groups;
    private static Map<String, UserInfo> _users;

    private static Map<String, GroupInfo> _found_os_groups;
    private static Map<String, UserInfo> _found_os_users;

    private static HashMap<String, Set<UserInfo>> _groupsToUsers;
    private static HashMap<String, Set<GroupInfo>> _usersToGroups;

    private static ISecurityAccountsLibrary _lib;

    @BeforeClass
    public static void testSetup()
    {
        ISecurityAccountsLibrary lib = getSamLib();

        if( lib != null )
        {
            GroupInfo g1 = new GroupInfo( group1Name, group1Description );
            GroupInfo g2 = new GroupInfo( group2Name, group2Description );

            _groups = new HashMap<String, GroupInfo>();
            _groups.put(g1.getName(), g1);
            _groups.put(g2.getName(), g2);

            _found_os_groups = new HashMap<String, GroupInfo>();
            _found_os_groups.put(g1.getName(), g1);
            if (SystemUtils.IS_OS_WINDOWS)
            {// Windows samlib makes g2 a search hit
                _found_os_groups.put(g2.getName(), g2);
            }

            UserInfo u1 = new UserInfo(
                    user1Name, user1FirstName, user1LastName, userPassword, user1Description, false, false );
            UserInfo u2 = new UserInfo(
                    user2Name, user2FirstName, user2LastName, userPassword, user2Description, false, false );
            UserInfo u3 = new UserInfo(
                    user3Name, user3FirstName, user3LastName, userPassword, user3Description, true, false );
            UserInfo u4 = new UserInfo(
                    user4Name, user4FirstName, user4LastName, userPassword, user4Description, false, true );
            UserInfo u5 = new UserInfo(
                    user5Name, user5FirstName, user5LastName, userPassword, user5Description, true, true );
            UserInfo u6 = new UserInfo(
                    user6Name, user6FirstName, user6LastName, userPassword,     user6Description, false, false );

            _users = new HashMap<String, UserInfo>();
            _users.put(u1.getName(), u1);
            _users.put(u2.getName(), u2);
            _users.put(u3.getName(), u3);
            _users.put(u4.getName(), u4);
            _users.put(u5.getName(), u5);
            _users.put(u6.getName(), u6);

            _found_os_users = new HashMap<String, UserInfo>();
            _found_os_users.put(u1.getName(), u1);
            _found_os_users.put(u3.getName(), u3);
            _found_os_users.put(u5.getName(), u5);
            _found_os_users.put(u6.getName(), u6);
            if (SystemUtils.IS_OS_WINDOWS)
            {//For Windows, we are getting two additional users due to samlib impl.
                _found_os_users.put(u2.getName(), u2);
                _found_os_users.put(u4.getName(), u4);
            }

            _groupsToUsers = new HashMap<String, Set<UserInfo>>();
            _groupsToUsers.put( g1.getName(), new HashSet<UserInfo>() );
            _groupsToUsers.put( g2.getName(), new HashSet<UserInfo>() );

            _groupsToUsers.get( g1.getName() ).add( u1 );
            _groupsToUsers.get( g1.getName() ).add( u2 );
            _groupsToUsers.get( g1.getName() ).add( u3 );
            _groupsToUsers.get( g1.getName() ).add( u4 );
            _groupsToUsers.get( g1.getName() ).add( u5 );

            _groupsToUsers.get( g2.getName() ).add( u1 );
            _groupsToUsers.get( g2.getName() ).add( u3 );

            _usersToGroups = new HashMap<String, Set<GroupInfo>>();
            _usersToGroups.put( u1.getName(), new HashSet<GroupInfo>() );
            _usersToGroups.put( u2.getName(), new HashSet<GroupInfo>() );
            _usersToGroups.put( u3.getName(), new HashSet<GroupInfo>() );
            _usersToGroups.put( u4.getName(), new HashSet<GroupInfo>() );
            _usersToGroups.put( u5.getName(), new HashSet<GroupInfo>() );
            _usersToGroups.put( u6.getName(), new HashSet<GroupInfo>() );

            _usersToGroups.get( u1.getName() ).add( g1 );
            _usersToGroups.get( u1.getName() ).add( g2 );
            _usersToGroups.get( u2.getName() ).add( g1 );
            _usersToGroups.get( u3.getName() ).add( g1 );
            _usersToGroups.get( u3.getName() ).add( g2 );
            _usersToGroups.get( u4.getName() ).add( g1 );
            _usersToGroups.get( u5.getName() ).add( g1 );

            for( UserInfo userInfo : _users.values() )
            {
                lib.AddUser(
                        userInfo.getName(), userInfo.getPassword(),
                        userInfo.getFirstName() + " " + userInfo.getLastName(),
                        userInfo.getDescription(), userInfo.isDisabled(), userInfo.isLocked()
                );
            }

            for(GroupInfo groupInfo : _groups.values())
            {
                lib.AddGroup( groupInfo.getName(), groupInfo.getDescription() );
            }

            for( Map.Entry<String, Set<UserInfo>> entry : _groupsToUsers.entrySet())
            {
                ArrayList<String> usersList = new ArrayList<String>();
                for(UserInfo info : entry.getValue())
                {
                    usersList.add( info.getName() );
                }
                if(usersList.isEmpty() == false)
                {
                    lib.AddUsersToGroup( entry.getKey(), usersList );
                }
            }

            // create the localOs provider
            ProviderFactory factory = new ProviderFactory();
            ServerIdentityStoreData storeData = new ServerIdentityStoreData( DomainType.LOCAL_OS_DOMAIN, domainName );
            storeData.setAuthenticationType( AuthenticationType.PASSWORD );
            storeData.setProviderType( IdentityStoreType.IDENTITY_STORE_TYPE_LOCAL_OS );
            storeData.setAlias( domainAlias );

            Map<String, String> map = new HashMap<String, String>();
            map.put( "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity", LocalOsIdentityProvider.GROUPS_ATTRIBUTE);
            map.put( "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname", LocalOsIdentityProvider.FIRST_NAME_ATTRIBUTE);
            map.put( "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname", LocalOsIdentityProvider.LAST_NAME_ATTRIBUTE);
            map.put( "http://vmware.com/schemas/attr-names/2011/07/isSolution", LocalOsIdentityProvider.SUBJECT_TYPE_ATTRIBUTE);
            map.put( "http://schemas.xmlsoap.org/claims/UPN", LocalOsIdentityProvider.USER_PRINCIPAL_NAME_ATTRIBUTE);

            storeData.setAttributeMap( map );

            try
            {
                localOsProvider = factory.buildProvider( "vsphere.local", storeData, null );
            }
            catch (Exception e)
            {
                Assert.fail( "Building local provider failed" );
            }
        }
    }

    @AfterClass
    public static void testTearDown()
    {
        try
        {
            ISecurityAccountsLibrary lib = getSamLib();

            if( lib != null )
            {
                try
                {
                    if( _users != null )
                    {
                        for( UserInfo userInfo : _users.values() )
                        {
                            lib.DeleteUser( userInfo.getName() );
                        }

                        _users = null;
                    }

                    if( _groups != null )
                    {
                        for(GroupInfo groupInfo : _groups.values())
                        {
                            lib.DeleteGroup( groupInfo.getName() );
                        }

                        _groups = null;
                    }
                }
                finally
                {
                    lib.close();
                    _lib = null;
                }
            }
        }
        finally
        {
            System.gc();
            System.runFinalization();
        }
    }

    private static void validateUser( PersonUser personUser, UserInfo userInfo)
    {
        Assert.assertNotNull(personUser.getId());
        assertEqualsString( userInfo.getName(), personUser.getId().getName() );
        if( providerHasAlias() )
        {
            Assert.assertNotNull(personUser.getAlias());
            assertEqualsString( userInfo.getName(), personUser.getAlias().getName() );
            assertEqualsString( domainAlias, personUser.getAlias().getDomain() );
        }
        assertEqualsString( domainName, personUser.getId().getDomain() );
        //Assert.assertEquals( userInfo.isDisabled(), personUser.isDisabled() );
        //Assert.assertEquals( userInfo.isLocked(), personUser.isLocked() );
        Assert.assertNotNull( personUser.getDetail() );
        assertEqualsString( userInfo.getFirstName(), personUser.getDetail().getFirstName() );
        assertEqualsString( userInfo.getLastName(), personUser.getDetail().getLastName() );
        //assertEqualsString( userInfo.getDescription(), personUser.getDetail().getDescription() );
    }

    private static void validateGroupsSubset(Set<GroupInfo> expectedSubset, Set<Group> actualSet, String domainName, String domainAlias)
    {
        HashMap< String, Group > superSet = new HashMap<String, Group>();
        if(actualSet != null)
        {
            for(Group g : actualSet)
            {
                Assert.assertNotNull( g );
                superSet.put( g.getName(), g );
            }
        }

        if(expectedSubset != null)
        {
            for(GroupInfo gd : expectedSubset)
            {
                Assert.assertTrue( superSet.containsKey( gd.getName() ) );
                Group g = superSet.get( gd.getName() );

                assertEqualsString( gd.getName(), g.getName() );
                if ( providerHasAlias(domainAlias) )
                {
                    Assert.assertNotNull(g.getAlias());
                    assertEqualsString( gd.getName(), g.getAlias().getName() );
                    assertEqualsString( domainAlias, g.getAlias().getDomain() );
                }
                assertEqualsString( domainName, g.getDomain() );
                Assert.assertNotNull( g.getDetail() );
                // assertEqualsString( gd.getDescription(), g.getDetail().getDescription() );
            }
        }
    }

    private static void assertEqualsString( String strExpected, String strActual )
    {
        if (strExpected == null)
        {
            strExpected = "";
        }

        if (strActual == null)
        {
            strActual = "";
        }

        Assert.assertEquals( strExpected, strActual );
    }

    private final static String userPassword = "defaultPwd#123";
    // these search criteria must match either name or description (substring)
    private final static String usersSearchCriteria = "SamOSTestUser";
    private final static String groupsSearchCriteria = "SamOSTestGroup";

    private final static String user1Name = "SamOSTestUser1";
    private final static String user1FirstName = "user1FirstName";
    private final static String user1LastName = "user1LastName";
    private final static String user1Description = "user1Description";

    private final static String user2Name = "SamTestUser2";
    private final static String user2FirstName = "user2FirstName";
    private final static String user2LastName = "user2LastName";
    private final static String user2Description = "This is SamOSTestUser2 Description";

    private final static String user3Name = "SamOSTestUser3";
    private final static String user3FirstName = "user3FirstName";
    private final static String user3LastName = "user3LastName";
    private final static String user3Description = "user3Description";

    private final static String user4Name = "SamTestUser4";
    private final static String user4FirstName = "user4FirstName";
    private final static String user4LastName = "user4LastName";
    private final static String user4Description = "This is SamOSTestUser4 Description";

    private final static String user5Name = "SamOSTestUser5";
    private final static String user5FirstName = "user5FirstName";
    private final static String user5LastName = "user5LastName";
    private final static String user5Description = null;

    private final static String user6Name = "SamOSTestUser6";
    private final static String user6FirstName = "user6FirstName";
    private final static String user6LastName = "user6LastName";
    private final static String user6Description = "";

    private final static String group1Name = "SamOSTestGroup1";
    private final static String group1Description = "group1Description";

    private final static String group2Name = "SamTestGroup2";
    private final static String group2Description = "This is SamOSTestGroup2 Description";

    private static ISecurityAccountsLibrary getSamLib()
    {
        if(_lib == null)
        {
            if( SystemUtils.IS_OS_WINDOWS )
            {
                _lib = new WinSecurityAccountsLibrary();
            }
            else if ( SystemUtils.IS_OS_LINUX )
            {
                _lib = new LinuxSecurityAccountsLibrary();
            }
            else
            {
                throw new RuntimeException( "Only Linux and Windows os's are suported." );
            }
        }

        return _lib;
    }

    private static final String FIRST_NAME_FRIENDLY_NAME = "givenName";
    private static final String LAST_NAME_FRIENDLY_NAME = "surname";
    private static final String GROUPS_FRIENDLY_NAME = "Groups";
    private static final String SUBJECT_TYPE_FRIENDLY_NAME = "SubjectType";
    private static final String USER_PRINCIPAL_NAME_FRIENDLY_NAME = "userPrincipalName";

    private static Collection<Attribute> getAttributes()
    {
        Collection<Attribute> attrList = new ArrayList<Attribute>();

        Attribute attr = new Attribute("http://rsa.com/schemas/attr-names/2009/01/GroupIdentity");
        attr.setFriendlyName(GROUPS_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname");
        attr.setFriendlyName(FIRST_NAME_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname");
        attr.setFriendlyName(LAST_NAME_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute("http://vmware.com/schemas/attr-names/2011/07/isSolution");
        attr.setFriendlyName(SUBJECT_TYPE_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute("http://schemas.xmlsoap.org/claims/UPN");
        attr.setFriendlyName(USER_PRINCIPAL_NAME_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        return attrList;

    }
}

