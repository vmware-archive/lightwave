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

import static org.easymock.EasyMock.expect;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;
import com.vmware.identity.rest.idm.server.mapper.SolutionUserMapper;

/**
 *
 * Unit tests for SolutionUserMapper
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class SolutionUserMapperTest {

    // Constants
    private static final String TEST_SOLN_USER_NAME = "vmware_service";
    private static final String TEST_SOLN_USER_DOMAIN = "vmware.domain";
    private static final boolean IS_SOLN_USER_DISABLED = false;
    private static final String TEST_SOLN_USER_ALIAS_NAME = "soln_usr-alias";
    private static final String TEST_SOLN_USER_ALIAS_DOMAIN = "soln_usr_alias_domain";
    private static final String MOCK_CERT = "A spurious mock certificate created for purpose of unit test";
    private X509Certificate mockCertificate;
    private IMocksControl mControl;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();
        mockCertificate = mControl.createMock(X509Certificate.class);
    }

    @Test
    public void testGetSolutionUserDTO() throws DTOMapperException, CertificateEncodingException {
        PrincipalId solutionUsrId = new PrincipalId(TEST_SOLN_USER_NAME, TEST_SOLN_USER_DOMAIN);
        PrincipalId solutionUsrAlias = new PrincipalId(TEST_SOLN_USER_ALIAS_NAME, TEST_SOLN_USER_ALIAS_DOMAIN);
        SolutionDetail solutionDetail = new SolutionDetail(mockCertificate);
        SolutionUser idmSolutionUser = new SolutionUser(solutionUsrId, solutionUsrAlias, null, solutionDetail, IS_SOLN_USER_DISABLED);
        expect(mockCertificate.getEncoded()).andReturn(MOCK_CERT.getBytes());
        mControl.replay();
        SolutionUserDTO solutionUserDTO = SolutionUserMapper.getSolutionUserDTO(idmSolutionUser);

        // Assertion
        assertEquals(TEST_SOLN_USER_NAME, solutionUserDTO.getName());
        assertEquals(TEST_SOLN_USER_DOMAIN, solutionUserDTO.getDomain());
        assertNotNull(solutionUserDTO.getAlias());
        assertEquals(TEST_SOLN_USER_ALIAS_NAME, solutionUserDTO.getAlias().getName());
        assertEquals(TEST_SOLN_USER_ALIAS_DOMAIN, solutionUserDTO.getAlias().getDomain());
    }

}
