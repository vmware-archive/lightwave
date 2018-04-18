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
package com.vmware.identity.rest.core.server.test.authorization.token.request;

import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;

import org.junit.BeforeClass;
import org.junit.Test;

import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.RoleGroup;
import com.vmware.identity.rest.core.server.authorization.RoleMapper;
import com.vmware.identity.rest.core.server.authorization.exception.InsufficientRoleException;
import com.vmware.identity.rest.core.server.authorization.request.ResourceAccessRequest;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerToken;
import com.vmware.identity.rest.core.server.test.util.JWTBuilder;
import com.vmware.identity.rest.core.test.util.KeyPairUtil;

public class RoleCheckTest {

    protected static PublicKey publicKey;
    protected static PrivateKey privateKey;

    private static final String VSPHERE_DOMAIN = "vsphere.local";
    private static final String SYSTEM_DOMAIN = "system.local";

    private static final String SUBJECT_ADMINISTRATOR = "administrator@" + VSPHERE_DOMAIN;
    private static final String SUBJECT_CONFIGURATOR = "configuration_user@"+ SYSTEM_DOMAIN;

    @BeforeClass
    public static void setup() {
        KeyPair keypair = KeyPairUtil.getKeyPair();
        publicKey = keypair.getPublic();
        privateKey = keypair.getPrivate();
    }

    @Test
    public void testRoleField() throws Exception {
        RoleMapper mapper = getMapper(VSPHERE_DOMAIN, VSPHERE_DOMAIN);

        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject(SUBJECT_ADMINISTRATOR)
            .issuer("OAUTH")
            .audience(Config.RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .build();

        AccessToken token = new JWTBearerToken(jwt, Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD, Config.JWT_MUTITENANTED_FIELD);

        ResourceAccessRequest request = new ResourceAccessRequest(TokenStyle.HEADER, TokenType.BEARER, token, null, false, mapper);

        request.validateRole(Role.ADMINISTRATOR);
        request.validateRole(Role.CONFIGURATION_USER);
        request.validateRole(Role.REGULAR_USER);
    }

    public void testRoleField_WrongRole() throws Exception {
        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject(SUBJECT_CONFIGURATOR)
            .issuer("OAUTH")
            .audience(Config.RESOURCE_SERVER_AUDIENCE)
            .role(Role.CONFIGURATION_USER.getRoleName())
            .build();

        AccessToken token = new JWTBearerToken(jwt, Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD, Config.JWT_MUTITENANTED_FIELD);

        ResourceAccessRequest request = new ResourceAccessRequest(TokenStyle.HEADER, TokenType.BEARER, token, null, false, null);

        request.validateRole(Role.ADMINISTRATOR);
    }

    @Test
    public void testGroupsField() throws Exception {
        RoleMapper mapper = getMapper(VSPHERE_DOMAIN, VSPHERE_DOMAIN);

        List<String> groups = Arrays.asList( new String[] { mapper.getRoleGroup(Role.ADMINISTRATOR).getGroupNetbios() } );

        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject(SUBJECT_ADMINISTRATOR)
            .issuer("OAUTH")
            .audience(Config.RESOURCE_SERVER_AUDIENCE)
            .groups(groups)
            .build();

        AccessToken token = new JWTBearerToken(jwt, Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD, Config.JWT_MUTITENANTED_FIELD);

        ResourceAccessRequest request = new ResourceAccessRequest(TokenStyle.HEADER, TokenType.BEARER, token, null, false, mapper);

        request.validateRole(Role.ADMINISTRATOR);
        request.validateRole(Role.CONFIGURATION_USER);
        request.validateRole(Role.REGULAR_USER);
        request.validateRole(Role.GUEST_USER);
    }

    @Test(expected = InsufficientRoleException.class)
    public void testGroupsField_WrongRole() throws Exception {
        RoleMapper mapper = getMapper(VSPHERE_DOMAIN, VSPHERE_DOMAIN);

        List<String> groups = Arrays.asList( new String[] { mapper.getRoleGroup(Role.CONFIGURATION_USER).getGroupNetbios() } );

        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject(SUBJECT_ADMINISTRATOR)
            .issuer("OAUTH")
            .audience(Config.RESOURCE_SERVER_AUDIENCE)
            .groups(groups)
            .build();

        AccessToken token = new JWTBearerToken(jwt, Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD, Config.JWT_MUTITENANTED_FIELD);

        ResourceAccessRequest request = new ResourceAccessRequest(TokenStyle.HEADER, TokenType.BEARER, token, null, false, mapper);

        request.validateRole(Role.ADMINISTRATOR);
    }

    @Test
    public void testSystemConfigurationRole_DomainDiffers() throws Exception {
        RoleMapper mapper = getMapper(VSPHERE_DOMAIN, SYSTEM_DOMAIN);

        List<String> groups = Arrays.asList( new String[] { mapper.getRoleGroup(Role.CONFIGURATION_USER).getGroupNetbios() } );

        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject(SUBJECT_CONFIGURATOR)
            .issuer("OAUTH")
            .audience(Config.RESOURCE_SERVER_AUDIENCE)
            .groups(groups)
            .build();

        AccessToken token = new JWTBearerToken(jwt, Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD, Config.JWT_MUTITENANTED_FIELD);

        ResourceAccessRequest request = new ResourceAccessRequest(TokenStyle.HEADER, TokenType.BEARER, token, null, false, mapper);

        request.validateRole(Role.CONFIGURATION_USER);
    }

    @Test(expected = InsufficientRoleException.class)
    public void testSystemConfigurationRole_WrongDomain() throws Exception {
        RoleMapper mapper = getMapper(VSPHERE_DOMAIN, SYSTEM_DOMAIN);

        List<String> groups = Arrays.asList( new String[] { new RoleGroup(Role.CONFIGURATION_USER, Role.CONFIGURATION_USER.getRoleName(), VSPHERE_DOMAIN).getGroupNetbios() } );

        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject(SUBJECT_ADMINISTRATOR)
            .issuer("OAUTH")
            .audience(Config.RESOURCE_SERVER_AUDIENCE)
            .groups(groups)
            .build();

        AccessToken token = new JWTBearerToken(jwt, Config.JWT_TYPE_FIELD, Config.JWT_ROLE_FIELD, Config.JWT_GROUPS_FIELD, Config.JWT_MUTITENANTED_FIELD);

        ResourceAccessRequest request = new ResourceAccessRequest(TokenStyle.HEADER, TokenType.BEARER, token, null, false, mapper);

        request.validateRole(Role.CONFIGURATION_USER);
    }

    public static String buildHeader(String tokenType, String accessToken, String signature) {
        StringBuilder builder = new StringBuilder();
        if (tokenType != null) {
            builder.append(tokenType).append(" ");
        }

        if (accessToken != null) {
            builder.append(accessToken);
        }

        if (signature!= null) {
            builder.append(":").append(signature);
        }

        return builder.toString();
    }

    private static RoleMapper getMapper(String tenantDomain, String systemTenantDomain) {
        HashSet<String> trustedDomains = new HashSet<String>();
        trustedDomains.add(tenantDomain);
        trustedDomains.add(systemTenantDomain);

        RoleMapper mapper = new RoleMapper(tenantDomain, systemTenantDomain, trustedDomains);
        return mapper;
    }
}
