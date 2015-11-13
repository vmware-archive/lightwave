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

import java.util.Collection;
import java.util.List;
import java.util.Properties;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.InvalidProviderException;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.Tenant;

@Ignore("bugzilla#1173915")
public class AdTopologyTest
{
    private static Properties props = null;
    private static CasIdmClient idmClient = null;

    private static final String ATTR_NAME_IS_SOLUTION =
            "http://vmware.com/schemas/attr-names/2011/07/isSolution";
    private static final String ATTR_NAME_UPN =
            "http://schemas.xmlsoap.org/claims/UPN";
    private static final String ATTRIBUTE_GROUPS =
            "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";

    @BeforeClass
    public static void init() throws Exception
    {
        props = IdmClientTestUtil.getProps();
        String hostname = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_HOSTNAME);
        Assert.assertNotNull(hostname);
        idmClient = new CasIdmClient(hostname);
        testAddSsolabsAsProviderWithADAPI();
    }

    @AfterClass
    public static void tearDown() throws Exception
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        final String adProviderName =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);
        // cleanup
        idmClient.deleteProvider(tenantName, adProviderName);
        IIdentityStoreData store = idmClient.getProvider(tenantName, adProviderName);
        Assert.assertNull(store);

        IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
    }

    @Test
    public void testFindUserSsolabs() throws Exception, IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        // uc1s2@child1.ssolabs2.com
        String trustedPrincipal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER2_UPN);
        Assert.assertNotNull(trustedPrincipal);

        int idx = trustedPrincipal.indexOf('@');
        String adUserName = trustedPrincipal.substring(0, idx);
        String adDomainName = trustedPrincipal.substring(idx+1);

        PersonUser user =
                idmClient.findPersonUser(tenantName, new PrincipalId(adUserName, adDomainName));
        Assert.assertNotNull(user);

        // us3@ssolabs3.com
        trustedPrincipal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER3_UPN);
        Assert.assertNotNull(trustedPrincipal);

        idx = trustedPrincipal.indexOf('@');
        adUserName = trustedPrincipal.substring(0, idx);
        adDomainName = trustedPrincipal.substring(idx+1);

        user = idmClient.findPersonUser(tenantName, new PrincipalId(adUserName, adDomainName));
        Assert.assertNotNull(user);

        // uc1s3@child1.ssolabs3.com
        trustedPrincipal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER4_UPN);
        Assert.assertNotNull(trustedPrincipal);

        idx = trustedPrincipal.indexOf('@');
        adUserName = trustedPrincipal.substring(0, idx);
        adDomainName = trustedPrincipal.substring(idx+1);

        user = idmClient.findPersonUser(tenantName, new PrincipalId(adUserName, adDomainName));
        Assert.assertNotNull(user);

        // us0@ssolabs.com
        trustedPrincipal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER5_UPN);
        Assert.assertNotNull(trustedPrincipal);

        idx = trustedPrincipal.indexOf('@');
        adUserName = trustedPrincipal.substring(0, idx);
        adDomainName = trustedPrincipal.substring(idx+1);

        boolean foundUser = true;
        try
        {
           user = idmClient.findPersonUser(tenantName, new PrincipalId(adUserName, adDomainName));
        }
        catch(InvalidPrincipalException ex)
        {
            foundUser = false;
        } finally
        {
            Assert.assertEquals(true, foundUser);
        }
    }

    @Test
    public void testFindGroupSsolabs() throws Exception, IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        // dl-c1s2-wfu@child1.ssolabs2.com
        String trustedGroup =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP1_UPN);
        Assert.assertNotNull(trustedGroup);

        int idx = trustedGroup.indexOf('@');
        String adGroupName = trustedGroup.substring(0, idx);
        String adDomainName = trustedGroup.substring(idx+1);

        Group group =
                idmClient.findGroup(tenantName, new PrincipalId(adGroupName, adDomainName));
        Assert.assertNotNull(group);

        // dl-s2-wfu@ssolabs2.com
        trustedGroup =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP2_UPN);
        Assert.assertNotNull(trustedGroup);

        idx = trustedGroup.indexOf('@');
        adGroupName = trustedGroup.substring(0, idx);
        adDomainName = trustedGroup.substring(idx+1);

        group =
                idmClient.findGroup(tenantName, new PrincipalId(adGroupName, adDomainName));
        Assert.assertNotNull(group);

        // dl-c1s3-wfu@child1.ssolabs3.com
        trustedGroup =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP3_UPN);
        Assert.assertNotNull(trustedGroup);

        idx = trustedGroup.indexOf('@');
        adGroupName = trustedGroup.substring(0, idx);
        adDomainName = trustedGroup.substring(idx+1);

        group =
                idmClient.findGroup(tenantName, new PrincipalId(adGroupName, adDomainName));
        Assert.assertNotNull(group);

        // dl-s1-wfu@ssolabs.com
        trustedGroup =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP4_UPN);
        Assert.assertNotNull(trustedGroup);

        idx = trustedGroup.indexOf('@');
        adGroupName = trustedGroup.substring(0, idx);
        adDomainName = trustedGroup.substring(idx+1);

        boolean foundGroup = true;
        try
        {
           group = idmClient.findGroup(tenantName, new PrincipalId(adGroupName, adDomainName));
        }
        catch(InvalidPrincipalException ex)
        {
            foundGroup = false;
        } finally
        {
            Assert.assertEquals(false, foundGroup);
        }
    }

    @Test
    public void testBatchFindUserAndGroupsSsolabs() throws Exception
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        final String adTrustedDomainName =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_TRUSTED_DOMAIN_NAME1);
        Assert.assertNotNull(adTrustedDomainName);

        SearchCriteria allStsUsers =
                new SearchCriteria("Security Token Service", adTrustedDomainName);
        Set<PersonUser> allUsers =
                idmClient.findPersonUsers(tenantName, allStsUsers, -1);
        Assert.assertEquals(1, allUsers.size());

        SearchCriteria alldomainLocalGroups =
                new SearchCriteria("dl-", adTrustedDomainName);
        Set<Group> allGroups =
                idmClient.findGroups(tenantName, alldomainLocalGroups, -1);
        Assert.assertEquals(1, allGroups.size());

        // This exercise 'AD.findDisabledPersonUsers' with paged_search (it grabs all possibly users first)
        idmClient.findDisabledPersonUsers(tenantName, "sts", -1);

        // This exercise 'AD.findLockedPersonUsers' with paged_search (it grabs all possibly users first)
        idmClient.findLockedUsers(tenantName, "sts", -1);

    }

    @Test
    public void testADProviderGetNestedParentGroupsSsolabs() throws Exception,
    IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        String trustedPrincipal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER2_UPN);
        Assert.assertNotNull(trustedPrincipal);

        int idx = trustedPrincipal.indexOf('@');
        String adUserName = trustedPrincipal.substring(0, idx);
        String adDomainName = trustedPrincipal.substring(idx+1);

        Set<Group> nestedGroups =
                idmClient.findNestedParentGroups(tenantName, new PrincipalId(adUserName, adDomainName));
        Assert.assertNotNull("nestedGroups must not be null", nestedGroups);
        Assert.assertFalse("nestedGroups must not be empty",
                nestedGroups.isEmpty());

        final String candidateGroupName = "-wfu";
        boolean foundCandidateGroup = false;
        for (Group g : nestedGroups)
        {
            if (g.getName().toLowerCase().contains(candidateGroupName.toLowerCase()))
            {
                foundCandidateGroup = true;
                break;
            }
        }
        Assert.assertTrue(String.format("nestedGroups must include '%s'",
                candidateGroupName), foundCandidateGroup);
    }

    @Test
    public void testAuthenticateSsolabs() throws Exception, IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        // Authenticate user that is used to do ldap bind
        String principal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_BIND_UPN);
        Assert.assertNotNull(principal);
        String password =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_BIND_PASSWORD);
        Assert.assertNotNull(password);
        PrincipalId id =
                idmClient.authenticate(tenantName, principal, password);
        Assert.assertNotNull(id);

        // Authenticate uses in trusted domain requires sso server is joined to AD IDS
        /*principal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER1_UPN);
        Assert.assertNotNull(principal);
        password =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER1_UPN_PASSWORD);
        Assert.assertNotNull(password);
        id = idmClient.authenticate(tenantName, principal, password);
        Assert.assertNotNull(id);

        // uc1s2@child1.ssolabs2.com
        String trustedPrincipal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER2_UPN);
        Assert.assertNotNull(trustedPrincipal);
        String trustedPrincipalPwd =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER2_UPN_PASSWORD);
        Assert.assertNotNull(trustedPrincipalPwd);
        PrincipalId trusted_Id =
                idmClient.authenticate(tenantName, trustedPrincipal, trustedPrincipalPwd);
        Assert.assertNotNull(trusted_Id);

        // us3@ssolabs3.com
        String trustedPrincipal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER3_UPN);
        Assert.assertNotNull(trustedPrincipal);
        String trustedPrincipalPwd =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER3_UPN_PASSWORD);
        Assert.assertNotNull(trustedPrincipalPwd);
        PrincipalId trusted_Id =
                idmClient.authenticate(tenantName, trustedPrincipal, trustedPrincipalPwd);
        Assert.assertNotNull(trusted_Id);*/
    }

    /* sso server is joined to child1.ssolabs2.com
     * lookup use 'us0@ssolabs.com', should not be able to 'findUser', nor 'getAttributes'
     * after authenticate, should be able to 'getAttributes' using PacInfo
     */
    @Ignore("This test requires sso server to join to the AD IDP registered")
    @Test
    public void testGetAttributeValues() throws Exception, IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        final String adUpn =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER1_UPN);
        Assert.assertNotNull(adUpn);

        String principalPwd =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER1_UPN_PASSWORD);
        Assert.assertNotNull(principalPwd);

        int idx = adUpn.indexOf('@');
        String adUserName = adUpn.substring(0, idx);
        String adDomainName = adUpn.substring(idx+1);

        PrincipalId principal = new PrincipalId(adUserName, adDomainName);

        Collection<Attribute> supportedAttributes =
                idmClient.getAttributeDefinitions(tenantName);

        try
        {
            PersonUser user = idmClient.findPersonUser(tenantName, principal);
            Assert.fail("Should not reach here...");
        }
        catch(Exception e)
        {
            // ignore
            try
            {
                Collection<AttributeValuePair> attributes =
                         idmClient.getAttributeValues(tenantName, principal,
                                                      supportedAttributes);
                Assert.fail("Should not reach here...");

            }
            catch(Exception e_inner)
            {
                // This is expected
            }
        }

        PrincipalId trusted_Id =
                idmClient.authenticate(tenantName, adUpn, principalPwd);
        Assert.assertNotNull(trusted_Id);

        /* getAttributeValues makes use of 'pacInfo' to retrieve attributes */
        Collection<AttributeValuePair> attributes =
                idmClient.getAttributeValues(tenantName, principal,
                        supportedAttributes);
        Assert.assertNotNull(attributes);

        AttributeValuePair isSolutionAttr =
                IdmClientTestUtil.findAttribute(attributes, ATTR_NAME_IS_SOLUTION);

        Assert.assertNotNull(isSolutionAttr);

        List<String> values = isSolutionAttr.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() == 1);
        Assert.assertEquals("false", values.get(0));

        AttributeValuePair upnAttr = IdmClientTestUtil.findAttribute(attributes, ATTR_NAME_UPN);

        Assert.assertNotNull(upnAttr);

        values = upnAttr.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() == 1);
        Assert.assertTrue(values.get(0).length() > 0);

        AttributeValuePair groups = IdmClientTestUtil.findAttribute(attributes, ATTRIBUTE_GROUPS);

        Assert.assertNotNull(groups);

        values = groups.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() > 0);

        Pattern pattern = Pattern.compile("^([^\\\\])+\\\\([^\\\\])+$");

        for (String value : values)
        {
            Matcher matcher = pattern.matcher(value);

            Assert.assertTrue(matcher.matches());
        }
    }

    @Ignore("This test has presumption that local host is not joined or joined to domain other than ssolabs.eng.vmware.com")
    @Test
    public void testAddSSOLABS() throws Exception
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        // Add Active Directory IDP with null alias
        final String adProviderName =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);

        IIdentityStoreData store =
                idmClient.getProvider(tenantName, adProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, adProviderName);
            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNull(store);
        }

        IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, false);
        // check before create
        if (store == null)
        {
            try
            {
                idmClient.addProvider(tenantName, adStore);
            }
            catch(InvalidProviderException ex)
            {
                // ignore
            }
            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNull(store);
        }
    }

    @Test
    public void testFindNestedParentGroups() throws Exception
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        String principal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_USER1_UPN);
        Assert.assertNotNull(principal);

        int idx = principal.indexOf('@');
        String adUserName = principal.substring(0, idx);
        String adDomainName = principal.substring(idx+1);

        Set<Group> nestedGroups =
                idmClient.findNestedParentGroups(tenantName, new PrincipalId(adUserName, adDomainName));
        Assert.assertNotNull("nestedGroups must not be null", nestedGroups);
        Assert.assertFalse("nestedGroups must not be empty",
                nestedGroups.isEmpty());

        principal =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_USER2_UPN);
        Assert.assertNotNull(principal);

        idx = principal.indexOf('@');
        adUserName = principal.substring(0, idx);
        adDomainName = principal.substring(idx+1);

        nestedGroups =
                idmClient.findNestedParentGroups(tenantName, new PrincipalId(adUserName, adDomainName));
        Assert.assertNotNull("nestedGroups must not be null", nestedGroups);
        Assert.assertFalse("nestedGroups must not be empty",
                nestedGroups.isEmpty());
    }

    private static void testAddSsolabsAsProviderWithADAPI() throws Exception
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        // Add Active Directory IDP with null alias
        final String adProviderName =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);

        IIdentityStoreData store =
                idmClient.getProvider(tenantName, adProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, adProviderName);
            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNull(store);
        }

        IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore_SSOLABS_S3_WithADAPI(idmClient);
        // check before create
        if (store == null)
        {
            idmClient.addProvider(tenantName, adStore);
            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNotNull(store);
        }
    }
}
