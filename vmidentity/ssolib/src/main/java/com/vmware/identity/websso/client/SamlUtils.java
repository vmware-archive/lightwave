/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.Collection;
import java.util.TimeZone;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;
import java.util.zip.InflaterOutputStream;

import javax.servlet.http.HttpServletRequest;
import javax.xml.namespace.QName;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SAMLVersion;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.common.impl.SecureRandomIdentifierGenerator;
import org.opensaml.saml2.core.Assertion;
import org.opensaml.saml2.core.Conditions;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.NameID;
import org.opensaml.saml2.core.Response;
import org.opensaml.saml2.core.SessionIndex;
import org.opensaml.saml2.core.Status;
import org.opensaml.saml2.core.StatusCode;
import org.opensaml.saml2.core.StatusMessage;
import org.opensaml.saml2.core.impl.IssuerBuilder;
import org.opensaml.saml2.core.impl.LogoutRequestBuilder;
import org.opensaml.saml2.core.impl.SessionIndexBuilder;
import org.opensaml.security.SAMLSignatureProfileValidator;
import org.opensaml.ws.transport.http.HTTPTransportUtils;
import org.opensaml.xml.Configuration;
import org.opensaml.xml.ConfigurationException;
import org.opensaml.xml.XMLObject;
import org.opensaml.xml.io.Marshaller;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.io.Unmarshaller;
import org.opensaml.xml.io.UnmarshallerFactory;
import org.opensaml.xml.io.UnmarshallingException;
import org.opensaml.xml.security.CriteriaSet;
import org.opensaml.xml.security.SecurityException;
import org.opensaml.xml.security.credential.Credential;
import org.opensaml.xml.security.keyinfo.KeyInfoCredentialResolver;
import org.opensaml.xml.security.keyinfo.KeyInfoCriteria;
import org.opensaml.xml.security.x509.BasicX509Credential;
import org.opensaml.xml.signature.KeyInfo;
import org.opensaml.xml.signature.SignatureValidator;
import org.opensaml.xml.util.Base64;
import org.opensaml.xml.util.XMLHelper;
import org.opensaml.xml.validation.ValidationException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

/**
 * SamlUtils class
 *
 */
public class SamlUtils {

    public static final String SAML_REQUEST_PARAMETER = "SAMLRequest";
    public static final String SAML_RESPONSE_PARAMETER = "SAMLResponse";
    public static final String RELAY_STATE_PARAMETER = "RelayState";
    public static final String SIGNATURE_ALGORITHM_PARAMETER = "SigAlg";
    public static final String SIGNATURE_PARAMETER = "Signature";

    private static final Logger log = LoggerFactory.getLogger(SamlUtils.class);

    private String issuer;
    private X509Certificate certificate;
    private PrivateKey privateKey;
    private String signAlgorithm;
    private String checkAlgorithm;

    public SamlUtils(X509Certificate cert, PrivateKey key, String signAlgorithm, String checkAlgorithm, String issuer)
            throws NoSuchAlgorithmException {
        this.certificate = cert;
        this.privateKey = key;
        this.signAlgorithm = signAlgorithm;
        this.checkAlgorithm = checkAlgorithm;
        this.issuer = issuer;
    }

    private static SecureRandomIdentifierGenerator generator;

    static {
        try {
            generator = new SecureRandomIdentifierGenerator();
        } catch (NoSuchAlgorithmException e) {
            log.error("Unexpected error in creating SecureRandomIdentifierGenerator");
        }
    }

    /**
     * @return the issuer
     */
    public String getIssuer() {
        return this.issuer;
    }

    /**
     * @param issuer
     *            the issuer to set
     */
    public void setIssuer(String issuer) {
        this.issuer = issuer;
    }

    /**
     * @return the privateKey
     */
    public PrivateKey getPrivateKey() {
        return this.privateKey;
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
    public String getSignAlgorithm() {
        return this.signAlgorithm;
    }

    /**
     * @param signAlgorithm
     *            the signAlgorithm to set
     */
    public void setSignAlgorithm(String signAlgorithm) {
        this.signAlgorithm = signAlgorithm;
    }

    /**
     * @return the checkAlgorithm
     */
    public String getCheckAlgorithm() {
        return this.checkAlgorithm;
    }

    /**
     * @param checkAlgorithm
     *            the checkAlgorithm to set
     */
    public void setCheckAlgorithm(String checkAlgorithm) {
        this.checkAlgorithm = checkAlgorithm;
    }

    /**
     * @return the certPath
     */
    public X509Certificate getCertificate() {
        return this.certificate;
    }

    /**
     * @param certPath
     *            the certPath to set
     */
    public void setCertificate(X509Certificate cert) {
        this.certificate = cert;
    }

    /**
     * Signs message with known private key and algorithm. Throws on error.
     *
     * @param message
     * @return signature
     */
    public String signMessage(String message) throws IllegalStateException {
        Validate.notNull(getSignAlgorithm(), "Signature Algorithm");
        Validate.notNull(getPrivateKey(), "PrivateKey for signing");
        Validate.notEmpty(message, "message string");
        log.debug("Signing  message " + message + ", sigAlg " + getSignAlgorithm());

        try {
            SignatureAlgorithm algo = SignatureAlgorithm.getSignatureAlgorithmForURI(getSignAlgorithm());
            Signature sig = Signature.getInstance(algo.getAlgorithmName());
            sig.initSign(this.privateKey);

            byte[] messageBytes = message.getBytes("UTF-8");
            sig.update(messageBytes);

            byte[] sigBytes = sig.sign();
            String signature = SharedUtils.encodeBytes(sigBytes);
            log.debug("signature: " + signature);

            return signature;
        } catch (Exception e) {
            log.error("Failed to sign the message: exception cause", e);
            throw new IllegalStateException(e);
        }
    }

    /**
     * Verifies signature of the message with a known public key and signature
     * algorithm. Throws on error.
     *
     * @param message
     *            Raw message extracted from query.
     * @param signature
     *            Raw signature extracted from query without decoding.
     * @param algorithmUri
     *            Raw algorithm uri extracted from query.
     */
    public void verifySignature(String message, String signature, String algorithmUri) throws IllegalStateException,
            WebssoClientException {
        log.debug("Verify signature.");
        Validate.notNull(message, "signed content");
        Validate.notNull(signature, "signature");
        Validate.notNull(algorithmUri, "algorithm uri");
        Validate.notNull(getCertificate(), "signing certificate");

        boolean verifies = false;

        // decode algo uri and signature
        String decodedAlgorithmUri = null;
        byte[] decodedSignature = null;
        try {
            decodedAlgorithmUri = URLDecoder.decode(algorithmUri, "UTF-8");
            log.debug("Signature algorithm uri {}", decodedAlgorithmUri);
            decodedSignature = Base64.decode(URLDecoder.decode(signature, "UTF-8"));
        } catch (UnsupportedEncodingException e) {
            throw new WebssoClientException("Could not decode algorithm uri or signature", e);
        }

        try {
            /* create a Signature object and initialize it with the public key */
            SignatureAlgorithm algo = SignatureAlgorithm.getSignatureAlgorithmForURI(decodedAlgorithmUri);
            Signature sig = Signature.getInstance(algo.getAlgorithmName());

            X509Certificate cert = getCertificate();
            sig.initVerify(cert.getPublicKey());
            sig.update(message.getBytes("UTF-8"));

            verifies = sig.verify(decodedSignature);
            log.debug("signature verifies: " + verifies);
        } catch (Exception e) {
            log.error("Caught exception ", e);
            throw new IllegalStateException(e);
        }
        if (!verifies) {
            throw new IllegalStateException("Signature verification failed.");
        }
    }

    /**
     * Create SAML Logout request
     *
     * @param id
     *            optional
     * @param where
     *            destination, required
     * @param nameIDFormat
     *            nameIDFormat of the princeple, optional
     * @param nameID
     *            nameID value. required
     * @param sessionIndex
     *            SP maintained sessionID. required.
     * @return LogoutReuest
     */
    public LogoutRequest createSamlLogoutRequest(String id, String where, String nameIDFormat, String nameID,
            String sessionIndex) {
        if (id == null) {
            id = generator.generateIdentifier();
        }
        log.info("Building SAML LogoutRequest for id: " + id + "\n 	destination: " + where + "\n	nameIDFormat: "
                + nameIDFormat + "\n	nameID: " + nameID + "\n	sessionIndex: " + sessionIndex);

        Validate.notNull(where, "destination is null");
        Validate.notNull(nameID, "nameID is null");

        DateTime issueInstant = new DateTime();
        LogoutRequestBuilder logoutRequestBuilder = new LogoutRequestBuilder();
        LogoutRequest logoutRequest = logoutRequestBuilder.buildObject(SamlNames.PROTOCOL, "LogoutRequest", "samlp");
        logoutRequest.setDestination(where);
        logoutRequest.setIssueInstant(issueInstant);
        logoutRequest.setIssuer(spawnIssuer(getIssuer()));
        logoutRequest.setNameID(spawnNameID(nameIDFormat, nameID));
        logoutRequest.setID(id);
        logoutRequest.setVersion(SAMLVersion.VERSION_20);

        // Create o[tional SessionIndex
        if (sessionIndex != null) {
            SessionIndexBuilder sessionIndexBuilder = new SessionIndexBuilder();
            SessionIndex sessionIndexElement = sessionIndexBuilder.buildObject(SamlNames.PROTOCOL, "SessionIndex",
                    "samlp");
            sessionIndexElement.setSessionIndex(sessionIndex);
            logoutRequest.getSessionIndexes().add(sessionIndexElement);
        }

        return logoutRequest;
    }

    /**
     * Create SAML Logout response
     *
     * @param issuerVal
     *            optional
     * @param inResponseTo
     *            required
     * @param where
     *            required. destination
     * @param status
     * @param substatus
     * @param message
     * @return
     */

    public static LogoutResponse createSamlLogoutResponse(String issuerVal, String inResponseTo, String where,
            String status, String substatus, String message) {

        log.info("SP Creating SAML Logout Response in response to:" + inResponseTo + ", destination: " + where);
        log.info("SP Creating SAML Logout Response status:" + status + ", substatus: " + substatus + ", message:"
                + message);
        Validate.notNull(where, "destination is null");

        if (status == null) {
            // assume success
            status = SamlNames.SUCCESS;
        }

        LogoutResponse response = create(LogoutResponse.class, LogoutResponse.DEFAULT_ELEMENT_NAME);
        response.setID(SamlUtils.generator.generateIdentifier());
        if (inResponseTo != null) {
            response.setInResponseTo(inResponseTo);
        }
        if (issuerVal != null) {
            IssuerBuilder issuerBuilder = new IssuerBuilder();
            Issuer issuer = issuerBuilder.buildObject(SamlNames.ASSERTION, "Issuer", "samlp");
            issuer.setValue(issuerVal);
            response.setIssuer(issuer);
        }
        response.setVersion(SAMLVersion.VERSION_20);
        DateTime now = new DateTime();
        response.setIssueInstant(now);
        response.setDestination(where);

        Status samlStatus = spawnStatus(status, substatus, message);

        response.setStatus(samlStatus);
        return response;
    }

    /**
     * create saml sso response in the form of opensaml Response with given
     * parameters.
     *
     */
    public Response createSamlLoginResponse(String inResponseTo, String where, String status, String substatus,
            String message, Document token) throws UnmarshallingException {
        log.info("Creating SAML Response in response to:" + inResponseTo + ", destination:" + where + ", issuer:"
                + getIssuer());
        log.info("Creating SAML Response status:" + status + ", substatus: " + substatus);
        Validate.notNull(where, "destination is null");

        if (status == null) {
            // assume success
            status = SamlNames.SUCCESS;
        }

        Response response = create(Response.class, Response.DEFAULT_ELEMENT_NAME);
        response.setID(generator.generateIdentifier());
        if (inResponseTo != null) {
            response.setInResponseTo(inResponseTo);
        }
        response.setVersion(SAMLVersion.VERSION_20);
        DateTime now = new DateTime();
        response.setIssueInstant(now);
        response.setDestination(where);

        response.setIssuer(spawnIssuer(getIssuer()));

        Status samlStatus = spawnStatus(status, substatus, message);

        response.setStatus(samlStatus);
        if (token != null) {
            Element root = token.getDocumentElement();

            // get appropriate unmarshaller
            UnmarshallerFactory unmarshallerFactory = Configuration.getUnmarshallerFactory();
            Unmarshaller unmarshaller = unmarshallerFactory.getUnmarshaller(root);

            // unmarshall using root document element which needs to be
            // Assertion
            Assertion assertion = (Assertion) unmarshaller.unmarshall(root);

            response.getAssertions().add(assertion);
        }

        return response;
    }

    /**
     * Create openSAML Status object
     *
     * @param status
     * @param substatus
     * @param message
     * @return
     */
    private static Status spawnStatus(String status, String substatus, String message) {

        StatusCode statusCodeElement = create(StatusCode.class, StatusCode.DEFAULT_ELEMENT_NAME);
        statusCodeElement.setValue(status);
        if (substatus != null) {
            StatusCode statusCodeElement2 = create(StatusCode.class, StatusCode.DEFAULT_ELEMENT_NAME);
            statusCodeElement2.setValue(substatus);
            statusCodeElement.setStatusCode(statusCodeElement2);
        }

        Status samlStatus = create(Status.class, Status.DEFAULT_ELEMENT_NAME);
        samlStatus.setStatusCode(statusCodeElement);

        if (message != null) {
            StatusMessage statusMessage = create(StatusMessage.class, StatusMessage.DEFAULT_ELEMENT_NAME);
            statusMessage.setMessage(message);
            samlStatus.setStatusMessage(statusMessage);
        }
        return samlStatus;

    }

    private NameID spawnNameID(String format, String nameIDValue) {
        NameID result = null;
        result = create(NameID.class, NameID.DEFAULT_ELEMENT_NAME);
        result.setValue(nameIDValue);
        if (format != null) {
            result.setFormat(format);
        }

        return result;
    }

    public static Issuer spawnIssuer(String myId) {
        Issuer result = null;
        result = create(Issuer.class, Issuer.DEFAULT_ELEMENT_NAME);
        result.setValue(myId);

        return result;
    }

    /**
     * Encode opensaml object (request/response) as a query string parameter.
     *
     * @param signableSAMLObject
     *            The object to be encoded.
     * @param doCompress
     *            whether include compression step. For REDIRECT binding, this
     *            should be true.
     *
     * @return encoded object
     * @throws MarshallingException
     * @throws IOException
     */
    public static String encodeSAMLObject(SignableSAMLObject signableSAMLObject, boolean doCompress)
            throws MarshallingException, IOException {

        // Now we must build our representation to put into the html form to be
        // submitted to the idp
        Marshaller marshaller = org.opensaml.Configuration.getMarshallerFactory().getMarshaller(signableSAMLObject);
        org.w3c.dom.Element authDOM = marshaller.marshall(signableSAMLObject);
        StringWriter rspWrt = new StringWriter();
        XMLHelper.writeNode(authDOM, rspWrt);
        String messageXML = rspWrt.toString();

        String samlRequestParameter = null;

        log.debug("encodeSAMLObject...before encoding: " + messageXML);
        if (doCompress) {
            Deflater deflater = new Deflater(Deflater.DEFLATED, true);
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
            DeflaterOutputStream deflaterOutputStream = new DeflaterOutputStream(byteArrayOutputStream, deflater);
            deflaterOutputStream.write(messageXML.getBytes("UTF-8"));
            deflaterOutputStream.close();
            samlRequestParameter = SharedUtils.encodeBytes(byteArrayOutputStream.toByteArray());
        } else {
            samlRequestParameter = SharedUtils.encodeString(messageXML);
        }
        log.trace("encodeSAMLObject...after encoding: " + samlRequestParameter);
        return samlRequestParameter;
    }

    /**
     * Return a decompressed and base64bit decoded slo response. This is used by
     * extracting SAMLObject received via redirect binding where compression is
     * applied.
     *
     * @throws TransformerFactoryConfigurationError
     * @throws Exception
     */
    public static String extractResponse(String samlRequestParameter) throws Exception {
        // inflate
        Inflater decompresser = new Inflater(true);
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        InflaterOutputStream inflaterOutputStream = new InflaterOutputStream(byteArrayOutputStream, decompresser);
        inflaterOutputStream.write(Base64.decode(samlRequestParameter));
        inflaterOutputStream.close();
        String outputString = new String(byteArrayOutputStream.toByteArray(), "UTF-8");

        return outputString;
    }

    /**
     * generate QueryString Parameters.
     *
     * @param samlRequest
     * @param relayState
     *            need for relay
     * @param sigAlg
     * @param signature
     * @throws UnsupportedEncodingException
     */

    public static String generateRedirectUrlQueryStringParameters(String samlRequest, String samlResponse,
            String relayState, String sigAlg, String signature) throws UnsupportedEncodingException {
        StringBuilder sb = new StringBuilder();
        appendOptionalParameter(sb, SamlUtils.SAML_REQUEST_PARAMETER, samlRequest, false);
        appendOptionalParameter(sb, SamlUtils.SAML_RESPONSE_PARAMETER, samlResponse, false);
        appendOptionalParameter(sb, SamlUtils.RELAY_STATE_PARAMETER, relayState, false);
        appendOptionalParameter(sb, SamlUtils.SIGNATURE_ALGORITHM_PARAMETER, sigAlg, false);
        appendOptionalParameter(sb, SamlUtils.SIGNATURE_PARAMETER, signature, false);
        return sb.toString();
    }

    /**
     * Validate signature in the http request. Exception throw if fails to
     * verify the signature when the request must be signed.
     *
     * @param request
     *            the request from httpget
     * @param mustSigned
     *            true if the message must be signed.
     * @return indicate whether the request is signed true: it is signed. false:
     *         not signed.
     * @throws Exception
     *
     */
    public boolean validateRequestSignature(HttpServletRequest request, Boolean mustSigned) throws Exception {
        Validate.notNull(request, "HttpServletRequest");

        String signatureRawQueryString = HTTPTransportUtils.getRawQueryStringParameter(request.getQueryString(),
                SamlUtils.SIGNATURE_PARAMETER);
        String sigAlgRawQueryString = HTTPTransportUtils.getRawQueryStringParameter(request.getQueryString(),
                SamlUtils.SIGNATURE_ALGORITHM_PARAMETER);
        ;

        if (StringUtils.isEmpty(signatureRawQueryString) || StringUtils.isEmpty(sigAlgRawQueryString))
            if (mustSigned) {
                String errMsg = "Http request query string missing signature or algorithm.";
                log.error(errMsg);
                throw new IllegalArgumentException(errMsg);
            } else {
                return false;
                // acceptable. but let caller know it is not signed.
            }

        try {
            // validate cert
            Validate.notNull(getCertificate(), "Certificate");
            String signature = signatureRawQueryString.split("=")[1];
            String algorithmUri = sigAlgRawQueryString.split("=")[1];
            String signedContent = parseSignedMessage(request);
            verifySignature(signedContent, signature, algorithmUri);
            log.info("Successfully validated SAML message signature. ");
        } catch (Exception e) {
            log.error("Unable to verify signature of the query string. ", e);
            throw e;
        }
        return true;
        // we are good.
    }

    /**
     * unmarshall dom object to opensaml XMLObject.
     *
     * @param samlRequest
     * @param relayState
     *            need for relay
     * @param sigAlg
     * @param signature
     * @throws UnsupportedEncodingException
     */
    public static XMLObject unmarshallSAMLObj(Document document) throws UnmarshallingException, ConfigurationException {

        Validate.notNull(document);
        Element root = document.getDocumentElement();

        // get appropriate unmarshaller
        UnmarshallerFactory unmarshallerFactory = Configuration.getUnmarshallerFactory();
        Unmarshaller unmarshaller = unmarshallerFactory.getUnmarshaller(root);

        // unmarshall using root document element can be /LogoutResponse or
        // LogoutRequest
        XMLObject obj = unmarshaller.unmarshall(root);
        return obj;
    }

    /**
     * Create Document object from openSaml SignableSAMLObject.
     *
     * @param signableSAMLObject
     *            assertion or other signable object
     * @return Document
     */
    public static Document createDomFromSignable(SignableSAMLObject signableSAMLObject) {

        try {
            Marshaller marshaller = org.opensaml.Configuration.getMarshallerFactory().getMarshaller(signableSAMLObject);
            org.w3c.dom.Element signableDomEle = marshaller.marshall(signableSAMLObject);

            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            Document doc = builder.newDocument();
            doc.appendChild(doc.importNode(signableDomEle, true));
            return doc;

        } catch (Exception e) {
            log.error("Caught exception " + e.toString());
            return null;
        }
    }

    /**
     * append optional parameter.
     *
     * @param sb
     * @param paramName
     * @param paramValue
     * @param base64Encode
     * @throws UnsupportedEncodingException
     */
    private static void appendOptionalParameter(StringBuilder sb, String paramName, String paramValue,
            boolean base64Encode) throws UnsupportedEncodingException {
        if (paramValue != null && paramValue != "") {
            if (sb.length() != 0) {
                sb.append("&");
            }
            sb.append(paramName + "=");
            if (base64Encode) {
                paramValue = SharedUtils.encodeString(paramValue);
            }
            try {
                sb.append(URLEncoder.encode(paramValue, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
            }
        }
    }

    /**
     * Validate Conditions. Return expiration date. Throw if expired
     *
     * @param Conditions
     * @param clockTolerance
     *            in seconds
     * @return void:
     */
    public static void ValidateConditions(Conditions conditions, int clockTolerance) throws ValidationException {

        try {
            log.info("Validate assertion condition with clock tolerance = " + clockTolerance);
            Validate.isTrue(clockTolerance >= 0, "Negative clock tolerance setting");
            Calendar currentDate = Calendar.getInstance();
            TimeZone gmtTimeZone = TimeZone.getTimeZone("GMT");
            currentDate.setTimeZone(gmtTimeZone);

            if (conditions.getNotBefore() != null) {
                DateTime notBefore = conditions.getNotBefore().minusSeconds(clockTolerance);
                Validate.isTrue(currentDate.getTime().after(notBefore.toDate()),
                        "Validating notBefore fails! Tolerance-adjusted notBefore date is:" + notBefore.toString());
            }

            if (conditions.getNotOnOrAfter() != null) {
                DateTime notOnOrAfter = conditions.getNotOnOrAfter().plusSeconds(clockTolerance);
                Validate.isTrue(currentDate.getTime().before(notOnOrAfter.toDate()),
                        "Validating notAfter fails! Tolerance-adjusted notAfter date is:" + notOnOrAfter.toString());
            }
        } catch (IllegalArgumentException e) {
            log.error(Error.CONDITION, e);
            throw new ValidationException(e);
        }
    }

    /**
     * Validate Sesssion's ExpiryNotOnOrAfter date. Throw if expired
     *
     * @param sessionNotOnOrAfter
     * @param clockTolerance
     *            in seconds
     * @return void:
     */
    public static void ValidateSessionExpiry(DateTime sessionNotOnOrAfter, int clockTolerance)
            throws ValidationException {

        try {
            log.info("Validate sessionNotOnOrAfter with clock tolerance = " + clockTolerance);
            Validate.isTrue(clockTolerance >= 0, "Negative clock tolerance setting");
            Calendar currentDate = Calendar.getInstance();
            TimeZone gmtTimeZone = TimeZone.getTimeZone("GMT");
            currentDate.setTimeZone(gmtTimeZone);

            if (sessionNotOnOrAfter != null) {
                DateTime notOnOrAfter = sessionNotOnOrAfter.plusSeconds(clockTolerance);
                Validate.isTrue(currentDate.getTime().before(notOnOrAfter.toDate()),
                        "Validating notAfter fails! Tolerance-adjusted notAfter date is:" + notOnOrAfter.toString());
            }
        } catch (IllegalArgumentException e) {
            log.error(Error.IDP_SESSION_EXPIRED, e);
            throw new ValidationException(e);
        }
    }

    /**
     * Validate an openSaml signature object with given trusted certificate of
     * the supposed signer. Currently only used by in validating assetion
     * signature.
     *
     * @param signature
     *            opensaml Signature
     * @param cert
     *            optional signing certificate from context such as IDP/SP
     *            metadata.
     * @throws ValidationException
     * @return void:
     * @throws SecurityException
     */
    public static void ValidateSignature(org.opensaml.xml.signature.Signature signature, X509Certificate cert)
            throws ValidationException, SecurityException {
        if (signature == null) {
            log.warn("Null signature!");
            return;
        }

        BasicX509Credential pubCredential = new BasicX509Credential();
        // check server identity per binding 3.1.2.1. via certificate's subject
        // DN field,
        // subjectAltName attribute, etc.

        Credential cred = null;
        KeyInfo keyInfo = signature.getKeyInfo();
        if (keyInfo == null) {
            if (cert == null) {
                throw new ValidationException("No signing certificate found.");
            }
            pubCredential.setEntityCertificate(cert);
            cred = pubCredential;
        } else {
            KeyInfoCredentialResolver kiResolver = Configuration.getGlobalSecurityConfiguration()
                    .getDefaultKeyInfoCredentialResolver();
            CriteriaSet criteriaSet = new CriteriaSet(new KeyInfoCriteria(keyInfo));
            cred = kiResolver.resolveSingle(criteriaSet);
        }

        SAMLSignatureProfileValidator pv = new SAMLSignatureProfileValidator();
        pv.validate(signature);

        SignatureValidator sigValidator = new SignatureValidator(cred);
        sigValidator.validate(signature);
    }

    /**
     * return signed message in a http SAML request or response
     *
     * @param request
     * @return signed message string in SAML request and response:
     * @throws IllegalArgumentException
     */
    public static String parseSignedMessage(HttpServletRequest request) throws IllegalArgumentException {
        String queryString = request.getQueryString();
        String samlRequest = HTTPTransportUtils.getRawQueryStringParameter(queryString,
                SamlUtils.SAML_REQUEST_PARAMETER);
        String samlResponse = HTTPTransportUtils.getRawQueryStringParameter(queryString,
                SamlUtils.SAML_RESPONSE_PARAMETER);
        Validate.isTrue(samlRequest != null || samlResponse != null);

        // HTTPTransportUtils.getRawQueryStringParameter returns null if the
        // component is not found
        String signature = HTTPTransportUtils.getRawQueryStringParameter(queryString, SamlUtils.SIGNATURE_PARAMETER);
        String sigAlgo = HTTPTransportUtils.getRawQueryStringParameter(queryString,
                SamlUtils.SIGNATURE_ALGORITHM_PARAMETER);
        String relayState = HTTPTransportUtils.getRawQueryStringParameter(queryString, SamlUtils.RELAY_STATE_PARAMETER);

        if (signature == null || sigAlgo == null) {
            return null;
        }

        StringBuilder builder = new StringBuilder();
        if (samlRequest != null) {
            builder.append(samlRequest);
        } else {
            builder.append(samlResponse);
        }

        if (relayState != null) {
            builder.append('&');
            builder.append(relayState);
        }

        builder.append('&');
        builder.append(sigAlgo);

        String signedMessage = builder.toString();

        log.debug("Constructed signed message {}", signedMessage);
        return signedMessage;
    }

    /**
     * Return IDP SSO service location with given binding. Caller should
     * validate returned string.
     *
     * @param idpConfig
     * @param binding
     *            SSO binding
     * @return null or URL string
     */
    public static String getIdpSsoLocation(IDPConfiguration idpConfig, String binding) {

        Validate.notNull(idpConfig, "IDPConfiguration");
        Validate.notNull(binding, "Null binding");

        Collection<SingleSignOnService> services = idpConfig.getSingleSignOnServices();

        for (SingleSignOnService service : services) {
            if (service.getBinding().equals(binding)) {
                return service.getLocation();
            }
        }
        return null;
    }

    /**
     * Return IDP SLO service location with given binding. Caller should
     * validate returned string.
     *
     * @param idpConfig
     * @param binding
     *            SLO binding
     * @return IDP's single logout service url.
     */
    public static String getIdpSloLocation(IDPConfiguration idpConfig, String binding) {

        Validate.notNull(idpConfig, "IDPConfiguration");
        Validate.notNull(binding, "Null binding");

        Collection<SingleLogoutService> services = idpConfig.getSingleLogoutServices();

        for (SingleLogoutService service : services) {
            if (service.getBinding().equals(binding)) {
                return service.getLocation();
            }
        }
        return null;
    }

    /**
     * Return Service Provider SLO service location with given binding. Caller
     * should validate returned string.
     *
     * @param spConfig
     *            SPCongiguration
     * @param binding
     * @return URL String
     */
    public static String getSpSloLocation(SPConfiguration spConfig, String binding) {

        Validate.notNull(spConfig, "SPConfiguration");
        Validate.notNull(binding, "Null binding");
        Collection<SingleLogoutService> services = spConfig.getSingleLogoutServices();

        for (SingleLogoutService service : services) {
            if (service.getBinding().equals(binding)) {
                return service.getLocation();
            }
        }
        return null;
    }

    /**
     * Find IDP certificate given an issuer of a SAML message from IDP.
     *
     * @param metadataSettings
     *            Required. if not provided the function will return null.
     * @param issuer
     *            Required. if not provided the function will return null.
     * @return valid cert or null.
     * @throws WebssoClientException
     *             unknown idp
     * @throws IllegalArgumentException
     */
    public static X509Certificate getIDPCertByIssuer(MetadataSettings metadataSettings, String issuerVal)
            throws WebssoClientException {
        Validate.notEmpty(issuerVal, "issuerVal");
        Validate.notNull(metadataSettings, "metadataSettings");
        log.debug("Getting IDP config for:" + issuerVal);

        IDPConfiguration idpConfig = metadataSettings.getIDPConfigurationByEntityID(issuerVal);
        if (idpConfig == null) {
            throw new WebssoClientException("Uknown IDP configuration. IDP entity ID = : "+issuerVal);
        }
        return idpConfig.getSigningCertificate();
    }

    /**
     * Create object using OpenSAML's builder system.
     */
    // cast to SAMLObjectBuilder<T> is caller's choice
    @SuppressWarnings({ "unchecked", "rawtypes" })
    private static <T> T create(Class<T> cls, QName qname) {
        return (T) Configuration.getBuilderFactory().getBuilder(qname).buildObject(qname);
    }

    // Helper function to check whether IDP has SLO end point defined.
    public static boolean isIdpSupportSLO(MetadataSettings metadataSettings, SloRequestSettings requestSettings) {
        IDPConfiguration idpConfig = metadataSettings.getIDPConfiguration(requestSettings.getIDPAlias());
        return !(SamlUtils.getIdpSloLocation(idpConfig, SamlNames.HTTP_REDIRECT) == null);
    }
}
