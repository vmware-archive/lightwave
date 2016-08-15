package com.vmware.directory.rest.client.test.integration.util;

import com.vmware.directory.rest.common.data.PasswordDetailsDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;

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

public class UserGenerator {

    public static final String FIRST_NAME = "Firstname";
    public static final String LAST_NAME = "Lastname";
    public static final String EMAIL = "Email@vmware.com";
    public static final String PASSWORD = "Password!23";

    public static UserDTO generateUser(String name, String domain, String description) {
        return new UserDTO.Builder()
            .withName(name)
            .withDomain(domain)
            .withDetails(generateUserDetails(description))
            .withPasswordDetails(generatePasswordDetails())
            .withLocked(false)
            .withDisabled(false)
            .build();
    }

    private static UserDetailsDTO generateUserDetails(String description) {
        return new UserDetailsDTO.Builder()
            .withFirstName(FIRST_NAME)
            .withLastName(LAST_NAME)
            .withDescription(description)
            .withEmail(EMAIL)
            .build();
    }

    private static PasswordDetailsDTO generatePasswordDetails() {
        return new PasswordDetailsDTO.Builder()
            .withPassword(PASSWORD)
            .build();
    }

}
