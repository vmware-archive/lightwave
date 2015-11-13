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

package com.vmware.identity.openidconnect.server;

import java.util.List;

/**
 * @author Yehia Zayour
 */
public class UserInformation {
    private final List<String> groupMembership;
    private final String adminServerRole;
    private final String givenName;
    private final String familyName;

    public UserInformation(
            List<String> groupMembership,
            String adminServerRole,
            String givenName,
            String familyName) {
        this.groupMembership = groupMembership;
        this.adminServerRole = adminServerRole;
        this.givenName = givenName;
        this.familyName = familyName;
    }

    public List<String> getGroupMembership() {
        return groupMembership;
    }

    public String getAdminServerRole() {
        return adminServerRole;
    }

    public String getGivenName() {
        return givenName;
    }

    public String getFamilyName() {
        return familyName;
    }
}
