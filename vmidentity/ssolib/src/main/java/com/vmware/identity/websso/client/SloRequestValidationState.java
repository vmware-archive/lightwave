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
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.NameID;
import org.opensaml.saml2.core.SessionIndex;
import org.opensaml.xml.util.Base64;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Element;

import com.vmware.identity.websso.client.endpoint.SloListener;

/**
 * ValidationState for slo request received by slo controller.
 *
 */
public class SloRequestValidationState extends ValidationState {

    private static final Logger logger = LoggerFactory.getLogger(SloRequestValidationState.class);

    private final SloListener controller;
    private LogoutRequest logoutRequest;

    /**
	 *
	 */
    public SloRequestValidationState(HttpServletRequest request, SloListener controller) {
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

        logger.info("Validating SLO Request..");
        Validate.notNull(getRequest());

        DecodeResponse();
        logoutRequest = (LogoutRequest) SamlUtils.unmarshallSAMLObj(getSamlDom());

        Validate.notNull(logoutRequest, "LogoutRequest object");

        this.setMessageID(logoutRequest.getID());
        this.setIssuerVal(logoutRequest.getIssuer().getValue());
        this.setIssueInstant(logoutRequest.getIssueInstant());
        this.setDestination(logoutRequest.getDestination());

        // now we have session index and SubjectData from original request.
        NameID nameID = logoutRequest.getNameID();
        Validate.notNull(nameID, "nameID");
        this.setMessageData(new SubjectData(nameID.getFormat(), nameID.getValue()));

        List<SessionIndex> sessionIndexs = logoutRequest.getSessionIndexes();
        if (sessionIndexs != null && !sessionIndexs.isEmpty()) {
            this.setSessionIndex(sessionIndexs.get(0).getSessionIndex());
        }
        logger.info("Optional sessionIndex value is: ", this.getSessionIndex());

        // These validations should be done regardless the server validation
        // status.
        validateDestination();

        // validate issuer against corresponding request message found in
        // message store.
        // Issuer is required.
        validateSloIssuer(null, this.getIssuerVal());
        logger.info("Successfully validated issuer: ", this.getIssuerVal());

        String checkAlg = getRequest().getParameter(SamlUtils.SIGNATURE_ALGORITHM_PARAMETER);
        X509Certificate cert = SamlUtils.getIDPCertByIssuer(controller.getMetadataSettings(), this.getIssuerVal());

        SamlUtils samlUtils = new SamlUtils(cert, null, // does not need signing
                                                        // key
                null, // no need for signing algo
                checkAlg, // needed for verification
                null);
        try {
            samlUtils.validateRequestSignature(getRequest(), true // logout
                                                                  // request
                                                                   // have to be
                                                                   // signed
                    );
        } catch (Exception error) {
            logger.info("SLO request signature validation failed.", error);
            if (this.getValidationResult().isValid()) {
                this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                        Error.SIGNATURE));
            }
            throw error;
        }
        logger.info("SLO request validation succeeded");
    }

    /**
     * @param parameter
     * @throws Exception
     */
    private void DecodeResponse() throws Exception {

        // 1. extract and decode saml request.

        String samlRequestStr = this.getRequest().getParameter(SamlUtils.SAML_REQUEST_PARAMETER);
        logger.trace("Coded SAML Request is " + samlRequestStr);

        // decompress and base64 decoding the response str.
        String decodedResponseStr = SamlUtils.extractResponse(samlRequestStr);
        Validate.notEmpty(decodedResponseStr, "decodedResponseStr");

        setSamlDom(SharedUtils.createDOM(decodedResponseStr));
        // 2. format and print out xml
        if (logger.isDebugEnabled()) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            Element rootE = getSamlDom().getDocumentElement();
            rootE.normalize();
            SharedUtils.formattedPrint(rootE, baos);
            decodedResponseStr = baos.toString("UTF-8");
            logger.info("Decoded SAML Request is " + decodedResponseStr);
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
