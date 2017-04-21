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
 * An enumeration describing the hierarchy of roles that are accepted
 * by authorization.
 * <p>
 * <b>Note:</b> Roles are compared in a hierarchical fashion. That is, any role
 * defined is considered to be a superset of the role before it.
 * <p>
 * <i>e.g.</i> {@link Role#ADMINISTRATOR} > {@link Role#CONFIGURATION_USER}
 */
public enum Role {
    /**
     * Anonymous user.
     */
    GUEST_USER("GuestUser"),

    /**
     * A regular user with no special permissions.
     */
    REGULAR_USER("RegularUser"),

    /**
     * A trusted user with more permissions than a regular user.
     */
    TRUSTED_USER("TrustedUser"),

    /**
     * A configuration user with slightly elevated permissions.
     *
     * <p>This role should <b>only</b> be valid on the system tenant.
     */
    CONFIGURATION_USER("ConfigurationUser"),

    /**
     * An administrative user with full permissions.
     */
    ADMINISTRATOR("Administrator");

    private String roleName = null;

    /**
     * Find the Role object by its associated name.
     *
     * @param name the name to search the enum for
     * @return a role if the role name exists, otherwise <tt>null</tt>
     */
    public static Role findByRoleName(String name) {
        if (name != null) {
            for (Role r : values()) {
                if (r.getRoleName().equals(name)) {
                    return r;
                }
            }
        }
        return null;
    }

    private Role(String roleName) {
        this.roleName = roleName;
    }

    /**
     * Retrieve the name associated with the role.
     *
     * @return the name associated with this role
     */
    public String getRoleName() {
        return roleName;
    }

    /**
     * Checks to see if this role is a superset or equivalent
     * to another role.
     *
     * @param other a role to compare against
     * @return <tt>true</tt> if this role is a superset or equivalent to
     * the other role.
     */
    public boolean is(Role other) {
        if (other == null) {
            return false;
        }
        return compareTo(other) >= 0;
    }

}
