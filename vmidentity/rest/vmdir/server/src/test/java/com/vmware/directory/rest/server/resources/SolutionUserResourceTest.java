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
package com.vmware.directory.rest.server.resources;

import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import com.vmware.directory.rest.common.data.PrincipalDTO;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.server.mapper.SolutionUserMapper;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.exception.server.NotImplementedError;
import com.vmware.identity.rest.core.util.CertificateHelper;

/**
 *
 * Unit tests for SolutionUser Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class SolutionUserResourceTest {

    private static final String TEST_TENANT = "test.local";
    private static final String TEST_DOMAIN = "test.local";
    private static final int MAX_USERS_TO_FETCH = 10;
    private static final String TEST_SOLN_USER_NAME = "testSolnUser";
    private static final String TEST_SOLN_USER_ALIAS = "testSolnUserAlias";
    private static final String TEST_CERT_LOC = "src/test/resources/test_cert.pem";
    private static final String SOLN_USER_DESC = "Test solution user";

    private static final boolean IS_DISABLED = false;

    private SolutionUserResource solnUserResource;
    private X509Certificate testCertificate;

    private IMocksControl mControl;
    private CasIdmClient mockCasIDMClient;
    private ContainerRequestContext request;

    @Before
    public void setUp() throws CertificateException, IOException {
        mControl = EasyMock.createControl();

        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        mockCasIDMClient = mControl.createMock(CasIdmClient.class);
        testCertificate = CertificateHelper.convertToX509(getTestPEMCert());
        solnUserResource = new SolutionUserResource(TEST_TENANT, request, null);
        solnUserResource.setIDMClient(mockCasIDMClient);
    }

    @Test
    public void testGet() throws Exception {
        expect(mockCasIDMClient.findSolutionUser(TEST_TENANT, TEST_SOLN_USER_NAME))
                .andReturn(getTestSolutionUser(TEST_SOLN_USER_NAME, TEST_SOLN_USER_ALIAS));
        mControl.replay();

        SolutionUserDTO user = solnUserResource.get(TEST_SOLN_USER_NAME);
        assertNotNull(user);
        assertEquals(TEST_SOLN_USER_NAME, user.getName());
        assertEquals(TEST_TENANT, user.getDomain());
        Assert.assertNotNull(user.getCertificate().getEncoded());

        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.findSolutionUser(TEST_TENANT, TEST_SOLN_USER_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        solnUserResource.get(TEST_SOLN_USER_NAME);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.findSolutionUser(TEST_TENANT, TEST_SOLN_USER_NAME);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        solnUserResource.get(TEST_SOLN_USER_NAME);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.findSolutionUser(TEST_TENANT, TEST_SOLN_USER_NAME);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        solnUserResource.get(TEST_SOLN_USER_NAME);
        mControl.verify();
    }

    @Test(expected=NotImplementedError.class)
    public void testGetGroups() {
        mControl.replay();
        solnUserResource.getGroups(TEST_SOLN_USER_NAME, MAX_USERS_TO_FETCH);
    }

    @Test
    public void testUpdateSolutionUser() throws Exception {
        SolutionUser user = getTestSolutionUser(TEST_SOLN_USER_NAME, TEST_SOLN_USER_ALIAS);
        SolutionUserDTO solnUserToUpdate = SolutionUserMapper.getSolutionUserDTO(user);

        expect(mockCasIDMClient.updateSolutionUserDetail(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), isA(SolutionDetail.class))).andReturn(new PrincipalId(TEST_SOLN_USER_NAME, TEST_TENANT));
        expect(mockCasIDMClient.findSolutionUser(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME))).andReturn(user);
        mControl.replay();

        SolutionUserDTO updated = solnUserResource.update(TEST_SOLN_USER_NAME, solnUserToUpdate);
        assertEquals(TEST_SOLN_USER_NAME, updated.getName());
        assertEquals(TEST_TENANT, updated.getDomain());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.updateSolutionUserDetail(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), isA(SolutionDetail.class));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        solnUserResource.update(TEST_SOLN_USER_NAME, getTestSolutionUserDTO());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testUpdateOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.updateSolutionUserDetail(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), isA(SolutionDetail.class));
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        solnUserResource.update(TEST_SOLN_USER_NAME, getTestSolutionUserDTO());
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testUpdateOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.updateSolutionUserDetail(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), isA(SolutionDetail.class));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        solnUserResource.update(TEST_SOLN_USER_NAME, getTestSolutionUserDTO());
        mControl.verify();
    }

    @Test
    public void testCreate() throws Exception{
        SolutionUser user = getTestSolutionUser(TEST_SOLN_USER_NAME, TEST_SOLN_USER_ALIAS);
        SolutionUserDTO solnUserToAdd = SolutionUserMapper.getSolutionUserDTO(user);

        expect(mockCasIDMClient.addSolutionUser(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), eq(user.getDetail()))).andReturn(new PrincipalId(TEST_SOLN_USER_NAME, TEST_TENANT));
        expect(mockCasIDMClient.findSolutionUser(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME))).andReturn(user);
        mControl.replay();

        SolutionUserDTO newUser = solnUserResource.create(solnUserToAdd);
        assertEquals(TEST_SOLN_USER_NAME, newUser.getName());
        assertEquals(TEST_TENANT, newUser.getDomain());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testCreateOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.addSolutionUser(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), isA(SolutionDetail.class));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        solnUserResource.create(getTestSolutionUserDTO());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testCreateOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.addSolutionUser(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), isA(SolutionDetail.class));
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        solnUserResource.create(getTestSolutionUserDTO());
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testCreateOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.addSolutionUser(eq(TEST_TENANT), eq(TEST_SOLN_USER_NAME), isA(SolutionDetail.class));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        solnUserResource.create(getTestSolutionUserDTO());
        mControl.verify();
    }

    @Test
    public void testDelete() throws Exception {
        mockCasIDMClient.deletePrincipal(TEST_TENANT, TEST_SOLN_USER_NAME);
        mControl.replay();

        solnUserResource.delete(TEST_SOLN_USER_NAME);

        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.deletePrincipal(TEST_TENANT, TEST_SOLN_USER_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        solnUserResource.delete(TEST_SOLN_USER_NAME);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.deletePrincipal(TEST_TENANT, TEST_SOLN_USER_NAME);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        solnUserResource.delete(TEST_SOLN_USER_NAME);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testDeleteOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.deletePrincipal(TEST_TENANT, TEST_SOLN_USER_NAME);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        solnUserResource.delete(TEST_SOLN_USER_NAME);
        mControl.verify();
    }

    private Set<SolutionUser> getTestSolutionUsers(int numOfUsers){
        Set<SolutionUser> solutionUsers = new HashSet<SolutionUser>();
        for(int i = 0;i< numOfUsers;i++){
            String username = TEST_SOLN_USER_NAME + i;
            String userAlias = TEST_SOLN_USER_ALIAS + i;
            solutionUsers.add(getTestSolutionUser(username, userAlias));
        }
        return solutionUsers;
    }

    private SolutionUser getTestSolutionUser(String solutionUserName, String solnUserAlias) {
        PrincipalId solutionUserId = new PrincipalId(solutionUserName, TEST_TENANT);
        SolutionDetail detail = new SolutionDetail(testCertificate);
        return new SolutionUser(solutionUserId, detail, IS_DISABLED);
    }

    private SolutionUserDTO getTestSolutionUserDTO() throws IOException, CertificateException {
        PrincipalDTO alias = new PrincipalDTO(TEST_SOLN_USER_ALIAS, TEST_DOMAIN);
        return SolutionUserDTO.builder()
                              .withAlias(alias)
                              .withDescription(SOLN_USER_DESC)
                              .withCertificate(new CertificateDTO(getTestPEMCert()))
                              .withDisabled(false)
                              .withName(TEST_SOLN_USER_NAME)
                              .build();
    }

    private static String getTestPEMCert() throws IOException {
        byte[] encoded = Files.readAllBytes(Paths.get(TEST_CERT_LOC));
        return new String(encoded, Charset.defaultCharset());
    }

}