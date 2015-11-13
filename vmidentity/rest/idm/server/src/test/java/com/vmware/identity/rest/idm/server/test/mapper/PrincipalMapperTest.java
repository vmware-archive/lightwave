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

import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.server.mapper.PrincipalMapper;

/**
 *
 * Unit tests for PrincipalMapper
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class PrincipalMapperTest {

    private static final String PRINCIPAL_NAME = "testPrincipal";
    private static final String DOMAIN = "test.local";

    @Test
    public void testGetPrincipalDTO() {
        PrincipalId principalId = new PrincipalId(PRINCIPAL_NAME, DOMAIN);
        PrincipalDTO principalDTO = PrincipalMapper.getPrincipalDTO(principalId);
        assertEquals(PRINCIPAL_NAME, principalDTO.getName());
        assertEquals(DOMAIN, principalDTO.getDomain());
    }

    @Test
    public void testGetPrincipalId() {
        PrincipalDTO principalDTO = new PrincipalDTO(PRINCIPAL_NAME, DOMAIN);
        PrincipalId principalId = PrincipalMapper.getPrincipal(principalDTO);
        assertEquals(PRINCIPAL_NAME, principalId.getName());
        assertEquals(DOMAIN, principalId.getDomain());
    }

    @Test(expected=DTOMapperException.class)
    public void testGetPrincipalId_OnNullDomain() {
        PrincipalMapper.getPrincipal(new PrincipalDTO(PRINCIPAL_NAME, null));
    }
}
