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

package com.vmware.identity.idm.server.test.integration;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.KnownSamlAttributes;
import com.vmware.identity.idm.OperatorAccessPolicy;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.provider.ILdapConnectionProvider;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PooledLdapConnectionIdentity;
import com.vmware.identity.idm.server.provider.PrincipalGroupLookupInfo;
import com.vmware.identity.idm.server.provider.UserSet;
import com.vmware.identity.idm.server.provider.vmwdirectory.SystemTenantProvider;
import com.vmware.identity.idm.server.test.integration.util.ConnectionPoolProvider;
import com.vmware.identity.idm.server.test.integration.util.LdapConnectionProvider;
import com.vmware.identity.idm.server.test.integration.util.testUtils;
import com.vmware.identity.interop.ldap.AlreadyExistsLdapException;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.AfterClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.experimental.runners.Enclosed;
import org.junit.rules.TestWatcher;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameter;
import org.junit.runners.Parameterized.Parameters;

@RunWith(Enclosed.class)
public class SystemTenantProviderIT extends IDMServerBaseIT {

    @RunWith(Parameterized.class)
    public static class ProviderContextTests {

        @Rule
        public TestWatcher testExecLogger = testUtils.getTestLogger();

        @Parameters
        public static Collection<TestParam> data() {
            ArrayList<TestParam> testParams = new ArrayList<TestParam>();
            testParams.add(new TestParam(null, systemTenantProviderDefaultBase));
            testParams.add(new TestParam(null, systemTenantProviderCustomBase));

            return testParams;
        }

        @Parameter // first data value (0) is default
        public TestParam param;

        @Test
        public void test_getName() throws IDMException {
            try{
                assertEquals(this.param.getContextPrefix(), domainName, this.param.provider.getName());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }
        @Test
        public void test_getDomain() {
            try{
                assertEquals(this.param.getContextPrefix(), domainName, this.param.provider.getDomain());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_getAlias() {
            try{
                assertEmpty(this.param.getContextPrefix(), this.param.provider.getAlias());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
       }

        @Test
        public void test_getRegisteredUpnSuffixes() {
            try{
                assertEmpty(this.param.getContextPrefix(), this.param.provider.getRegisteredUpnSuffixes());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_getDomains() {
            try{
                assertEquals(this.param.getContextPrefix(), true, this.param.provider.getDomains()!= null);
                assertEquals(this.param.getContextPrefix(), true, this.param.provider.getDomains().size() == 1);
                SecurityDomain secDomain = systemTenantProviderDefaultBase.getDomains().iterator().next();
                assertEquals(this.param.getContextPrefix(), domainName, secDomain.getName());
                assertEmpty(this.param.getContextPrefix(), secDomain.getAlias());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_getStoreUPNAttributeName() {
            try{
                assertEquals(this.param.getContextPrefix(), attrNameUserPrincipalName, this.param.provider.getStoreUPNAttributeName());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_getStoreUserHintAttributeName() {
            try{
                assertEmpty(this.param.getContextPrefix(), this.param.provider.getStoreUserHintAttributeName());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_getCertificateMappingUseUPN() {
            try{
                assertEquals(this.param.getContextPrefix(), true, this.param.provider.getCertificateMappingUseUPN());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findUsers() {
            try{
                Set<PersonUser> users1 = this.param.provider.findUsersByName("", domainName, -1);
                Set<PersonUser> users2 = this.param.provider.findUsers("", domainName, -1);

                assertSetEquals(this.param.getContextPrefix(), users1, users2);
                users1 = this.param.provider.findUsersByName("jit", domainName, -1);
                assertTrue(this.param.getContextPrefix() + "jit user result null", users1==null || users1.isEmpty());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findDisabledUsers() throws Exception {
            try{
                Set<PersonUser> users1 = this.param.provider.findDisabledUsers("", -1);
                assertActiveUsers(this.param.getContextPrefix(), users1);
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findLockedUsers() throws Exception {
            try{
                Set<PersonUser> users1 = this.param.provider.findLockedUsers("", -1);
                assertActiveUsers(this.param.getContextPrefix(), users1);
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findGroups() {
            try{
                Set<Group> groups1 = this.param.provider.findGroups("", domainName, -1);
                Set<Group> groups2 = this.param.provider.findGroupsByName("", domainName, -1);

                assertGroupSetEquals(this.param.getContextPrefix(), groups1, groups2);
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findGroupsByNameInGroup() {
            try{
                Set<Group> groups = this.param.provider.findGroupsByNameInGroup(opsgroup2, "group", -1);
                if (this.param.isDomainScopedProvider()){
                    assertNotNull(this.param.getContextPrefix() + "search group in opsgroup2 result not null", groups);
                    assertEquals(this.param.getContextPrefix() + "search grpup in opsgroup2 result size==1", 1, groups.size());
                    assertTrue(this.param.getContextPrefix() + "search group in opsgroup2 result has group2", getgroups(groups).contains(group2));
                } else {
                    assertTrue(this.param.getContextPrefix() + "search group in opsgroup2 result null", groups==null || groups.isEmpty());
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findUsersByNameInGroup() {
            try{
                Set<PersonUser> users = this.param.provider.findUsersByNameInGroup(opsgroup2, "operator", -1);
                assertNotNull(this.param.getContextPrefix() + "search operator in opsgroup2 result not null", users);
                assertEquals(this.param.getContextPrefix() + "search operator in opsgroup2 result size==2", 2, users.size());
                Set<PrincipalId> usersIds = getusers(users);
                assertTrue(this.param.getContextPrefix() + "search operator in opsgroup2 result has operaotr1", usersIds.contains(op1));
                assertTrue(this.param.getContextPrefix() + "search operator in opsgroup2 result has operaotr1", usersIds.contains(op2));

                users = this.param.provider.findUsersByNameInGroup(opsgroup2, "jit", -1);
                assertTrue(this.param.getContextPrefix() + "jit user in opsgroup2 result null", users==null || users.isEmpty());
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_find(){
            try{
                SearchResult result = this.param.provider.findByName("op", domainName, -1);
                assertNotNull(this.param.getContextPrefix() + "findbyname(op) != null", result);
                assertNotNull(this.param.getContextPrefix() + "findbyname(op).getPersonUsers != null", result.getPersonUsers());
                assertNotNull(this.param.getContextPrefix() + "findbyname(op).getGroups != null", result.getGroups());
                if (this.param.isDomainScopedProvider() ){
                    assertTrue(this.param.getContextPrefix() + "findbyname(op).getPersonUsers.size() >= 2", result.getPersonUsers().size() >= 2);
                    assertTrue(this.param.getContextPrefix() + "findbyname(op).getGroups.size() >== 3", result.getGroups().size() >= 3);
                } else {
                    assertTrue(this.param.getContextPrefix() + "findbyname(op).getGroups == null || size==0",
                        (result.getSolutionUsers() == null || result.getSolutionUsers().size() == 0));
                    assertEquals(this.param.getContextPrefix() + "findbyname(op).getPersonUsers.size() ==2", 2, result.getPersonUsers().size());
                    assertEquals(this.param.getContextPrefix() + "findbyname(op).getGroups.size() == 3", result.getGroups().size(),3);
                }

                Set<PrincipalId> users = getusers(result.getPersonUsers());
                Set<PrincipalId> groups = getgroups(result.getGroups());
                assertTrue(this.param.getContextPrefix() + "findbyname(op).getPersonUsers.conmtains (op1)", users.contains(op1));
                assertTrue(this.param.getContextPrefix() + "findbyname(op).getPersonUsers.conmtains (op2)", users.contains(op2));
                assertTrue(this.param.getContextPrefix() + "findbyname(op).getGroups.conmtains (opsgroup1)", groups.contains(opsgroup1));
                assertTrue(this.param.getContextPrefix() + "findbyname(op).getGroups.conmtains (opsgroup2)", groups.contains(opsgroup2));
                assertTrue(this.param.getContextPrefix() + "findbyname(op).getGroups.conmtains (opsgroup3)", groups.contains(opsgroup3));
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }
    }

    @RunWith(Parameterized.class)
    public static class UserContextTests {

        @Rule
        public TestWatcher testExecLogger = testUtils.getTestLogger();

        @Parameters
        public static Collection<TestParam> data() {
            ArrayList<TestParam> testParams = new ArrayList<TestParam>();
            for(PrincipalId u : domainUsers) {
                testParams.add(new TestParam(u, systemTenantProviderDefaultBase));
                testParams.add(new TestParam(u, systemTenantProviderCustomBase));
            }
            for(PrincipalId u : opsUsers) {
                testParams.add(new TestParam(u, systemTenantProviderCustomBase));
            }
            return testParams;
        }

        @Parameter // first data value (0) is default
        public TestParam param;

        @Test
        public void test_authenticate() {
            try{
                PrincipalId authUser = this.param.provider.authenticate(this.param.principal, systemTenantAdminPassword);
                assertPrincipalId(this.param.getContextPrefix(), this.param.principal, authUser);
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }

            try{
                this.param.provider.authenticate(this.param.principal, systemTenantAdminPassword+"1");
                fail(String.format("%s: invalid pwd login should fail", this.param.getContextPrefix()));
            } catch(Exception t) {}
        }

        @Test
        public void test_getAttributes(){
            try{
                if ( expectedToFailForScopedUser(this.param) ) {
                    try{
                        this.param.provider.getAttributes(this.param.principal, getAttributes());
                        fail(String.format("%s: getAttributes should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}
                } else {
                    Collection<AttributeValuePair> attributes = this.param.provider.getAttributes(this.param.principal, getAttributes());
                    assertAtributes(this.param.getContextPrefix(), this.param.principal, attributes, this.param.isOpsScopedProvider());
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_IsActive() {
            try{
                if ( expectedToFailForScopedUser(this.param) ) {
                    try{
                        this.param.provider.IsActive(this.param.principal);
                        fail(String.format("%s: IsActive should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}
                } else {
                    assertTrue(this.param.getContextPrefix(), this.param.provider.IsActive(this.param.principal) );
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findActiveUser() {
            try{
                if ( expectedToFailForScopedUser(this.param) ) {
                    PrincipalId activeUser = this.param.provider.findActiveUser(attrNameUserPrincipalName, this.param.principal.getUPN());
                    assertNull(this.param.getContextPrefix() + "findActiveUser==null", activeUser);
                } else {
                    PrincipalId activeUser = this.param.provider.findActiveUser(attrNameUserPrincipalName, this.param.principal.getUPN());
                    assertPrincipalId(this.param.getContextPrefix(), this.param.principal, activeUser);
                    activeUser = this.param.provider.findActiveUser("samAccountName", this.param.principal.getName());
                    assertPrincipalId(this.param.getContextPrefix(), this.param.principal, activeUser);
                }
            }catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findActiveUsersInDomain() {
            try{
                if ( expectedToFailForScopedUser(this.param) ) {
                    UserSet activeUsers = this.param.provider.findActiveUsersInDomain(
                        attrNameUserPrincipalName, this.param.principal.getUPN(), "", "");
                    assertTrue(this.param.getContextPrefix()+ "findActiveUsersInDomain==null", activeUsers == null || activeUsers.isEmpty());
                } else {
                    UserSet activeUsers = this.param.provider.findActiveUsersInDomain(
                        attrNameUserPrincipalName, this.param.principal.getUPN(), "", "");
                    assertNotNull(this.param.getContextPrefix() + "findActiveUsersInDomain() != null", activeUsers);
                    assertEquals(this.param.getContextPrefix() + "findActiveUsersInDomain().size() == 1", 1, activeUsers.size() );
                    assertPrincipalId(this.param.getContextPrefix(), this.param.principal, activeUsers.keySet().iterator().next());
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findUser() {
            try{
                if ( expectedToFailForScopedUser(this.param) ) {
                    PersonUser foundUser = this.param.provider.findUser(this.param.principal);
                    assertNull(this.param.getContextPrefix() + "findUser == null;", foundUser);
                } else {
                    PersonUser foundUser = this.param.provider.findUser(this.param.principal);
                    assertPersonUser(this.param.getContextPrefix(), this.param.principal, foundUser);
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findUserByObjectId() {
            try{
                if (!expectedToFailForScopedUser(this.param)){
                    PersonUser foundUser = this.param.provider.findUser(this.param.principal);
                    assertPersonUser(this.param.getContextPrefix(), this.param.principal, foundUser);

                    if (!expectedToFailForScopedUser(this.param)){
                        foundUser = this.param.provider.findUserByObjectId(foundUser.getObjectId());
                        assertPersonUser(this.param.getContextPrefix(), this.param.principal, foundUser);
                    } else {
                        try{
                            this.param.provider.findUserByObjectId(foundUser.getObjectId());
                            fail(String.format("%s: findUserByObjectId should fail", this.param.getContextPrefix()));
                        } catch(Exception ex) {}
                    }
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findDirectParentGroups() {
            try{
                if ( expectedToFailForScopedUser(this.param) ) {
                    try{
                        this.param.provider.findDirectParentGroups(this.param.principal);
                        fail(String.format("%s: findDirectParentGroups should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}
                } else {
                    if (!this.param.isAdmin()) {
                        PrincipalGroupLookupInfo direct = this.param.provider.findDirectParentGroups(this.param.principal);
                        assertDirectParentGroups(this.param.getContextPrefix(), this.param.principal, direct, this.param.isDomainScopedProvider());
                    }
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findNestedParentGroups() {
            try{
                if ( expectedToFailForScopedUser(this.param) ) {
                    try{
                        this.param.provider.findNestedParentGroups(this.param.principal);
                        fail(String.format("%s: findNestedParentGroups should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}
                } else {
                    if (!this.param.isAdmin()) {
                        PrincipalGroupLookupInfo direct = this.param.provider.findNestedParentGroups(this.param.principal);
                        assertNestedParentGroups(this.param.getContextPrefix(), this.param.principal, direct, this.param.isDomainScopedProvider());
                    }
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }
    }

    @RunWith(Parameterized.class)
    public static class GroupContextTests {

        @Rule
        public TestWatcher testExecLogger = testUtils.getTestLogger();

        @Parameters
        public static Collection<TestParam> data() {
            ArrayList<TestParam> testParams = new ArrayList<TestParam>();
            for(PrincipalId g : domainGroups) {
                testParams.add(new TestParam(g, systemTenantProviderDefaultBase));
                testParams.add(new TestParam(g, systemTenantProviderCustomBase));
            }
            for(PrincipalId u : opsGroups) {
                testParams.add(new TestParam(u, systemTenantProviderCustomBase));
            }
            return testParams;
        }

        @Parameter // first data value (0) is default
        public TestParam param;

        @Test
        public void test_findGroup() {
            try{
                if ( expectedToFailForScopedGroup(this.param) ) {
                    Group foundGroup = this.param.provider.findGroup(this.param.principal);
                    assertNull(this.param.getContextPrefix() + "findGroup==null", foundGroup);
                } else {
                    Group foundGroup = this.param.provider.findGroup(this.param.principal);
                    assertGroup(this.param.getContextPrefix(), this.param.principal, foundGroup);
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findGroupByObjectId() {
            try{
                if (!expectedToFailForScopedGroup(this.param)){
                    Group foundGroup = this.param.provider.findGroup(this.param.principal);
                    assertGroup(this.param.getContextPrefix(), this.param.principal, foundGroup);

                    if (!expectedToFailForScopedGroup(this.param)){
                        foundGroup = this.param.provider.findGroupByObjectId(foundGroup.getObjectId());
                        assertGroup(this.param.getContextPrefix(), this.param.principal, foundGroup);
                    } else {
                        try{
                            this.param.provider.findGroupByObjectId(foundGroup.getObjectId());
                            fail(String.format("%s: findGroupByObjectId should fail", this.param.getContextPrefix()));
                        } catch(Exception ex) {}
                    }
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findGroupsInGroup() {
            try{
                if ( expectedToFailForScopedGroup(this.param) ) {
                    try{
                        this.param.provider.findGroupsInGroup(this.param.principal, "", -1);
                        fail(String.format("%s: findGroupsInGroup should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}
                    try{
                        this.param.provider.findGroupsByNameInGroup(this.param.principal, "", -1);
                        fail(String.format("%s: findGroupsByNameInGroup should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}

                } else {
                    Set<Group> groups1 = this.param.provider.findGroupsInGroup(this.param.principal, "", -1);
                    Set<Group> groups2 = this.param.provider.findGroupsByNameInGroup(this.param.principal, "", -1);

                    assertGroupSetEquals(this.param.getContextPrefix(), groups1, groups2);
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }

        @Test
        public void test_findUsersInGroup() {
            try{
                if ( expectedToFailForScopedGroup(this.param) ) {
                    try{
                        this.param.provider.findUsersInGroup(this.param.principal, "", -1);
                        fail(String.format("%s: findUsersInGroup should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}
                    try{
                        this.param.provider.findUsersByNameInGroup(this.param.principal, "", -1);
                        fail(String.format("%s: findUsersByNameInGroup should fail", this.param.getContextPrefix()));
                    } catch(Exception ex) {}
                } else {
                    Set<PersonUser> users1 = this.param.provider.findUsersInGroup(this.param.principal, "", -1);
                    Set<PersonUser> users2 = this.param.provider.findUsersByNameInGroup(this.param.principal, "", -1);

                    assertSetEquals(this.param.getContextPrefix(), users1, users2);
                }
            } catch(Exception t) {
                System.out.printf("%s: Unexpected exception: %s\n", this.param.getContextPrefix(), t.toString());
                t.printStackTrace();
                fail(String.format("%s: Unexpected exception: ", this.param.getContextPrefix(), t.getMessage()));
            }
        }
    }


    private static List<String> objctsToClean;

    private static SystemTenantProvider systemTenantProviderDefaultBase;
    private static SystemTenantProvider systemTenantProviderCustomBase;

    private static final String unique;

    private static final String attrNameGivenName = "givenName";
    private static final String attrNameSn = "sn";
    private static final String attrNameMemberOf = "memberOf";
    private static final String attrNameSubjectType = "subjectType";
    private static final String attrNameUserPrincipalName = "userPrincipalName";
    private static String domainName;

    private static ConnectionPoolProvider pool = new ConnectionPoolProvider();
    private static ILdapConnectionProvider ldapProvider = new LdapConnectionProvider();

    private static PrincipalId user1;
    private static PrincipalId admin;
    private static PrincipalId op1;
    private static PrincipalId op2;
    private static PrincipalId jitUser1;
    private static PrincipalId jitUser2;
    private static PrincipalId group1;
    private static PrincipalId group2;
    private static PrincipalId opsgroup1;
    private static PrincipalId opsgroup2;
    private static PrincipalId opsgroup3;


    private static final String USERS_CNT = "Users";
    private static final String GROUPS_CNT = "Groups";
    private static final String OPS_USERS_CNT = "OperatorsUsers";
    private static final String OPS_GROUPS_CNT = "OperatorsGroups";
    private static final String USER1 = "user1";
    private static final String OP_USER1 = "operator1";
    private static final String OP_USER2 = "operator2";
    private static final String JIT_USER1 = "jitUser1";
    private static final String JIT_USER2 = "jitUser2";
    private static final String GROUP1 = "group1";
    private static final String GROUP2 = "group2";
    private static final String OPS_GROUP1 = "opsGroup1";
    private static final String OPS_GROUP2 = "opsGroup2";
    private static final String OPS_GROUP3 = "opsGroup3";

    private static Set<PrincipalId> domainUsers;
    private static Set<PrincipalId> opsUsers;

    private static Set<PrincipalId> domainGroups;
    private static Set<PrincipalId> opsGroups;

    private static Map<PrincipalId, HashSet<PrincipalId>> usersToGroupsDirect;
    private static Map<PrincipalId, HashSet<PrincipalId>> usersToGroupsNested;
    private static Map<PrincipalId, HashSet<PrincipalId>> groupsToUsers;

    private static class TestParam{
        public PrincipalId principal;
        public SystemTenantProvider provider;

        public TestParam(PrincipalId principal, SystemTenantProvider provider) {
            this.principal = principal;
            this.provider = provider;
        }

        public boolean isDomainUser() {
            return (this.principal != null) && (domainUsers.contains(principal));
        }

        public boolean isOpsUser() {
            return (this.principal != null) && (opsUsers.contains(principal));
        }

        public boolean isDomainGroup() {
            return (this.principal != null) && (domainGroups.contains(principal));
        }

        public boolean isOpsGroup() {
            return (this.principal != null) && (opsGroups.contains(principal));
        }

        public boolean isUser() {
            return (this.principal != null) && (domainUsers.contains(principal));
        }

        public boolean isGroup() {
            return (this.principal != null) && (domainGroups.contains(principal));
        }

        public boolean isProviderWide() {
            return (this.principal == null);
        }

        public boolean isOpsScopedProvider() {
            return this.provider == systemTenantProviderCustomBase;
        }

        public boolean isDomainScopedProvider() {
            return this.provider == systemTenantProviderDefaultBase;
        }

        public boolean isAdmin(){
            return this.isUser() && this.principal == admin;
        }
        public boolean isJitUser(){
            return this.isUser() && (this.principal == jitUser1 || this.principal == jitUser2);
        }

        public String getContextPrefix(){
            return String.format("%s;%s",
                (this.isDomainScopedProvider() ? "Domain scoped provider" : "Ops scoped provider"),
                    (this.isProviderWide()
                        ? ""
                        : String.format( " %s=%s;", (this.isGroup() ? "group" : "user"), this.principal.getUPN())));
        }
    }

    static {
        unique = java.util.UUID.randomUUID().toString();
        try {
            setUp();
		} catch (Exception e) {
			fail(String.format("Test setup failed. Error='%s'",e.toString()));
		}
    }

    private static void setUp() throws Exception {

        IDMServerBaseIT.setUp("idmservertest.properties");

        domainName = systemTenant;

        user1 = new PrincipalId(uniqueName(USER1), domainName);
        admin = new PrincipalId(systemTenantAdminUsername.split("@")[0], systemTenantAdminUsername.split("@")[1]);
        op1 = new PrincipalId(uniqueName(OP_USER1), domainName);
        op2 = new PrincipalId(uniqueName(OP_USER2), domainName);
        jitUser1 = new PrincipalId(uniqueName(JIT_USER1), domainName);
        jitUser2 = new PrincipalId(uniqueName(JIT_USER2), domainName);
        group1 = new PrincipalId(uniqueName(GROUP1), domainName);
        group2 = new PrincipalId(uniqueName(GROUP2), domainName);
        opsgroup1 = new PrincipalId(uniqueName(OPS_GROUP1), domainName);
        opsgroup2 = new PrincipalId(uniqueName(OPS_GROUP2), domainName);
        opsgroup3 = new PrincipalId(uniqueName(OPS_GROUP3), domainName);

        domainUsers = new HashSet<PrincipalId>();
        domainUsers.add( user1 );
        domainUsers.add( admin );
        domainUsers.add( op1 );
        domainUsers.add( op2 );

        opsUsers = new HashSet<PrincipalId>();
        opsUsers.add(op1);
        opsUsers.add(op2);

        domainGroups = new HashSet<PrincipalId>();
        domainGroups.add(group1);
        domainGroups.add(group2);
        domainGroups.add(opsgroup1);
        domainGroups.add(opsgroup2);
        domainGroups.add(opsgroup3);

        opsGroups = new HashSet<PrincipalId>();
        opsGroups.add(opsgroup1);
        opsGroups.add(opsgroup2);
        opsGroups.add(opsgroup3);

        OperatorAccessPolicy.Builder b = new OperatorAccessPolicy.Builder()
            .withEnabled(true);

        systemTenantProviderDefaultBase = new SystemTenantProvider(
            systemTenant, getIDS(domainDN, domainDN), b.build(), pool, ldapProvider, true);

        b = new OperatorAccessPolicy.Builder()
            .withEnabled(true)
            .withUserBaseDn(dn( uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT)))
            .withGroupBaseDn(dn( uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)));

        systemTenantProviderCustomBase = new SystemTenantProvider(
            systemTenant,
            getIDS(domainDN, domainDN),
            b.build(),
            pool, ldapProvider, true
        );

        try
        {
            setupTestUsersGroups();
        }
        catch(Exception ex) {
            cleanupTestUsersGroups();
            throw ex;
        }
    }

    @AfterClass
    public static void cleanUp() throws Exception {
        cleanupTestUsersGroups();
        if (pool != null) {
            pool.close();
            pool = null;
        }
    }

    private static void setupTestUsersGroups() throws Exception {

        // cn = users
        //   cn = user1 // user
        //   cn = jitUser1 // jit user
        //   cn = OperatorsUsers
        //      cn = operator1
        //      cn = operator2
        //      cn =jitUser2 // jit user2
        // cn = groups
        //   cn = group1
        //   cn = OperatorsGroups
        //      cn = opsGroup1
        //      cn = opsGroup2

        objctsToClean = new ArrayList<String>();

        try(PooledLdapConnection conn = pool.borrowConnection(
            new PooledLdapConnectionIdentity.Builder(
                "ldaps://"+ domainControllerFQDN, AuthenticationType.SRP)
                .setTenantName(systemTenant)
                .setUsername(systemTenantAdminUsername)
                .setPassword(systemTenantAdminPassword)
                .build())) {

            ILdapConnectionEx c = conn.getConnection();
            addLdapObject(c, dn(uniqueName(USERS_CNT)), getContainerLdapMods(uniqueName(USERS_CNT)) );
                addLdapObject(c, dn(uniqueName(USER1), uniqueName(USERS_CNT) ), getUserLdapMods(uniqueName(USER1)) );
                addLdapObject(c, dn(uniqueName(JIT_USER1), uniqueName(USERS_CNT) ), getJitUserLdapMods(uniqueName(JIT_USER1)) );
                addLdapObject(c, dn(uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ), getContainerLdapMods(uniqueName(OPS_USERS_CNT)));
                    addLdapObject(c, dn(uniqueName(OP_USER1), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ), getUserLdapMods(uniqueName(OP_USER1)) );
                    addLdapObject(c, dn(uniqueName(OP_USER2), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ), getUserLdapMods(uniqueName(OP_USER2)) );
                    addLdapObject(c, dn(uniqueName(JIT_USER2), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ), getJitUserLdapMods(uniqueName(JIT_USER2)) );
            addLdapObject(c, dn(uniqueName(GROUPS_CNT)), getContainerLdapMods(uniqueName(GROUPS_CNT)) );
                addLdapObject(c, dn(uniqueName(GROUP1), uniqueName(GROUPS_CNT) ), getGroupsLdapMods(uniqueName(GROUP1)) );
                addLdapObject(c, dn(uniqueName(GROUP2), uniqueName(GROUPS_CNT) ), getGroupsLdapMods(uniqueName(GROUP2)) );
                addLdapObject(c, dn(uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT) ), getContainerLdapMods(uniqueName(OPS_GROUPS_CNT)));
                    addLdapObject(c, dn(uniqueName(OPS_GROUP1), uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)), getGroupsLdapMods(uniqueName(OPS_GROUP1)) );
                    addLdapObject(c, dn(uniqueName(OPS_GROUP2), uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)), getGroupsLdapMods(uniqueName(OPS_GROUP2)) );
                    addLdapObject(c, dn(uniqueName(OPS_GROUP3), uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)), getGroupsLdapMods(uniqueName(OPS_GROUP3)) );

            // group memberships
            //   group1: user1, jitUser1, jitUser2
            //   opsGroup1: operator1, Administrator
            //   opsGroup2: operator1, operator2, jitUser1, jitUser2
            //   group2: opsGroup2
            //   opsGroup3: group2, user1, operator1
            modifyLdapObject(
                c, dn(uniqueName(GROUP1), uniqueName(GROUPS_CNT) ),
                getGroupMemberLdapMod(
                    dn(uniqueName(USER1), uniqueName(USERS_CNT) ),
                    dn(uniqueName(JIT_USER1), uniqueName(USERS_CNT) ),
                    dn(uniqueName(JIT_USER2), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ))
            );
            modifyLdapObject(
                c, dn(uniqueName(OPS_GROUP1), uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)),
                getGroupMemberLdapMod(
                    dn(uniqueName(OP_USER1), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ),
                    adminUserdn())
            );
            modifyLdapObject(
                c, dn(uniqueName(OPS_GROUP2), uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)),
                getGroupMemberLdapMod(
                    dn(uniqueName(OP_USER1), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ),
                    dn(uniqueName(OP_USER2), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ),
                    dn(uniqueName(JIT_USER1), uniqueName(USERS_CNT) ),
                    dn(uniqueName(JIT_USER2), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ),
                    dn(uniqueName(GROUP2), uniqueName(GROUPS_CNT) ))
            );
            modifyLdapObject(
                c, dn(uniqueName(GROUP2), uniqueName(GROUPS_CNT)),
                getGroupMemberLdapMod(
                    dn(uniqueName(OPS_GROUP3), uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)))
            );
            modifyLdapObject(
                c, dn(uniqueName(OPS_GROUP3), uniqueName(OPS_GROUPS_CNT), uniqueName(GROUPS_CNT)),
                getGroupMemberLdapMod(
                    dn(uniqueName(USER1), uniqueName(USERS_CNT) ),
                    dn(uniqueName(OP_USER1), uniqueName(OPS_USERS_CNT), uniqueName(USERS_CNT) ))
            );

            usersToGroupsDirect = new HashMap<PrincipalId, HashSet<PrincipalId>>();
            usersToGroupsNested = new HashMap<PrincipalId, HashSet<PrincipalId>>();
            groupsToUsers = new HashMap<PrincipalId, HashSet<PrincipalId>>();

            // group memberships
            //   group1: user1, jitUser1, jitUser2
            //   opsGroup1: operator1, Administrator
            //   opsGroup2: operator1, operator2, jitUser1, jitUser2
            //   group2: opsGroup2
            //   opsGroup3: group2, user1, operator1

            groupsToUsers.put(group1, new HashSet<PrincipalId>());
            groupsToUsers.get(group1).add(user1);
            groupsToUsers.get(group1).add(jitUser1);
            groupsToUsers.get(group1).add(jitUser2);

            groupsToUsers.put(opsgroup1, new HashSet<PrincipalId>());
            groupsToUsers.get(opsgroup1).add(op1);
            groupsToUsers.get(opsgroup1).add(admin);

            groupsToUsers.put(opsgroup2, new HashSet<PrincipalId>());
            groupsToUsers.get(opsgroup2).add(op1);
            groupsToUsers.get(opsgroup2).add(op2);
            groupsToUsers.get(opsgroup2).add(jitUser1);
            groupsToUsers.get(opsgroup2).add(jitUser2);

            groupsToUsers.put(group2, new HashSet<PrincipalId>());
            groupsToUsers.get(group2).add(opsgroup2);

            groupsToUsers.put(opsgroup3, new HashSet<PrincipalId>());
            groupsToUsers.get(opsgroup3).add(group2);
            groupsToUsers.get(opsgroup3).add(user1);
            groupsToUsers.get(opsgroup3).add(op1);

            usersToGroupsDirect.put(user1, new HashSet<PrincipalId>());
            usersToGroupsDirect.get(user1).add(group1);
            usersToGroupsDirect.get(user1).add(opsgroup3);

            usersToGroupsDirect.put(jitUser1, new HashSet<PrincipalId>());
            usersToGroupsDirect.get(jitUser1).add(group1);
            usersToGroupsDirect.get(jitUser1).add(opsgroup2);

            usersToGroupsDirect.put(jitUser2, new HashSet<PrincipalId>());
            usersToGroupsDirect.get(jitUser2).add(group1);
            usersToGroupsDirect.get(jitUser2).add(opsgroup2);

            usersToGroupsDirect.put(op1, new HashSet<PrincipalId>());
            usersToGroupsDirect.get(op1).add(opsgroup1);
            usersToGroupsDirect.get(op1).add(opsgroup2);
            usersToGroupsDirect.get(op1).add(opsgroup3);

            usersToGroupsDirect.put(op2, new HashSet<PrincipalId>());
            usersToGroupsDirect.get(op2).add(opsgroup2);

            usersToGroupsDirect.put(admin, new HashSet<PrincipalId>());
            usersToGroupsDirect.get(admin).add(opsgroup1);



            usersToGroupsNested.put(user1, new HashSet<PrincipalId>());
            usersToGroupsNested.get(user1).add(group1);
            usersToGroupsNested.get(user1).add(opsgroup3);
            usersToGroupsNested.get(user1).add(group2);
            usersToGroupsNested.get(user1).add(opsgroup2);

            usersToGroupsNested.put(jitUser1, new HashSet<PrincipalId>());
            usersToGroupsNested.get(jitUser1).add(group1);
            usersToGroupsNested.get(jitUser1).add(opsgroup2);

            usersToGroupsNested.put(jitUser2, new HashSet<PrincipalId>());
            usersToGroupsNested.get(jitUser2).add(group1);
            usersToGroupsNested.get(jitUser2).add(opsgroup2);

            usersToGroupsNested.put(op1, new HashSet<PrincipalId>());
            usersToGroupsNested.get(op1).add(opsgroup1);
            usersToGroupsNested.get(op1).add(opsgroup2);
            usersToGroupsNested.get(op1).add(opsgroup3);
            usersToGroupsNested.get(op1).add(group2);

            usersToGroupsNested.put(op2, new HashSet<PrincipalId>());
            usersToGroupsNested.get(op2).add(opsgroup2);

            usersToGroupsNested.put(admin, new HashSet<PrincipalId>());
            usersToGroupsNested.get(admin).add(opsgroup1);
        }
    }

    private static void cleanupTestUsersGroups() throws Exception {
        try(PooledLdapConnection conn = pool.borrowConnection(
            new PooledLdapConnectionIdentity.Builder(
                "ldaps://"+ domainControllerFQDN, AuthenticationType.SRP)
                .setTenantName(systemTenant)
                .setUsername(systemTenantAdminUsername)
                .setPassword(systemTenantAdminPassword)
                .build())) {

            if ( objctsToClean != null ) {
                for (int i = objctsToClean.size() -1; i >= 0; i-- ) {
                    deleteLdapObject(conn.getConnection(), objctsToClean.get(i));
                }
            }
        }
    }

    private static void deleteLdapObject(ILdapConnectionEx connection, String dn) {
        try
        {
            connection.deleteObject(dn);
        }
        catch(NoSuchObjectLdapException ex)
        {
            // ignore
        }
    }

    private static void addLdapObject(ILdapConnectionEx connection, String dn, LdapMod[] props) {

        try {
            connection.addObject(dn, props);
            objctsToClean.add(dn);
        }
        catch(AlreadyExistsLdapException ex) {
            // ignore
        }
    }

    private static void modifyLdapObject(ILdapConnectionEx connection, String dn, LdapMod prop) {

        connection.modifyObject(dn, prop);
    }

    private static LdapMod[] getContainerLdapMods(String name)
    {
        return new LdapMod[] {
            new LdapMod(LdapModOperation.ADD, "objectClass",
                    new LdapValue[] {
                            LdapValue.fromString("container")}),
            new LdapMod(LdapModOperation.ADD, "cn",
                    new LdapValue[] {
                            LdapValue.fromString(name)}),

        };
    }

    private static LdapMod[] getUserLdapMods(String name)
    {
        return new LdapMod[] {
            new LdapMod(LdapModOperation.ADD, "objectClass",
                    new LdapValue[] {
                            LdapValue.fromString("user")}),
            new LdapMod(LdapModOperation.ADD, "cn",
                    new LdapValue[] {
                            LdapValue.fromString(name)}),
            new LdapMod(LdapModOperation.ADD, "samAccountName",
                new LdapValue[] {
                    LdapValue.fromString(name)}),
            new LdapMod(LdapModOperation.ADD, "userPrincipalName",
                new LdapValue[] {
                    LdapValue.fromString(name + "@" + domainName)}),
            new LdapMod(LdapModOperation.ADD, "givenName",
                new LdapValue[] {
                    LdapValue.fromString(name)}),
            new LdapMod(LdapModOperation.ADD, "sn",
                new LdapValue[] {
                    LdapValue.fromString(domainName)}),
            new LdapMod(LdapModOperation.ADD, "userPassword",
                new LdapValue[] {
                    LdapValue.fromString(systemTenantAdminPassword)}),
        };
    }

    private static LdapMod[] getJitUserLdapMods(String name)
    {
        return new LdapMod[] {
            new LdapMod(LdapModOperation.ADD, "objectClass",
                    new LdapValue[] {
                            LdapValue.fromString("user"),
                            LdapValue.fromString("vmwExternalIdpUser")}),
            new LdapMod(LdapModOperation.ADD, "cn",
                    new LdapValue[] {
                            LdapValue.fromString(name)}),
            new LdapMod(LdapModOperation.ADD, "samAccountName",
                new LdapValue[] {
                    LdapValue.fromString(name)}),
            new LdapMod(LdapModOperation.ADD, "vmwSTSTenantizedUserPrincipalName",
                new LdapValue[] {
                    LdapValue.fromString(name + "@" + domainName + "/" + systemTenant.toLowerCase())}),
            new LdapMod(LdapModOperation.ADD, "givenName",
                new LdapValue[] {
                    LdapValue.fromString(name)}),
            new LdapMod(LdapModOperation.ADD, "sn",
                new LdapValue[] {
                    LdapValue.fromString(domainName)}),
            new LdapMod(LdapModOperation.ADD, "vmwSTSEntityId",
                new LdapValue[] {
                    LdapValue.fromString("https://csp-issuer")}),
            new LdapMod(LdapModOperation.ADD, "vmwSTSExternalIdpUserId",
                new LdapValue[] {
                    LdapValue.fromString(
                        name + "@" + domainName + "/" + systemTenant.toLowerCase())}),
        };
    }

    private static LdapMod[] getGroupsLdapMods(String name)
    {
        return new LdapMod[] {
            new LdapMod(LdapModOperation.ADD, "objectClass",
                    new LdapValue[] {
                            LdapValue.fromString("group")}),
            new LdapMod(LdapModOperation.ADD, "cn",
                    new LdapValue[] {
                            LdapValue.fromString(name)}),
            new LdapMod(LdapModOperation.ADD, "samAccountName",
                    new LdapValue[] {
                            LdapValue.fromString(name)}),

        };
    }

    private static LdapMod getGroupMemberLdapMod(String... name)
    {
        return new LdapMod(LdapModOperation.ADD, "member",
                    ServerUtils.getLdapValue(Arrays.asList(name)));
    }

    private static String uniqueName(String name){
        return String.format("%s-%s", name, unique);
    }

    private static String dn(String cn, String... parents){
        StringBuilder builder = new StringBuilder();
        builder.append("cn=");
        builder.append(cn);
        builder.append(",");
        if (parents!= null) {
            for (int i = 0; i < parents.length; i++){
                builder.append("cn=");
                builder.append(parents[i]);
                builder.append(",");
            }
        }
        builder.append(domainDN);

        return builder.toString();
    }

    private static String adminUserdn() {
        return dn(admin.getName(), "Users");
    }

    private static void assertEmpty(String message, String value) {
        assertEquals(message, true, (value == null || value.length() == 0));
    }

    private static void assertEmpty(String message, Set<String> value) {
        assertEquals(message, true, (value == null || value.size() == 0));
    }

    private static IIdentityStoreData getIDS(String userBaseDn, String groupBaseDn) {

        ServerIdentityStoreData data = new ServerIdentityStoreData(
            DomainType.EXTERNAL_DOMAIN, domainName);

        data.setConnectionStrings(Arrays.asList("ldaps://"+ domainControllerFQDN));
        data.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY);
        data.setUserBaseDn(userBaseDn);
        data.setGroupBaseDn(groupBaseDn);
        data.setUserName(systemTenantAdminUsername);
        data.setPassword(systemTenantAdminPassword);
        data.setAuthenticationType(AuthenticationType.SRP);

        HashMap<String, String> attrMap = new HashMap<String, String>();
        attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_FIRST_NAME, attrNameGivenName);
        attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_LAST_NAME, attrNameSn);
        attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_GROUPS, attrNameMemberOf);
        attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_SUBJECT_TYPE, attrNameSubjectType);
        attrMap.put(KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME, attrNameUserPrincipalName);

        data.setAttributeMap(attrMap);

        return data;
    }

    private static void assertPrincipalId(String prefix, PrincipalId expected, PrincipalId actual) {
        assertNotNull(prefix + "authUser != null", actual);
        assertEquals( prefix + "authUser.name==user.Name", expected.getName().toLowerCase(), actual.getName().toLowerCase() );
        assertEquals( prefix + "authUser.domain==user.domain", expected.getDomain().toLowerCase(), actual.getDomain().toLowerCase() );
    }


    private static void assertPersonUser(String prefix, PrincipalId expected, PersonUser actual) {
        assertNotNull(prefix + "authUser != null", actual);
        assertPrincipalId(prefix, expected, actual.getId());
    }

    private static void assertGroup(String prefix, PrincipalId expected, Group actual) {
        assertNotNull(prefix + "group != null", actual);
        assertPrincipalId(prefix, expected, actual.getId());
    }

    private static void assertSetEquals(String prefix, Set<PersonUser> users1, Set<PersonUser> users2) {
        assertNotNull(prefix + "usersset1 != null", users1 );
        assertNotNull(prefix + "usersset2 != null", users2 );
        assertEquals(prefix + "usersset1.size() == usersset2.size()", users1.size(), users2.size() );
        for(PersonUser u: users1){
            assertNotNull(prefix + "user != null", u );
            assertTrue(prefix + "user from set 1 in set2", users2.contains(u) );
        }
    }

    private static void assertGroupSetEquals(String prefix, Set<Group> groups1, Set<Group> groups2) {
        assertNotNull(prefix + "groups1 != null", groups1 );
        assertNotNull(prefix + "groups2 != null", groups2 );
        assertEquals(prefix + "groups1.size() == groups2.size()", groups1.size(), groups2.size() );
        for(Group g: groups1){
            assertNotNull(prefix + "group != null", g );
            assertTrue(prefix + "groups from set 1 in set2", groups2.contains(g) );
        }
    }

    private static void assertActiveUsers(String prefix, Set<PersonUser> nonActive) {
        assertNotNull(prefix + "nonActive != null", nonActive );

        for(PersonUser u : nonActive) {
            assertFalse(prefix + "user not in domainUsers", domainUsers.contains(u.getId()));
        }
    }

    private static void assertDirectParentGroups(String prefix, PrincipalId user, PrincipalGroupLookupInfo membership, boolean defaultBase){
        Set<PrincipalId> actualGroups = getgroups(membership);
        assertGroupMembership(prefix, user, actualGroups, defaultBase, true);
    }

    private static void assertNestedParentGroups(String prefix, PrincipalId user, PrincipalGroupLookupInfo membership, boolean defaultBase){
        Set<PrincipalId> actualGroups = getgroups(membership);
        assertGroupMembership(prefix, user, actualGroups, defaultBase, false);
    }

    private static void assertGroupMembership(String prefix, PrincipalId user, Set<PrincipalId> membership, boolean defaultBase, boolean direct) {
        // we do not know exact group membership for admin, skip test
        if (user != admin) {
            HashSet<PrincipalId> expectedMembership = usersToGroupsNested.get(user);
            if (direct) {
                expectedMembership = usersToGroupsDirect.get(user);
            }

            // 1. all attribute values must be in the nestedGroups
            for (PrincipalId g : membership) {
                boolean contains = expectedMembership.contains(g);
                if (!contains) {
                    for(PrincipalId gid : expectedMembership){
                        System.out.printf("expected Group:%s\n", gid.getUPN());
                    }
                }
                assertTrue(
                    prefix + " groups belong to nestedGroups list;group: " + g.getUPN(),
                    expectedMembership.contains(g) );
            }

            // 2. possibly only ops groups
            int numGroups = 0;
            for(PrincipalId expectedGroup : expectedMembership) {
                if (defaultBase || opsGroups.contains(expectedGroup)) {
                    numGroups = numGroups+1;
                }
            }

            if (numGroups != membership.size()) {
                for(PrincipalId gid : expectedMembership){
                    System.out.printf("expected Group:%s\n", gid.getUPN());
                }
                for(PrincipalId gid : membership){
                    System.out.printf("actual Group:%s\n", gid.getUPN());
                }
            }
            assertEquals(prefix + " ATTRIBUTE_USER_GROUPS.getValues().size() as expected", numGroups, membership.size());
        }
    }

    private static void assertAtributes(String prefix, PrincipalId expected, Collection<AttributeValuePair> attributes, boolean customBase) throws IDMLoginException {
        assertNotNull( prefix + " attrbute collection != null", attributes );
        assertFalse( prefix + " attrbute collection.size() > 0", attributes.size() <= 0 );

        for(AttributeValuePair pair : attributes){
            assertNotNull( prefix + " attrbuteValue != null", pair );
            assertNotNull( prefix + " attrbuteValue.getAttrDefinition() != null", pair.getAttrDefinition() );
            if ( KnownSamlAttributes.ATTRIBUTE_USER_FIRST_NAME.equalsIgnoreCase(pair.getAttrDefinition().getName())) {
                assertNotNull( prefix + " ATTRIBUTE_USER_FIRST_NAME.getValues() != null", pair.getValues() );
                assertFalse( prefix + " ATTRIBUTE_USER_FIRST_NAME.getValues().size() == 1", pair.getValues().size() != 1);
                assertEquals( prefix + "ATTRIBUTE_USER_FIRST_NAME.value==user.Name", expected.getName().toLowerCase(), pair.getValues().get(0).toLowerCase() );
            } else if (KnownSamlAttributes.ATTRIBUTE_USER_LAST_NAME.equalsIgnoreCase(pair.getAttrDefinition().getName())) {
                assertNotNull( prefix + " ATTRIBUTE_USER_LAST_NAME.getValues() != null", pair.getValues() );
                assertFalse( prefix + " ATTRIBUTE_USER_LAST_NAME.getValues().size() == 1", pair.getValues().size() != 1);
                assertEquals( prefix + "ATTRIBUTE_USER_LAST_NAME.value==user.Name", expected.getDomain().toLowerCase(), pair.getValues().get(0).toLowerCase() );
            } else if (KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME.equalsIgnoreCase(pair.getAttrDefinition().getName())) {
                assertNotNull( prefix + " ATTRIBUTE_USER_PRINCIPAL_NAME.getValues() != null", pair.getValues() );
                assertFalse( prefix + " ATTRIBUTE_USER_PRINCIPAL_NAME.getValues().size() == 1", pair.getValues().size() != 1);
                assertEquals( prefix + "ATTRIBUTE_USER_PRINCIPAL_NAME.value==user.Name", expected.getUPN().toLowerCase(), pair.getValues().get(0).toLowerCase() );
            } else if (KnownSamlAttributes.ATTRIBUTE_USER_SUBJECT_TYPE.equalsIgnoreCase(pair.getAttrDefinition().getName())) {
                assertNotNull( prefix + " ATTRIBUTE_USER_SUBJECT_TYPE.getValues() != null", pair.getValues() );
                assertFalse( prefix + " ATTRIBUTE_USER_SUBJECT_TYPE.getValues().size() == 1", pair.getValues().size() != 1);
                assertEquals( prefix + "ATTRIBUTE_USER_SUBJECT_TYPE.value==user.Name", "false", pair.getValues().get(0).toLowerCase() );
            } else if (KnownSamlAttributes.ATTRIBUTE_USER_GROUPS.equalsIgnoreCase(pair.getAttrDefinition().getName())) {
                assertNotNull( prefix + " ATTRIBUTE_USER_GROUPS.getValues() != null", pair.getValues() );
                Set<PrincipalId> actualGroups = getgroups(pair.getValues());
                assertGroupMembership(prefix, expected, actualGroups, !customBase, false);
            }
        }
    }

    private static Set<PrincipalId> getgroups(PrincipalGroupLookupInfo membership) {
        HashSet<PrincipalId> set = new HashSet<PrincipalId>();
        if( (membership!= null) && (membership.getGroups() != null) ){
            for(Group g: membership.getGroups()) {
                set.add(g.getId());
            }
        }
        return set;
    }

    private static Set<PrincipalId> getgroups(List<String> gs) throws IDMLoginException {
        HashSet<PrincipalId> set = new HashSet<PrincipalId>();
        if(gs!= null){
            for(String gn: gs) {
                String[] parts = ServerUtils.separateUserIDAndDomain(gn);
                PrincipalId p = new PrincipalId(parts[0], parts[1]);
                set.add(p);
            }
        }
        return set;
    }

    private static Set<PrincipalId> getgroups(Set<Group> groups) {
        HashSet<PrincipalId> set = new HashSet<PrincipalId>();
        if(groups!= null){
            for(Group g: groups) {
                set.add(g.getId());
            }
        }
        return set;
    }

    private static Set<PrincipalId> getusers(Set<PersonUser> users) {
        HashSet<PrincipalId> set = new HashSet<PrincipalId>();
        if(users!= null){
            for(PersonUser pu: users) {
                set.add(pu.getId());
            }
        }
        return set;
    }


    private static final String FIRST_NAME_FRIENDLY_NAME = "givenName";
    private static final String LAST_NAME_FRIENDLY_NAME = "surname";
    private static final String GROUPS_FRIENDLY_NAME = "Groups";
    private static final String SUBJECT_TYPE_FRIENDLY_NAME = "SubjectType";
    private static final String USER_PRINCIPAL_NAME_FRIENDLY_NAME = "userPrincipalName";

    private static Collection<Attribute> getAttributes()
    {
        Collection<Attribute> attrList = new ArrayList<Attribute>();

        Attribute attr = new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME);
        attr.setFriendlyName(GROUPS_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_GROUPS);
        attr.setFriendlyName(FIRST_NAME_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_LAST_NAME);
        attr.setFriendlyName(LAST_NAME_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_SUBJECT_TYPE);
        attr.setFriendlyName(SUBJECT_TYPE_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        attr = new Attribute(KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME);
        attr.setFriendlyName(USER_PRINCIPAL_NAME_FRIENDLY_NAME);
        attr.setNameFormat("urn:oasis:names:tc:SAML:2.0:attrname-format:uri");

        attrList.add(attr);

        return attrList;
    }

    private static boolean expectedToFailForScopedUser(TestParam param) {
        return (
            (param.isDomainUser()) &&
            (param.isOpsUser() == false) &&
            (param.isOpsScopedProvider())
        );
    }
    private static boolean expectedToFailForScopedGroup(TestParam param) {
        return (
            (param.isDomainGroup()) &&
            (param.isOpsGroup() == false) &&
            (param.isOpsScopedProvider())
        );
    }
}

