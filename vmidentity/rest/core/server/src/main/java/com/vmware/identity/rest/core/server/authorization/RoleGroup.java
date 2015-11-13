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

public class RoleGroup {

    private Role role;
    private String group;
    private String domain;

    public RoleGroup(Role role, String group, String domain) {
        this.role = role;
        this.group = group;
        this.domain = domain;
    }

    public Role getRole() {
        return role;
    }

    public String getGroup() {
        return group;
    }

    public String getDomain() {
        return domain;
    }

    /**
     * Creates a NETBIOS formatted group name
     *
     * @return a NETBIOS formatted group name
     */
    public String getGroupNetbios() {
        return domain + "\\" + group;
    }

    /**
     * Creates a UPN formatted group name
     *
     * @return a UPN formatted group name
     */
    public String getGroupUpn() {
        return group + "@" + domain;
    }

}
