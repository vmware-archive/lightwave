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
package com.vmware.identity.samlservice.impl;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URLEncoder;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Hashtable;
import java.util.List;
import java.util.Locale;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.XMLConstants;
import javax.xml.namespace.QName;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.Validate;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.conn.ssl.SSLContextBuilder;
import org.apache.http.conn.ssl.TrustSelfSignedStrategy;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.joda.time.DateTime;
import org.opensaml.common.SAMLVersion;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.common.binding.BasicSAMLMessageContext;
import org.opensaml.common.impl.SecureRandomIdentifierGenerator;
import org.opensaml.saml2.binding.decoding.HTTPRedirectDeflateDecoder;
import org.opensaml.saml2.core.Assertion;
import org.opensaml.saml2.core.AuthnContextClassRef;
import org.opensaml.saml2.core.AuthnContextComparisonTypeEnumeration;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.NameID;
import org.opensaml.saml2.core.NameIDPolicy;
import org.opensaml.saml2.core.RequestedAuthnContext;
import org.opensaml.saml2.core.Response;
import org.opensaml.saml2.core.SessionIndex;
import org.opensaml.saml2.core.Status;
import org.opensaml.saml2.core.StatusCode;
import org.opensaml.saml2.core.StatusMessage;
import org.opensaml.saml2.core.impl.AuthnRequestBuilder;
import org.opensaml.saml2.core.impl.LogoutRequestBuilder;
import org.opensaml.saml2.core.impl.NameIDPolicyBuilder;
import org.opensaml.saml2.core.impl.SessionIndexBuilder;
import org.opensaml.ws.message.decoder.MessageDecodingException;
import org.opensaml.ws.transport.http.HttpServletRequestAdapter;
import org.opensaml.xml.Configuration;
import org.opensaml.xml.io.Marshaller;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.io.Unmarshaller;
import org.opensaml.xml.io.UnmarshallerFactory;
import org.opensaml.xml.io.UnmarshallingException;
import org.opensaml.xml.parse.BasicParserPool;
import org.opensaml.xml.security.SecurityException;
import org.opensaml.xml.util.Base64;
import org.opensaml.xml.util.XMLHelper;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.client.SAMLNames;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.LogoutState;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SamlService;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.websso.client.AssertionConsumerService;
import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.IDPConfigurationFactory;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SPConfiguration;

public class SamlServiceImpl implements SamlService {

    private static final String FEATURE_LOAD_EXTERNAL_DTD = "http://apache.org/xml/features/nonvalidating/load-external-dtd";
    private static final String FEATURE_EXTERNAL_PARAMETER_ENTITIES = "http://xml.org/sax/features/external-parameter-entities";
    private static final String FEATURE_EXTERNAL_GENERAL_ENTITIES = "http://xml.org/sax/features/external-general-entities";

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(SamlServiceImpl.class);
    static {
        System.setProperty("elementAttributeLimit", "20");
        System.setProperty("entityExpansionLimit", "100");
    }

    private String issuer;
    private CertPath certPath;
    private PrivateKey privateKey;
    private SignatureAlgorithm signAlgorithm;
    private SignatureAlgorithm checkAlgorithm;
    private final SecureRandomIdentifierGenerator generator;
    private final RelaxedURIComparator comparator;

    public SamlServiceImpl() throws NoSuchAlgorithmException {
        this.generator = new SecureRandomIdentifierGenerator();
        this.comparator = new RelaxedURIComparator();
    }

    @Override
	public void verifySignature(String message, String signature)
            throws IllegalStateException {

        Validate.notNull(this.getCheckAlgorithm());
        Validate.notNull(this.getCertPath());

        boolean verifies = false;
        try {
            /* create a Signature object and initialize it with the public key */
            Signature sig = Signature.getInstance(this.getCheckAlgorithm()
                    .getAlgorithmName());
            X509Certificate[] chain = this.getCertPath().getCertificates()
                    .toArray(new X509Certificate[0]);

            for (int i = 0; i < chain.length && !verifies; i++) {
                sig.initVerify(chain[i].getPublicKey());

                /* add buffer to verify */
                byte[] buffer = message.getBytes("UTF-8");
                sig.update(buffer);

                byte[] sigToVerify = Base64.decode(signature);

				verifies = sig.verify(sigToVerify);
				if (!verifies) {
					log.error("Unable to verify the signature, message "
							+ message + ", sigAlg "
							+ this.getCheckAlgorithm().getAlgorithmName()
							+ ", signature " + signature);
				}
			}

			log.debug("signature verifies: {}", verifies);
		} catch (Exception e) {
			log.error("Caught exception while verifying signature, message: "
					+ message + ", sigAlg "
					+ this.getCheckAlgorithm().getAlgorithmName()
					+ ", signature " + signature);
			log.error("Exception is: {}", e.toString());
			throw new IllegalStateException(e);
		}
        if (!verifies) {
            throw new IllegalStateException("Signature verification failed.");
        }
    }

    @Override
	public AuthnRequest decodeSamlAuthnRequest(HttpServletRequest request)
            throws MessageDecodingException, SecurityException {
        log.debug("Decoding SAML AuthnRequest: {}" , request);
        return (AuthnRequest) decodeSamlRequest(request);
    }

    @Override
	public Response createSamlResponse(String inResponseTo, String where,
            String status, String substatus, String message, Document token)
            throws UnmarshallingException {
        log.debug("Creating SAML Response in response to:" + inResponseTo
                + ", destination: " + where + ", myId:" + this.getIssuer());
        log.debug("Creating SAML Response status:" + status + ", substatus: "
                + substatus + ", message:" + message);
        log.debug("Creating SAML Response document:" + token);

        if (where == null) {
            // ACS is not validated. We can not generate SAML response to the
            // requester.
            return null;
        }
        Validate.notNull(this.getIssuer(), "Issuer");

        if (status == null) {
            // assume success
            status = OasisNames.SUCCESS;
            Validate.notNull(token, "token");
        }

        Response response = create(Response.class,
                Response.DEFAULT_ELEMENT_NAME);
        response.setID(generator.generateIdentifier());
        if (inResponseTo != null) {
            response.setInResponseTo(inResponseTo);
        }
        response.setVersion(SAMLVersion.VERSION_20);
        DateTime now = new DateTime();
        response.setIssueInstant(now);
        response.setDestination(where);

        response.setIssuer(spawnIssuer(this.getIssuer()));

        Status samlStatus = spawnStatus(status, substatus, message);

        response.setStatus(samlStatus);
        if (token != null) {
            Element root = token.getDocumentElement();

            // get appropriate unmarshaller
            UnmarshallerFactory unmarshallerFactory = Configuration
                    .getUnmarshallerFactory();
            Unmarshaller unmarshaller = unmarshallerFactory
                    .getUnmarshaller(root);

            // unmarshall using root document element which needs to be
            // Assertion
            Assertion assertion = (Assertion) unmarshaller.unmarshall(root);

            response.getAssertions().add(assertion);
        }

        return response;
    }

    private Status spawnStatus(String status, String substatus, String message) {
        StatusCode statusCodeElement = create(StatusCode.class,
                StatusCode.DEFAULT_ELEMENT_NAME);
        statusCodeElement.setValue(status);
        if (substatus != null) {
            StatusCode statusCodeElement2 = create(StatusCode.class,
                    StatusCode.DEFAULT_ELEMENT_NAME);
            statusCodeElement2.setValue(substatus);
            statusCodeElement.setStatusCode(statusCodeElement2);
        }

        Status samlStatus = create(Status.class, Status.DEFAULT_ELEMENT_NAME);
        samlStatus.setStatusCode(statusCodeElement);

        if (message != null) {
            StatusMessage statusMessage = create(StatusMessage.class,
                    StatusMessage.DEFAULT_ELEMENT_NAME);
            statusMessage.setMessage(message);
            samlStatus.setStatusMessage(statusMessage);
        }
        return samlStatus;
    }

    private Issuer spawnIssuer(String myId) {
        Issuer result = null;
        result = create(Issuer.class, Issuer.DEFAULT_ELEMENT_NAME);
        result.setValue(myId);
        //result.setFormat(OasisNames.ENTITY);

        return result;
    }

    private NameID spawnNameID(String format, String nameIDValue) {
        NameID result = null;
        result = create(NameID.class, NameID.DEFAULT_ELEMENT_NAME);
        result.setValue(nameIDValue);
        result.setFormat(format);

        return result;
    }

    @Override
	public String buildPostResponseForm(Response response, String relayState,
            String where) {
        log.debug("Building HTTP POST response form for relayState :"
                + relayState + ", destination: " + where + ", response:"
                + response.toString());
        Validate.notNull(where);
        Validate.notNull(response);

        StringWriter sw = new StringWriter();
        // We must build our representation to put into the html form
        Marshaller marshaller = org.opensaml.Configuration
                .getMarshallerFactory().getMarshaller(response);
        org.w3c.dom.Element authDOM;
        try {
            authDOM = marshaller.marshall(response);
        } catch (MarshallingException e) {
            log.debug("Caught exception " + e.toString());
            return null;
        }
        StringWriter rspWrt = new StringWriter();
        XMLHelper.writeNode(authDOM, rspWrt);
        String messageXML = rspWrt.toString();

        try
        {
            String samlResponseParameter = Shared
                    .encodeBytes(messageXML.getBytes("UTF-8"));
            sw.write("<form method=\"post\" action=\""
                    + StringEscapeUtils.escapeHtml(where)
                    + "\" id=\"SamlPostForm\"> <input type=\"hidden\" name=\"SAMLResponse\" value=\""
                    + StringEscapeUtils.escapeHtml(samlResponseParameter) + "\" />");
        }
        catch (UnsupportedEncodingException e)
        {
            log.debug("Caught exception " + e.toString());

            return null;
        }

        if (relayState != null) {
            sw.write("<input type=\"hidden\" name=\"RelayState\" value=\""
                    + StringEscapeUtils.escapeHtml(relayState) + "\" />");
        }
        sw.write("<input type=\"submit\" value=\"Submit\" style=\"display:none;\" /> </form>");
        String resultForm = sw.toString();

        StringWriter sw2 = new StringWriter();
        sw2.write("<html> <head> <script language=\"JavaScript\" type=\"text/javascript\">function load(){ document.getElementById('SamlPostForm').submit(); }</script> </head> <body onload=\"load()\">");
        sw2.write(resultForm);
        sw2.write("</body></html>");
        String result = sw2.toString();

        return result;
    }

    /**
     * Create object using OpenSAML's builder system.
     */
    // cast to SAMLObjectBuilder<T> is caller's choice
    @SuppressWarnings({ "unchecked", "rawtypes" })
    private <T> T create(Class<T> cls, QName qname) {
        return (T) Configuration.getBuilderFactory()
                .getBuilder(qname).buildObject(qname);
    }

    /**
     * @return the issuer
     */
    public String getIssuer() {
        return issuer;
    }

    /**
     * @param issuer
     *            the issuer to set
     */
    public void setIssuer(String issuer) {
        this.issuer = issuer;
    }

    /**
     * @return the certPath
     */
    public CertPath getCertPath() {
        return certPath;
    }

    /**
     * @param certPath
     *            the certPath to set
     */
    public void setCertPath(CertPath certPath) {
        this.certPath = certPath;
    }

    /**
     * @return the checkAlgorithm
     */
    public SignatureAlgorithm getCheckAlgorithm() {
        return checkAlgorithm;
    }

    /**
     * @param checkAlgorithm
     *            the checkAlgorithm to set
     */
    public void setCheckAlgorithm(SignatureAlgorithm checkAlgorithm) {
        this.checkAlgorithm = checkAlgorithm;
    }

    /**
     * Create SAML request (expect callers to ensure that ServerConfig is
     * loaded)
     *
     * @param id
     * @param destination
     * @param providerEntityID
     * @param nameIDFormat
     * @param assertionConsumerServiceIndex
     * @param attributeConsumerServiceIndex
     * @param forceAuthn
     * @param isPassive
     * @return
     */
    @Override
	public AuthnRequest createSamlAuthnRequest(String id, String destination,
            String providerEntityID, String nameIDFormat,
            Integer assertionConsumerServiceIndex,
            Integer attributeConsumerServiceIndex, Boolean forceAuthn,
            Boolean isPassive) {
        if (id == null) {
            id = generator.generateIdentifier();
        }
        log.debug("Building SAML AuthnRequest for id " + id);
        log.debug("Building SAML AuthnRequest for destination " + destination);
        log.debug("Building SAML AuthnRequest for providerEntityID "
                + providerEntityID);
        if (nameIDFormat == null) {
            nameIDFormat = OasisNames.PERSISTENT;
        }
        log.debug("Building SAML AuthnRequest for nameIDFormat " + nameIDFormat);
        log.debug("Building SAML AuthnRequest for assertionConsumerServiceIndex "
                + assertionConsumerServiceIndex);
        log.debug("Building SAML AuthnRequest for attributeConsumerServiceIndex "
                + attributeConsumerServiceIndex);
        log.debug("Building SAML AuthnRequest for forceAuthn " + forceAuthn);
        log.debug("Building SAML AuthnRequest for isPassive " + isPassive);

        // Create NameIDPolicy
        NameIDPolicyBuilder nameIdPolicyBuilder = new NameIDPolicyBuilder();
        NameIDPolicy nameIdPolicy = nameIdPolicyBuilder.buildObject();
        nameIdPolicy.setFormat(nameIDFormat);
        nameIdPolicy.setSPNameQualifier(destination);
        nameIdPolicy.setAllowCreate(true);

        DateTime issueInstant = new DateTime();
        AuthnRequestBuilder authRequestBuilder = new AuthnRequestBuilder();
        AuthnRequest authRequest = authRequestBuilder.buildObject(
                OasisNames.PROTOCOL, "AuthnRequest", "samlp");
        authRequest.setForceAuthn(forceAuthn);
        authRequest.setIsPassive(isPassive);
        authRequest.setIssueInstant(issueInstant);
        authRequest.setIssuer(spawnIssuer(providerEntityID));
        authRequest.setNameIDPolicy(nameIdPolicy);
        authRequest.setRequestedAuthnContext(spawnRequestedAuthnContext());
        authRequest.setID(id);
        authRequest.setVersion(SAMLVersion.VERSION_20);
        authRequest.setProviderName("Demo Provider");
        authRequest.setDestination(destination);
        authRequest
                .setAssertionConsumerServiceIndex(assertionConsumerServiceIndex);
        authRequest
                .setAttributeConsumingServiceIndex(attributeConsumerServiceIndex);

        return authRequest;
    }

    private RequestedAuthnContext spawnRequestedAuthnContext() {
        // Create AuthnContextClassRef
        AuthnContextClassRef authnContextClassRef = create(AuthnContextClassRef.class,
                AuthnContextClassRef.DEFAULT_ELEMENT_NAME);
        authnContextClassRef
                .setAuthnContextClassRef(OasisNames.PASSWORD_PROTECTED_TRANSPORT);

        RequestedAuthnContext result = null;
        result = create(RequestedAuthnContext.class, RequestedAuthnContext.DEFAULT_ELEMENT_NAME);
        result.setComparison(AuthnContextComparisonTypeEnumeration.EXACT);
        result.getAuthnContextClassRefs().add(
                authnContextClassRef);

        return result;
    }

    @Override
	public String encodeSAMLObject(SignableSAMLObject signableSAMLObject)
            throws MarshallingException, IOException {
        log.debug("Encoding SAML Object " + signableSAMLObject);

        // Now we must build our representation to put into the html form to be
        // submitted to the idp
        Marshaller marshaller = org.opensaml.Configuration
                .getMarshallerFactory().getMarshaller(signableSAMLObject);
        org.w3c.dom.Element authDOM = marshaller.marshall(signableSAMLObject);
        StringWriter rspWrt = new StringWriter();
        XMLHelper.writeNode(authDOM, rspWrt);
        String messageXML = rspWrt.toString();

        Deflater deflater = new Deflater(Deflater.DEFLATED, true);
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        DeflaterOutputStream deflaterOutputStream = new DeflaterOutputStream(
                byteArrayOutputStream, deflater);
        deflaterOutputStream.write(messageXML.getBytes("UTF-8"));
        deflaterOutputStream.close();
        String samlRequestParameter = Shared.encodeBytes(byteArrayOutputStream
                .toByteArray());
        return samlRequestParameter;
    }

    /**
     * @return the privateKey
     */
    public PrivateKey getPrivateKey() {
        return privateKey;
    }

    /**
     * @param privateKey
     *            the privateKey to set
     */
    public void setPrivateKey(PrivateKey privateKey) {
        this.privateKey = privateKey;
    }

    /**
     * @return the signAlgorithm
     */
    public SignatureAlgorithm getSignAlgorithm() {
        return signAlgorithm;
    }

    /**
     * @param signAlgorithm
     *            the signAlgorithm to set
     */
    public void setSignAlgorithm(SignatureAlgorithm signAlgorithm) {
        this.signAlgorithm = signAlgorithm;
    }

    @Override
	public String signMessage(String message) throws IllegalStateException {
        Validate.notNull(this.getSignAlgorithm());
        Validate.notNull(this.getPrivateKey());

        try {
            Signature sig = Signature.getInstance(
                    this.getSignAlgorithm().getAlgorithmName());
            sig.initSign(privateKey);

            byte[] messageBytes = message.getBytes("UTF-8");
            sig.update(messageBytes);

            byte[] sigBytes = sig.sign();
            String signature = Shared.encodeBytes(sigBytes);
			if (signature == null || signature.isEmpty()) {
				log.debug("Invalid signature - either null or empty. ");
			}

			return signature;
		} catch (Exception e) {
			log.error("Caught exception while signing  message " + message
					+ ", sigAlg " + this.getSignAlgorithm().getAlgorithmName());
            throw new IllegalStateException(e);
        }
    }

    @Override
	public SignableSAMLObject decodeSamlRequest(HttpServletRequest request)
            throws MessageDecodingException, SecurityException {
        log.debug("Decoding SAML object: {}", request);
        Validate.notNull(request);

        Hashtable<String,Boolean> features = new Hashtable<String,Boolean>();
        Hashtable<String,Object> attributes = new Hashtable<String,Object>();

        BasicParserPool ParserPool = new BasicParserPool();

        features.put(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        ParserPool.setBuilderFeatures(features);


        attributes.put(FEATURE_EXTERNAL_GENERAL_ENTITIES, false);
        attributes.put(FEATURE_EXTERNAL_PARAMETER_ENTITIES, false);
        attributes.put(FEATURE_LOAD_EXTERNAL_DTD, false);
        ParserPool.setBuilderAttributes(attributes);

        final HTTPRedirectDeflateDecoder decode = new HTTPRedirectDeflateDecoder(
                ParserPool);
        decode.setURIComparator(comparator);
        final HttpServletRequestAdapter adapter = new HttpServletRequestAdapter(
                request);
        @SuppressWarnings("rawtypes")
        final BasicSAMLMessageContext context = new BasicSAMLMessageContext();
        context.setInboundMessageTransport(adapter);
        decode.decode(context);
        // Save the SAML Request as a SAML Object
        return (SignableSAMLObject) context.getInboundMessage();
    }

    @Override
	public LogoutRequest createSamlLogoutRequest(String id, String where,
            String nameIDFormat, String nameID, String sessionIndex) {
        if (id == null) {
            id = generator.generateIdentifier();
        }
        log.debug("Building SAML LogoutRequest for id " + id);
        log.debug("Building SAML LogoutRequest for destination " + where);
        log.debug("Building SAML LogoutRequest for nameIDFormat " + nameIDFormat);
        log.debug("Building SAML LogoutRequest for nameID " + nameID);
        log.debug("Building SAML LogoutRequest for sessionIndex " + sessionIndex);

        Validate.notNull(where);
        Validate.notNull(nameIDFormat);
        Validate.notNull(nameID);
        Validate.notNull(sessionIndex);
        Validate.notNull(this.getIssuer());

        // Create SessionIndex
        SessionIndexBuilder sessionIndexBuilder = new SessionIndexBuilder();
        SessionIndex sessionIndexElement = sessionIndexBuilder.buildObject(
                OasisNames.PROTOCOL, "SessionIndex", "samlp");
        sessionIndexElement.setSessionIndex(sessionIndex);

        DateTime issueInstant = new DateTime();
        LogoutRequestBuilder logoutRequestBuilder = new LogoutRequestBuilder();
        LogoutRequest logoutRequest = logoutRequestBuilder.buildObject(
                OasisNames.PROTOCOL, "LogoutRequest", "samlp");
        logoutRequest.setDestination(where);
        logoutRequest.setIssueInstant(issueInstant);
        logoutRequest.setIssuer(spawnIssuer(this.getIssuer()));
        logoutRequest.setNameID(
                spawnNameID(nameIDFormat, nameID));
        logoutRequest.getSessionIndexes().add(sessionIndexElement);
        logoutRequest.setID(id);
        logoutRequest.setVersion(SAMLVersion.VERSION_20);

        return logoutRequest;
    }

    @Override
	public LogoutResponse createSamlLogoutResponse(String inResponseTo,
            String where, String status, String substatus, String message) {
        log.debug("Creating SAML Logout Response in response to:" + inResponseTo
                + ", destination: " + where + ", myId:" + this.getIssuer());
        log.debug("Creating SAML Logout Response status:" + status + ", substatus: "
                + substatus + ", message:" + message);
        Validate.notNull(where);
        Validate.notNull(this.getIssuer());

        if (status == null) {
            // assume success
            status = OasisNames.SUCCESS;
        }

        LogoutResponse response = create(LogoutResponse.class,
                LogoutResponse.DEFAULT_ELEMENT_NAME);
        response.setID(generator.generateIdentifier());
        if (inResponseTo != null) {
            response.setInResponseTo(inResponseTo);
        }
        response.setVersion(SAMLVersion.VERSION_20);
        DateTime now = new DateTime();
        response.setIssueInstant(now);
        response.setDestination(where);

        response.setIssuer(spawnIssuer(this.getIssuer()));

        Status samlStatus = spawnStatus(status, substatus, message);

        response.setStatus(samlStatus);
        return response;
    }

    @Override
	public String generateRedirectUrlQueryStringParameters(String samlRequest,
            String samlResponse, String relayState, String sigAlg, String signature) {
        StringBuilder sb = new StringBuilder();
        appendOptionalParameter(sb, Shared.SAML_REQUEST_PARAMETER, samlRequest, false);
        appendOptionalParameter(sb, Shared.SAML_RESPONSE_PARAMETER, samlResponse, false);
        appendOptionalParameter(sb, Shared.RELAY_STATE_PARAMETER, relayState, false);
        appendOptionalParameter(sb, Shared.SIGNATURE_ALGORITHM_PARAMETER, sigAlg, false);
        appendOptionalParameter(sb, Shared.SIGNATURE_PARAMETER, signature, false);
        return sb.toString();
    }

    static public void initMetadataSettings(MetadataSettings settings, IDPConfig idpConfig, IdmAccessor accessor) {
        IDPConfiguration idpConfiguration =
                 SamlServiceImpl.generateIDPConfiguration(idpConfig);
        settings.addIDPConfiguration(idpConfiguration);
        SPConfiguration spConfig = SamlServiceImpl.generateSPConfiguration(accessor);
        settings.addSPConfiguration(spConfig);
     }

    static public IDPConfiguration generateIDPConfiguration(IDPConfig idmIdpConfig) {

        Validate.notNull(idmIdpConfig, "IDPConfig for the external IDP is not intialized");

        ArrayList<String> nameIDFormatStrs = null;
        if (idmIdpConfig.getNameIDFormats() != null) {
            nameIDFormatStrs = new ArrayList<String>(idmIdpConfig.getNameIDFormats());
        }

        //Creating a com.vmware.identity.websso.client.SingleSignOnService list
        Validate.notNull(idmIdpConfig.getSsoServices(), "IDPConfig SsoServices is not initialized");
        ArrayList<ServiceEndpoint> idm_sso_services = new ArrayList<ServiceEndpoint>(idmIdpConfig.getSsoServices());
        ArrayList<com.vmware.identity.websso.client.SingleSignOnService> ssolib_sso_services =
                    new ArrayList<com.vmware.identity.websso.client.SingleSignOnService>();

        for (ServiceEndpoint s : idm_sso_services) {
            com.vmware.identity.websso.client.SingleSignOnService ssolib_sso_service =
                        new com.vmware.identity.websso.client.SingleSignOnService(s.getEndpoint(), s.getBinding());
            ssolib_sso_services.add(ssolib_sso_service);
        }
        Validate.notEmpty(ssolib_sso_services, "Empty client lib SingleSignOnService");

        //Creating a com.vmware.identity.websso.client.SingleLogoutService list
        ArrayList<ServiceEndpoint> idm_slo_services = new ArrayList<ServiceEndpoint>();
        if (idmIdpConfig.getSloServices() != null) {
            idm_slo_services.addAll(idmIdpConfig.getSloServices());
        }
        ArrayList<com.vmware.identity.websso.client.SingleLogoutService> ssolib_slo_services =
                new ArrayList<com.vmware.identity.websso.client.SingleLogoutService>();

        for (ServiceEndpoint m : idm_slo_services) {
            com.vmware.identity.websso.client.SingleLogoutService ssolib_slo_service =
                    new com.vmware.identity.websso.client.SingleLogoutService(m.getEndpoint(), m.getBinding());
            ssolib_slo_services.add(ssolib_slo_service);
        }

        String idpAlias = (idmIdpConfig.getAlias()==null || idmIdpConfig.getAlias().isEmpty())? idmIdpConfig.getEntityID():idmIdpConfig.getAlias();
        IDPConfiguration clientIdpConfig = IDPConfigurationFactory
                .createIDPConfigurationWithoutSiteAffinity(idpAlias,
                idmIdpConfig.getEntityID(),
                idmIdpConfig.getSigningCertificateChain().get(0), //TODO should client lib take a chain?
                nameIDFormatStrs,
                ssolib_sso_services,
                    ssolib_slo_services);
        log.debug("created IDPConfig for external authn : idpAlias="
                +idmIdpConfig.getAlias()+"entityID="+idmIdpConfig.getEntityID());
        return clientIdpConfig;
    }


    static public SPConfiguration generateSPConfiguration(IdmAccessor accessor) {
        log.debug("AuthnRequestStateExternalAuthenticationFilter.generateSPConfiguration is called");
        Validate.notNull(accessor, "accessor");
        String tenantName = accessor.getTenant();
        String entityID = accessor.getIdpEntityId(); //SP entityid will be the same as it idp role id.

        SPConfiguration spConfig;

        PrivateKey signingPrivateKey;
        List<String> nameIDFormats;
        List<AssertionConsumerService> assertionConsumerServices;
        List<com.vmware.identity.websso.client.SingleLogoutService> singleLogoutServices;

        // set endpoints

        assertionConsumerServices = new ArrayList<AssertionConsumerService>();
        assertionConsumerServices.add(new AssertionConsumerService(entityID
.replaceAll(SAMLNames.ENTITY_ID_PLACEHOLDER,
                SAMLNames.SP_ASSERTIONCONSUMERSERVICE_PLACEHOLDER), true,
                SAMLNames.HTTP_POST_BINDING, 0));

        singleLogoutServices = new ArrayList<com.vmware.identity.websso.client.SingleLogoutService>();
        singleLogoutServices.add(new com.vmware.identity.websso.client.SingleLogoutService(entityID.replaceAll(SAMLNames.ENTITY_ID_PLACEHOLDER,
                SAMLNames.SP_SINGLELOGOUTSERVICE_PLACEHOLDER),
                SAMLNames.HTTP_REDIRECT_BINDING));

        // set nameID formats
        nameIDFormats = new ArrayList<String>();
        nameIDFormats.add(OasisNames.PERSISTENT);
        nameIDFormats.add(OasisNames.EMAIL_ADDRESS);
        nameIDFormats.add(SAMLNames.IDFORMAT_VAL_UPN.toString());

        // set keys
        List<Certificate> certs = accessor.getTenantCertificate();
        signingPrivateKey = accessor.getSAMLAuthorityPrivateKey();


        //TODO hardcode algo for now.  accessor.getTenantSignatureAlgorithm() returns null for some reason.
        String spSigningAlgo = SignatureAlgorithm.RSA_SHA256.toString();

        // create configuration object and store it
        spConfig = new SPConfiguration(tenantName, entityID, true,
                signingPrivateKey, (X509Certificate) certs.get(0),spSigningAlgo,
                nameIDFormats, assertionConsumerServices, singleLogoutServices);
        return spConfig;
    }

    /**
     * @param locale
     * @param response
     * @param logoutState
     * @throws IOException
     */
    static public void sendLogoutError(Locale locale, HttpServletResponse response,
            LogoutState logoutState, MessageSource messageSource) throws IOException {
        // use validation result code to return error to client
        ValidationResult vr = logoutState.getValidationResult();
        String message = null;
        if (messageSource != null && locale != null) {
        	message = vr.getMessage(messageSource, locale);
        } else {
        	message = vr.getBaseLocaleMessage();
        }
        response.sendError(vr.getResponseCode(), message);
        log.info("Responded with ERROR " + vr.getResponseCode()
                + ", message " + message);
    }

    /**
     * Helper method to generate LogoutRequests and send to non-initiating participants
     *
     * @param locale
     * @param tenant
     * @param logoutState
     * @return
     * @throws IOException
     */
    public static void sendSLORequestsToOtherParticipants(String tenant,
            LogoutState logoutState) throws IOException {
        log.info("Sending SAML logout response to other participants.");
        Locale locale = logoutState.getLocale();
        HttpServletResponse response = logoutState.getResponse();

        Validate.notNull(response);

        Collection<String> samlRequestUrls = logoutState.generateRequestUrlsForTenant(
                tenant, logoutState.getMessageSource(), locale);


        if (samlRequestUrls == null)
            return;

        for (String requestUrl : samlRequestUrls) {

            log.info("SAML Redirect URL is " + requestUrl);

            if (requestUrl != null) {
                Throwable exception = null;

                try {
                    sendSLORequestToOtherParticipant(requestUrl);
                } catch (URISyntaxException e) {
                    exception = e;
                } catch (IOException e) {
                    exception = e;
                } catch (KeyManagementException e) {
                    exception = e;
                } catch (NoSuchAlgorithmException e) {
                    exception = e;
                } catch (KeyStoreException e) {
                    exception = e;
                }

                if (exception != null) {
                    log.error(
                        "exception in sending out single log out request to participent. Request string: " + requestUrl,
                        exception);
                }
            } else {
                //send error response to the initiating participant
                SamlServiceImpl.sendLogoutError(locale, response,
                		logoutState, logoutState.getMessageSource());
            }
        }

    }
    /**
     * Utility method to send a slo request to a participant via GET message.
     * @param requestUrl
     * @throws URISyntaxException
     * @throws IOException
     * @throws ClientProtocolException
     * @throws KeyStoreException
     * @throws NoSuchAlgorithmException
     * @throws KeyManagementException
     */
    static void sendSLORequestToOtherParticipant(
            String requestUrl) throws URISyntaxException, ClientProtocolException, IOException, NoSuchAlgorithmException, KeyStoreException, KeyManagementException  {

        if (requestUrl == null || requestUrl.isEmpty())
            return;

        SSLContextBuilder builder = new SSLContextBuilder();
        builder.loadTrustMaterial(null, new TrustSelfSignedStrategy());
        SSLConnectionSocketFactory socketFactory = new SSLConnectionSocketFactory(
            builder.build(),
            SSLConnectionSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER);
        CloseableHttpClient client = HttpClients.custom().setSSLSocketFactory(socketFactory).build();
        URI httpUri = new URI(requestUrl);
        HttpGet httpGet = new HttpGet(httpUri);
        Shared.addNoCacheHeader(httpGet);
        CloseableHttpResponse response = client.execute(httpGet);
        response.close();
    }

    /**
     * Helper method to generate LogoutResponse redirect url
     *
     * @param locale
     * @param tenant
     * @param logoutState
     * @return
     */
    public static String buildResponseUrl(String tenant,
            LogoutState logoutState) {
        String retval = null;

        LogoutResponse samlResponse = logoutState.generateResponseForTenant(
                tenant, logoutState.getMessageSource(), logoutState.getLocale());
        if (samlResponse != null) {
            log.info("SAML SLO Response is " + samlResponse.toString());

            retval = logoutState
                    .generateResponseUrlForTenant(samlResponse, tenant);
        }
        return retval;
    }

    private void appendOptionalParameter(
            StringBuilder sb, String paramName, String paramValue, boolean base64Encode) {
        if (paramValue != null && paramValue != "") {
            if (sb.length() != 0) {
                sb.append("&");
            }
            sb.append(paramName + "=");

            try {
                if (base64Encode) {
                    paramValue = Shared.encodeString(paramValue);
                }
                sb.append(URLEncoder.encode(paramValue, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
            }
        }
    }
 }
