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
import java.util.List;
import java.util.Properties;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DuplicateProviderException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidProviderException;

public class AddProviderNegativeTest
{

    private static String DIFF_PREFIX = "diff-";
    private static Properties props = null;
    private static CasIdmClient client = null;

    @BeforeClass
    public static void init() throws Exception
    {
        props = IdmClientTestUtil.getProps();
        String hostname = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_HOSTNAME);
        Assert.assertNotNull(hostname);
        client = new CasIdmClient(hostname);

        String tenant1 = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        IdmClientTestUtil.ensureTenantExists(client, tenant1);
        IdmClientTestUtil.ensureADIdentityStoreExistForTenant(client, tenant1);
    }

    @AfterClass
    public static void tearDown() throws Exception
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        final String adProviderName =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);
        // cleanup
        client.deleteProvider(tenantName, adProviderName);
        IIdentityStoreData store = client.getProvider(tenantName, adProviderName);
        Assert.assertNull(store);

        IdmClientTestUtil.ensureTenantDoesNotExist(client, tenantName);
    }

    @Test
    public void testDuplicatedSystemStore() throws Exception
    {
        String systemTenantName =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String systemStoreName = systemTenantName;
        List<String> connStrs = Arrays.asList(
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI));

        IdentityStoreData duplicatedSystemStore =
                IdentityStoreData.CreateExternalIdentityStoreData(
                        systemStoreName, null,
                        IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                        AuthenticationType.PASSWORD, null, 0,
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME_DN),
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD),
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERBASE_DN),
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_GROUPBASE_DN), connStrs, null);

        try
        {
            client.addProvider(systemTenantName, duplicatedSystemStore);
        } catch (DuplicateProviderException dpe)
        {
            Assert.assertTrue(dpe.getMessage().contains(systemStoreName));
            return;
        }
        Assert.fail("Adding system store with duplicated name should not succeed");
    }

    @Test
    public void testDuplicatedADStoreByName() throws Exception
    {
        String tenant1 = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        String duplicatedADStoreName =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        List<String> connStrs = Arrays.asList(
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI));
        IdentityStoreData duplicatedADStoreByName =
                IdentityStoreData.CreateExternalIdentityStoreData(
                        duplicatedADStoreName, null,
                        IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
                        AuthenticationType.PASSWORD, null, 0,
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME_DN),
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD),
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERBASE_DN),
                        props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_GROUPBASE_DN), connStrs, null);

        try
        {
            client.addProvider(tenant1, duplicatedADStoreByName);
        } catch (DuplicateProviderException dpe)
        {
            Assert.assertTrue(dpe.getMessage().contains(duplicatedADStoreName));
            return;
        }
        Assert.fail("Adding AD store with duplicated name should not succeed");
    }

    @Test
    public void testDuplicatedADStoreByAlias() throws Exception
    {
        String tenant1 = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        String duplicatedADStoreAlias =
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);
        List<String> connStrs = Arrays.asList(
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI));
        IdentityStoreData duplicatedADStoreByAlias =
                IdentityStoreData
                        .CreateExternalIdentityStoreData(
                                DIFF_PREFIX
                                        + props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME),
                                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS),
                                IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
                                AuthenticationType.PASSWORD, null, 0,
                                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME_DN),
                                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD),
                                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERBASE_DN),
                                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_GROUPBASE_DN),
                                connStrs, null);

        try
        {
            client.addProvider(tenant1, duplicatedADStoreByAlias);
        } catch (DuplicateProviderException dpe)
        {
            Assert.assertTrue(dpe.getMessage().contains(duplicatedADStoreAlias));
            return;
        }
        Assert.fail("Adding AD store with duplicated alias should not succeed");
    }

    @Ignore("AD Data Store Config validation logic is now dependent on Net API's dcInfo, REVISIT")
    @Test
    public void testADStoreMismatchingDomain() throws Exception
    {
        final String MISMATCHING_CONN_STR = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_KDC);

        String tenant1 = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        String providerName = DIFF_PREFIX
                + props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        String providerAlias = DIFF_PREFIX
                + props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);

        IdmClientTestUtil.ensureProviderDoesNOTExist(client, tenant1, providerName);
        IdentityStoreData invalidADStore =
            IdentityStoreData.CreateExternalIdentityStoreData(
                providerName,
                providerAlias,
                IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,
                AuthenticationType.USE_KERBEROS, null, 0,
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME),
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD),
                null, null, Arrays.asList(MISMATCHING_CONN_STR), null);

        try
        {
            client.addProvider(tenant1, invalidADStore);
        } catch (InvalidProviderException ipe)
        {
            Assert.assertTrue(ipe.getMessage().contains(MISMATCHING_CONN_STR));
            return;
        }
        Assert.fail("Adding AD store with mismatching domain should not succeed");
    }

    @Ignore("AD Data Store Config validation logic is now dependent on Net API's dcInfo, REVISIT")
    @Test
    public void testADStoreMismatchingIPAddr() throws Exception
    {
        final String IP_ADDR_URI = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI);

        String tenant1 = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);
        String providerName  = DIFF_PREFIX+props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        String providerAlias = DIFF_PREFIX+props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);

        IdmClientTestUtil.ensureProviderDoesNOTExist(client, tenant1, providerName);
        IdentityStoreData skipCheckingADStore =
            IdentityStoreData.CreateExternalIdentityStoreData(
                providerName,
                providerAlias,
                IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY,
                AuthenticationType.USE_KERBEROS, null, 0,
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME),
                props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD),
                null, null, Arrays.asList(IP_ADDR_URI), null);

        try {
            client.addProvider(tenant1, skipCheckingADStore);
        } catch (InvalidProviderException ipe) {
            Assert.assertTrue(ipe.getMessage().contains(providerName));
        }
        finally {
            IdmClientTestUtil.ensureProviderDoesNOTExist(client, tenant1, providerName);
        }
    }
}
