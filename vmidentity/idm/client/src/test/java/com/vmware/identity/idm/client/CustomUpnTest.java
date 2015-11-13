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

/**
 *
 * Copyright 2011 VMware, Inc.  All rights reserved.
 */
package com.vmware.identity.idm.client;

import java.util.Collection;
import java.util.Collections;
import java.util.Properties;
import java.util.UUID;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.Principal;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.Tenant;

public class CustomUpnTest
{
    private static final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERSAMEDOMAINNAME = "idm.tenant-1.ol-provider-1.upnuser1-account";
    private static final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERSAMEDOMAINUPNNAME = "idm.tenant-1.ol-provider-1.upnuser1-upn-name";
    private static final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERCUSTOMDOMAINNAME = "idm.tenant-1.ol-provider-1.upnuser2-account";
    private static final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERCUSTOMDOMAINUPN = "idm.tenant-1.ol-provider-1.upnuser2-upn-name";
    private static final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERCUSTOMDOMAIN = "idm.tenant-1.ol-provider-1.upnuser2-upn-domain";
    private static final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERPWD = "idm.tenant-1.ol-provider-1.upnuser1-pwd";

    private static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_UPNUSERSAMEDOMAINNAME = "idm.tenant-1.ad-provider-1.upnuser-account";
    private static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_UPNUSERSAMEDOMAINUPNNAME = "idm.tenant-1.ad-provider-1.upnuser-upn-name";
    private static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_UPNUSERPWD = "idm.tenant-1.ad-provider-1.upnuser-pwd";

    private static final String CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERSAMEDOMAINNAME = "idm.tenant-1.vmdir-provider-1.upnuser1-account";
    private static final String CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERSAMEDOMAINUPNNAME = "idm.tenant-1.vmdir-provider-1.upnuser1-upn-name";
    private static final String CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERCUSTOMDOMAINNAME = "idm.tenant-1.vmdir-provider-1.upnuser2-account";
    private static final String CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERCUSTOMDOMAINUPN = "idm.tenant-1.vmdir-provider-1.upnuser2-upn-name";
    private static final String CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERCUSTOMDOMAIN = "idm.tenant-1.vmdir-provider-1.upnuser2-upn-domain";
    private static final String CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERPWD = "idm.tenant-1.vmdir-provider-1.upnuser1-pwd";

    private static Properties _testProps;
    private static CasIdmClient _idmClient;
    private static Tenant _userTenant;

    private static Attribute upnAttribute;

    // once ad supports custom domain, add the test
    private static UserInfo adUser1;
    private static UserInfo vmdirUser1;
    private static UserInfo vmdirUser2;
    private static UserInfo olUser1;
    private static UserInfo olUser2;

    @BeforeClass
    public static void testSetup()
    {
        try
        {
            _testProps = IdmClientTestUtil.getProps();
            Assert.assertNotNull("Test properties shouold not be null.", _testProps);
            String hostname = _testProps.getProperty(IdmClientTestUtil.CFG_KEY_IDM_HOSTNAME);
            Assert.assertNotNull(hostname);

            _idmClient = new CasIdmClient(hostname);

            String userTenantName = UUID.randomUUID().toString();
            Assert.assertNotNull(userTenantName);

            _userTenant = IdmClientTestUtil.ensureTenantExists(_idmClient, userTenantName);

            IdmClientTestUtil.ensureADIdentityStoreExistForTenant(_idmClient, _userTenant.getName());
            IdmClientTestUtil.ensureOLIdentityStoreExistForTenant(_idmClient, _userTenant.getName());

            upnAttribute = new Attribute(com.vmware.identity.idm.KnownSamlAttributes.ATTRIBUTE_USER_PRINCIPAL_NAME);

            final String adDomainName = _testProps.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
            final String adAliasName = _testProps.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);
            final String adUpnUserSameDomain_Account = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_UPNUSERSAMEDOMAINNAME);
            final String adUpnUserSameDomain_UpnName = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_UPNUSERSAMEDOMAINUPNNAME);
            final String adUpnUserPwd = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_UPNUSERPWD);
            adUser1 = new UserInfo(adUpnUserSameDomain_Account, adDomainName, adUpnUserSameDomain_UpnName, adDomainName, adAliasName, adUpnUserPwd);

            final String vmdirUpnUserSameDomain_Account = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERSAMEDOMAINNAME);
            final String vmdirUpnUserSameDomain_UpnName = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERSAMEDOMAINUPNNAME);
            final String vmdirUpnUserPwd = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERPWD);
            final String vmdirDomainName = _userTenant.getName();
            vmdirUser1 = new UserInfo(vmdirUpnUserSameDomain_Account, vmdirDomainName, vmdirUpnUserSameDomain_UpnName, vmdirDomainName, null, vmdirUpnUserPwd);
            final String vmdirUpnUserCustomDomain_Account = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERCUSTOMDOMAINNAME);
            final String vmdirUpnUserCustomDomain_UpnName = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERCUSTOMDOMAINUPN);
            final String vmdirCustomDomainName = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_VMDIR_PROVIDER_UPNUSERCUSTOMDOMAIN);
            vmdirUser2 = new UserInfo(vmdirUpnUserCustomDomain_Account, vmdirDomainName, vmdirUpnUserCustomDomain_UpnName, vmdirCustomDomainName, null, vmdirUpnUserPwd);

            PersonDetail.Builder builder = new PersonDetail.Builder();
            builder.firstName(vmdirUser1.getAccountName());
            builder.userPrincipalName(vmdirUser1.getEUpn());
            _idmClient.addPersonUser(_userTenant.getName(), vmdirUser1.getAccountName(), builder.build(), vmdirUser1.getPassword().toCharArray());
            _idmClient.registerUpnSuffix(_userTenant.getName(), vmdirUser2.getDomainName(), vmdirUser2.getUpnDomain());
            builder = new PersonDetail.Builder();
            builder.firstName(vmdirUser2.getAccountName());
            builder.userPrincipalName(vmdirUser2.getEUpn());
            _idmClient.addPersonUser(_userTenant.getName(), vmdirUser2.getAccountName(), builder.build(), vmdirUser2.getPassword().toCharArray());

            final String olDomainName = _testProps.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
            final String olAliasName = _testProps.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS);
            final String olUpnUserSameDomain_Account = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERSAMEDOMAINNAME);
            final String olUpnUserSameDomain_UpnName = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERSAMEDOMAINUPNNAME);
            final String olUpnUserPwd = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERPWD);
            final String olUpnUserCustomDomain_Account = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERCUSTOMDOMAINNAME);
            final String olUpnUserCustomDomain_UpnName = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERCUSTOMDOMAINUPN);
            final String olCustomDomainName = _testProps.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_UPNUSERCUSTOMDOMAIN);
            olUser1 = new UserInfo(olUpnUserSameDomain_Account, olDomainName, olUpnUserSameDomain_UpnName, olDomainName, olAliasName, olUpnUserPwd);
            olUser2 = new UserInfo(olUpnUserCustomDomain_Account, olDomainName, olUpnUserCustomDomain_UpnName, olCustomDomainName, olAliasName, olUpnUserPwd);
        }
        catch(Exception ex)
        {
            Assert.fail(String.format("Test setup failed with: %s", ex.toString()));
        }

        Assert.assertNotNull("IdmClient should not be null.", _idmClient);
        Assert.assertNotNull("Test properties shouold not be null.", _testProps);
        Assert.assertNotNull("Tenant must be created.", _userTenant);
        Assert.assertNotNull("Upn attribute should not be null.", upnAttribute);
        Assert.assertNotNull("Ad user1 should be created.", adUser1);
        //Assert.assertNotNull("Ad user2 should be created.", adUser2);
        Assert.assertNotNull("Vmdir user1 should be created.", vmdirUser1);
        Assert.assertNotNull("Vmdir user2 should be created.", vmdirUser2);
        Assert.assertNotNull("Ol user1 should be created.", olUser1);
        Assert.assertNotNull("Ol user2 should be created.", olUser2);
    }

    @AfterClass
    public static void testTearDown() throws Exception
    {
        if(_idmClient != null)
        {
            _idmClient.deletePrincipal(_userTenant.getName(), vmdirUser1.getAccountName());
            _idmClient.deletePrincipal(_userTenant.getName(), vmdirUser2.getAccountName());
        }

        try{
            if(_idmClient != null)
            {
                IdmClientTestUtil.ensureTenantDoesNotExist(_idmClient, _userTenant.getName());
            }
        }
        catch(Exception ex)
        {}
        _idmClient = null;
        _testProps = null;
        _userTenant = null;
        upnAttribute = null;
        adUser1 = null;
        //adUser2 = null;
        vmdirUser1 = null;
        vmdirUser2 = null;
        olUser1 = null;
        olUser2 = null;
    }

    @Test
    public void testAdFindIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindIUpn("adUpnUserSameDomain_Account@adDomainName", adUser1);
    }

    @Test
    public void testAdFindEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindEUpn("adUpnUserSameDomain_UpnName@adDomainName", adUser1);
    }

    @Test
    public void testAdFindWithAliasSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindWithAlias("adAliasName\\adUpnUserSameDomain_Account", adUser1);
    }

    @Test
    public void testAdGetAttributesIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByIUpn("adUpnUserSameDomain_Account@adDomainName", adUser1, true);
    }

    @Test
    public void testAdGetAttributesEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByEUpn("adUpnUserSameDomain_UpnName@adDomainName", adUser1, true);
    }

    @Test
    public void testAdGetAttributesWithAliasSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesWithAlias("adUpnUserSameDomain_UpnName@adDomainName", adUser1, true);
    }

    @Test
    public void testAdAuthenticateIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByIUpn("adUpnUserSameDomain_Account@adDomainName", adUser1);
    }

    @Test
    public void testAdAuthenticateEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByEUpn("adUpnUserSameDomain_UpnName@adDomainName", adUser1);
    }

    @Test
    public void testAdAuthenticateWithAliasSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateWithAlias("adUpnUserSameDomain_UpnName@adDomainName", adUser1);
    }

    @Test
    public void testLotusFindIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindIUpn("vmdirUpnUserSameDomain_Account@vmdirDomainName", vmdirUser1);
    }

    @Test
    public void testLotusFindEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindEUpn("vmdirUpnUserSameDomain_UpnName@vmdirDomainName", vmdirUser1);
    }

    @Test
    public void testLotusAuthenticateIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByIUpn("vmdirUpnUserSameDomain_Account@vmdirDomainName", vmdirUser1);
    }

    @Test
    public void testLotusAuthenticateEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByEUpn("vmdirUpnUserSameDomain_UpnName@vmdirDomainName", vmdirUser1);
    }

    @Test
    public void testLotusGetAttributesIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByIUpn("vmdirUpnUserSameDomain_Account@vmdirDomainName", vmdirUser1, false);
    }

    @Test
    public void testLotusGetAttributesEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByEUpn("vmdirUpnUserSameDomain_UpnName@vmdirDomainName", vmdirUser1, false);
    }

    @Test
    public void testLotusFindIUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindIUpn("vmdirUpnUserSameDomain_Account@customVmdirDomainName", vmdirUser2);
    }

    @Test
    public void testLotusFindEUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindEUpn("vmdirUpnUserSameDomain_UpnName@customVmdirDomainName", vmdirUser2);
    }

    @Test
    public void testLotusAuthenticateIUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByIUpn("vmdirUpnUserSameDomain_Account@customVmdirDomainName", vmdirUser2);
    }

    @Test
    public void testLotusAuthenticateEUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByEUpn("vmdirUpnUserSameDomain_UpnName@customVmdirDomainName", vmdirUser2);
    }

    @Test
    public void testLotusGetAttributesIUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByIUpn("vmdirUpnUserSameDomain_Account@customVmdirDomainName", vmdirUser2, false);
    }

    @Test
    public void testLotusGetAttributesEUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByEUpn("vmdirUpnUserSameDomain_UpnName@customVmdirDomainName", vmdirUser2, false);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOLFindIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindIUpn("olUpnUserSameDomain_Account@olDomainName", olUser1);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlFindEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindEUpn("olUpnUserSameDomain_UpnName@olDomainName", olUser1);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlFindWithAliasSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindWithAlias("olAliasName\\olUpnUserSameDomain_Account", olUser1);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlAuthenticateIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByIUpn("olUpnUserSameDomain_Account@olDomainName", olUser1);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlAuthenticateEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByEUpn("olUpnUserSameDomain_UpnName@olDomainName", olUser1);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlGetAttributesWithAliasSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesWithAlias("olUpnUserSameDomain_UpnName@olDomainName", olUser1, true);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlGetAttributesIUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByIUpn("olUpnUserSameDomain_Account@olDomainName", olUser1, true);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlGetAttributesEUpnSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByEUpn("olUpnUserSameDomain_UpnName@volDomainName", olUser1, true);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlAuthenticateWithAliasSameDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateWithAlias("olUpnUserSameDomain_UpnName@olDomainName", olUser1);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlFindIUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindIUpn("olUpnUserSameDomain_Account@customolDomainName", olUser2);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlFindEUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindEUpn("olUpnUserSameDomain_UpnName@customOlDomainName", olUser2);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlFindWithAliasCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFindWithAlias("olAliasName\\olUpnUserSameDomain_Account", olUser2);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlAuthenticateIUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByIUpn("olUpnUserSameDomain_Account@customolDomainName", olUser2);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlAuthenticateEUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateByEUpn("olUpnUserSameDomain_UpnName@customOlDomainName", olUser2);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlAuthenticateWithAliasCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testAuthenticateWithAlias("olUpnUserSameDomain_UpnName@olDomainName", olUser2);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlGetAttributesIUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByIUpn("olUpnUserSameDomain_Account@customOlDomainName", olUser2, true);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlGetAttributesEUpnCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesByEUpn("olUpnUserSameDomain_UpnName@customolDomainName", olUser2, true);
    }

    @Ignore("userPrincipalName unsupported on ssolabs-openldap")
    @Test
    public void testOlGetAttributesWithAliasCustomDomain() throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testGetAttributesWithAlias("olUpnUserSameDomain_UpnName@olDomainName", olUser2, true);
    }

    private static void testFindIUpn(
            String fieldName, UserInfo userInfo
                ) throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFind( userInfo.getIUpn(), fieldName, userInfo );
    }

    private static void testFindEUpn(
            String fieldName, UserInfo userInfo
                ) throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        testFind( userInfo.getEUpn(), fieldName, userInfo );
    }

    private static void testFindWithAlias(
            String fieldName, UserInfo userInfo
                ) throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        if(userInfo.getAliasedId() != null)
        {
            testFind( userInfo.getAliasedId(), fieldName, userInfo );
        }
    }

    private static void testFind(
        String findId, String fieldName, UserInfo userInfo
            ) throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        Principal principal = _idmClient.findUser(_userTenant.getName(), findId);
        Assert.assertNotNull("should be able to find " + fieldName, principal);
        Assert.assertTrue("Principal should be PersonUser", principal instanceof PersonUser );
        Assert.assertEquals("account name should match", userInfo.getAccountName(), ((PersonUser)principal).getId().getName());
        Assert.assertEquals("domain name should match", userInfo.getDomainName(), ((PersonUser)principal).getId().getDomain());
        Assert.assertNotNull("Details should not be null", ((PersonUser)principal).getDetail());
        Assert.assertEquals("Upn name should match", userInfo.getEUpn().toLowerCase(), ((PersonUser)principal).getDetail().getUserPrincipalName().toLowerCase());
    }

    private static void testGetAttributesByIUpn(String fieldName, UserInfo userInfo, boolean principalShouldBeIUpn) throws Exception
    {
        testGetAttributes(fieldName, userInfo.getAccountName(), userInfo.getDomainName(), userInfo, principalShouldBeIUpn);
    }

    private static void testGetAttributesByEUpn(String fieldName, UserInfo userInfo, boolean principalShouldBeIUpn) throws Exception
    {
        testGetAttributes(fieldName, userInfo.getUpnName(), userInfo.getUpnDomain(), userInfo, principalShouldBeIUpn);
    }

    private static void testGetAttributesWithAlias(String fieldName, UserInfo userInfo, boolean principalShouldBeIUpn) throws Exception
    {
        if(userInfo.getAlias() != null)
        {
            testGetAttributes(fieldName, userInfo.getAccountName(), userInfo.getAlias(), userInfo, principalShouldBeIUpn);
        }
    }

    private static void testGetAttributes(String fieldName, String principalIdName,
        String principalIdDomain, UserInfo userInfo, boolean principalShouldBeIUpn) throws Exception
    {
        Collection<Attribute> attributes = Collections.<Attribute>singletonList(upnAttribute);
        Collection<AttributeValuePair> attributeValues = _idmClient.getAttributeValues(
            _userTenant.getName(),
            new PrincipalId(principalIdName, principalIdDomain),
            attributes
        );
        Assert.assertNotNull("should be able to retrieve upnAttriubute by " + fieldName, attributeValues);
        Assert.assertTrue("should get 1 upnAttribute by " + fieldName, attributeValues.size() == 1 );
        AttributeValuePair value = attributeValues.iterator().next();
        Assert.assertNotNull("should get non null AttributeValuePair for upnAttribute by " + fieldName, value );
        Assert.assertNotNull("should get non-null value for upn attribute by " + fieldName, value.getValues() );
        Assert.assertTrue("should get 1 value for upn attribute by " + fieldName, value.getValues().size() == 1 );
        Assert.assertEquals("Upn must match by " + fieldName,
            ((principalShouldBeIUpn) ? userInfo.getIUpn().toLowerCase() : userInfo.getEUpn().toLowerCase()),
            value.getValues().get(0).toLowerCase() );
    }

    private static void testAuthenticateByIUpn(String fieldName, UserInfo userInfo ) throws Exception
    {
        testAuthenticate(fieldName, userInfo.getIUpn(), userInfo);
    }
    private static void testAuthenticateByEUpn(String fieldName, UserInfo userInfo) throws Exception
    {
        testAuthenticate(fieldName, userInfo.getEUpn(), userInfo);
    }
    private static void testAuthenticateWithAlias(String fieldName, UserInfo userInfo) throws Exception
    {
        if(userInfo.getAliasedId() != null)
        {
            testAuthenticate(fieldName, userInfo.getAliasedId(), userInfo);
        }
    }

    private static void testAuthenticate(String fieldName, String authUserName, UserInfo userInfo) throws Exception
    {
        PrincipalId principal = _idmClient.authenticate(_userTenant.getName(), authUserName, userInfo.getPassword());
        Assert.assertNotNull("should be able to authenticate by " + fieldName, principal);
    }

    private static class UserInfo
    {
        private final String _accountName;
        private final String _domainName;
        private final String _upnName;
        private final String _upnDomain;
        private final String _alias;
        private final String _pwd;

        public UserInfo(String accountName, String domainName, String upnName, String upnDomain, String alias, String pwd)
        {
            Assert.assertNotNull("accountName", accountName);
            Assert.assertNotNull("domainName", domainName);
            Assert.assertNotNull("upnName", upnName);
            Assert.assertNotNull("upnDomain", upnDomain);
            Assert.assertNotNull("pwd", pwd);
            this._accountName = accountName;
            this._domainName = domainName;
            this._upnName = upnName;
            this._upnDomain = upnDomain;
            this._alias = alias;
            this._pwd = pwd;
        }

        public String getAccountName()
        {
            return this._accountName;
        }
        public String getDomainName()
        {
            return this._domainName;
        }
        public String getUpnName()
        {
            return this._upnName;
        }
        public String getUpnDomain()
        {
            return this._upnDomain;
        }
        public String getAlias()
        {
            return this._alias;
        }
        public String getPassword()
        {
            return this._pwd;
        }

        public String getIUpn()
        {
            return this.getAccountName() + "@" + this.getDomainName();
        }

        public String getEUpn()
        {
            return this.getUpnName() + "@" + this.getUpnDomain();
        }

        public String getAliasedId()
        {
            if (this._alias == null)
            {
                return null;
            }
            else
            {
                return this.getAlias() + "\\" + this.getAccountName();
            }
        }
    }
}
