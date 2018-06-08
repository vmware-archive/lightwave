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
import static com.vmware.identity.SharedUtils.createSamlLogoutRequest;
import static com.vmware.identity.SharedUtils.encodeRequest;
import static com.vmware.identity.SharedUtils.getAssertionConsumerService;
import static com.vmware.identity.SharedUtils.getMockIdmAccessorFactory;
import static com.vmware.identity.SharedUtils.getSTSCertificate;
import static com.vmware.identity.SharedUtils.getSTSPrivateKey;
import static com.vmware.identity.SharedUtils.messageSource;
import static com.vmware.identity.SharedUtils.prepareMockIdmClient;
import static org.easymock.EasyMock.capture;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.security.PrivateKey;
import java.security.Signature;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.zip.Inflater;
import java.util.zip.InflaterOutputStream;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.easymock.Capture;
import org.junit.BeforeClass;
import org.junit.Test;
import org.opensaml.saml.saml2.core.LogoutRequest;
import org.opensaml.saml.saml2.core.LogoutResponse;
import org.springframework.ui.Model;
import org.springframework.validation.support.BindingAwareModelMap;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.IdmAccessorFactory;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.impl.CasIdmAccessor;
import com.vmware.identity.samlservice.impl.LogoutStateProcessingFilter;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.impl.SessionManagerImpl;

import net.shibboleth.utilities.java.support.codec.Base64Support;

/**
 * Single Logout Controller test
 *
 */
public class SloControllerTest {
    private static SloController controller;
    private static PrivateKey privateKey;
    private static IDiagnosticsLogger log;
    private static String tenant;
    private static final int tenantId = 0;
    private static String sigAlgParameter;
    private static SessionManagerImpl sessionManager;

    /**
     * @throws java.lang.Exception
     */
    @BeforeClass
    public static void setUp() throws Exception {
        log = DiagnosticsLoggerFactory.getLogger(SloControllerTest.class);

        controller = new SloController();
        controller.setMessageSource(messageSource());
        LogoutStateProcessingFilter filter = new LogoutStateProcessingFilter();
        controller.setProcessor(filter);
        sessionManager = new SessionManagerImpl();
        controller.setSessionManager(sessionManager);

        Shared.bootstrap();
        bootstrap();
        tenant = ServerConfig.getTenant(tenantId);
        privateKey = getSTSPrivateKey();

        sigAlgParameter = TestConstants.SIGNATURE_ALGORITHM;
    }

    /**
     * Test method for {@link com.vmware.identity.SloController#slo(java.util.Locale, java.lang.String, org.springframework.ui.Model, javax.servlet.http.HttpServletRequest, javax.servlet.http.HttpServletResponse)}.
     * @throws Exception
     */
    @Test
    public final void testSlo() throws Exception {
        testSlo(true);
    }

    @Test
    public final void testSloNoIDPSLO() throws Exception {
        testSlo(false);
    }

    /**
     * Test method for {@link com.vmware.identity.SloController#slo(java.util.Locale, java.lang.String, org.springframework.ui.Model, javax.servlet.http.HttpServletRequest, javax.servlet.http.HttpServletResponse)}.
     * We will create two RPs in this SLO test. This test will make two calls to constoller.slo
     * 1) RP0 sends SLO request to IDP expecting IDP to successfully process the request and send SLO request to RP1, as well as a SLO response to RP0.
     * 2) RP1 sends SLO response to IDP expecting no response and no exception.
     * @throws Exception
     */
    @Test
    public final void testSloMultiSP() throws Exception {
        // Step 1.  create a (fake) session for logout to succeed
        Calendar calendar = new GregorianCalendar();
        calendar.add(Calendar.MINUTE, Shared.TOKEN_LIFETIME_MINUTES);
        Date endTime = calendar.getTime();

        Session session = new Session(
                new PrincipalId(TestConstants.USER, TestConstants.DOMAIN),
                endTime,
                AuthnMethod.KERBEROS);
        sessionManager.add(session);

        // add our relying party as participant
        String tenantName = ServerConfig.getTenant(tenantId);
        String rpName = ServerConfig.getRelyingParty(tenantName, 0);
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);
        String participantSessionId = session.ensureSessionParticipant(issuerUrl);
        sessionManager.update(session);

        // add second relying party
        rpName = ServerConfig.getRelyingParty(tenantName, 1);
        issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);
        participantSessionId = session.ensureSessionParticipant(issuerUrl);
        sessionManager.update(session);

        //Step 2.  Testing RP0 initiated SLO request

        //construct RP issued SLO request URL
        StringBuffer sbRequestUrl = new StringBuffer();
        LogoutRequest logoutRequest = createSamlLogoutRequest("42", participantSessionId);
        sbRequestUrl.append(logoutRequest.getDestination());
        Model model = new BindingAwareModelMap();

        String samlRequestParameter = SharedUtils.encodeRequest(logoutRequest);

        // produce signature
        SignatureAlgorithm algo = SignatureAlgorithm
                .getSignatureAlgorithmForURI(TestConstants.SIGNATURE_ALGORITHM);
        Signature sig = Signature.getInstance(algo.getAlgorithmName());
        sig.initSign(privateKey);

        String messageToSign = Shared.SAML_REQUEST_PARAMETER + "="
                + URLEncoder.encode(samlRequestParameter, "UTF-8") + "&"
                + Shared.SIGNATURE_ALGORITHM_PARAMETER + "="
                + URLEncoder.encode(algo.toString(), "UTF-8");

        byte[] messageBytes = messageToSign.getBytes();
        sig.update(messageBytes);

        byte[] sigBytes = sig.sign();
        String signature = Shared.encodeBytes(sigBytes);

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, logoutRequest, null, null, null,
                null);

        // build mock initial servltrequest object to be sent to IDP
        int tenantId = 0;
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                logoutRequest, null,
                sigAlgParameter, signature, sbRequestUrl,
                TestConstants.AUTHORIZATION, null,tenantId);

        // build mock servlet response object
        Capture<String> capturedOutboundSLOResponse = new Capture<String>();
        HttpServletResponse response = buildMockResponseSuccessObject(capturedOutboundSLOResponse, true);
        Capture<HttpGet> capturedOutboundRequest = new Capture<HttpGet>();
        CloseableHttpClient httpClient = createMock(CloseableHttpClient.class);
        CloseableHttpResponse closeableResponse = createMock(CloseableHttpResponse.class);
        closeableResponse.close();
        expectLastCall().once();
        expect(httpClient.execute(capture(capturedOutboundRequest))).andReturn(closeableResponse);
        replay(closeableResponse);
        replay(httpClient);

        //Simulating receiving slo request from RP 0
        assertSlo(model, request, response, httpClient, null);

        // parse request sent to non-initiator participating RP
        String requestUrl = capturedOutboundRequest.getValue().getURI().toString();;
        String decodedSamlRequest = extractRequest(requestUrl);
        assertNotNull(decodedSamlRequest);

        // parse response sent to initiator RP
        String decodedSamlResponse = extractResponse(capturedOutboundSLOResponse);
        assertNotNull(decodedSamlResponse);

        //Step 3.  Test receiving slo response from RP1
        // Second controler.slo call to send out successful LogoutResponse from RP 1 to IDP
        LogoutResponse logoutResponse =
                SharedUtils.createSamlLogoutResponse(
                        session.getLogoutRequestData().getCurrentRequestId());
        model = new BindingAwareModelMap();

        String samlResponseParameter = SharedUtils.encodeRequest(logoutResponse);

        // produce signature
        algo = SignatureAlgorithm
                .getSignatureAlgorithmForURI(TestConstants.SIGNATURE_ALGORITHM);
        sig = Signature.getInstance(algo.getAlgorithmName());
        sig.initSign(privateKey);

        messageToSign = Shared.SAML_RESPONSE_PARAMETER + "="
                + URLEncoder.encode(samlResponseParameter, "UTF-8") + "&"
                + Shared.SIGNATURE_ALGORITHM_PARAMETER + "="
                + URLEncoder.encode(algo.toString(), "UTF-8");

        messageBytes = messageToSign.getBytes();
        sig.update(messageBytes);

        sigBytes = sig.sign();
        signature = Shared.encodeBytes(sigBytes);

        // print out complete GET url of the slo response send back to IDP in response to request send to the non-initiating RP.
        SharedUtils.logUrl(log, sbRequestUrl, logoutResponse, null, null, null,
                null);

        // build mock servlet request object for the slo response send back to IDP
        request = SharedUtils.buildMockRequestObject(
                logoutResponse, null,
                sigAlgParameter, signature, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        response = createMock(HttpServletResponse.class);

        assertSlo(model, request, response);

        //There will be no response send back to RP in response to the SLO response message.
        //So we are good as far as there was no exception.
    }

    private String extractResponse(Capture<String> captured) throws Exception {
        String redirectUrl = captured.getValue();
        URL url = new URL(redirectUrl);

        String samlRequestParameter = URLDecoder.decode(getQueryMap(url.getQuery()).get(Shared.SAML_RESPONSE_PARAMETER), "UTF-8");

        // inflate
        Inflater decompresser = new Inflater(true);
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        InflaterOutputStream inflaterOutputStream = new InflaterOutputStream(
                byteArrayOutputStream, decompresser);
        inflaterOutputStream.write(Base64Support.decode(samlRequestParameter));
        inflaterOutputStream.close();
        String outputString = new String(byteArrayOutputStream.toByteArray());

        return outputString;
    }

    private String extractRequest(String redirectUrl) throws Exception{
        URL url = new URL(redirectUrl);

        String samlRequestParameter = URLDecoder.decode(getQueryMap(url.getQuery()).get(Shared.SAML_REQUEST_PARAMETER), "UTF-8");

        // inflate
        Inflater decompresser = new Inflater(true);
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        InflaterOutputStream inflaterOutputStream = new InflaterOutputStream(
                byteArrayOutputStream, decompresser);
        inflaterOutputStream.write(Base64Support.decode(samlRequestParameter));
        inflaterOutputStream.close();
        String outputString = new String(byteArrayOutputStream.toByteArray());

        return outputString;
    }

    private static Map<String, String> getQueryMap(String query)
    {
        String[] params = query.split("&");
        Map<String, String> map = new HashMap<String, String>();
        for (String param : params)
        {
            String name = param.split("=")[0];
            String value = param.split("=")[1];
            map.put(name, value);
        }
        return map;
    }

    //Mock a HttpServletResponse object in SLO expecting sending one redirect call.
    private HttpServletResponse buildMockResponseSuccessObject(Capture<String> request,
            boolean expectCookie) throws IOException {
        HttpServletResponse response = createMock(HttpServletResponse.class);
        Shared.addNoCacheHeader(response);
        if (expectCookie) {
            //sso session cookie
            response.addCookie(isA(Cookie.class));
            //IWA context id cookie
            response.addCookie(isA(Cookie.class));
        }
        response.sendRedirect(capture(request));
        replay(response);
        return response;
    }

    private void assertSlo(Model model, HttpServletRequest request, HttpServletResponse response) throws Exception {
        assertSlo(model, request, response, null, null);
    }

    private void assertSlo(Model model, HttpServletRequest request, HttpServletResponse response,
            CloseableHttpClient httpClient, IdmAccessorFactory factory) throws Exception {
        factory = factory == null ? getMockIdmAccessorFactory(0, 0, 1) : factory;
        controller.slo(Locale.US, tenant, model, request, response, factory, httpClient);
        assertEquals(tenant, model.asMap().get("tenant"));
        assertNull(model.asMap().get("serverTime"));
    }

    private void testSlo(boolean isSPSupportSLO) throws Exception {
        // create a (fake) session for logout to succeed
        Calendar calendar = new GregorianCalendar();
        calendar.add(Calendar.MINUTE, Shared.TOKEN_LIFETIME_MINUTES);
        Date endTime = calendar.getTime();

        Session session = new Session(
                new PrincipalId(TestConstants.USER, TestConstants.DOMAIN),
                endTime,
                AuthnMethod.KERBEROS);
        sessionManager.add(session);

        // add our relying party as participant
        String tenantName = ServerConfig.getTenant(0);
        String rpName = ServerConfig.getRelyingParty(tenantName, 0);
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);
        String participantSessionId = session.ensureSessionParticipant(issuerUrl);
        sessionManager.update(session);

        StringBuffer sbRequestUrl = new StringBuffer();
        LogoutRequest logoutRequest = createSamlLogoutRequest("42", participantSessionId);
        sbRequestUrl.append(logoutRequest.getDestination());
        Model model = new BindingAwareModelMap();

        String samlRequestParameter = encodeRequest(logoutRequest);

        // produce signature
        SignatureAlgorithm algo = SignatureAlgorithm
                .getSignatureAlgorithmForURI(TestConstants.SIGNATURE_ALGORITHM);
        Signature sig = Signature.getInstance(algo.getAlgorithmName());
        sig.initSign(privateKey);

        String messageToSign = Shared.SAML_REQUEST_PARAMETER + "="
                + URLEncoder.encode(samlRequestParameter, "UTF-8") + "&"
                + Shared.SIGNATURE_ALGORITHM_PARAMETER + "="
                + URLEncoder.encode(algo.toString(), "UTF-8");

        byte[] messageBytes = messageToSign.getBytes();
        sig.update(messageBytes);

        byte[] sigBytes = sig.sign();
        String signature = Shared.encodeBytes(sigBytes);

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, logoutRequest, null, null, null,
                null);

        // build mock request object
        int tenantId = 0;
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                logoutRequest, null,
                sigAlgParameter, signature, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        Capture<String> captured = new Capture<String>();
        HttpServletResponse response = buildMockResponseSuccessObject(captured, true);

        if (isSPSupportSLO) {
            assertSlo(model, request, response);

            // parse response
            String decodedSamlResponse = extractResponse(captured);
            assertTrue(decodedSamlResponse.contains(OasisNames.SUCCESS));
        } else {
            CasIdmClient mockIdmClient = prepareMockIdmClient(tenantId);
            prepareMockRPWithNoSLO(mockIdmClient);
            IdmAccessor idmAccessor = new CasIdmAccessor(mockIdmClient);
            IdmAccessorFactory mockIdmAccessorFactory = createMock(IdmAccessorFactory.class);
            expect(mockIdmAccessorFactory.getIdmAccessor()).andReturn(idmAccessor).anyTimes();
            replay(mockIdmAccessorFactory);
            replay(mockIdmClient);
            assertSlo(model, request, response, null, mockIdmAccessorFactory);

            // parse response, redirect should not happen and nothing should be captured.
            assertTrue(!captured.hasCaptured());
        }
    }

    private static void prepareMockRPWithNoSLO(CasIdmClient mockIdmClient) throws Exception {
        String tenant = ServerConfig.getTenant(tenantId);
        String rpName = ServerConfig.getRelyingParty(tenant, 0);
        String rpEntityId = ServerConfig.getRelyingPartyUrl(ServerConfig.getRelyingParty(tenant, 0));

        Collection<ServiceEndpoint> sloServices = new HashSet<>();

        Collection<AssertionConsumerService> assertionServices = new HashSet<>();
        assertionServices.add(getAssertionConsumerService(0, 0, 0, OasisNames.HTTP_REDIRECT));
        assertionServices.add(getAssertionConsumerService(0, 0, 0, OasisNames.HTTP_POST));
        RelyingParty mockRP = createMock(RelyingParty.class);
        String defaultAcs = ServerConfig.getDefaultAssertionConsumerService(rpName);

        expect(mockRP.getDefaultAssertionConsumerService()).andReturn(defaultAcs).anyTimes();
        expect(mockRP.getCertificate()).andReturn(getSTSCertificate()).anyTimes();
        expect(mockRP.getAssertionConsumerServices()).andReturn(assertionServices).anyTimes();
        expect(mockRP.getSingleLogoutServices()).andReturn(sloServices).anyTimes();
        expect(mockRP.isAuthnRequestsSigned()).andReturn(false).anyTimes();
        expect(mockIdmClient.getRelyingPartyByUrl(tenant, rpEntityId)).andReturn(mockRP).anyTimes();
        replay(mockRP);
    }
}
