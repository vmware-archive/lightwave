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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import junit.framework.Assert;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.Tenant;

class IdmClientTestUtil
{
    static final String CFG_KEY_IDM_SYSTEM_TENANT_NAME =
            "idm.server.service-provider-name";
    static final String CFG_KEY_IDM_TENANT_1_NAME = "idm.tenant-1.name";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME =
            "idm.tenant-1.ad-provider-1.domain-name";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS =
            "idm.tenant-1.ad-provider-1.alias";
    static final String CFG_KEY_IDM_HOSTNAME = "idm.server.hostname";

    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_UPN =
            "idm.tenant-1.ad-provider-1.bind-upn";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_DN =
            "idm.tenant-1.ad-provider-1.bind-dn";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_KDC =
            "idm.tenant-1.ad-provider-1.kdc-1";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_PASSWORD =
            "idm.tenant-1.ad-provider-1.password";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_BASE_DN =
            "idm.tenant-1.ad-provider-1.user-search-base-dn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_GROUP_BASE_DN =
            "idm.tenant-1.ad-provider-1.group-search-base-dn";
    static final String CFG_KEY_IDM_ROUGE_URL       = "idm.AddProviderNegativeCases.rogueURL";
    static final String CFG_KEY_IDM_ROUGE_ADMIN_DN  = "idm.AddProviderNegativeCases.rogueAdminDN";
    static final String CFG_KEY_IDM_ROUGE_ADMIN_PWD = "idm.AddProviderNegativeCases.rogueAdminPwd";

    static final String CFG_KEY_IDM_PROBECONNECTIVITY_TENANTNAME       = "idm.probeConnectivity.tenantName";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI      = "idm.probeConnectivity.providerUri";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME_DN      = "idm.probeConnectivity.userDNName";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME         = "idm.probeConnectivity.userName";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_SPN              = "idm.probeConnectivity.spn";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD         = "idm.probeConnectivity.password";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_BADPASSWORD      = "idm.probeConnectivity.badPassword";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_USERBASE_DN      = "idm.probeConnectivity.userBaseDn";
    static final String CFG_KEY_IDM_PROBECONNECTIVITY_GROUPBASE_DN      = "idm.probeConnectivity.groupBaseDn";

    static final String SAMLATTR_GIVEN_NAME =
            "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
    static final String SAMLATTR_SUR_NAME =
            "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
    static final String SAMLATTR_GROUP_IDENTITY =
            "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";
    static final String SAMLATTR_SUBJECT_TYPE =
            "http://vmware.com/schemas/attr-names/2011/07/isSolution";
    static final String SAMLATTR_EMAIL_ADDRESS =
            "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress";
    static final String SAMLATTR_UPN = "http://schemas.xmlsoap.org/claims/UPN";

    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_DOMAIN_NAME =
            "idm.tenant-1.ad-provider-2.domain-name";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_BIND_UPN =
            "idm.tenant-1.ad-provider-2.bind-upn";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_SPN =
            "idm.tenant-1.ad-provider-2.spn";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_USE_MACHINE_ACCT =
            "idm.tenant-1.ad-provider-2.use-machine-account";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_BIND_PASSWORD =
            "idm.tenant-1.ad-provider-2.password";

    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER1_UPN =
            "idm.tenant-1.ad-provider-2.user1-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER1_UPN_PASSWORD =
            "idm.tenant-1.ad-provider-2.user1-upn-password";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER2_UPN =
            "idm.tenant-1.ad-provider-2.user2-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER2_UPN_PASSWORD =
            "idm.tenant-1.ad-provider-2.user2-upn-password";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER3_UPN =
            "idm.tenant-1.ad-provider-2.user3-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER3_UPN_PASSWORD =
            "idm.tenant-1.ad-provider-2.user3-upn-password";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER4_UPN =
            "idm.tenant-1.ad-provider-2.user4-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER4_UPN_PASSWORD =
            "idm.tenant-1.ad-provider-2.user4-upn-password";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER5_UPN =
            "idm.tenant-1.ad-provider-2.user5-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_USER5_UPN_PASSWORD =
            "idm.tenant-1.ad-provider-2.user5-upn-password";

    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP1_UPN =
            "idm.tenant-1.ad-provider-2.group1-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP2_UPN =
            "idm.tenant-1.ad-provider-2.group2-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP3_UPN =
            "idm.tenant-1.ad-provider-2.group3-upn";
    static final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_2_GROUP4_UPN =
            "idm.tenant-1.ad-provider-2.group4-upn";

    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_TRUSTED_DOMAIN_NAME1 =
            "idm.tenant-1.ad-provider-2.trusted-domain-name1";

    final static String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME =
            "idm.tenant-1.ol-provider-1.domain-name";
    final static String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS =
            "idm.tenant-1.ol-provider-1.alias";
    final static String CFG_KEY_IDM_TENANT_1_OL_HOST_NAME =
            "idm.tenant-1.ol-provider-1.host-name";
    final static String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_BIND_DN =
            "idm.tenant-1.ol-provider-1.bind-dn";
    final static String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_PASSWORD =
            "idm.tenant-1.ol-provider-1.password";
    final static String CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_BASE_DN =
            "idm.tenant-1.ol-provider-1.user-search-base-dn";
    final static String CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_BASE_DN =
            "idm.tenant-1.ol-provider-1.group-search-base-dn";
    final static String CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_FSPTEST_ID =
            "idm.tenant-1.ol-provider-1.user-1-id";
    final static String CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_FSPTEST1_ID =
            "idm.tenant-1.ol-provider-1.user-2-id";
    final static String CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_FSPTEST1_ID =
            "idm.tenant-1.ol-provider-1.subgroup-1-id";
    final static String CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_FSPTEST2_ID =
            "idm.tenant-1.ol-provider-1.subgroup-2-id";

    final static String CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_FSPTEST_ID =
            "idm.tenant-1.ad-provider-1.user-3-id";

    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_3_DOMAIN_NAME =
            "idm.tenant-1.ad-provider-3.domain-name";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_3_BIND_UPN =
            "idm.tenant-1.ad-provider-3.bind-upn";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_3_SPN =
            "idm.tenant-1.ad-provider-3.spn";
    static final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_3_BIND_PASSWORD =
            "idm.tenant-1.ad-provider-3.password";

    final static String CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_USER1_UPN =
            "idm.tenant-1.ad-provider-3.user1-upn";

    final static String CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_USER2_UPN =
            "idm.tenant-1.ad-provider-3.user2-upn";

    final static String CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_ADMIN =
            "idm.tenant-1.ad-provider-3.dc-admin";

    final static String CFG_KEY_IDM_TENANT_1_ADPROVIDER_3_ADMIN_PASSWORD =
            "idm.tenant-1.ad-provider-3.dc-admin-password";

     // Test admin user credentials
    private static final String DEFAULT_TENANT_ADMIN_NAME     = "Administrator";
    private static final String DEFAULT_TENANT_ADMIN_PASSWORD = "defaultPwd#1";

    private static Properties _props = null;

    static Tenant ensureTenantExists(CasIdmClient idmClient, String tenantName)
            throws Exception, IDMException
    {
        Tenant tenant = null;

        try
        {
            tenant = idmClient.getTenant(tenantName);
        } catch (NoSuchTenantException ex)
        {
            Tenant tenantToCreate = new Tenant(tenantName);
            idmClient.addTenant(tenantToCreate, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());
            tenant = idmClient.getTenant(tenantName);
            Assert.assertNotNull(tenant);
            Assert.assertEquals(tenantName, tenant.getName());
        }

        return tenant;
    }

    static void ensureTenantDoesNotExist(CasIdmClient idmClient, String tenantName)
           throws Exception, IDMException
    {
        Tenant tenant = null;

        try
        {
            tenant = idmClient.getTenant(tenantName);
        } catch (NoSuchTenantException ex)
        {
            // do nothing continue
        }

        if (tenant != null)
        {
            idmClient.deleteTenant(tenantName);
            try
            {
                tenant = null;
                tenant = idmClient.getTenant(tenantName);
            } catch (NoSuchTenantException ex)
            {
                // do nothing continue
            }
        }
        Assert.assertNull(tenant);
    }

    static void ensureProviderDoesNOTExist(CasIdmClient client, String tenantName, String providerName) throws Exception
    {
        if (null != client.getProvider(tenantName, providerName))
        {
            client.deleteProvider(tenantName, providerName);
        }
    }

    static void ensureADIdentityStoreExistForTenant(CasIdmClient client, String tenantName)
            throws Exception
    {
        Properties props = IdmClientTestUtil.getProps();

        String adProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        IIdentityStoreData store =
                client.getProvider(tenantName, adProviderName);
        if (store != null)
        {
            return;
        } else
        {
            IdentityStoreData adStore = prepareADIdentityStore(client, true);
            client.addProvider(tenantName, adStore);
        }
    }

    static IdentityStoreData prepareADIdentityStore(CasIdmClient client, boolean bWithAlias)
            throws Exception, IDMException
    {
        Properties props = IdmClientTestUtil.getProps();

        // Add Active Directory IDP
        final String adProviderName =
            props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);

        Assert.assertNotNull(adProviderName);

        final String alias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);
        Assert.assertNotNull(alias);

        final String kdc = props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_KDC);
        Assert.assertNotNull(kdc);

        final List<String> kdcList = new ArrayList<String>();
        kdcList.add(kdc);

        final String adUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_DN);
        Assert.assertNotNull(adUserName);

        final String adPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_PASSWORD);

        Assert.assertNotNull(adPwd);

        final String userSearchBaseDn =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_BASE_DN);

        final String groupSearchBaseDn =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_GROUP_BASE_DN);

        final Map<String, String> attrMap = new HashMap<String, String>();

        attrMap.put(SAMLATTR_GIVEN_NAME, "givenName");
        attrMap.put(SAMLATTR_SUR_NAME, "sn");
        attrMap.put(SAMLATTR_GROUP_IDENTITY, "memberof");
        attrMap.put(SAMLATTR_SUBJECT_TYPE, "subjectType");
        attrMap.put(SAMLATTR_EMAIL_ADDRESS, "mail");
        attrMap.put(SAMLATTR_UPN, "userPrincipalName");

        IdentityStoreData adStore =
                IdentityStoreData.CreateExternalIdentityStoreData(
                        adProviderName, bWithAlias ? alias : null,
                        //bugzilla#1173915 - for now switching to ad over ldap,
                        //needs to be looked at in light of AD provider changes
                        //IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,
                        IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
                        AuthenticationType.PASSWORD, null, 0, adUserName,
                        adPwd, userSearchBaseDn, groupSearchBaseDn, kdcList,
                        attrMap);

        return adStore;
    }

    static void ensureOLIdentityStoreExistForTenant(CasIdmClient client, String tenantName)
            throws Exception
    {
        Properties props = IdmClientTestUtil.getProps();

        String olProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        IIdentityStoreData store =
                client.getProvider(tenantName, olProviderName);
        if (store != null)
        {
            return;
        } else
        {
            IIdentityStoreData olStore = prepareOLIdentityStore(client, true);
            client.addProvider(tenantName, olStore);
        }
    }

    static IIdentityStoreData prepareOLIdentityStore(CasIdmClient client, boolean bWithAlias)
            throws Exception, IDMException
    {
        Properties props = IdmClientTestUtil.getProps();

        final String olProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olProviderName);

        final String olProviderAlias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS);
        Assert.assertNotNull(olProviderAlias);

        final String olHost =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_HOST_NAME);
        Assert.assertNotNull(olHost);

        final String olUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_BIND_DN);
        Assert.assertNotNull(olUserName);
        final String olPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_PASSWORD);
        Assert.assertNotNull(olPwd);

        final ArrayList<String> olldapHosts = new ArrayList<String>();
        olldapHosts.add(olHost);

        final String userOlSearchBaseDn =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_BASE_DN);

        final String groupOlSearchBaseDn =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_BASE_DN);

        final Map<String, String> attrMap = new HashMap<String, String>();
        attrMap.put(SAMLATTR_GIVEN_NAME, "givenName");
        attrMap.put(SAMLATTR_SUR_NAME, "sn");
        attrMap.put(SAMLATTR_GROUP_IDENTITY, "memberof");
        attrMap.put(SAMLATTR_SUBJECT_TYPE, "subjectType");
        attrMap.put(SAMLATTR_EMAIL_ADDRESS, "mail");
        attrMap.put(SAMLATTR_UPN, "userPrincipalName");

        IdentityStoreData olStore =
                IdentityStoreData.CreateExternalIdentityStoreData(
                        olProviderName, bWithAlias ? olProviderAlias : null,
                                IdentityStoreType.IDENTITY_STORE_TYPE_LDAP,
                                AuthenticationType.PASSWORD, null, 0, olUserName,
                                olPwd, userOlSearchBaseDn, groupOlSearchBaseDn,
                                olldapHosts, attrMap);
        return olStore;
    }

    static IdentityStoreData prepareADIdentityStore_SSOLABS_S3_WithADAPI(CasIdmClient client)
            throws Exception, IDMException
    {
        Properties props = IdmClientTestUtil.getProps();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = ensureTenantExists(client, tenantName);

        Assert.assertNotNull(tenant);

        // Add Active Directory IDP
        final String adProviderName =

        props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);

        final String adUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_BIND_UPN);
        Assert.assertNotNull(adUserName);

        final String adSPN =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_SPN);
        Assert.assertNotNull(adSPN);

        final String adUseMachineAccount =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_USE_MACHINE_ACCT);
        boolean useMachineAcct = adUseMachineAccount != null ? Boolean.getBoolean(adUseMachineAccount) : false;

        final String adPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_2_BIND_PASSWORD);

        Assert.assertNotNull(adPwd);


        final Map<String, String> attrMap = new HashMap<String, String>();

        attrMap.put(SAMLATTR_GIVEN_NAME, "givenName");
        attrMap.put(SAMLATTR_SUR_NAME, "sn");
        attrMap.put(SAMLATTR_GROUP_IDENTITY, "memberof");
        attrMap.put(SAMLATTR_SUBJECT_TYPE, "subjectType");
        attrMap.put(SAMLATTR_EMAIL_ADDRESS, "mail");
        attrMap.put(SAMLATTR_UPN, "userPrincipalName");

        IdentityStoreData adStore =
                IdentityStoreData.createActiveDirectoryIdentityStoreData(
                        adProviderName,
                        adUserName,
                        useMachineAcct,
                        adSPN,
                        adPwd,
                        attrMap,
                        null,
                        null);

        return adStore;
    }

    static AttributeValuePair findAttribute(
            Collection<AttributeValuePair> attributes, String attrName)
    {
        AttributeValuePair result = null;

        for (AttributeValuePair candidate : attributes)
        {
            Attribute attr = candidate.getAttrDefinition();

            if (attr.getName().equals(attrName))
            {
                result = candidate;
                break;
            }
        }

        return result;
    }

    static Properties getProps() throws IOException
    {
        if (_props == null)
        {
            _props = new Properties();
            _props.load(IdmClientTestUtil.class.getResourceAsStream("/config.properties"));
        }
        return _props;
    }
}
