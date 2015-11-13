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

import static junit.framework.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import org.junit.Test;

import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.GroupDetailsDTO;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.server.mapper.GroupMapper;

/**
 *
 * Unit tests for GroupMapper
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */

public class GroupMapperTest {

    //Constants
    private static final String TEST_GROUP_NAME = "testGroup";
    private static final String TEST_DOMAIN = "test.local";
    private static final String TEST_GROUP_ALIAS_NAME = "testAliasGroup";
    private static final String TEST_GROUP_ALIAS_DOMAIN = "testALias.local";

    private static final String TEST_GROUP_DESC = "test group Description";

    @Test
    public void testGetGroupDTO() throws DTOMapperException{
        // prepare test data
        PrincipalId groupId = new PrincipalId(TEST_GROUP_NAME, TEST_DOMAIN);
        PrincipalId alias = new PrincipalId(TEST_GROUP_ALIAS_NAME, TEST_GROUP_ALIAS_DOMAIN);
        GroupDetail groupDetail = new GroupDetail(TEST_GROUP_DESC);
        Group group = new Group(groupId, alias, null, groupDetail);

        //Invocation
        GroupDTO actualData = GroupMapper.getGroupDTO(group);

        //Assertion
        assertEquals(TEST_GROUP_NAME, actualData.getName());
        assertEquals(TEST_DOMAIN, actualData.getDomain());
        assertNotNull(actualData.getAlias());
        assertEquals(TEST_GROUP_ALIAS_NAME, actualData.getAlias().getName());
        assertEquals(TEST_GROUP_ALIAS_DOMAIN, actualData.getAlias().getDomain());
    }

    @Test
    public void testGetGroup() throws DTOMapperException{
        //Prepare test data
        PrincipalDTO alias = new PrincipalDTO(TEST_GROUP_ALIAS_NAME, TEST_GROUP_ALIAS_DOMAIN);

        GroupDTO groupDTO = GroupDTO.builder()
                    .withName(TEST_GROUP_NAME)
                    .withDomain(TEST_DOMAIN)
                    .withDetails(new GroupDetailsDTO(TEST_GROUP_DESC))
                    .withAlias(alias)
                    .build();

        //Invocation
        Group actualData = GroupMapper.getGroup(groupDTO);

        //Assertion
        assertEquals(TEST_GROUP_NAME, actualData.getId().getName());
        assertEquals(TEST_DOMAIN, actualData.getId().getDomain());
        assertEquals(TEST_GROUP_ALIAS_NAME, actualData.getAlias().getName());
        assertEquals(TEST_GROUP_ALIAS_DOMAIN, actualData.getAlias().getDomain());
        assertEquals(TEST_GROUP_DESC, actualData.getDetail().getDescription());
    }
}
