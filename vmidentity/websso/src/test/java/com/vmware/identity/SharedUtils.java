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
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
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
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.SignatureException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Conditions;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.impl.ConditionsBuilder;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.util.Base64;
import org.w3c.dom.Document;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IIdentityManager;
import com.vmware.identity.idm.IdmDataCreator;
import com.vmware.identity.idm.IdmDataRemover;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.ext.DelegableType;
import com.vmware.identity.saml.ext.RenewableType;
import com.vmware.identity.saml.ext.impl.DelegableTypeBuilder;
import com.vmware.identity.saml.ext.impl.RenewableTypeBuilder;
import com.vmware.identity.samlservice.DefaultSamlServiceFactory;
import com.vmware.identity.samlservice.IdmAccessor;
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

    private static final String IDM_HOST_NAME_PROPERTY = "idm.hostname";
    private static String _idmHostName = null;
    private static Registry _registry = null;

    // null in the beginning, true if "official" config has been committed last,
    // false if test config has been committed last
    private static Boolean lastData = null;

    // create sample SAML request
    // (expect callers to ensure that ServerConfig is loaded)
    public static AuthnRequest createSamlAuthnRequest(String id, int tenantId) {
        SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
        SamlService service = samlFactory.createSamlService(null, null, null,
                null, null);

        // get parameters
        String tenantName = ServerConfig.getTenant(tenantId);
        String rpName = ServerConfig.getRelyingParty(tenantName, 0);
        String destination = ServerConfig.getTenantEntityId(tenantName)
                .replace("/Metadata", "/SSO");
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);

        // create SAML request
        return service.createSamlAuthnRequest(id, destination, issuerUrl, null,
                null, null, null, null);
    }

    public static String encodeRequest(SignableSAMLObject samlObject)
            throws MarshallingException, IOException {
        SamlServiceFactory samlFactory = new DefaultSamlServiceFactory();
        SamlService service = samlFactory.createSamlService(null, null, null,
                null, null);
        return service.encodeSAMLObject(samlObject);
    }

    public static String getIdmHostName()
    {
        return _idmHostName;
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
        if (samlObject instanceof LogoutResponse) {
            expect(request.getParameter(Shared.SAML_RESPONSE_PARAMETER))
                    .andReturn(SharedUtils.encodeRequest(samlObject))
                    .anyTimes();
            expect(request.getParameter(Shared.SAML_REQUEST_PARAMETER))
                    .andReturn(null).anyTimes();
        } else {
            expect(request.getHeader(Shared.IWA_AUTH_REQUEST_HEADER)).andReturn(null).anyTimes();
            expect(request.getParameter(Shared.SAML_REQUEST_PARAMETER))
                    .andReturn(SharedUtils.encodeRequest(samlObject))
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
     * Loads up IDM configuration (specify testData=true to load test data)
     *
     * @param testData
     * @throws Exception
     */
    public static void bootstrap(boolean testData) throws Exception {
        if (lastData == null || testData != lastData) {
            // need to cleanup
            IdmDataCreator.setForceCleanup(true);
        }
        if (testData) {
            loadData();
        } else {
            IdmDataCreator.loadData();
        }

        String idmHost = getIdmHostName();

        CasIdmClient idmClient = new CasIdmClient(idmHost);
        IdmDataCreator.createData(idmClient);
        lastData = testData;
    }

    public static void removeSLOfromRelyingParties(String tenant) throws Exception {
        String idmHost = getIdmHostName();

        CasIdmClient idmClient = new CasIdmClient(idmHost);
        for (RelyingParty rp : idmClient.getRelyingParties(tenant)) {
            rp.setSingleLogoutServices(new ArrayList<ServiceEndpoint>());
            idmClient.setRelyingParty(tenant, rp);
        }
    }

    /**
     * Cleans up any tenant data created previously
     * @throws Exception
     */
    public static void cleanupTenant() throws Exception {
        CasIdmClient idmClient = new CasIdmClient(getIdmHostName());
        // delete tenants
        int i = 0;
        String tenantName = ServerConfig.getTenant(i);
        while (tenantName != null) {
            IdmDataRemover.addTenant(tenantName);
            i++;
            tenantName = ServerConfig.getTenant(i);
        }
        try {
            IdmDataRemover.removeData(idmClient);
        } catch (Exception e) {
            logger.debug("Caught exception while removing data "
                    + e.toString());
        }
    }

    /**
     * Determine SSO endpoint address for the default tenant
     *
     * @return
     */
    public static String getDefaultTenantEndpoint() {
        CasIdmClient idmClient = new CasIdmClient(getIdmHostName());
        IdmAccessor idmAccessor = new CasIdmAccessor(idmClient);
        idmAccessor.setDefaultTenant();
        return idmAccessor.getDefaultIdpSsoEndpoint();
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

    private static String readIdmHostName()
    {
        String idmHostName = null;
        logger.debug("SharedUtils.readIdmHostName called");

        InputStream is = getInputStream(CONFIG_FILE);
        Validate.notNull(is);

        try {
            Properties props = new Properties();
            props.load(is);
            idmHostName = props.getProperty(IDM_HOST_NAME_PROPERTY, null);
        } catch (IOException e) {
            logger.error("Unable to read prperties file", e);
            idmHostName = null;
        } finally {
            try {
                is.close();
            } catch (IOException e) {
            idmHostName = null;
            }
        }

        if ( ( idmHostName == null ) || ( idmHostName.length() == 0 ) )
        {
            idmHostName = Shared.IDM_HOSTNAME;
        }

        return idmHostName;
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
     * @param idmClient
     * @param tenant
     * @param resourceName
     * @throws FileNotFoundException
     * @throws ParserConfigurationException
     * @throws SAXException
     * @throws IOException
     * @throws Exception
     */
    public static void importConfiguration(CasIdmClient idmClient,
            String tenant, String resourceName) throws FileNotFoundException,
            ParserConfigurationException, SAXException, IOException, Exception {
        InputStream is = new FileInputStream(SsoControllerTest.class
                .getResource(resourceName).getFile());

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        DocumentBuilder docBuilder;
        docBuilder = factory.newDocumentBuilder();
        Document doc;
        doc = docBuilder.parse(is);

        idmClient.importTenantConfiguration(tenant, doc);
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
    public static AuthnRequest createSamlAuthnRequestWithOptions(String id, int tenantId) {
        AuthnRequest retval = createSamlAuthnRequest(id, tenantId);

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

}

class NullResolver implements EntityResolver {

    @Override
    public InputSource resolveEntity(String publicId, String systemId)
            throws SAXException, IOException {
        return new InputSource(new StringReader(""));
    }
}