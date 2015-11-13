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
package com.vmware.identity.rest.idm.server.test.mapper;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.io.IOException;
import java.security.GeneralSecurityException;

import org.junit.Test;

import com.vmware.identity.idm.Tenant;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.server.mapper.TenantMapper;
import com.vmware.identity.rest.idm.server.test.util.TestDataGenerator;

/**
 *
 * Unit tests for TenantMapper
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class TenantMapperTest {

    private static final String TENANT_NAME = "test.local";
    private static final String TENANT_LONG_NAME = "test.local.long";
    private static final String TENANT_KEY = "foo";
    private static final String TENANT_GUID = "foo-bar-foo";
    private static final String TENANT_ISSUER = "testIssuer";

    private static final String ADMIN_UPN = "admin@test.local";
    private static final String ADMIN_PWD = "testThis!23";

    @Test
    public void testGetTenantDTO() {
        Tenant idmTenant = new Tenant(TENANT_NAME, TENANT_LONG_NAME, TENANT_KEY);
        idmTenant._issuerName = TENANT_ISSUER;
        idmTenant._guid = TENANT_GUID;
        TenantDTO tenantDTO = TenantMapper.getTenantDTO(idmTenant);
        assertEquals(TENANT_NAME, tenantDTO.getName());
        assertEquals(TENANT_LONG_NAME, tenantDTO.getLongName());
        assertEquals(TENANT_KEY, tenantDTO.getKey());
        assertEquals(TENANT_ISSUER, tenantDTO.getIssuer());
        assertEquals(TENANT_GUID, tenantDTO.getGuid());
    }

    @Test
    public void testGetTenant() throws IOException, GeneralSecurityException {
        TenantDTO tenantDTO = new TenantDTO(TENANT_NAME, TENANT_LONG_NAME, TENANT_KEY, TENANT_GUID, TENANT_ISSUER, TestDataGenerator.getTestTenantCredentialsDTO(), ADMIN_UPN, ADMIN_PWD);
        Tenant tenant = TenantMapper.getTenant(tenantDTO);
        assertEquals(TENANT_NAME, tenant.getName());
        assertEquals(TENANT_LONG_NAME, tenant._longName);
        assertEquals(TENANT_KEY, tenant._tenantKey);
        assertNull(tenant._issuerName);
        assertNull(tenant._guid);
    }

    @Test(expected = DTOMapperException.class)
    public void testGetTenantOnInvalidTenantDTO() {
        TenantMapper.getTenant(null);
    }

}
