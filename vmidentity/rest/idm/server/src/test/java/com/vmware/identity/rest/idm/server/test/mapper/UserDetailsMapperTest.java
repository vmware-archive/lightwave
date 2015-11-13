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
package com.vmware.identity.rest.idm.server.test.mapper;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import org.junit.Test;

import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.UserDetailsDTO;
import com.vmware.identity.rest.idm.server.mapper.UserDetailsMapper;

/**
 *
 * Unit tests for UserDetailsMapper
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class UserDetailsMapperTest {

    // Test constants
    private static final String DESC = "Test User Description";
    private static final String TEST_EMAIL = "testUser@test.local";
    private static final String TEST_FIRST_NAME = "UserfirstName";
    private static final String TEST_LAST_NAME = "UserLastName";
    private static final String TEST_UPN = "testUser";

    @Test
    public void testGetUserDetails() throws DTOMapperException{
        //Test data
        UserDetailsDTO userDetailsDTO = UserDetailsDTO.builder()
                                                               .withDescription(DESC)
                                                               .withFirstName(TEST_FIRST_NAME)
                                                               .withLastName(TEST_LAST_NAME)
                                                               .withEmail(TEST_EMAIL)
                                                               .withUPN(TEST_UPN)
                                                               .build();

        //Invocation
        PersonDetail actualData = UserDetailsMapper.getPersonDetail(userDetailsDTO);

        //Assertions
        assertEquals(DESC, actualData.getDescription());
        assertEquals(TEST_FIRST_NAME, actualData.getFirstName());
        assertEquals(TEST_LAST_NAME, actualData.getLastName());
        assertEquals(TEST_UPN, actualData.getUserPrincipalName());
        assertEquals(TEST_EMAIL, actualData.getEmailAddress());
    }

    @Test
    public void testGetPersonDetailDTO() throws DTOMapperException{
        //Test data
        PersonDetail personDetail = new PersonDetail.Builder()
                                                    .description(DESC)
                                                    .emailAddress(TEST_EMAIL)
                                                    .firstName(TEST_FIRST_NAME)
                                                    .lastName(TEST_LAST_NAME)
                                                    .userPrincipalName(TEST_UPN)
                                                    .build();

        //Invocation
        UserDetailsDTO actualData = UserDetailsMapper.getUserDetailsDTO(personDetail);

        //Assertions
        assertEquals(DESC, actualData.getDescription());
        assertEquals(TEST_FIRST_NAME, actualData.getFirstName());
        assertEquals(TEST_LAST_NAME, actualData.getLastName());
        assertEquals(TEST_UPN, actualData.getUPN());
        assertEquals(TEST_EMAIL, actualData.getEmail());
    }

    @Test(expected=DTOMapperException.class)
    public void testGetPersonDetailOnInvalidInput() {
        UserDetailsMapper.getPersonDetail(null);
    }

    @Test
    public void testGetPersonDetailDTO_OnNullMembers() {
        //Test data
        PersonDetail personDetail = new PersonDetail.Builder()
                                                    .firstName(TEST_FIRST_NAME)
                                                    .lastName(TEST_LAST_NAME)
                                                    .userPrincipalName(TEST_UPN)
                                                    .build();

        UserDetailsDTO UserDetailsDTO = UserDetailsMapper.getUserDetailsDTO(personDetail);
        assertNull(UserDetailsDTO.getDescription());
        assertNull(UserDetailsDTO.getEmail());
    }

}
