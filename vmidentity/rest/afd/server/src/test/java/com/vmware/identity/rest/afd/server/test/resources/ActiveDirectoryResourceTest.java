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

package com.vmware.identity.rest.afd.server.test.resources;

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.util.Locale;

import javax.ws.rs.BadRequestException;
import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IdmADDomainAccessDeniedException;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinInfoDTO;
import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinRequestDTO;
import com.vmware.identity.rest.afd.server.resources.ActiveDirectoryResource;
import com.vmware.identity.rest.core.data.CredentialsDTO;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;


public class ActiveDirectoryResourceTest {

    private static final String DOMAIN_NAME = "domain.test.local";
    private static final String ALIAS = "alias.test.local";
    private static final String DN = "FooBar";
    private static final String USER_NAME = "testUser";
    private static final String PWD = "testPwd!@3";
    private static final String ORG_UNIT = "unitTesting";
    private ActiveDirectoryResource activeDirectoryResource;
    private CasIdmClient mockIDMClient;
    private IMocksControl mControl;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();

        request = EasyMock.createMock(ContainerRequestContext.class);
        expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(request);

        mockIDMClient = mControl.createMock(CasIdmClient.class);
        activeDirectoryResource = new ActiveDirectoryResource(request, null);
        activeDirectoryResource.setIDMClient(mockIDMClient);
    }

    @Test
    public void testGetADJoinInfo() throws Exception {
        expect(mockIDMClient.getActiveDirectoryJoinStatus()).andReturn(new ActiveDirectoryJoinInfo(DOMAIN_NAME, ALIAS, DN));
        mControl.replay();
        ActiveDirectoryJoinInfoDTO adJoinInfo = activeDirectoryResource.getActiveDirectoryStatus();
        assertEquals(DOMAIN_NAME, adJoinInfo.getName());
        assertEquals(ALIAS, adJoinInfo.getAlias());
        assertEquals(DN, adJoinInfo.getDn());
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetADJoinInfoOnIDMError_ThrowsInternalServerErrorEx() throws Exception {
        mockIDMClient.getActiveDirectoryJoinStatus();
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        activeDirectoryResource.getActiveDirectoryStatus();
        mControl.verify();
    }

    @Test
    public void testJoinAD() throws Exception {
        ActiveDirectoryJoinRequestDTO joinRequest = new ActiveDirectoryJoinRequestDTO(USER_NAME, PWD, DOMAIN_NAME, ORG_UNIT);
        mockIDMClient.joinActiveDirectory(USER_NAME, PWD, DOMAIN_NAME, ORG_UNIT);
        expect(mockIDMClient.getActiveDirectoryJoinStatus()).andReturn(new ActiveDirectoryJoinInfo(DOMAIN_NAME, ALIAS, DN));
        mControl.replay();
        ActiveDirectoryJoinInfoDTO joinInfo = activeDirectoryResource.joinActiveDirectory(joinRequest);
        assertEquals(DOMAIN_NAME, joinInfo.getName());
        assertEquals(ALIAS, joinInfo.getAlias());
        assertEquals(DN, joinInfo.getDn());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testSearchOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockIDMClient.joinActiveDirectory(USER_NAME, PWD, DOMAIN_NAME, ORG_UNIT);
        expectLastCall().andThrow(new IllegalArgumentException("illegal argument"));
        mControl.replay();
        ActiveDirectoryJoinRequestDTO joinRequest = new ActiveDirectoryJoinRequestDTO(USER_NAME, PWD, DOMAIN_NAME, ORG_UNIT);
        activeDirectoryResource.joinActiveDirectory(joinRequest);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testSearchOnIDMError_ThrowsInternalServerError() throws Exception {
        mockIDMClient.joinActiveDirectory(USER_NAME, PWD, DOMAIN_NAME, ORG_UNIT);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        ActiveDirectoryJoinRequestDTO joinRequest = new ActiveDirectoryJoinRequestDTO(USER_NAME, PWD, DOMAIN_NAME, ORG_UNIT);
        activeDirectoryResource.joinActiveDirectory(joinRequest);
        mControl.verify();
    }

    @Test
    public void testLeaveAD() throws Exception {
        mockIDMClient.leaveActiveDirectory(USER_NAME, PWD);
        mControl.replay();
        activeDirectoryResource.leaveActiveDirectory(new CredentialsDTO(USER_NAME, PWD));
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testLeaveADOnDenialOfAccess_ThrowsBadRequestEx() throws Exception {
        mockIDMClient.leaveActiveDirectory(USER_NAME, PWD);
        expectLastCall().andThrow(new IdmADDomainAccessDeniedException(DOMAIN_NAME, USER_NAME));
        mControl.replay();
        activeDirectoryResource.leaveActiveDirectory(new CredentialsDTO(USER_NAME, PWD));
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testLeaveADOnIDMError_ThrowsInternalServerError() throws Exception {
        mockIDMClient.leaveActiveDirectory(USER_NAME, PWD);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        activeDirectoryResource.leaveActiveDirectory(new CredentialsDTO(USER_NAME, PWD));
        mControl.verify();
    }

}
