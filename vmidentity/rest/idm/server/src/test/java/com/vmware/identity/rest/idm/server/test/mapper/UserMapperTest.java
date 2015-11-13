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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import org.junit.Test;

import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.data.UserDetailsDTO;
import com.vmware.identity.rest.idm.server.mapper.UserMapper;

/**
 *
 * Unit tests for UserResource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class UserMapperTest {

    // Test constants
    private static final String TEST_PRINCIPAL_NAME = "testUser";
    private static final String TEST_PRINCIPAL_TENANT = "test.local";
    private static final String TEST_ALIAS_NAME = "testUserAlias";
    private static final String TEST_ALIAS_DOMAIN = "testAlias.local";
    private static final String DESC = "Test User Description";
    private static final String TEST_EMAIL = "testUser@test.local";
    private static final String TEST_FIRST_NAME = "UserfirstName";
    private static final String TEST_LAST_NAME = "UserLastName";
    private static final String TEST_UPN = "testUser";
    private static final Long TEST_PASSWORD_LAST_SET = System.currentTimeMillis();
    private static final Long TEST_PASSWORD_LIFETIME = 100L;
    private static final boolean IS_DISABLED = false;;
    private static final boolean IS_LOCKED = false;

    @Test
    public void testPersonUserToPersonDTO() throws DTOMapperException {
        // Test data
        PrincipalId principalId = new PrincipalId(TEST_PRINCIPAL_NAME, TEST_PRINCIPAL_TENANT);
        PersonUser personUser = new PersonUser(principalId, getPersonDetail(), IS_DISABLED, IS_LOCKED);

        // Invocation
        UserDTO actualData = UserMapper.getUserDTO(personUser, true);

        // Assertions
        assertEquals(TEST_PRINCIPAL_TENANT, actualData.getDomain());
        assertEquals(TEST_PRINCIPAL_NAME, actualData.getName());
        assertNotNull(actualData.getDetails());
        assertEquals(TEST_LAST_NAME, actualData.getDetails().getLastName());
        assertEquals(TEST_FIRST_NAME, actualData.getDetails().getFirstName());
        assertEquals(TEST_UPN, actualData.getDetails().getUPN());
        assertEquals(TEST_EMAIL, actualData.getDetails().getEmail());
        assertEquals(TEST_PASSWORD_LAST_SET, actualData.getPasswordDetails().getLastSet());
        assertEquals(TEST_PASSWORD_LIFETIME, actualData.getPasswordDetails().getLifetime());
        assertNull(actualData.getAlias());
        assertFalse(actualData.isDisabled());
        assertFalse(actualData.isLocked());
    }

    @Test
    public void testPersonDTOtoPersonUser() throws DTOMapperException {
        // Test data
        UserDTO personDTO = getPersonDTO();

        //Invocation
        PersonUser actualData = UserMapper.getPersonUser(personDTO);

        //Assertions
        assertEquals(TEST_PRINCIPAL_TENANT, actualData.getId().getDomain());
        assertEquals(TEST_PRINCIPAL_NAME, actualData.getId().getName());
        assertNotNull(actualData.getAlias());
        assertEquals(TEST_ALIAS_NAME, actualData.getAlias().getName());
        assertEquals(TEST_ALIAS_DOMAIN, actualData.getAlias().getDomain());
        assertNotNull(actualData.getDetail());
        assertEquals(TEST_LAST_NAME, actualData.getDetail().getLastName());
        assertEquals(TEST_FIRST_NAME, actualData.getDetail().getFirstName());
        assertEquals(TEST_UPN, actualData.getDetail().getUserPrincipalName());
        assertEquals(TEST_EMAIL, actualData.getDetail().getEmailAddress());
        assertFalse(actualData.isDisabled());
        assertFalse(actualData.isLocked());
    }

    /*
     * Test if mapping succeeds from rest based person user on alias and person details being null
     */
    @Test
    public void testGetPersonUserOnInvalidInput() {
        UserDTO userToTest = UserDTO.builder().withName(TEST_PRINCIPAL_NAME).withDomain(TEST_PRINCIPAL_TENANT).build();
        PersonUser user = UserMapper.getPersonUser(userToTest);
        assertEquals(TEST_PRINCIPAL_NAME, user.getId().getName());
        assertEquals(TEST_PRINCIPAL_TENANT, user.getId().getDomain());
    }

    /**
     * Helpers for preparing test data
     */
    private PersonDetail getPersonDetail(){
       return new PersonDetail.Builder()
                              .description(DESC)
                              .emailAddress(TEST_EMAIL)
                              .firstName(TEST_FIRST_NAME)
                              .lastName(TEST_LAST_NAME)
                              .userPrincipalName(TEST_UPN)
                              .pwdLastSet(TEST_PASSWORD_LAST_SET, IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY)
                              .pwdLifeTime(TEST_PASSWORD_LIFETIME, IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY)
                              .build();
    }

    private UserDTO getPersonDTO(){
        PrincipalDTO alias = new PrincipalDTO(TEST_ALIAS_NAME, TEST_ALIAS_DOMAIN);

        return UserDTO.builder()
                .withName(TEST_PRINCIPAL_NAME)
                .withDomain(TEST_PRINCIPAL_TENANT)
                .withAlias(alias)
                .withDetails(mockPersonDetails())
                .withLocked(IS_LOCKED)
                .withDisabled(IS_DISABLED)
                .build();
    }

    private UserDetailsDTO mockPersonDetails(){
        return UserDetailsDTO.builder()
                                    .withDescription(DESC)
                                    .withEmail(TEST_EMAIL)
                                    .withFirstName(TEST_FIRST_NAME)
                                    .withLastName(TEST_LAST_NAME)
                                    .withUPN(TEST_UPN)
                                    .build();
    }
}
