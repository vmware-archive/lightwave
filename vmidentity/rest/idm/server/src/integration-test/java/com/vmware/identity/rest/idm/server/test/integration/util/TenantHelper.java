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
package com.vmware.identity.rest.idm.server.test.integration.util;

import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * Tenant utility which helps calling IDM directly. This helper is mostly used in two phases while running tenant resource integration tests :
 * <li> Preparing test set-up - Before running integration tests </li>
 * <li> Cleaning up set-up - After running integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 */
public class TenantHelper {

    private CasIdmClient idmClient;

    // Test admin user credentials
    private static final String DEFAULT_TENANT_ADMIN_NAME     = "Administrator";
    private static final String DEFAULT_TENANT_ADMIN_PASSWORD = "defaultPwd#1";

    public TenantHelper(CasIdmClient idmClient) {
        this.idmClient = idmClient;
    }

    public void deleteTenant(String tenantName) throws Exception {
        idmClient.deleteTenant(tenantName);
    }

    public void createTenant(String tenantName, String tenantLongname, String tenantKey) throws Exception {
        Tenant t = new Tenant(tenantName, tenantLongname, tenantKey);
        idmClient.addTenant(t, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());
    }

    public Tenant getTenant(String tenantName) throws Exception {
        return idmClient.getTenant(tenantName);
    }
}
