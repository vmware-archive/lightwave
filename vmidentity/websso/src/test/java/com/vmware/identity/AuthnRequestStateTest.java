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

import static org.easymock.EasyMock.createMock;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.StringWriter;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.impl.AuthnRequestMarshaller;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.TestAuthnRequestStateAuthenticationFilter;
import com.vmware.identity.samlservice.impl.AuthnRequestStateCookieWrapper;
import com.vmware.identity.session.impl.SessionManagerImpl;

/**
 * @author root
 *
 */
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class AuthnRequestStateTest {
	private static final int MAX_RETRIES = 5; // number of times we're allowed to submit request before it fails

	private static IDiagnosticsLogger log;
	private static AuthenticationFilter<AuthnRequestState> filter;
	private static SessionManagerImpl sessionManager;
	private static String tenant;
	private static String relayStateParameter;

	@BeforeClass
	public static void setUp() throws Exception {
		log = DiagnosticsLoggerFactory.getLogger(AuthnRequestStateTest.class);
		filter = new AuthnRequestStateCookieWrapper(
		        new TestAuthnRequestStateAuthenticationFilter());
		sessionManager = new SessionManagerImpl();
		Shared.bootstrap();
		SharedUtils.bootstrap(false); // use real data
		tenant = ServerConfig.getTenant(0);
		relayStateParameter = Shared.encodeString(TestConstants.RELAY_STATE);
	}

	@AfterClass
	public static void cleanUp() throws Exception {
	    SharedUtils.cleanupTenant();
	}

	@Test
	public void TestParseReplayAttack() throws Exception {
		AuthnRequestState requestState = null;

		SharedUtils.bootstrap(false); // use real data

		StringBuffer sbRequestUrl = new StringBuffer();
		int tenantId = 0;
		AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
		sbRequestUrl.append(authnRequest.getDestination());

		// build mock request object
		HttpServletRequest request = SharedUtils.buildMockRequestObject(
				authnRequest,
				relayStateParameter,
				null,
				null,
				sbRequestUrl,
				null,
				null,
				tenantId);
        HttpServletResponse response = createMock(HttpServletResponse.class);
		requestState = new AuthnRequestState(request,response, sessionManager, ServerConfig.getTenant(tenantId));
		for (int i = 0; i < MAX_RETRIES; i++) {
			try {
				requestState.parseRequestForTenant(tenant, filter);
			} catch (IllegalStateException e) {
				if (requestState.getValidationResult().getResponseCode() ==
						HttpServletResponse.SC_FORBIDDEN) {
					fail(); // don't fail yet
				}
			}
		}
		try {
			requestState.parseRequestForTenant(tenant, filter);
			fail(); // exception expected
		} catch (IllegalStateException e) {
			ValidationResult vr = requestState.getValidationResult();
			assertEquals(vr.getResponseCode(), HttpServletResponse.SC_FORBIDDEN);
		}
	}

    @Test
    public void TestParseInvalidSignatureAlgorithm() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());

        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest,
                relayStateParameter,
                "InvalidSignatureAlgorithmGoesHere",
                null,
                sbRequestUrl,
                TestConstants.AUTHORIZATION,
                null,
                tenantId);

        HttpServletResponse response = SharedUtils.buildMockResponseSuccessObject(new StringWriter(), Shared.HTML_CONTENT_TYPE, false, null);
        AuthnRequestState requestState = new AuthnRequestState(request, response, sessionManager, ServerConfig.getTenant(tenantId));

        boolean invalidSigAlg = false;

        try {
            requestState.parseRequestForTenant(tenant, filter);
        } catch (IllegalStateException e) {
            if (e.getMessage().equals("authn request has invalid signature algorithm") &&
                    requestState.getValidationResult().getResponseCode() == HttpServletResponse.SC_BAD_REQUEST) {
                invalidSigAlg = true;
            }
        }

        if (!invalidSigAlg) {
            fail();
        }
    }

	@Test
	public void TestAuthenticate() throws Exception {
		AuthnRequestState requestState = null;

		SharedUtils.bootstrap(false); // use real data

		StringBuffer sbRequestUrl = new StringBuffer();
		int tenantId = 0;
		AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
		sbRequestUrl.append(authnRequest.getDestination());

		// build mock request object
		HttpServletRequest request = SharedUtils.buildMockRequestObject(
				authnRequest,
				relayStateParameter,
				null,
				null,
				sbRequestUrl,
				TestConstants.AUTHORIZATION,
				null,
				tenantId);

		// build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);
		requestState = new AuthnRequestState(request, response, sessionManager, ServerConfig.getTenant(tenantId));
		try {
			requestState.parseRequestForTenant(tenant, filter);
		} catch (IllegalStateException e) {
			if (requestState.getValidationResult().getResponseCode() ==
					HttpServletResponse.SC_FORBIDDEN) {
				fail(); // don't fail on other codes
			}
		}

		Document token = requestState.authenticate(tenant, filter);
		assertNotNull(token);
		log.debug("Document generated: " + token);
	}

    @Test
    public void TestAuthenticateWithOptions() throws Exception {
        AuthnRequestState requestState = null;

        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequestWithOptions("42", tenantId);

        AuthnRequestMarshaller marshaller = new AuthnRequestMarshaller();
        Element authnRequestElement = marshaller.marshall(authnRequest);

        String authnRequestString = Shared.getStringFromElement(authnRequestElement);
        log.debug("Created AuthnRequest: " + authnRequestString);
        sbRequestUrl.append(authnRequest.getDestination());

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest,
                relayStateParameter,
                null,
                null,
                sbRequestUrl,
                TestConstants.AUTHORIZATION,
                null,
                tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        requestState = new AuthnRequestState(request, response, sessionManager, ServerConfig.getTenant(tenantId));
        try {
            requestState.parseRequestForTenant(tenant, filter);
        } catch (IllegalStateException e) {
            if (requestState.getValidationResult().getResponseCode() ==
                    HttpServletResponse.SC_FORBIDDEN) {
                fail(); // don't fail on other codes
            }
        }

        Document token = requestState.authenticate(tenant, filter);
        assertNotNull(token);
        log.debug("Document generated: " + token);
    }

	@Test
	public void TestAuthenticateFail() throws Exception {
		AuthnRequestState requestState = null;

		SharedUtils.bootstrap(false); // use real data

		StringBuffer sbRequestUrl = new StringBuffer();
		int tenantId = 0;
		AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
		sbRequestUrl.append(authnRequest.getDestination());

		// build mock request object
		HttpServletRequest request = SharedUtils.buildMockRequestObject(
				authnRequest,
				relayStateParameter,
				null,
				null,
				sbRequestUrl,
				TestConstants.AUTHORIZATION,
				null,
				tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

		requestState = new AuthnRequestState(request, response, sessionManager, ServerConfig.getTenant(tenantId));
		try {
			requestState.parseRequestForTenant(tenant, filter);
		} catch (IllegalStateException e) {
			if (requestState.getValidationResult().getResponseCode() ==
					HttpServletResponse.SC_FORBIDDEN) {
				fail(); // don't fail on other codes
			}
		}

		try {
			requestState.authenticate(tenant, null); // pass null authenticator
			fail(); // should've thrown
		} catch (Exception e) {
			assertNotNull(e);
		}
	}
}
