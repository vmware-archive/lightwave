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

package com.vmware.identity.openidconnect.client;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.PasswordDetailsDTO;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.identity.openidconnect.common.ErrorCode;
import com.vmware.identity.rest.idm.data.OperatorsAccessPolicyDTO;
import com.vmware.identity.rest.idm.data.PrincipalIdentifiersDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.attributes.TenantConfigType;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import org.junit.Assert;

public class OperatorsAccessIT extends OIDCClientITBase {

    @Test
    public void test_SystemDisabledTenantDisabled() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, OIDCClientException, TokenValidationException, SSLConnectionException, OIDCServerException {
        testExpected(false, false);
    }

    @Test
    public void test_SystemEnabledTenantDisabled() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, OIDCClientException, TokenValidationException, SSLConnectionException, OIDCServerException {
        testExpected(true, false);
    }

    @Test
    public void test_SystemDisabledTenantEnabled() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, OIDCClientException, TokenValidationException, SSLConnectionException, OIDCServerException {
        testExpected(false, true);
    }

    @Test
    public void test_SystemEnabledTenantEnabled() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, OIDCClientException, TokenValidationException, SSLConnectionException, OIDCServerException {
        testExpected(true, true);
    }

    static GroupDTO opsGroup;
    static UserDTO opsUser;
    static PrincipalIdentifiersDTO principals;
    static String opsGroupUpn;
    static String opsUserUpn;
    static String opsGroupNB;

    @BeforeClass
    public static void setUp() throws Exception {
        setUp("config.properties");

        // in system tenant: create an ops group
        // create an op user; add to ops group
        PasswordDetailsDTO pwdDetails = new PasswordDetailsDTO.Builder()
            .withPassword(passwordGrantForSystemTenant.getPassword()).build();
        opsGroup = new GroupDTO.Builder().withName("OperatorsGroup").withDomain(systemTenant)
            .withDetails(new GroupDetailsDTO("description")).build();
        opsUser = new UserDTO.Builder().withName("operatorUser").withDomain(systemTenant)
            .withPasswordDetails(pwdDetails).build();
        opsGroupUpn = (opsGroup.getName() + "@" + opsGroup.getDomain()).toLowerCase();
        opsUserUpn = (opsUser.getName() + "@" + opsUser.getDomain()).toLowerCase();
        opsGroupNB = (opsGroup.getDomain() + "\\" + opsGroup.getName()).toLowerCase();
        principals = new PrincipalIdentifiersDTO.Builder()
            .withIds(
                Arrays.<String>asList( opsUserUpn, opsGroupUpn )).build();
        vmdirClientForSystemTenant.group().create(systemTenant, opsGroup);
        vmdirClientForSystemTenant.user().create(systemTenant, opsUser);
        vmdirClientForSystemTenant.group().addMembers(
            systemTenant, opsGroup.getName(), opsGroup.getDomain(),
            Arrays.<String>asList(opsUser.getName() + "@" + opsUser.getDomain()),
            MemberType.USER);
    }

    @AfterClass
    public static void cleanup() throws Exception {
        try{
            vmdirClientForSystemTenant.user().delete(systemTenant, opsUser.getName(), opsUser.getDomain());
        } catch(Exception ex) {
            System.out.printf("Error on cleanup ops user: %s\n", ex.toString());
        }
        try{
            vmdirClientForSystemTenant.group().delete(systemTenant, opsGroup.getName(), opsGroup.getDomain());
        } catch(Exception ex) {
            System.out.printf("Error on cleanup ops group: %s\n", ex.toString());
        }

        tearDown();
    }

    private void testExpected(boolean systemEnabled, boolean tenantEnabled)
        throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, OIDCClientException, TokenValidationException, SSLConnectionException, OIDCServerException {

        OperatorsAccessPolicyDTO systemTenantCurrentConfig = null;
        OperatorsAccessPolicyDTO tenantCurrentConfig = null;

        try{
            systemTenantCurrentConfig = ensureExpectedConfig(getExpectedConfig(systemEnabled), idmClientForSystemTenant, systemTenant);
            tenantCurrentConfig = ensureExpectedConfig(getExpectedConfig(tenantEnabled), idmClientForRegularTenant, regularTenant);

            // expect failures
            if ( systemEnabled == false || tenantEnabled == false ) {
                try{
                    regClientWithoutAuthnForRegularTenant.acquireTokensByPassword(
                        opsUser.getName() + "@" + opsUser.getDomain(), opsUser.getPasswordDetails().getPassword(),
                        withoutRefreshSpec);
                    fail(
                        String.format(
                            "Ops access is not fully enabled, expect login to fail. [SystemTenantAccess='%s', Tenant='%s']",
                            systemEnabled ? "TRUE" : "FALSE",
                            tenantEnabled ? "TRUE" : "FALSE"));
                } catch(OIDCServerException ex){
                    // expected un-auth
                    Assert.assertEquals("login for ops user should produce invalid grant.", ErrorCode.INVALID_GRANT, ex.getErrorObject().getErrorCode());
                }

                try{
                    idmClientForRegularTenant.tenant().findPrincipalIds(regularTenant, principals);
                    fail(
                        String.format(
                            "Ops access is not fully enabled, expect findPrincipalIds to fail. [SystemTenantAccess='%s', Tenant='%s']",
                            systemEnabled ? "TRUE" : "FALSE",
                            tenantEnabled ? "TRUE" : "FALSE"));
                } catch(WebApplicationException ex) {
                    // expected
                }
            } else {
                // expect success
                OIDCTokens tokens = regClientWithoutAuthnForRegularTenant.acquireTokensByPassword(
                    opsUser.getName() + "@" + opsUser.getDomain(), opsUser.getPasswordDetails().getPassword(),
                    withoutRefreshSpec);
                assertEquals("token subject", (opsUser.getName() + "@" + opsUser.getDomain()).toLowerCase(), tokens.getIDToken().getSubject().getValue().toLowerCase());
                assertContainsOpsGroup( tokens.getIDToken().getGroups() );

                // find principals returns the 2
                PrincipalIdentifiersDTO found = idmClientForRegularTenant.tenant().findPrincipalIds(regularTenant, principals);
                assertNotNull("should be able to find operator user and group", found );
                assertNotNull("should be able to find operator user and group (ids)", found.getIds() );
                assertEquals("should be able to find both operator user and group", 2, found.getIds().size() );
            }

        } finally {
            restoreConfig(systemTenantCurrentConfig, idmClientForSystemTenant, systemTenant);
            restoreConfig(tenantCurrentConfig, idmClientForRegularTenant, regularTenant);
        }
    }

    private static void assertContainsOpsGroup(Collection<String> groups) {
        boolean found = false;
        if (groups!= null ) {
            for(String g:groups) {
                if (opsGroupNB.equalsIgnoreCase(g)) {
                    found = true;
                    break;
                }
            }
        }
        // if not found, log groups
        if (found == false) {
            System.out.printf("Token groups are(lacking ops group):\n");
            if (groups!= null ) {
                for(String g:groups) {
                    System.out.println(g);
                }
            }
        }
        assertTrue("token should comntain ops group", found);
    }

    private static OperatorsAccessPolicyDTO getExpectedConfig(boolean enabled) {
        OperatorsAccessPolicyDTO.Builder builder = new OperatorsAccessPolicyDTO.Builder();
        builder.withEnabled(enabled);
        return builder.build();
    }

    private static OperatorsAccessPolicyDTO ensureExpectedConfig(
        OperatorsAccessPolicyDTO dto, IdmClient client, String tenant)
            throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {

        TenantConfigurationDTO origConfig = client.tenant().getConfig(tenant,
            Arrays.<TenantConfigType>asList(TenantConfigType.OPERATORS_ACCESS));
        OperatorsAccessPolicyDTO originalOpConfig = origConfig.getOperatorsAccessPolicy();
        if (originalOpConfig == null) {
            originalOpConfig = new OperatorsAccessPolicyDTO.Builder().withEnabled(false).build();
        }

        if ( (originalOpConfig.getEnabled() != dto.getEnabled())
             ||
             (origConfig.getOperatorsAccessPolicy().getUserBaseDn() != dto.getUserBaseDn())
             ||
             (origConfig.getOperatorsAccessPolicy().getGroupBaseDn() != dto.getGroupBaseDn())
           )
        {
            TenantConfigurationDTO config = TenantConfigurationDTO.builder()
                .withOperatorsAccessPolicy(dto).build();
            client.tenant().updateConfig(tenant, config);
            return originalOpConfig;
        }
        else
        {
            // matches expected
            return null;
        }
    }

    private static void restoreConfig(
        OperatorsAccessPolicyDTO original, IdmClient client, String tenant){
        // if original is null, it means we didn't alter it
        if (original != null) {
            try{
                TenantConfigurationDTO config = TenantConfigurationDTO.builder()
                    .withOperatorsAccessPolicy(original).build();
                client.tenant().updateConfig(tenant, config);
            } catch (Exception ex) {
                // we will not fail; we want to restore as many as possible.
                System.out.printf("Error on restore original operatos policy: %s\n", ex.toString());
            }
        }
    }
}