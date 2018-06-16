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
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertTrue;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.SignatureException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.UUID;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SAMLVersion;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.Assertion;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Conditions;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.NameIDType;
import org.opensaml.saml2.core.Response;
import org.opensaml.saml2.core.impl.AssertionBuilder;
import org.opensaml.saml2.core.impl.AssertionMarshaller;
import org.opensaml.saml2.core.impl.ConditionsBuilder;
import org.opensaml.saml2.core.impl.IssuerBuilder;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.io.UnmarshallingException;
import org.opensaml.xml.util.Base64;
import org.springframework.context.support.ResourceBundleMessageSource;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.CredentialDescriptor;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.idm.client.SAMLNames;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.ext.DelegableType;
import com.vmware.identity.saml.ext.RenewableType;
import com.vmware.identity.saml.ext.impl.DelegableTypeBuilder;
import com.vmware.identity.saml.ext.impl.RenewableTypeBuilder;
import com.vmware.identity.samlservice.DefaultSamlServiceFactory;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.IdmAccessorFactory;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SamlService;
import com.vmware.identity.samlservice.SamlServiceFactory;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.impl.CasIdmAccessor;

/**
 * Shared test methods
 *
 */
public class SharedUtils {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(SharedUtils.class);

    private static final String CONFIG_FILE = "testconfig.properties";

    // create sample SAML request
    // (expect callers to ensure that ServerConfig is loaded)
    public static AuthnRequest createSamlAuthnRequest(String id, int tenantId, int rpId) {
        SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
        SamlService service = samlFactory.createSamlService(null, null, null,
                null, null);

        // get parameters
        String tenantName = ServerConfig.getTenant(tenantId);
        String rpName = ServerConfig.getRelyingParty(tenantName, rpId);
        String destination = ServerConfig.getTenantEntityId(tenantName).replace("/Metadata", "/SSO");
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);

        // create SAML request
        return service.createSamlAuthnRequest(id, destination, issuerUrl, null,
                null, null, null, null);
    }

    public static AuthnRequest createSamlAuthnRequest(String id, int tenantId) {
        return createSamlAuthnRequest(id, tenantId, 0);
    }

    public static Response createSamlAuthnResponse(String id, int tenantId, int rpId)
            throws UnmarshallingException, MarshallingException, NoSuchAlgorithmException {
        String tenant = ServerConfig.getTenant(tenantId);
        String issuer = ServerConfig.getTenantEntityId(tenant);
        String relyingParty = ServerConfig.getRelyingParty(tenant, rpId);
        String destination = ServerConfig.getRelyingPartyUrl(relyingParty);
        Document doc = generatSamlToken(ServerConfig.getTenantEntityId(tenant));

        SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
        SamlService service = samlFactory.createSamlService(null, null, null, issuer, null);
        // create SAML response
        return service.createSamlResponse(id, destination, null, null, null, doc);
    }

    public static String encodeRequest(SignableSAMLObject samlObject)
            throws MarshallingException, IOException {
        return encodeRequest(samlObject, true);
    }

    public static String encodeRequest(SignableSAMLObject signableSAMLObject, boolean doCompress)
            throws MarshallingException, IOException  {
        SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
        SamlService service = samlFactory.createSamlService(null, null, null, null, null);
        return service.encodeSAMLObject(signableSAMLObject, doCompress);
    }

    public static HttpServletRequest buildMockRequestObject(
            SignableSAMLObject samlObject, String relayStateParameter,
            String sigAlg, String signature, StringBuffer sbRequestUrl,
            String authorization, String sessionId, int tenantId)
            throws MarshallingException, IOException {

        Cookie[] expectedCookies = new Cookie[] {};
        String tenant = ServerConfig.getTenant(tenantId);

        if (sessionId != null) {
            expectedCookies = new Cookie[] { new Cookie(
                    Shared.getTenantSessionCookieName(tenant), sessionId) };
        }

        // build mock request object
        HttpServletRequest request = createMock(HttpServletRequest.class);
        expect(request.getCookies()).andReturn(expectedCookies).anyTimes();
        if (samlObject instanceof LogoutResponse || samlObject instanceof Response) {
            expect(request.getParameter(Shared.SAML_RESPONSE_PARAMETER))
                    .andReturn(encodeRequest(samlObject, false))
                    .anyTimes();
            expect(request.getParameter(Shared.SAML_REQUEST_PARAMETER))
                    .andReturn(null).anyTimes();
        } else {
            expect(request.getHeader(Shared.IWA_AUTH_REQUEST_HEADER)).andReturn(null).anyTimes();
            expect(request.getParameter(Shared.SAML_REQUEST_PARAMETER))
                    .andReturn(encodeRequest(samlObject))
                    .anyTimes();
            expect(request.getParameter(Shared.SAML_RESPONSE_PARAMETER))
                    .andReturn(null).anyTimes();
        }

        expect(request.getParameter(Shared.RELAY_STATE_PARAMETER)).andReturn(
                relayStateParameter).anyTimes();
        expect(request.getParameter(Shared.SIGNATURE_ALGORITHM_PARAMETER))
                .andReturn(sigAlg).anyTimes();
        expect(request.getParameter(Shared.SIGNATURE_PARAMETER)).andReturn(
                signature).anyTimes();
        expect(request.getRequestURL()).andReturn(sbRequestUrl).anyTimes();
        expect(request.getParameter(Shared.REQUEST_AUTH_PARAM)).andReturn(authorization)
                .anyTimes();
        String queryString = Shared.SAML_REQUEST_PARAMETER + "="
                + SharedUtils.encodeRequest(samlObject);
        if (relayStateParameter != null) {
            queryString = queryString + "&" + Shared.RELAY_STATE_PARAMETER
                    + "=" + relayStateParameter;
        }
        if (sigAlg != null) {
            queryString = queryString + "&"
                    + Shared.SIGNATURE_ALGORITHM_PARAMETER + "=" + sigAlg;
        }
        if (signature != null) {
            queryString = queryString + "&" + Shared.SIGNATURE_PARAMETER + "="
                    + signature;
        }
        expect(request.getQueryString()).andReturn(queryString).anyTimes();
        expect(request.getAuthType()).andReturn(null).anyTimes();

        replay(request);
        return request;
    }

    /**
     * Read XML as DOM.
     */
    public static Document readXml(InputStream is) throws SAXException,
            IOException, ParserConfigurationException {

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();

        dbf.setValidating(false);
        dbf.setIgnoringComments(false);
        dbf.setIgnoringElementContentWhitespace(true);
        dbf.setNamespaceAware(true);
        // dbf.setCoalescing(true);
        // dbf.setExpandEntityReferences(true);

        DocumentBuilder db = null;
        db = dbf.newDocumentBuilder();
        db.setEntityResolver(new NullResolver());

        // db.setErrorHandler( new MyErrorHandler());

        return db.parse(is);
    }

    // open input stream
    private static InputStream getInputStream(String filename) {
        return SharedUtils.class.getResourceAsStream("/" + filename);
    }

    /**
     * Loads up IDM configuration.
     *
     * @param testData
     * @throws Exception
     */
    public static void bootstrap() throws Exception {
        loadData();
    }

    /**
     * Load IDM data from test resource
     *
     * @throws Exception
     */
    private static void loadData() throws Exception {
        logger.debug("SharedUtils.loadData called");

        InputStream is = getInputStream(CONFIG_FILE);
        Validate.notNull(is);

        try {
            ServerConfig.initialize(is);
        } finally {
            is.close();
        }
    }

    /**
     * Create a RP issued logout request to be sent to IDP.
     *
     * @param id
     * @param sessionIndex
     * @return
     */
    public static LogoutRequest createSamlLogoutRequest(String id,
            String sessionIndex) {
        // get parameters
        String tenantName = ServerConfig.getTenant(0);
        String rpName = ServerConfig.getRelyingParty(tenantName, 0);
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);
        String destination = ServerConfig.getTenantEntityId(tenantName)
                .replace("/Metadata", "/SLO");

        SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
        SamlService service = samlFactory.createSamlService(null, null, null,
                issuerUrl, null);

        // create SAML request
        return service.createSamlLogoutRequest(id, destination,
                OasisNames.PERSISTENT, TestConstants.USER + "@"
                        + TestConstants.DOMAIN, sessionIndex);
    }

    /**
     * Log the url
     *
     * @param sbRequestUrl
     * @param authnRequest
     * @param relayStateParameter
     * @param signatureAlgorithm
     * @param signature
     * @param extra
     * @throws MarshallingException
     * @throws IOException
     */
    @SuppressWarnings("deprecation")
    public static void logUrl(IDiagnosticsLogger log, StringBuffer sbRequestUrl,
            SignableSAMLObject samlObject, String relayStateParameter,
            String signatureAlgorithm, String signature, String extra)
            throws MarshallingException, IOException {
        String samlParameterName = Shared.SAML_REQUEST_PARAMETER;
        if (samlObject instanceof LogoutResponse) {
            samlParameterName = Shared.SAML_RESPONSE_PARAMETER;
        }
        log.debug("We are going to GET URL "
                + sbRequestUrl.toString()
                + (samlObject != null ? "?"
                        + samlParameterName
                        + "="
                        + URLEncoder.encode(SharedUtils
                                .encodeRequest(samlObject)) : "")
                + (relayStateParameter != null ? ("&RelayState=" + URLEncoder
                        .encode(relayStateParameter)) : "")
                + (signatureAlgorithm != null ? ("&SigAlg=" + URLEncoder
                        .encode(signatureAlgorithm)) : "")
                + (signature != null ? ("&Signature=" + URLEncoder
                        .encode(signature)) : "")
                + (extra != null ? "&" + extra : ""));
    }

    /**
     * Create mock request based on the url
     *
     * @param string
     * @return
     * @throws MalformedURLException
     * @throws UnsupportedEncodingException
     */
    public static HttpServletRequest buildMockRequestObjectFromUrl(String string)
            throws MalformedURLException, UnsupportedEncodingException {
        Cookie[] expectedCookies = new Cookie[] {};

        URL url = new URL(string);
        Map<String, String> queryMap = getQueryMap(url.getQuery());

        String samlRequestParameter = getParameterFromQueryMap(queryMap,
                Shared.SAML_REQUEST_PARAMETER);
        String relayStateParameter = getParameterFromQueryMap(queryMap,
                Shared.RELAY_STATE_PARAMETER);
        String sigAlgParameter = getParameterFromQueryMap(queryMap,
                Shared.SIGNATURE_ALGORITHM_PARAMETER);
        String signatureParameter = getParameterFromQueryMap(queryMap,
                Shared.SIGNATURE_PARAMETER);
        StringBuffer sbRequestUrl = new StringBuffer();
        sbRequestUrl.append(string.replace("?" + url.getQuery(), ""));

        // build mock request object
        HttpServletRequest request = createMock(HttpServletRequest.class);
        expect(request.getCookies()).andReturn(expectedCookies).anyTimes();
        expect(request.getParameter(Shared.SAML_REQUEST_PARAMETER)).andReturn(
                samlRequestParameter).anyTimes();
        expect(request.getParameter(Shared.RELAY_STATE_PARAMETER)).andReturn(
                relayStateParameter).anyTimes();
        expect(request.getParameter(Shared.SIGNATURE_ALGORITHM_PARAMETER))
                .andReturn(sigAlgParameter).anyTimes();
        expect(request.getParameter(Shared.SIGNATURE_PARAMETER)).andReturn(
                signatureParameter).anyTimes();
        expect(request.getRequestURL()).andReturn(sbRequestUrl).anyTimes();
        expect(request.getParameter(Shared.REQUEST_AUTH_PARAM)).andReturn(
                TestConstants.AUTHORIZATION).anyTimes();
        String queryString = Shared.SAML_REQUEST_PARAMETER + "="
                + samlRequestParameter;
        if (relayStateParameter != null) {
            queryString = queryString + "&" + Shared.RELAY_STATE_PARAMETER
                    + "=" + relayStateParameter;
        }
        if (sigAlgParameter != null) {
            queryString = queryString + "&"
                    + Shared.SIGNATURE_ALGORITHM_PARAMETER + "="
                    + sigAlgParameter;
        }
        if (signatureParameter != null) {
            queryString = queryString + "&" + Shared.SIGNATURE_PARAMETER + "="
                    + signatureParameter;
        }
        expect(request.getQueryString()).andReturn(queryString).anyTimes();

        replay(request);
        return request;
    }

    private static String getParameterFromQueryMap(
            Map<String, String> queryMap, String parameter)
            throws UnsupportedEncodingException {
        String retval = queryMap.get(parameter);
        if (retval != null) {
            retval = URLDecoder.decode(retval, "UTF-8");
        }
        return retval;
    }

    private static Map<String, String> getQueryMap(String query) {
        String[] params = query.split("&");
        Map<String, String> map = new HashMap<String, String>();
        for (String param : params) {
            String name = param.split("=")[0];
            String value = param.split("=")[1];
            map.put(name, value);
        }
        return map;
    }

    /**
     * Creates Saml Logout response from RP 1
     *
     * @param inResponseTo
     * @return
     */
    public static LogoutResponse createSamlLogoutResponse(String inResponseTo) {
        // get parameters
        String tenantName = ServerConfig.getTenant(0);
        String rpName = ServerConfig.getRelyingParty(tenantName, 1);
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);
        String destination = ServerConfig.getTenantEntityId(tenantName)
                .replace("/Metadata", "/SLO");

        SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
        SamlService service = samlFactory.createSamlService(null, null, null,
                issuerUrl, null);

        // create SAML response
        return service.createSamlLogoutResponse(inResponseTo, destination,
                OasisNames.SUCCESS, null, "Success");
    }

    /**
     * Build mock HttpServletResponse object with a StringWriter
     *  which receives actual response
     * @param sw
     * @param contentType
     * @param expectCookie
     * @return
     * @throws IOException
     */
    public static HttpServletResponse buildMockResponseSuccessObject(StringWriter sw,
            String contentType, boolean expectCookie, String contentDispositionValue) throws IOException {
        HttpServletResponse response = createMock(HttpServletResponse.class);
        Shared.addNoCacheHeader(response);
        if (expectCookie) {
            response.addCookie(isA(Cookie.class));
        }
        response.setContentType(contentType);
        if (contentDispositionValue != null && !contentDispositionValue.isEmpty()) {
            response.setHeader("Content-Disposition", contentDispositionValue);
        }
        expect(response.getWriter()).andReturn(new PrintWriter(sw)).anyTimes();
        replay(response);
        return response;
    }

    /**
     * Build AuthnRequest with all our options we support
     *  Currently these would be notBefore, renewable and delegable conditions
     * @param id
     * @return
     */
    public static AuthnRequest createSamlAuthnRequestWithOptions(String id, int tenantId, int rpId) {
        AuthnRequest retval = createSamlAuthnRequest(id, tenantId, rpId);

        ConditionsBuilder conditionsBuilder = new ConditionsBuilder();
        Conditions conditions = conditionsBuilder.buildObject();
        DateTime dt = new DateTime();
        conditions.setNotBefore(dt);
        conditions.getConditions().add(createRenewable());
        conditions.getConditions().add(createDelegable());

        // add conditions object to the request
        retval.setConditions(conditions);

        return retval;
    }

    private static RenewableType createRenewable() {

        RenewableType proxy = new RenewableTypeBuilder()
           .buildObject();

        logger.info("Added Renewable condition");
        return proxy;
     }

    private static DelegableType createDelegable() {

        DelegableType proxy = new DelegableTypeBuilder()
           .buildObject();

        logger.info("Added Delegable condition");
        return proxy;
     }

    /**
     * Produce a string with signature
     *
     * @param privateKey
     * @param relayStateParameter
     * @param samlRequestParameter
     * @return
     * @throws NoSuchAlgorithmException
     * @throws InvalidKeyException
     * @throws UnsupportedEncodingException
     * @throws SignatureException
     */
    public static String getSamlRequestSignature(PrivateKey privateKey, String relayStateParameter,
            String samlRequestParameter)
            throws NoSuchAlgorithmException, InvalidKeyException,
            UnsupportedEncodingException, SignatureException {
        // produce signature
        SignatureAlgorithm algo = SignatureAlgorithm
                .getSignatureAlgorithmForURI(TestConstants.SIGNATURE_ALGORITHM);
        Signature sig = Signature.getInstance(algo.getAlgorithmName());
        sig.initSign(privateKey);

        String messageToSign = Shared.SAML_REQUEST_PARAMETER + "="
                + URLEncoder.encode(samlRequestParameter, "UTF-8") + "&"
                + Shared.RELAY_STATE_PARAMETER + "="
                + URLEncoder.encode(relayStateParameter, "UTF-8") + "&"
                + Shared.SIGNATURE_ALGORITHM_PARAMETER + "="
                + URLEncoder.encode(algo.toString(), "UTF-8");

        byte[] messageBytes = messageToSign.getBytes();
        sig.update(messageBytes);

        byte[] sigBytes = sig.sign();
        String signature = Shared.encodeBytes(sigBytes);
        return signature;
    }

    /**
     * Determine SSO endpoint address for the default tenant
     *
     * @return
     */
    public static String getDefaultTenantEndpoint() {
        return getTenantEndpoint(0);
    }

    public static String getDefaultTenant() {
        return ServerConfig.getTenant(0);
    }

    public static String getTenantName(int tenantId) {
        return ServerConfig.getTenant(tenantId);
    }

    public static String getTenantEndpoint(int tenantId) {
        String entityId = ServerConfig.getTenantEntityId(ServerConfig.getTenant(tenantId));
        // change to SSO endpoint
        return entityId.replace("/Metadata", "/SSO");
    }

    /**
     * Extract Saml Response which was written to a stream
     *
     * @param sw
     * @return
     */
    public static String extractResponse(IDiagnosticsLogger log, StringWriter sw) {
        String samlResponseField = "<input type=\"hidden\" name=\"SAMLResponse\" value=\"";

        String responseAsString = sw.toString();
        log.debug("Received response " + responseAsString);
        int index = responseAsString.indexOf(samlResponseField);
        assertTrue(index >= 0);
        int startIndex = index + samlResponseField.length();
        int endIndex = responseAsString.indexOf('\"', startIndex);
        assertTrue(endIndex >= 0);
        String encodedSamlResponse = responseAsString.substring(startIndex,
                endIndex);
        String decodedSamlResponse = new String(
                Base64.decode(encodedSamlResponse));
        return decodedSamlResponse;
    }

    private static Document generatSamlToken(String issuerString) throws MarshallingException {
        Assertion assertion = new AssertionBuilder().buildObject();
        assertion.setID(UUID.randomUUID().toString());
        assertion.setVersion(SAMLVersion.VERSION_20);
        assertion.setIssueInstant(new DateTime());
        IssuerBuilder b = new IssuerBuilder();
        Issuer issuer = b.buildObject();
        issuer.setFormat(NameIDType.ENTITY);
        issuer.setValue(issuerString);
        assertion.setIssuer(issuer);

        AssertionMarshaller marshaller = new AssertionMarshaller();
        Element assertionElement = marshaller.marshall(assertion);

        return assertionElement.getOwnerDocument();
    }

    private static CredentialDescriptor getSTSCredentialDescriptor() {
        String tenant = getDefaultTenant();
        return ServerConfig.getTenantCredentialDescriptor(tenant);
    }

    public static PrivateKey getSTSPrivateKey() throws Exception {
        CredentialDescriptor descriptor = getSTSCredentialDescriptor();
        String stsAlias =  descriptor.getAlias();
        Key key = getSTSKeyStore().getKey(stsAlias, descriptor.getPassword().toCharArray());
        return (PrivateKey) key;
    }

    static X509Certificate getSTSCertificate() throws Exception {
        CredentialDescriptor descriptor = getSTSCredentialDescriptor();
        String stsAlias =  descriptor.getAlias();
        return (X509Certificate) getSTSKeyStore().getCertificate(stsAlias);
    }

    static KeyStore getSTSKeyStore() throws Exception {
        CredentialDescriptor descriptor = getSTSCredentialDescriptor();
        KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
        InputStream is = new FileInputStream(SharedUtils.class.getResource("/" + descriptor.getFilename()).getFile());
        ks.load(is, getSTSCredentialDescriptor().getPassword().toCharArray());
        return ks;
    }

    public static List<Certificate> getSTSCertificates() throws Exception {
        List<Certificate> certificates = new ArrayList<>();
        certificates.add(getSTSCertificate());
        return certificates;
    }

    static IdmAccessor getIdmAccessor(int tenantId, int rpId1, int rpId2) throws Exception {
        CasIdmClient mockIdmClient = prepareMockIdmClient(tenantId);
        IdmAccessor idmAccessor = new CasIdmAccessor(mockIdmClient);
        idmAccessor.setTenant(ServerConfig.getTenant(tenantId));
        RelyingParty rp1 = prepareMockRP(mockIdmClient, tenantId, rpId1, false);
        RelyingParty rp2 = prepareMockRP(mockIdmClient, tenantId, rpId2, false);
        replay(rp1);
        replay(rp2);
        replay(mockIdmClient);
        return idmAccessor;
    }

    public static IdmAccessor getIdmAccessor(int tenantId, int rpId, boolean requestSigned) throws Exception {
        CasIdmClient mockIdmClient = prepareMockIdmClient(tenantId);
        IdmAccessor idmAccessor = new CasIdmAccessor(mockIdmClient);
        idmAccessor.setTenant(ServerConfig.getTenant(tenantId));
        RelyingParty rp = prepareMockRP(mockIdmClient, tenantId, rpId, requestSigned);
        replay(rp);
        replay(mockIdmClient);
        return idmAccessor;
    }

    static IdmAccessor getIdmAccessor(int tenantId, int rpId) throws Exception {
        return getIdmAccessor(tenantId, rpId, false);
    }

    static CasIdmClient prepareMockIdmClient(int tenantId) throws Exception {
        String tenant = ServerConfig.getTenant(tenantId);
        CasIdmClient mockIdmClient = createMock(CasIdmClient.class);
        AuthnPolicy mockAuthnPolicy = getMockAuthnPolicy();

        expect(mockIdmClient.getAuthnPolicy(tenant)).andReturn(mockAuthnPolicy).anyTimes();
        expect(mockIdmClient.getEntityID(tenant)).andReturn(getTenantEndpoint(tenantId)).anyTimes();
        PrivateKey key = getSTSPrivateKey();
        expect(mockIdmClient.getTenantPrivateKey(tenant)).andReturn(key).anyTimes();
        expect(mockIdmClient.getClockTolerance(tenant)).andReturn(ServerConfig.getTenantClockTolerance(tenant)).anyTimes();
        Document doc = generateSaml2Metadata(tenantId, 0);
        expect(mockIdmClient.getSsoSaml2Metadata(tenant)).andReturn(doc).anyTimes();
        expect(mockIdmClient.isTenantIDPSelectionEnabled(tenant)).andReturn(false).anyTimes();
        expect(mockIdmClient.getAllExternalIdpConfig(tenant, IDPConfig.IDP_PROTOCOL_SAML_2_0)).andReturn(Collections.<IDPConfig>emptyList());
        mockIdmClient.incrementGeneratedTokens(tenant);
        expectLastCall().anyTimes();
        expect(mockIdmClient.getBrandName(tenant)).andReturn(ServerConfig.getTenantBrandName(tenant)).anyTimes();
        expect(mockIdmClient.getLogonBannerTitle(tenant)).andReturn("TestLogonBannerTitle").anyTimes();
        expect(mockIdmClient.getLogonBannerContent(tenant)).andReturn("Test Logon Banner Content.").anyTimes();
        expect(mockIdmClient.getLogonBannerCheckboxFlag(tenant)).andReturn(true).anyTimes();
        expect(mockIdmClient.getServerSPN()).andReturn("TestServerSPN").anyTimes();
        expect(mockIdmClient.getDefaultTenant()).andReturn(getDefaultTenant()).anyTimes();
        return mockIdmClient;
    }

    static RelyingParty prepareMockRP(CasIdmClient mockIdmClient, int tenantId, int rpId, boolean requestSigned) throws Exception {
        String tenant = ServerConfig.getTenant(tenantId);
        String rpName = ServerConfig.getRelyingParty(tenant, rpId);
        String rpEntityId = getRelyingPartyEntityId(tenantId, rpId);

        Collection<ServiceEndpoint> sloServices = new HashSet<>();
        ServiceEndpoint slo = getSloService(tenantId, rpId, 0, OasisNames.HTTP_REDIRECT);
        sloServices.add(slo);

        Collection<AssertionConsumerService> assertionServices = new HashSet<>();
        assertionServices.add(getAssertionConsumerService(tenantId, rpId, 0, OasisNames.HTTP_REDIRECT));
        assertionServices.add(getAssertionConsumerService(tenantId, rpId, 0, OasisNames.HTTP_POST));
        RelyingParty mockRP = createMock(RelyingParty.class);
        String defaultAcs = ServerConfig.getDefaultAssertionConsumerService(rpName);

        expect(mockRP.getDefaultAssertionConsumerService()).andReturn(defaultAcs).anyTimes();
        expect(mockRP.getCertificate()).andReturn(getSTSCertificate()).anyTimes();
        expect(mockRP.getAssertionConsumerServices()).andReturn(assertionServices).anyTimes();
        expect(mockRP.getSingleLogoutServices()).andReturn(sloServices).anyTimes();
        expect(mockRP.isAuthnRequestsSigned()).andReturn(requestSigned).anyTimes();
        expect(mockIdmClient.getRelyingPartyByUrl(tenant, rpEntityId)).andReturn(mockRP).anyTimes();

        return mockRP;
    }

    private static ServiceEndpoint getSloService(int tenantId, int rpId, int sloId, String binding) {
        String sloName = getRelyingPartySloServiceName(tenantId, rpId, 0);
        String sloEndpoint = getRelyingPartySloEndpoint(sloName);
        return new ServiceEndpoint(sloName, sloEndpoint, binding);
    }

    static AssertionConsumerService getAssertionConsumerService(int tenantId, int rpId, int acsId, String binding) {
        String acsName = getRelyingPartyAcsName(tenantId, rpId, 0);
        String acsUrl = getRelyingPartyAcsUrl(acsName);
        return new AssertionConsumerService(acsName, binding, acsUrl);
    }

    public static String getRelyingPartyEntityId(int tenantId, int rpId) {
        return ServerConfig.getRelyingPartyUrl(ServerConfig.getRelyingParty(ServerConfig.getTenant(tenantId), rpId));
    }

    public static String getRelyingPartyAcsUrl(String acsName) {
        return ServerConfig.getServiceEndpoint(acsName);
    }

    public static String getRelyingPartyAcsName(int tenantId, int rpId, int acsId) {
        return ServerConfig.getAssertionConsumerService(
                ServerConfig.getRelyingParty(ServerConfig.getTenant(tenantId), rpId), acsId);
    }

    public static String getRelyingPartySloServiceName(int tenantId, int rpId, int sloId) {
        return ServerConfig.getSingleLogoutService(ServerConfig.getRelyingParty(ServerConfig.getTenant(tenantId), rpId),
                sloId);
    }

    public static String getRelyingPartySloEndpoint(String sloName) {
        return ServerConfig.getServiceEndpoint(sloName);
    }

    static AuthnPolicy getMockAuthnPolicy() {
        AuthnPolicy mockAuthnPolicy = createMock(AuthnPolicy.class);
        expect(mockAuthnPolicy.IsPasswordAuthEnabled()).andReturn(true);
        expect(mockAuthnPolicy.IsRsaSecureIDAuthnEnabled()).andReturn(true);
        expect(mockAuthnPolicy.IsWindowsAuthEnabled()).andReturn(true);
        expect(mockAuthnPolicy.IsTLSClientCertAuthnEnabled()).andReturn(true);
        replay(mockAuthnPolicy);
        return mockAuthnPolicy;
    }

    static IdmAccessorFactory getMockIdmAccessorFactory(int tenantId, int rp1, int rp2) throws Exception {
        IdmAccessorFactory mockIdmAccessorFactory = createMock(IdmAccessorFactory.class);
        IdmAccessor mockIdmAccessor = getIdmAccessor(tenantId, rp1, rp2);
        expect(mockIdmAccessorFactory.getIdmAccessor()).andReturn(mockIdmAccessor).anyTimes();
        replay(mockIdmAccessorFactory);
        return mockIdmAccessorFactory;
    }

    static IdmAccessorFactory getMockIdmAccessorFactory(int tenantId, int rpId, boolean requestSigned) throws Exception {
        IdmAccessorFactory mockIdmAccessorFactory = createMock(IdmAccessorFactory.class);
        IdmAccessor mockIdmAccessor = getIdmAccessor(tenantId, rpId, requestSigned);
        expect(mockIdmAccessorFactory.getIdmAccessor()).andReturn(mockIdmAccessor).anyTimes();
        replay(mockIdmAccessorFactory);
        return mockIdmAccessorFactory;
    }

    static IdmAccessorFactory getMockIdmAccessorFactory(int tenantId, int rpId) throws Exception {
        return getMockIdmAccessorFactory(tenantId, rpId, false);
    }

    static ResourceBundleMessageSource messageSource() {
        ResourceBundleMessageSource messageSource = new ResourceBundleMessageSource();
        messageSource.setBasename("messages");
        return messageSource;
    }

    private static Document generateSaml2Metadata(int tenantId, int rpId) throws Exception {
        DocumentBuilder docBuilder = DocumentBuilderFactory.newInstance()
                .newDocumentBuilder();
        Document doc = docBuilder.newDocument();

        Element entityEle =
                doc.createElementNS(SAMLNames.NS_NAME_SAML_METADATA,
                      SAMLNames.ENTDESCRIPTOR);
        entityEle.setAttribute(SAMLNames.NS_NAME_SAML_SAML,
                SAMLNames.NS_VAL_SAML_SAML);
        entityEle.setAttribute(SAMLNames.NS_NAME_SAML_VMWARE_ES,
                SAMLNames.NS_VAL_SAML_VMWARE_ES);

        doc.appendChild(entityEle);

        String stsEntityId = getTenantEndpoint(tenantId);
        entityEle.setAttribute(SAMLNames.ENTID, stsEntityId);

        Element idpssoD = doc.createElement(SAMLNames.IDPSSODESCRIPTOR);
        idpssoD.setAttribute(SAMLNames.PSE, IDPConfig.IDP_PROTOCOL_SAML_2_0);
        idpssoD.setAttribute(SAMLNames.WANTSIGNED, SAMLNames.FALSE);

        Element x509DataEle = null;
        List<Certificate> certs = getSTSCertificates();
        if (!certs.isEmpty()) {
            x509DataEle = doc.createElement(SAMLNames.DS_X509DATA);
            for (Certificate cert : certs) {
                Element x509CertificateEle = doc
                        .createElement(SAMLNames.DS_X509CERTIFICATE);
                X509Certificate x509Cert = (X509Certificate) cert;
                String base64Str = Base64.encodeBytes(x509Cert.getEncoded());
                Node certText = doc.createTextNode(base64Str);

                x509CertificateEle.appendChild(certText);
                x509DataEle.appendChild(x509CertificateEle);
            }
        }
        Element keyD = doc.createElement(SAMLNames.KEYDESCRIPTOR);
        keyD.setAttribute(SAMLNames.NS_NAME_SAML_DS,
                SAMLNames.NS_NAME_SAML_DIGTALSIG);
        keyD.setAttribute(SAMLNames.USE, SAMLNames.SIGNING);

        Element keyInfoEle = doc.createElement(SAMLNames.DS_KEYINFO);
        if (x509DataEle != null) {
            keyInfoEle.appendChild(x509DataEle);
        }
        keyD.appendChild(keyInfoEle);
        idpssoD.appendChild(keyD);

        Element sloEle = doc.createElement(SAMLNames.SINGLELOGOUTSERVICE);
        ServiceEndpoint service = getSloService(tenantId, rpId, 0, OasisNames.HTTP_REDIRECT);
        sloEle.setAttribute(SAMLNames.BINDING, service.getBinding());
        sloEle.setAttribute(SAMLNames.LOCATION, service.getEndpoint());
        idpssoD.appendChild(sloEle);

        Element nameIdEle = doc.createElement(SAMLNames.NAMEIDFORMAT);
        nameIdEle.appendChild(doc.createTextNode(SAMLNames.IDFORMAT_VAL_EMAILADD.toString()));
        idpssoD.appendChild(nameIdEle);
        nameIdEle = doc.createElement(SAMLNames.NAMEIDFORMAT);
        nameIdEle.appendChild(doc.createTextNode(SAMLNames.IDFORMAT_VAL_UPN.toString()));
        idpssoD.appendChild(nameIdEle);

        String ssoEndpoint = stsEntityId.replaceAll("/Metadata/", "/SSO/");
        Element ssos = doc.createElement(SAMLNames.SSOS);
        ssos.setAttribute(SAMLNames.BINDING, SAMLNames.HTTP_REDIRECT_BINDING);
        ssos.setAttribute(SAMLNames.LOCATION, ssoEndpoint);

        idpssoD.appendChild(ssos);

        entityEle.appendChild(idpssoD);

        return doc;
    }
}

class NullResolver implements EntityResolver {

    @Override
    public InputSource resolveEntity(String publicId, String systemId)
            throws SAXException, IOException {
        return new InputSource(new StringReader(""));
    }
}