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

import java.util.Collections;
import java.util.List;
import java.util.Set;

/**
 * @author Yehia Zayour
 */
public class UserInfo {
    private final List<String> groupMembership; // idm returns groupMembership as a list, do not convert to set since that is expensive O(n)
    private final Set<String> groupMembershipFiltered;
    private final String adminServerRole;
    private final String givenName;
    private final String familyName;

    public UserInfo(
            List<String> groupMembership,
            Set<String> groupMembershipFiltered,
            String adminServerRole,
            String givenName,
            String familyName) {
        this.groupMembership = (groupMembership == null) ? null : Collections.unmodifiableList(groupMembership);
        this.groupMembershipFiltered = (groupMembershipFiltered == null) ? null : Collections.unmodifiableSet(groupMembershipFiltered);
        this.adminServerRole = adminServerRole;
        this.givenName = givenName;
        this.familyName = familyName;
    }

    public List<String> getGroupMembership() {
        return this.groupMembership;
    }

    public Set<String> getGroupMembershipFiltered() {
        return this.groupMembershipFiltered;
    }

    public String getAdminServerRole() {
        return this.adminServerRole;
    }

    public String getGivenName() {
        return this.givenName;
    }

    public String getFamilyName() {
        return this.familyName;
    }
}