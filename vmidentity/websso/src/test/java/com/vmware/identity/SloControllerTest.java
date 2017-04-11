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

import static org.easymock.EasyMock.capture;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.security.Key;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.Signature;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.zip.Inflater;
import java.util.zip.InflaterOutputStream;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.easymock.Capture;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.xml.util.Base64;
import org.springframework.context.support.ResourceBundleMessageSource;
import org.springframework.ui.Model;
import org.springframework.validation.support.BindingAwareModelMap;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.impl.LogoutStateProcessingFilter;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.impl.SessionManagerImpl;

/**
 * Single Logout Controller test
 *
 */
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class SloControllerTest {
    private static SloController controller;
    private static PrivateKey privateKey;
    private static IDiagnosticsLogger log;
    private static String tenant;
    private static String sigAlgParameter;
    private static SessionManagerImpl sessionManager;

    /**
     * @throws java.lang.Exception
     */
    @BeforeClass
    public static void setUp() throws Exception {
        log = DiagnosticsLoggerFactory.getLogger(SloControllerTest.class);

        controller = new SloController();
        ResourceBundleMessageSource ms = new ResourceBundleMessageSource();
        ms.setBasename("messages");
        controller.setMessageSource(ms);
        LogoutStateProcessingFilter filter = new LogoutStateProcessingFilter();
        controller.setProcessor(filter);
        sessionManager = new SessionManagerImpl();
        controller.setSessionManager(sessionManager);

        KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
        InputStream is = new FileInputStream(SsoControllerTest.class
                .getResource("/sts-store.jks").getFile());
        char[] stsKeystorePassword = "ca$hc0w".toCharArray();
        ks.load(is, stsKeystorePassword);

        String stsAlias = "stskey";
        Key key = ks.getKey(stsAlias, stsKeystorePassword);

        privateKey = (PrivateKey) key;
        Shared.bootstrap();
        SharedUtils.bootstrap(false); // use real data
        tenant = ServerConfig.getTenant(0);

        sigAlgParameter = TestConstants.SIGNATURE_ALGORITHM;
    }

    @AfterClass
    public static void cleanUp() throws Exception {
        SharedUtils.cleanupTenant();
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
    @Ignore("bugzilla#1175962")
    @Test
    public final void testSloMultiSP() throws Exception {
        SharedUtils.bootstrap(false); // use real data

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
        String tenantName = ServerConfig.getTenant(0);
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
        LogoutRequest logoutRequest =
                SharedUtils.createSamlLogoutRequest("42", participantSessionId);
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
        Capture<String> capturedOutboundRequest = new Capture<String>();
        Capture<String> capturedOutboundSLOResponse = new Capture<String>();
        HttpServletResponse response = buildMockResponseSuccessObjectForTwoRPs(
                capturedOutboundRequest,
                capturedOutboundSLOResponse, true);

        //Simulating receiving slo request from RP 0
        assertSlo(model, request, response);

        // parse request sent to non-initiator participating RP
        String decodedSamlRequest = extractRequest(capturedOutboundRequest);
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
        inflaterOutputStream.write(Base64.decode(samlRequestParameter));
        inflaterOutputStream.close();
        String outputString = new String(byteArrayOutputStream.toByteArray());

        return outputString;
    }

    private String extractRequest(Capture<String> captured) throws Exception {
        String redirectUrl = captured.getValue();
        URL url = new URL(redirectUrl);

        String samlRequestParameter = URLDecoder.decode(getQueryMap(url.getQuery()).get(Shared.SAML_REQUEST_PARAMETER), "UTF-8");

        // inflate
        Inflater decompresser = new Inflater(true);
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        InflaterOutputStream inflaterOutputStream = new InflaterOutputStream(
                byteArrayOutputStream, decompresser);
        inflaterOutputStream.write(Base64.decode(samlRequestParameter));
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


    //Mock a HttpServletResponse object in SLO expecting sending two redirect calls.
    //This can be used in testing SLO messages sent to two RPs by sso service.
    private HttpServletResponse buildMockResponseSuccessObjectForTwoRPs(
            Capture<String> capturedRequest, Capture<String> capturedResponse,
            boolean expectCookie) throws IOException {
        HttpServletResponse response = createMock(HttpServletResponse.class);
        if (expectCookie) {
            //sso session cookie
            response.addCookie(isA(Cookie.class));
            //IWA context id cookie
            response.addCookie(isA(Cookie.class));
        }
        response.sendRedirect(capture(capturedRequest));
        response.sendRedirect(capture(capturedResponse));
        replay(response);
        return response;
    }

    private void assertSlo(Model model, HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        controller.slo(Locale.US, tenant, model, request, response);
        assertEquals(tenant, model.asMap().get("tenant"));
        assertNull(model.asMap().get("serverTime"));
    }

    private void testSlo(boolean isSPSupportSLO) throws Exception {
        SharedUtils.bootstrap(false); // use real data

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
        LogoutRequest logoutRequest =
                SharedUtils.createSamlLogoutRequest("42", participantSessionId);
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

        // build mock request object
        int tenantId = 0;
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                logoutRequest, null,
                sigAlgParameter, signature, sbRequestUrl,
                TestConstants.AUTHORIZATION, null,tenantId);

        // build mock response object
        Capture<String> captured = new Capture<String>();
        HttpServletResponse response = buildMockResponseSuccessObject(captured, true);

        if (isSPSupportSLO) {
            assertSlo(model, request, response);

            // parse response
            String decodedSamlResponse = extractResponse(captured);
            assertTrue(decodedSamlResponse.contains(OasisNames.SUCCESS));
        } else {
            // Remove SLO setting in SP.
            SharedUtils.removeSLOfromRelyingParties(SloControllerTest.tenant);
            assertSlo(model, request, response);

            // parse response, redirect should not happen and nothing should be captured.
            assertTrue(!captured.hasCaptured());
        }
    }
}
