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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.data.UserDetailsDTO;

/**
 * Principal assertor to assert principal entities
 *
 * @author Balaji Boggaram Ramanarayan
 */
public class PrincipalAssertor {

    public static void assertUser(UserDTO expectedUser, UserDTO actualUser) {
        assertEquals(expectedUser.getDomain(), actualUser.getDomain());
        assertEquals(expectedUser.getName(), actualUser.getName());
        assertUserDetails(expectedUser.getDetails(), actualUser.getDetails());
    }

    public static void assertUserDetails(UserDetailsDTO expectedUserDetail, UserDetailsDTO actualUserDetail) {
        assertEquals(expectedUserDetail.getDescription(), actualUserDetail.getDescription());
        assertTrue(expectedUserDetail.getEmail().equalsIgnoreCase(actualUserDetail.getEmail()));
        assertEquals(expectedUserDetail.getFirstName(), actualUserDetail.getFirstName());
        assertEquals(expectedUserDetail.getLastName(), actualUserDetail.getLastName());
        assertTrue(String.format("Expected :'%s', Received :'%s'", expectedUserDetail.getUPN(), actualUserDetail.getUPN()),
                        expectedUserDetail.getUPN().equalsIgnoreCase(actualUserDetail.getUPN()));
    }

    public static void assertGroup(GroupDTO expectedGroup, GroupDTO actualGroup) {
        assertEquals(expectedGroup.getName(), actualGroup.getName());
        assertEquals(expectedGroup.getDomain(), actualGroup.getDomain());
        assertEquals(expectedGroup.getDetails().getDescription(), actualGroup.getDetails().getDescription());
        if (expectedGroup.getAlias() != null && actualGroup.getAlias() != null) {
            assertPrincipal(expectedGroup.getAlias(), actualGroup.getAlias());
        }
    }

    public static void assertPrincipal(PrincipalDTO expectedPrincipal, PrincipalDTO actualPrincipal) {
        assertEquals(expectedPrincipal.getDomain(), actualPrincipal.getDomain());
        assertEquals(expectedPrincipal.getName(), actualPrincipal.getName());
    }

}
