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
package com.vmware.identity;

import static com.vmware.identity.SharedUtils.bootstrap;
import static com.vmware.identity.SharedUtils.getMockIdmAccessorFactory;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertNotNull;

import java.util.Locale;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.context.support.ResourceBundleMessageSource;
import org.springframework.web.servlet.ModelAndView;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.samlservice.Shared;

/**
 * Test IDP-initiated SSO controller
 *
 */
public class IdpSsoControllerTest {
    private static IDiagnosticsLogger log;
    private static String tenant;
    private static IdpSsoController controller;

    /**
     * @throws java.lang.Exception
     */
    @BeforeClass
    public static void setUp() throws Exception {
        log = DiagnosticsLoggerFactory.getLogger(IdpSsoControllerTest.class);

        controller = new IdpSsoController();
        ResourceBundleMessageSource ms = new ResourceBundleMessageSource();
        ms.setBasename("messages");
        controller.setMessageSource(ms);

        Shared.bootstrap();
        bootstrap();
        tenant = ServerConfig.getTenant(0);
    }

    /**
     * Test method for
     * {@link com.vmware.identity.IdpSsoController#sso(java.util.Locale, java.lang.String, javax.servlet.http.HttpServletRequest, javax.servlet.http.HttpServletResponse)}.
     *
     * @throws Exception
     */
    @Test
    public final void testSso() throws Exception {
        // get provider id
        String tenantName = ServerConfig.getTenant(0);
        String rpName = ServerConfig.getRelyingParty(tenantName, 0);
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);

        HttpServletRequest request = createMock(HttpServletRequest.class);
        expect(request.getParameter(IdpSsoController.PROVIDER_ENTITY_ID)).andReturn(issuerUrl).anyTimes();
        expect(request.getParameter(IdpSsoController.NAME_ID_FORMAT)).andReturn(null).anyTimes();
        expect(request.getParameter(Shared.RELAY_STATE_PARAMETER)).andReturn(null).anyTimes();
        expect(request.getParameter(IdpSsoController.ASSERTION_CONSUMER_SERVICE_INDEX)).andReturn(null).anyTimes();
        expect(request.getParameter(IdpSsoController.ATTRIBUTE_CONSUMER_SERVICE_INDEX)).andReturn(null).anyTimes();
        expect(request.getParameter(IdpSsoController.FORCE_AUTHN)).andReturn(null).anyTimes();
        expect(request.getParameter(IdpSsoController.IS_PASSIVE)).andReturn(null).anyTimes();
        replay(request);
        HttpServletResponse response = createMock(HttpServletResponse.class);
        replay(response);

        ModelAndView result = controller.sso(Locale.US, tenant, request, response, getMockIdmAccessorFactory(0, 0));

        assertNotNull(result);
        log.debug("IDP SSO test passed!");
    }

}
