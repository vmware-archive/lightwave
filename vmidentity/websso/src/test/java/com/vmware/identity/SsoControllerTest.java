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
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.net.MalformedURLException;
import java.security.Key;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.joda.time.DateTime;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.opensaml.common.SAMLObjectBuilder;
import org.opensaml.common.SAMLVersion;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.IDPEntry;
import org.opensaml.saml2.core.IDPList;
import org.opensaml.saml2.core.NameIDPolicy;
import org.opensaml.saml2.core.Scoping;
import org.opensaml.saml2.core.impl.NameIDPolicyBuilder;
import org.opensaml.xml.Configuration;
import org.opensaml.xml.XMLObjectBuilderFactory;
import org.opensaml.xml.security.credential.BasicCredential;
import org.opensaml.xml.security.credential.Credential;
import org.opensaml.xml.signature.KeyInfo;
import org.opensaml.xml.signature.SignatureConstants;
import org.springframework.context.MessageSource;
import org.springframework.context.support.ResourceBundleMessageSource;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.ui.Model;
import org.springframework.validation.support.BindingAwareModelMap;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.TestAuthnRequestStateAuthenticationFilter;
import com.vmware.identity.samlservice.impl.AuthnRequestStateCookieWrapper;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.impl.SessionManagerImpl;

@SuppressWarnings("unchecked")
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class SsoControllerTest {
    private static final String SIGNATURE = "bogus";
    private static final String AUTH_HEADER = Shared.IWA_AUTH_RESPONSE_HEADER;
    private static final String AUTH_VALUE = Shared.KERB_AUTH_PREFIX;

    private static SsoController controller;
    private static PrivateKey privateKey;
    private static IDiagnosticsLogger log;
    private static String tenant;
    private static MessageSource messageSource;
    private static String relayStateParameter;
    private static String sigAlgParameter;

    @BeforeClass
    public static void setUp() throws Exception {
        log = DiagnosticsLoggerFactory.getLogger(SsoControllerTest.class);

        controller = new SsoController();
        ResourceBundleMessageSource ms = new ResourceBundleMessageSource();
        ms.setBasename("messages");
        controller.setMessageSource(ms);
        messageSource = ms;
        TestAuthnRequestStateAuthenticationFilter filter = new TestAuthnRequestStateAuthenticationFilter();
        AuthnRequestStateCookieWrapper cookieFilter = new AuthnRequestStateCookieWrapper(
                filter);
        controller.setKerbAuthenticator(cookieFilter);
        controller.setPasswordAuthenticator(cookieFilter);
        controller.setCookieAuthenticator(cookieFilter);
        SessionManagerImpl sessionManager = new SessionManagerImpl();
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

        relayStateParameter = Shared.encodeString(TestConstants.RELAY_STATE);
        sigAlgParameter = TestConstants.SIGNATURE_ALGORITHM;
    }

    @AfterClass
    public static void cleanUp() throws Exception {
        SharedUtils.cleanupTenant();
    }

    @Test
    public void testSsoNoAuth() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42",tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                null, null,tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(401,
                "Unauthorized", true);

        assertSso(model, request, response);
    }

    private void assertSso(Model model, HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        controller.sso(Locale.US, tenant, model, request, response);
        assertEquals(tenant, model.asMap().get("tenant"));
        assertNull(model.asMap().get("serverTime"));
    }

    @Test
    public void testSso() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        String samlRequestParameter = SharedUtils.encodeRequest(authnRequest);

        String signature = SharedUtils.getSamlRequestSignature(privateKey, relayStateParameter,
                samlRequestParameter);

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, sigAlgParameter, signature,
                sbRequestUrl, TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        true, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.SUCCESS));
    }

    @Test
    public void testSsoDefaultTenant() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        // determine default tenant endpoint
        authnRequest.setDestination(SharedUtils.getDefaultTenantEndpoint());
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        true, null);

        controller.ssoDefaultTenant(Locale.US, model, request, response);
        assertEquals(tenant, model.asMap().get("tenant"));
        assertNull(model.asMap().get("serverTime"));

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.SUCCESS));
    }

    @Test
    public void testSsoSignatureFail() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, sigAlgParameter, SIGNATURE, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, sigAlgParameter, SIGNATURE,
                sbRequestUrl, TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(200,
                "Responder.RequestDenied", false);
        assertSso(model, request, response);

    }

    @Test
    public void testSsoInvalidAcsIndex() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        authnRequest.setAssertionConsumerServiceIndex(0);
        authnRequest.setAssertionConsumerServiceURL(TestConstants.BAD_PROVIDER); // make
                                                                                 // it
                                                                                 // invalid
                                                                                 // by
                                                                                 // specifying
                                                                                 // both
                                                                                 // index
                                                                                 // and
                                                                                 // URL
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(400,
                "BadRequest.AssertionIndex", false);

        assertSso(model, request, response);
    }

    @Test
    public void testSsoInvalidAcsUrl() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        authnRequest.setAssertionConsumerServiceIndex(null);
        authnRequest.setAssertionConsumerServiceURL(TestConstants.BAD_PROVIDER); // make
                                                                                 // it
                                                                                 // invalid
                                                                                 // by
                                                                                 // specifying
                                                                                 // bad
                                                                                 // URL
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(400,
                "BadRequest.AssertionMetadata", false);

        assertSso(model, request, response);
    }

    @Test
    public void testSsoInvalidAcsBinding() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        authnRequest.setAssertionConsumerServiceIndex(null);
        authnRequest.setAssertionConsumerServiceURL(null);
        authnRequest.setProtocolBinding("POST"); // make it invalid by
                                                 // specifying bad binding
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null,tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(400,
                "BadRequest.AssertionBinding", false);

        assertSso(model, request, response);
    }

    @Test
    public void testSsoInvalidID() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("",tenantId); // pass
                                                                            // invalid
                                                                            // ID
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUESTER));
    }

    @Test
    public void testSsoVersionTooHigh() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        authnRequest.setVersion(SAMLVersion.valueOf(3, 0)); // version too high
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.VERSION_MISMATCH));
        assertTrue(decodedSamlResponse
                .contains(OasisNames.REQUEST_VERSION_TOO_HIGH));
    }

    @Test
    public void testSsoVersionTooLow() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        authnRequest.setVersion(SAMLVersion.VERSION_11); // version too low
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.VERSION_MISMATCH));
        assertTrue(decodedSamlResponse
                .contains(OasisNames.REQUEST_VERSION_TOO_LOW));
    }

    @Test
    public void testBadIssueInstant() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        // request was "issued" long time ago
        DateTime wayBack = new DateTime().minusWeeks(42);
        authnRequest.setIssueInstant(wayBack);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUESTER));
    }

    @Test
    public void testEmailAsNameID() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        // request email as name id format
        NameIDPolicyBuilder nameIdPolicyBuilder = new NameIDPolicyBuilder();
        NameIDPolicy nameIdPolicy = nameIdPolicyBuilder.buildObject();
        nameIdPolicy.setFormat(OasisNames.EMAIL_ADDRESS);
        authnRequest.setNameIDPolicy(nameIdPolicy);

        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        true, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.SUCCESS));
        String expectedNameID = String.format(TestConstants.NAME_ID_FORMAT,
                OasisNames.IDENTITY_FORMAT_EMAIL_ADDRESS);
        expectedNameID = expectedNameID + TestConstants.EMAIL_ADDRESS_VALUE;
        log.debug("Looking for NameID " + expectedNameID);
        assertTrue(decodedSamlResponse.contains(expectedNameID));
    }

    @Test
    public void testBadNameIDFormat() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        // request bogus name id format
        NameIDPolicyBuilder nameIdPolicyBuilder = new NameIDPolicyBuilder();
        NameIDPolicy nameIdPolicy = nameIdPolicyBuilder.buildObject();
        nameIdPolicy.setFormat(TestConstants.BAD_NAMEID);
        authnRequest.setNameIDPolicy(nameIdPolicy);

        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUESTER));
        assertTrue(decodedSamlResponse
                .contains(OasisNames.INVALID_NAMEID_POLICY));
    }

    @Test
    public void testBadDestination() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        authnRequest.setDestination("http://bogus"); // set wrong destination
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(400,
                "BadRequest", false);

        assertSso(model, request, response);
    }

    @Test
    public void testBadIssuer() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        authnRequest.setIssuer(null); // no issuer
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(400,
                "BadRequest.Issuer", false);

        assertSso(model, request, response);
    }

    @Test
    public void testBadSignature() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        org.opensaml.xml.signature.Signature signature = (org.opensaml.xml.signature.Signature) Configuration
                .getBuilderFactory()
                .getBuilder(
                        org.opensaml.xml.signature.Signature.DEFAULT_ELEMENT_NAME)
                .buildObject(
                        org.opensaml.xml.signature.Signature.DEFAULT_ELEMENT_NAME);
        Credential signingCredential = getSigningCredential();
        signature.setSigningCredential(signingCredential);
        signature
                .setSignatureAlgorithm(SignatureConstants.ALGO_ID_SIGNATURE_RSA_SHA1);
        signature
                .setCanonicalizationAlgorithm(SignatureConstants.ALGO_ID_C14N_EXCL_OMIT_COMMENTS);
        signature.setKeyInfo(getKeyInfo(signingCredential));

        authnRequest.setSignature(signature); // signature should NOT be present
                                              // in AuthnRequest!
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUESTER));
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUEST_UNSUPPORTED));
    }

    // get some key info
    private KeyInfo getKeyInfo(Credential signingCredential) {
        return null;
    }

    // get some credential
    private Credential getSigningCredential() {
        BasicCredential credential = new BasicCredential();
        credential.setPrivateKey(privateKey);
        return credential;
    }

    @Test
    public void testBadScoping() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());

        XMLObjectBuilderFactory builderFactory = Configuration
                .getBuilderFactory();
        SAMLObjectBuilder<IDPEntry> idpEntryBuilder = (SAMLObjectBuilder<IDPEntry>) builderFactory
                .getBuilder(IDPEntry.DEFAULT_ELEMENT_NAME);
        IDPEntry idpEntry = idpEntryBuilder.buildObject();
        idpEntry.setProviderID(TestConstants.BAD_PROVIDER);
        idpEntry.setLoc(TestConstants.BAD_PROVIDER);

        SAMLObjectBuilder<IDPList> idpListBuilder = (SAMLObjectBuilder<IDPList>) builderFactory
                .getBuilder(IDPList.DEFAULT_ELEMENT_NAME);
        IDPList idpList = idpListBuilder.buildObject();
        idpList.getIDPEntrys().add(idpEntry);

        SAMLObjectBuilder<Scoping> scopingBuilder = (SAMLObjectBuilder<Scoping>) builderFactory
                .getBuilder(Scoping.DEFAULT_ELEMENT_NAME);
        Scoping scoping = scopingBuilder.buildObject();
        scoping.setIDPList(idpList);

        authnRequest.setScoping(scoping); // we use bad scoping here

        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUESTER));
        assertTrue(decodedSamlResponse.contains(OasisNames.NO_SUPPORTED_IDP));
    }

    @Test
    public void testSsoInvalidNoDefault() throws Exception {
        // make IDM config invalid by removing default assertion consumer
        // services
        SharedUtils.bootstrap(true); // alternative config from test resources

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        HttpServletResponse response = buildMockResponseErrorObject(400,
                "BadRequest.AssertionNoDefault", false);

        assertSso(model, request, response);
    }

    @Test
    public void testSsoInvalidMissingSignature() throws Exception {
        // load up config which requires signature from relying party
        SharedUtils.bootstrap(true); // alternative config from test resources

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        // make sure we specify assertion service
        authnRequest.setAssertionConsumerServiceIndex(0);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, null, null, sbRequestUrl,
                TestConstants.AUTHORIZATION, null, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        false, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.RESPONDER));
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUEST_DENIED));
    }

    @Test
    public void testUnpEntry() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, Shared.PASSWORD_ENTRY);

        String viewName = controller.ssoPasswordEntry(Locale.US, tenant, model, null, null);

        assertUnpEntry(model, viewName);
    }

    @Test
    public void testDefaultTenantUnpEntry() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        // determine default tenant endpoint
        authnRequest.setDestination(SharedUtils.getDefaultTenantEndpoint());
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, Shared.PASSWORD_ENTRY);

        String viewName = controller.ssoDefaultTenantPasswordEntry(Locale.US,
                model, null, null);

        // parse response
        assertUnpEntry(model, viewName);
    }


    @Test
    public void testGetBrandName() throws Exception {
        SharedUtils.bootstrap(false); // use real data
        assertEquals(ServerConfig.getTenantBrandName(tenant), controller.getBrandName(tenant));

    }

    @Test
    public void testSsoByCookie() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        String samlRequestParameter = SharedUtils.encodeRequest(authnRequest);
        String signature = SharedUtils.getSamlRequestSignature(privateKey, relayStateParameter,
                samlRequestParameter);

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // create a session
        String sessionId = null;
        String participantSessionId = null;
        Calendar calendar = new GregorianCalendar();
        calendar.add(Calendar.MINUTE, Shared.SESSION_LIFETIME_MINUTES);
        Date sessionEndTime = calendar.getTime();
        try {
            Session currentSession = new Session(new PrincipalId(
                    TestConstants.USER, TestConstants.DOMAIN), sessionEndTime,
                    AuthnMethod.KERBEROS);
            participantSessionId = currentSession
                    .ensureSessionParticipant(authnRequest.getIssuer()
                            .getValue());
            controller.getSessionManager().add(currentSession);
            sessionId = currentSession.getId();
        } catch (NoSuchAlgorithmException e) {
            throw new SamlServiceException(e);
        }

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, sigAlgParameter, signature,
                sbRequestUrl, TestConstants.AUTHORIZATION, sessionId, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        true, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.SUCCESS));
        // expect to find [reused] participant session id
        assertTrue(decodedSamlResponse.contains(participantSessionId));

        // cleanup session
        controller.getSessionManager().remove(sessionId);
    }

    @Test
    public void testLoginPageControllerSsoByCookie() throws Exception {
        // this is regression test for PR 964366 - it does same thing as testSsoByCookie but calls different controller method
        SharedUtils.bootstrap(false); // use real data

        StringBuffer sbRequestUrl = new StringBuffer();
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest("42", tenantId);
        sbRequestUrl.append(authnRequest.getDestination());
        Model model = new BindingAwareModelMap();

        String samlRequestParameter = SharedUtils.encodeRequest(authnRequest);
        String signature = SharedUtils.getSamlRequestSignature(privateKey, relayStateParameter,
                samlRequestParameter);

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, authnRequest,
                relayStateParameter, null, null, null);

        // create a session
        String sessionId = null;
        String participantSessionId = null;
        Calendar calendar = new GregorianCalendar();
        calendar.add(Calendar.MINUTE, Shared.SESSION_LIFETIME_MINUTES);
        Date sessionEndTime = calendar.getTime();
        try {
            Session currentSession = new Session(new PrincipalId(
                    TestConstants.USER, TestConstants.DOMAIN), sessionEndTime,
                    AuthnMethod.KERBEROS);
            participantSessionId = currentSession
                    .ensureSessionParticipant(authnRequest.getIssuer()
                            .getValue());
            controller.getSessionManager().add(currentSession);
            sessionId = currentSession.getId();
        } catch (NoSuchAlgorithmException e) {
            throw new SamlServiceException(e);
        }

        // build mock request object
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, sigAlgParameter, signature,
                sbRequestUrl, TestConstants.AUTHORIZATION, sessionId, tenantId);

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        true, null);

        assertNull(controller.ssoPasswordEntry(Locale.US, tenant, model, request, response));
        assertEquals(tenant, model.asMap().get("tenant"));
        assertNull(model.asMap().get("serverTime"));

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        assertTrue(decodedSamlResponse.contains(OasisNames.SUCCESS));
        // expect to find [reused] participant session id
        assertTrue(decodedSamlResponse.contains(participantSessionId));

        // cleanup session
        controller.getSessionManager().remove(sessionId);

        // ... and try again. We are simulating scenario where cookie exists but server session does not.
        // (PR 970577)
        // In this case, we should stay on login page
        request = SharedUtils.buildMockRequestObject(
                authnRequest, relayStateParameter, sigAlgParameter, signature,
                sbRequestUrl, TestConstants.AUTHORIZATION, sessionId, tenantId);

        String viewName = controller.ssoPasswordEntry(Locale.US, tenant, model, request, null);

        assertUnpEntry(model, viewName);
    }

    @Test
    public void testVcdSso() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        Model model = new BindingAwareModelMap();

        // import VCD SP data
        CasIdmClient idmClient = new CasIdmClient(SharedUtils.getIdmHostName());

        SharedUtils.importConfiguration(idmClient, tenant, "/vcd.xml");

        // construct request from a URL
        HttpServletRequest request = SharedUtils
                .buildMockRequestObjectFromUrl("http://schai-sule-vm1.vmware.com:7080/websso/SAML2/SSO/csp?SAMLRequest=lZLBTsMwDIbvPEWVe5uk68oWrUUDhEACMdHCgZubmS1Sm4w4LTw%2B3QYCLkgcLdmff%2Fnz4uy9a6MBPRlnCyYTwSK02q2N3RTssb6KZ%2BysPFkQdG26U8s%2BbO0DvvZIIVoSoQ%2Fj3IWz1HfoK%2FSD0fj4cFuwbQg7UpxLkcjsNJmKZJKpLJtw3bp%2BzZ3fcE07vufyqrrn0BogPug1iy5HuLEQDon2nBFDegsmpr7FeOhkMnRv4DHRrlOnYib4GzZEjlfLu9v0gBvZLLpyXuMhcsFeoCVk0c1lwSCVc4B5M8s3kMmXzOi8wUZv9EzLfGyhFRCZAb%2BHiHq8sRTAhoKlQqaxmMYyr%2BVciamaThKZi2cWrbwLTrv23Njj%2FXpvlQMypCx0SCpotY%2Bo0kSo5thE6rquV%2FHqvqpZ9PTlId17GM1YUsfL%2F83afS5m5VGUOiT2Pwl%2FA%2BBLJSv%2FI67DAGsI8G1vwX8GKD%2FL349TfgA%3D&SigAlg=http%3A%2F%2Fwww.w3.org%2F2000%2F09%2Fxmldsig%23rsa-sha1&Signature=IOuETBMw87DUqqpdJRkZYB2nG7kFxxlfsm8F8rzllkbrWzSxcMr0MfltnYSAnMTnlQEr%2Frj4Shkj9j6LPDLDueDhnLk%2FnQW2obfG6kXUG2MMGdkezX%2FsEzSFdld5QDpdeKB%2FaOaX7%2BFuJEmsRebmjYOuZJaTsGuuoEnu28oYuH4%3D");

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw, Shared.HTML_CONTENT_TYPE,
                        true, null);

        assertSso(model, request, response);

        // parse response
        String decodedSamlResponse = SharedUtils.extractResponse(log, sw);
        // we would fail because of issue instant
        assertTrue(decodedSamlResponse.contains(OasisNames.REQUESTER));
    }

    @Test
    public void testSsoSSLDummyQueryString() throws MalformedURLException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        assertEquals(SsoController.ssoSSLDummyQueryString(request), "csp");

        request.addParameter(Shared.RELYINGPARTY_ENTITYID, "ENTITYID");
        assertEquals(SsoController.ssoSSLDummyQueryString(request), "csp");

        request.addParameter("p1", "v1");
        assertEquals(SsoController.ssoSSLDummyQueryString(request), "p1=v1&csp");

        request.addParameter("p2", new String[] { "v2A", "v2B" });
        assertEquals(SsoController.ssoSSLDummyQueryString(request), "p1=v1&p2=v2A&p2=v2B&csp");

        // value-less parameters are allowed
        request.addParameter("p3", "");
        request.addParameter("p4", "");
        assertEquals(SsoController.ssoSSLDummyQueryString(request), "p1=v1&p2=v2A&p2=v2B&p3&p4&csp");
    }

    // all assertions needed for unpentry page
    private void assertUnpEntry(Model model, String viewName) {
        assertEquals(viewName, "unpentry");
        assertEquals(
                messageSource.getMessage("LoginForm.UserName", null, Locale.US),
                model.asMap().get("username"));
        assertEquals(
                messageSource.getMessage("LoginForm.UserName.Placeholder", null, Locale.US),
                model.asMap().get("username_placeholder"));
        assertEquals(
                messageSource.getMessage("LoginForm.Password", null, Locale.US),
                model.asMap().get("password"));
        assertEquals(
                messageSource.getMessage("LoginForm.Submit", null, Locale.US),
                model.asMap().get("submit"));
        assertEquals(Shared.PASSWORD_ENTRY, model.asMap().get("searchstring"));
        assertEquals(Shared.PASSWORD_SUPPLIED,
                model.asMap().get("replacestring"));
    }

    private HttpServletResponse buildMockResponseErrorObject(int errorCode,
            String messageCode, boolean includeAuthHeader) throws IOException {
        HttpServletResponse response = createMock(HttpServletResponse.class);
        Shared.addNoCacheHeader(response);
        if (includeAuthHeader) {
            response.addHeader(AUTH_HEADER, AUTH_VALUE);
        }
        String message = controller.getMessageSource().getMessage(messageCode, null,
                Locale.US);
        response.addHeader(Shared.RESPONSE_ERROR_HEADER, Shared.encodeString(message));
        response.sendError(
                errorCode,
                message);
        replay(response);
        return response;
    }
}