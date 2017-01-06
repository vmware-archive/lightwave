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
import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.saml2.core.Assertion;
import org.opensaml.saml2.core.AttributeStatement;
import org.opensaml.saml2.core.AuthnContextClassRef;
import org.opensaml.saml2.core.AuthnStatement;
import org.opensaml.saml2.core.NameID;
import org.opensaml.saml2.core.Response;
import org.opensaml.saml2.core.Status;
import org.opensaml.saml2.core.StatusCode;
import org.opensaml.saml2.core.Subject;
import org.opensaml.saml2.core.SubjectConfirmation;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.signature.Signature;
import org.opensaml.xml.util.Base64;
import org.opensaml.xml.validation.ValidationException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import com.vmware.identity.websso.client.endpoint.SsoResponseListener;

/**
 * ValidationState implementation for sso controller.
 *
 */
public class SsoValidationState extends ValidationState {

    private static final Logger logger = LoggerFactory.getLogger(SsoValidationState.class);

    private final SsoResponseListener controller;
    private Response opensamlResponse;

    public SsoValidationState(HttpServletRequest request, SsoResponseListener controller) {
        super(controller.getMetadataSettings());
        this.setRequest(request);
        this.controller = controller;
    }

    /**
     * This function decode sso response and cache response attributes as
     * following members: status subStatus issuerVa issueInstant messageData
     *
     * @param
     * @throws Exception
     */
    @Override
    public void validate() throws Exception {
        logger.info("Validating SAMLResponse..");
        Validate.notNull(this.getRequest(), "HttpServletRequest");

        this.DecodeResponse();
        this.opensamlResponse = (Response) SamlUtils.unmarshallSAMLObj(this.getSamlDom());

        Validate.notNull(this.opensamlResponse, "Response object");

        Status status = this.opensamlResponse.getStatus();
        StatusCode statusCode = status.getStatusCode();

        this.setMessageID(this.opensamlResponse.getID());
        this.setStatus(statusCode.getValue());
        StatusCode substatus = statusCode.getStatusCode();
        if (null != substatus) {
            this.setSubStatus(substatus.getValue());
        }
        // sync up ValidationResult value with this server returned status.

        Validate.notNull(this.opensamlResponse.getIssuer(), "issuer");
        this.setIssuerVal(this.opensamlResponse.getIssuer().getValue());
        this.setIssueInstant(this.opensamlResponse.getIssueInstant());
        this.setDestination(this.opensamlResponse.getDestination());

        // These validations should be done regardless the server validation
        // status.
        this.validateDestination();
        Message requestMessage = this.validateInResponseTo();

        // remove the request message from the message store as we are done validation of inresponseto.
        if (null != requestMessage) {
            this.controller.getMessageStore().remove(requestMessage.getId());
        }
        if (statusCode.getValue().equals(StatusCode.SUCCESS_URI)) {

            this.validateAssertion();
            logger.info("Successfully validated received SAMLResponse");
        } else {
            logger.info("Received failure response from SSO server, status code: " + getStatus() + " substatus code: "
                    + getSubStatus());
        }
    }

    /**
     * @param parameter
     * @throws Exception
     */
    private void DecodeResponse() throws Exception {

        // 1. extract and decode saml response.

        String samlResponseStr = this.getRequest().getParameter(SamlUtils.SAML_RESPONSE_PARAMETER);

        Validate.notEmpty(samlResponseStr, "Empty SSO response string");
        String decodedResponseStr = new String(Base64.decode(samlResponseStr), "UTF-8");
        this.setSamlDom(SharedUtils.createDOM(decodedResponseStr));
        logger.debug("Decoded SAML Response without pretty formatting is " + decodedResponseStr);
        // 2. format and print out xml
        if (logger.isDebugEnabled()) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            Element rootE = this.getSamlDom().getDocumentElement();
            rootE.normalize();
            SharedUtils.formattedPrint(rootE, baos);
            decodedResponseStr = baos.toString("UTF-8");
        }

        // 3. relayState
        String relayState = this.getRequest().getParameter(SamlUtils.RELAY_STATE_PARAMETER);
        if (relayState != null) {
            String decodedRelayState = new String(Base64.decode(relayState), "UTF-8");
            this.setRelayState(relayState);
            logger.debug("Decoded Relay State is " + decodedRelayState);
        }
    }

    /**
     * @param opensamlResponse2
     */
    /**
     * @return The matching request Message object or null.
     * @throws ValidationException
     */
    private Message validateInResponseTo() throws ValidationException {

        // this is login request ID
        String responseTo = this.opensamlResponse.getInResponseTo();
        logger.info("Validating optional request ID: " + responseTo);
        if (responseTo == null) {
            this.isIdpInitiated = true;
            return null;
        }
        // else the value must match to a request message.
        Message message = this.controller.getMessageStore().get(responseTo);
        if (message == null) {
            this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_RESPONSE,
                    Error.IN_RESPONSE_TO));
            throw new ValidationException(Error.IN_RESPONSE_TO + " No matching logon request found for SAML response: "
                    + responseTo);
        }

        return message;
    }

    /**
     * Validate assertion
     *
     * @throws Exception
     */
    private void validateAssertion() throws Exception {

        logger.info("Validating assertion..");
        List<Assertion> assertions = this.opensamlResponse.getAssertions();
        Validate.notEmpty(assertions, "assertions");

        // we only look at the first assertion now. In delegated situation
        // there could be more that one assertion.

        Assertion assertion = assertions.get(0);
        Validate.notNull(assertion, "assertion");

        String checkAlg = this.getRequest().getParameter(SamlUtils.SIGNATURE_ALGORITHM_PARAMETER);

        X509Certificate cert = null;
        // validate issuer
        try {
            cert = SamlUtils.getIDPCertByIssuer(this.controller.getMetadataSettings(), this.getIssuerVal());
        } catch (Exception e) {
            logger.error("Can't find IDP certificate with IDP: " + this.getIssuerVal());
            this.setValidationResult(new ValidationResult(HttpServletResponse.SC_FORBIDDEN, Error.BAD_RESPONSE,
                    Error.ISSUER));
            throw e;
        }

        // validate response signature.
        SamlUtils samlUtils = new SamlUtils(cert, null, // does not need signing
                                                        // key
                null, // no need for signing algo
                checkAlg, // needed for verification
                null);
        boolean isResponseSigned = false;

        try {
            isResponseSigned = samlUtils.validateRequestSignature(this.getRequest(), false // Response
                                                                                           // does
                                                                                           // not
                                                                                           // have
                                                                                           // to
                                                                                           // be
                                                                                           // signed
                    );
        } catch (Exception error) {
            logger.info("Invalid Response signature");
            if (this.getValidationResult().isValid()) {
                this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                        Error.SIGNATURE));
            }
            throw error;
        }
        MessageData messageData = this.parseAssertion(assertion, isResponseSigned);
        this.setMessageData(messageData);
        this.setSessionIndex(((AuthnData) messageData).getSessionIndex());
        logger.info("Successfully validated SSO Assertion");
    }

    /**
     * Parse and validate Assertion. Construct AuthnData and return it
     *
     * @return MessageData (AuthnData)
     * @throws ValidationException
     * @throws WebssoClientException
     */
    private MessageData parseAssertion(final Assertion assertion, final boolean isResponseSigned)
            throws MarshallingException, ValidationException, WebssoClientException {

        logger.info("Parsing assertion..");
        Validate.notNull(assertion);

        Subject subject = assertion.getSubject();
        Validate.notNull(subject, "Assertion subject"); // We require response
                                                        // provide subject.

        // Create Token DOM object return it in a new authnData obj
        Document token = SamlUtils.createDomFromSignable(assertion);
        TokenType tokenType = TokenType.BEARER;
        List<SubjectConfirmation> confirmations = subject.getSubjectConfirmations();
        if (confirmations != null && !confirmations.isEmpty()) {
            SubjectConfirmation confirmation = confirmations.get(0);
            Validate.notNull(confirmation, "SubjectConfirmation");
            tokenType = confirmation.getMethod().equals(SamlNames.URI_BEARER) ? TokenType.BEARER
                    : TokenType.HOLDER_OF_KEY;
        }

        // Validate assertion signature
        assertion.validate(true);

        Signature signature = assertion.getSignature();
        // We do validate issuer for the SAML response.
        // Token issuer is validated that it matches the register IDP
        // The signature is validated with the IDP's signing cert.
        String issuer = assertion.getIssuer().getValue();
        IDPConfiguration idpConfig = this.controller.getMetadataSettings().getIDPConfigurationByEntityID(issuer);
        if (idpConfig == null) {
            throw new WebssoClientException("Uknown IDP configuration. IDP entity ID = : "+issuer);
        }

        if (signature != null) {
            X509Certificate cert = idpConfig.getSigningCertificate();
            try {
                SamlUtils.ValidateSignature(signature, cert);
            } catch (Exception e) {
                logger.error("SAML Assertion signature validation failed! ", e);
                if (this.getValidationResult().isValid()) {
                    this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                            Error.BAD_REQUEST, Error.SIGNATURE));
                }
                throw new ValidationException(e);
            }
        } else if (!isResponseSigned || this.controller.isAssertionMustBeSigned()) {
            // We enforce assertion to be signed if the enclosing object has not
            // been signed.
            logger.error("Neither response or assertion is signed");
            if (this.getValidationResult().isValid()) {
                this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                        Error.NOT_SIGNED));
            }
            throw new ValidationException(Error.NOT_SIGNED);
        }

        // Validate conditions
        int clockTolerance = 0;
        try {
            clockTolerance = (idpConfig == null) ? 600 : idpConfig.getClockTolerance();

            SamlUtils.ValidateConditions(assertion.getConditions(), clockTolerance);
        } catch (ValidationException e) {
            if (this.getValidationResult().isValid()) {
                this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                        Error.CONDITION));
            }
            throw e;
        }
        // parse nameID and setup SubjectData
        NameID nameID = subject.getNameID();
        Validate.notNull(nameID, "NameID");

        // TODO #906507. Validate subjectconfirmation.subjectconfirmationData
        String nameIDFormat = nameID.getFormat();
        String nameIDStr = nameID.getValue();
        logger.info("NameID: " + nameIDStr);
        logger.info("NameIDFormat: " + nameIDFormat);
        SubjectData subjectData = new SubjectData(nameIDFormat, nameIDStr);

        // Fetch authcontext
        String context = null;
        String sessionIndex = null;
        DateTime sessionNotOnOrAfter = null;
        List<AuthnStatement> statements = assertion.getAuthnStatements();
        if (statements != null && !statements.isEmpty()) {
            // AuthnContext is required
            AuthnStatement statement = statements.get(0);
            sessionIndex = statement.getSessionIndex();
            AuthnContextClassRef authnContextRef = statement.getAuthnContext().getAuthnContextClassRef();
            // AuthnContextClassRef is optional.
            if (authnContextRef != null) {
                context = authnContextRef.toString();
            }
            // sessionNotOnOrAfter attribute is not used by Castle for now. Will
            // need to add the checking if
            // other idp use it.
            sessionNotOnOrAfter = statement.getSessionNotOnOrAfter();
            if (sessionNotOnOrAfter != null) {
                // Validate sessionNotOnOrAfter attribute
                try {
                    SamlUtils.ValidateSessionExpiry(sessionNotOnOrAfter, clockTolerance);
                } catch (ValidationException e) {
                    if (this.getValidationResult().isValid()) {
                        this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                                Error.BAD_REQUEST, Error.IDP_SESSION_EXPIRED));
                    }
                    throw e;
                }
            }
        }
        // Fetch Attributes
        List<Attribute> attributes = null;
        List<AttributeStatement> attributeStatements = assertion.getAttributeStatements();
        if (attributeStatements != null && !attributeStatements.isEmpty()) {
            attributes = this.parseAttributes(attributeStatements);
        }

        // Fetch expiration date
        DateTime exp = assertion.getConditions().getNotOnOrAfter();
        Date expDate = (exp == null) ? null : exp.toDate();

        AuthnData authnData = new AuthnData(subjectData, expDate, sessionIndex, context, attributes, tokenType, token,
                sessionNotOnOrAfter);
        return authnData;
    }

    /**
     * Create a List of Attribute objects from a List of opensaml
     * AttributeStatement. TODO will there be problem if more than one statement
     * exist. In that case we collect attributes across statements into one
     * attribute list. Maybe we should just look at one statement and leave that
     * as restriction.
     *
     * @return List of Attribute
     */
    private List<Attribute> parseAttributes(List<AttributeStatement> statements) {

        List<Attribute> attributes = new LinkedList<Attribute>();
        Validate.notEmpty(statements, "statements");
        Iterator<AttributeStatement> iterator = statements.iterator();
        while (iterator.hasNext()) {
            // for each attribute statement
            AttributeStatement statement = iterator.next();
            List<org.opensaml.saml2.core.Attribute> opensamlAttrs = statement.getAttributes();

            Iterator<org.opensaml.saml2.core.Attribute> iter = opensamlAttrs.iterator();
            while (iter.hasNext()) {
                // for each attribute
                org.opensaml.saml2.core.Attribute samlAttr = iter.next();
                String name = samlAttr.getName();
                String friendlyName = samlAttr.getFriendlyName();
                // we could have multiple values for each attribute.
                List<String> valueStrs = this.parseSamlAttributeValues(samlAttr.getAttributeValues());
                Attribute attr = new Attribute(name, friendlyName, valueStrs);
                attributes.add(attr);
            }
        }

        return attributes;
    }

    /**
     * Create a List of String representing attribute value in SAML 2 Assertion
     *
     * @return List of string.
     */
    private List<String> parseSamlAttributeValues(List<org.opensaml.xml.XMLObject> list) {
        List<String> valStrings = new LinkedList<String>();
        Validate.notNull(list, "Attributes");
        Iterator<org.opensaml.xml.XMLObject> iter = list.iterator();
        while (iter.hasNext()) {
            String valStr = iter.next().getDOM().getTextContent();
            if (valStr != null) {
                valStrings.add(valStr);
            }
            logger.trace("attribute value " + valStr);
        }
        return valStrings;
    }

}
