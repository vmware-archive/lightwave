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
package com.vmware.identity.rest.core.server.authorization;

/**
 * A mapper that ties each role to a specific domain.
 *
 * <ul>
 *  <li>{@link Role.ADMINISTRATOR} => tenantDomain</li>
 *  <li>{@link Role.CONFIGURATION_USER} => systemTenantDomain</li>
 *  <li>{@link Role.TRUSTED_USER} => tenantDomain</li>
 *  <li>{@link Role.REGULAR_USER} => tenantDomain</li>
 *  <li>{@link Role.GUEST_USER} => tenantDomain</li>
 * </ul>
 */
public class RoleMapper {

    private String tenantDomain;
    private String systemTenantDomain;

    public RoleMapper(String tenantDomain, String systemTenantDomain) {
        this.tenantDomain = tenantDomain;
        this.systemTenantDomain = systemTenantDomain;
    }

    public String getTenantDomain() {
        return tenantDomain;
    }

    public String getSystemTenantDomain() {
        return systemTenantDomain;
    }

    public RoleGroup getRoleGroup(Role role) {
        switch (role) {
        case ADMINISTRATOR:
            return new RoleGroup(role, "Administrators", tenantDomain);
        case CONFIGURATION_USER:
            return new RoleGroup(role, "SystemConfiguration.Administrators", systemTenantDomain);
        case TRUSTED_USER:
            return new RoleGroup(role, "TrustedUsers", tenantDomain);
        case REGULAR_USER:
            return new RoleGroup(role, "Users", tenantDomain);
        case GUEST_USER:
            return new RoleGroup(role, "Everyone", tenantDomain);
        default:
            throw new IllegalArgumentException("Invalid Role: '" + role.toString() + "'");
        }
    }

}
