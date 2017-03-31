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
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.Status;
import org.opensaml.saml2.core.StatusCode;
import org.opensaml.xml.util.Base64;
import org.opensaml.xml.validation.ValidationException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Element;

import com.vmware.identity.websso.client.endpoint.SloListener;

/**
 * ValidationState for slo response received by slo controller.
 *
 */
public class SloResponseValidationState extends ValidationState {

    private static final Logger logger = LoggerFactory.getLogger(SloResponseValidationState.class);

    private final SloListener controller;
    private LogoutResponse opensamlResponse;

    /**
     *
	 */
    public SloResponseValidationState(HttpServletRequest request, SloListener controller) {
        super(controller.getMetadataSettings());
        this.setRequest(request);
        this.controller = controller;
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.websso.client.ValidationState#validate()
     */
    @Override
    public void validate() throws Exception {

        logger.info("Validating Logout Response..");
        Validate.notNull(getRequest());

        DecodeResponse();
        opensamlResponse = (LogoutResponse) SamlUtils.unmarshallSAMLObj(getSamlDom());

        Validate.notNull(opensamlResponse, "LogoutResponse object");

        Status status = opensamlResponse.getStatus();
        StatusCode statusCode = status.getStatusCode();

        this.setMessageID(opensamlResponse.getID());
        this.setStatus(statusCode.getValue());
        StatusCode substatus = statusCode.getStatusCode();
        if (null != substatus) {
            this.setSubStatus(substatus.getValue());
        }
        this.setIssuerVal(opensamlResponse.getIssuer().getValue());
        this.setIssueInstant(opensamlResponse.getIssueInstant());
        this.setDestination(opensamlResponse.getDestination());

        // These validations should be done regardless the server validation
        // status.
        validateDestination();
        Message requestMessage = validateInResponseTo();

        // now we have session index and SubjectData from original request.
        this.setMessageData(requestMessage.getMessageData());
        setSessionIndex(requestMessage.getSessionIndex());

        // validate issuer against corresponding request message found in
        // message store.
        // Issuer is required.
        validateSloIssuer(requestMessage, this.getIssuerVal());
        logger.info("Successfully validated issuer: ", this.getIssuerVal());

        String checkAlg = getRequest().getParameter(SamlUtils.SIGNATURE_ALGORITHM_PARAMETER);
        X509Certificate cert = SamlUtils.getIDPCertByIssuer(controller.getMetadataSettings(), this.getIssuerVal());

        SamlUtils samlUtils = new SamlUtils(cert, null, // does not need signing
                                                        // key
                null, // no need for signing algo
                checkAlg, // needed for verification
                null);
        try {
            samlUtils.validateRequestSignature(getRequest(), false // logout
                                                                   // response
                                                                   // does not
                                                                   // have to be
                                                                   // signed
                    );
        } catch (Exception error) {
            logger.error("Signature validation for logout response failed!", error);
            if (this.getValidationResult().isValid()) {
                this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, "BadResponse",
                        "Signature validation error."));
            }
            throw error;
        }
        logger.info("Successfully validated received Logout Response");

    }

    /**
     * @param opensamlResponse2
     */
    private Message validateInResponseTo() throws ValidationException {

        // this is login request ID
        String responseTo = opensamlResponse.getInResponseTo();
        if (responseTo == null) {
            this.isIdpInitiated = true;
            return null;
        }
        // else the value must match to a request message.
        Message message = this.controller.getMessageStore().get(responseTo);
        if (message == null) {
            setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_RESPONSE,
                    Error.IN_RESPONSE_TO));
            logger.info("No matching logout request: InResponseTo: " + responseTo);
            throw new ValidationException(Error.IN_RESPONSE_TO + " No matching logout request found.");
        }

        return message;
    }

    /**
     * @param parameter
     * @throws Exception
     */
    private void DecodeResponse() throws Exception {

        // 1. extract and decode saml response.

        String samlResponseStr = getRequest().getParameter(SamlUtils.SAML_RESPONSE_PARAMETER);

        // decompress and base64 decoding the response str.
        String decodedResponseStr = SamlUtils.extractResponse(samlResponseStr);
        Validate.notNull(decodedResponseStr, "decodedResponseStr");
        setSamlDom(SharedUtils.createDOM(decodedResponseStr));

        // 2. format and print out xml
        if (logger.isTraceEnabled()) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            Element rootE = getSamlDom().getDocumentElement();
            rootE.normalize();
            SharedUtils.formattedPrint(rootE, baos);
            decodedResponseStr = baos.toString("UTF-8");
            logger.trace("Decoded SAML Response is " + decodedResponseStr);
        }

        // 3. relayState
        String relayState = getRequest().getParameter(SamlUtils.RELAY_STATE_PARAMETER);
        if (relayState != null) {
            String decodedRelayState = new String(Base64.decode(relayState));
            this.setRelayState(relayState);
            logger.info("Decoded Relay State is " + decodedRelayState);
        }

    }

}
