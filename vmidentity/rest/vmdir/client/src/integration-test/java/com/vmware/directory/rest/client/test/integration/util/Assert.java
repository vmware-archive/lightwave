package com.vmware.directory.rest.client.test.integration.util;
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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import java.util.List;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.common.data.UserDTO;

public class Assert {

    public static void assertContainsGroup(GroupDTO expected, List<GroupDTO> actual) {
        for (GroupDTO g : actual) {
            if (expected.getName().equals(g.getName()) && expected.getDomain().equals(g.getDomain())) {
                assertGroupsEqual(expected, g);
                return;
            }
        }

        fail("Unable to find expected Group");
    }


    public static void assertContainsUser(UserDTO expected, List<UserDTO> actual) {
        for (UserDTO u : actual) {
            if (expected.getName().equals(u.getName()) && expected.getDomain().equals(u.getDomain())) {
                assertUsersEqual(expected, u);
                return;
            }
        }

        fail("Unable to find expected User");
    }


    public static void assertGroupsEqual(GroupDTO expected, GroupDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDetails().getDescription(), actual.getDetails().getDescription());
    }

    public static void assertSolutionUsersEqual(SolutionUserDTO expected, SolutionUserDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDescription(), actual.getDescription());
        assertEquals(expected.getCertificate().getEncoded(), actual.getCertificate().getEncoded());
    }

    public static void assertUsersEqual(UserDTO expected, UserDTO actual) {
        assertEquals(expected.getName(), actual.getName());
        assertEquals(expected.getDomain(), actual.getDomain());
        assertEquals(expected.getDetails().getDescription(), actual.getDetails().getDescription());
        assertEquals(expected.getDetails().getFirstName(), actual.getDetails().getFirstName());
        assertEquals(expected.getDetails().getLastName(), actual.getDetails().getLastName());
        assertEquals(expected.getDetails().getEmail(), actual.getDetails().getEmail());
    }

}
