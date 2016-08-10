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
package com.vmware.directory.rest.server.test.integration.datagenerator;

import com.vmware.directory.rest.common.data.PasswordDetailsDTO;
import com.vmware.directory.rest.common.data.PrincipalDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;

/**
 * Data provider for user resource integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class UserDataGenerator {

    private static final String ALIAS = "_alias";
    private static final String DESCRIPTION = "Description of test person user ";
    private static final String AT = "@";
    private static final String FIRST_NAME = "'s first name";
    private static final String LAST_NAME = "'s last name";

    public static PersonUser generateTestUser(String username, String tenant, boolean disabled, boolean locked) {
        PrincipalId userId = new PrincipalId(username, tenant);
        PrincipalId alias = new PrincipalId(username + ALIAS, tenant);
        PersonDetail userDetails = createPersonDetails(username, tenant);
        return new PersonUser(userId, alias, null, userDetails, disabled, locked);
    }


    private static PersonDetail createPersonDetails(String username, String tenant) {
        PersonDetail.Builder builder = new PersonDetail.Builder();
        return builder.description("Description for test person user " + username)
               .emailAddress(username + AT + tenant)
               .firstName(username + FIRST_NAME)
               .lastName(username + LAST_NAME)
               .userPrincipalName(username + AT + tenant)
               .build();
     }

    public static UserDTO generateTestUserDTO(String username, String tenant) {
            PrincipalDTO alias = new PrincipalDTO(username + ALIAS, tenant);
            PasswordDetailsDTO passwordDetails = new PasswordDetailsDTO("testUser!23", null, null);
            return UserDTO.builder()
                          .withName(username)
                          .withDomain(tenant)
                          .withAlias(alias)
                          .withDetails(createPersonDetailDTO(username, tenant))
                          .withLocked(false)
                          .withDisabled(false)
                          .withPasswordDetails(passwordDetails)
                          .build();

    }

    private static UserDetailsDTO createPersonDetailDTO(String username, String tenant) {
        return UserDetailsDTO.builder()
                              .withFirstName(FIRST_NAME)
                              .withLastName(LAST_NAME)
                              .withDescription(DESCRIPTION)
                              .withUPN(username + AT + tenant)
                              .withEmail(username + AT + tenant)
                              .build();
    }
}
