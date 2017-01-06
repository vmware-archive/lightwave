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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;

/**
 * ValidationState holds idp message authentication state information in websso
 * client lib controllers in its message exchange with IDP end points.
 */
public abstract class ValidationState {
    // state infor
    private String messageID;
    protected boolean isIdpInitiated;
    private String relayState;
    private String status;
    private String subStatus;
    private String issuerVal;
    private DateTime issueInstant;
    private String destination;
    private MessageData messageData;
    private String sessionIndex;
    private ValidationResult validationResult;
    private final RelaxedURIComparator comparator;

    // helper data
    private MetadataSettings metadataSettings;
    private HttpServletRequest request;
    private Document samlDom;

    private static final Logger logger = LoggerFactory.getLogger(ValidationState.class);

    protected ValidationState(MetadataSettings metadataSettings) {
        // instance variable members are default to null.
        validationResult = new ValidationResult();
        this.metadataSettings = metadataSettings;
        this.comparator = new RelaxedURIComparator();
    }

    public abstract void validate() throws Exception;

    /**
     * validate optional destination in a SAML Response.
     *
     * @return void
     * @throws WebssoClientException
     *             validation failed.
     */
    public void validateDestination() throws WebssoClientException {

        if (this.getDestination() == null) {
            return;
        }
        String requestUrl = this.getRequest().getRequestURL().toString();
        logger.info("Validating request destination: HttpservletRequest destination=" + requestUrl
                + "SAML message destination=" + this.getDestination());
        if (!this.comparator.compare(getDestination(), requestUrl)) {
            this.validationResult = new ValidationResult(HttpServletResponse.SC_FORBIDDEN, "Forbidden",
                    Error.SAML_DESTINATION);
            throw new WebssoClientException(Error.SAML_DESTINATION);
        }
    }

    /**
     * validate issuer parameter of SLO request from IDP
     *
     * @param message
     *            Optional. The matching SLO request message. Only available in
     *            in SLO response validation.
     * @param issuer
     *            Required. The Issuer object to be validated.
     * @return message object that corresponds to the logout request throw if
     *         there is no match
     * @throws WebssoClientException
     *             for missing or invalid issuer
     */
    protected void validateSloIssuer(final Message message, final String issuerVal) throws WebssoClientException {
        try {
            logger.info("Validating SLO message issuer: " + issuerVal);
            if (issuerVal == null) {
                throw new WebssoClientException("Issuer is missing in slo request/response messsage!");
            }

            MetadataSettings metaSettings = this.getMetadataSettings();
            IDPConfiguration idpConfig = metaSettings.getIDPConfigurationByEntityID(issuerVal);
            if (idpConfig == null) {
                throw new WebssoClientException("Uknown IDP configuration. IDP entity ID = : "+issuerVal);
            }

            if (message != null) {
                /*
                 * Validating SLO response given a matching slo request message
                 * found. To verify the issuer, match the target of the slo
                 * request to slo response issuer's slo service
                 */
                String originalRequstTarget = message.getTarget();
                Validate.notNull(originalRequstTarget, "Matching SLO request destination");

                String issuerSloLocation = SamlUtils.getIdpSloLocation(idpConfig, SamlNames.HTTP_REDIRECT);

                // Since a SLO response from IDP has been received and a matching SLO request is also found,
                // IDP's SLO end point should exist and can not be null.
                if (issuerSloLocation == null || !issuerSloLocation.equals(originalRequstTarget)) {
                    logger.warn("logout response issuer is a unknown IDP! Issuer: " + issuerVal
                            + "	The original slo request target is:" + originalRequstTarget
                            + "	The idp slo location is:" + issuerSloLocation);
                    throw new WebssoClientException("Issuer:" + issuerVal);
                }
            }
            //else no matching request was found. This is fine since InResponseTo is optional. This could be IDP-initiated profile.

        } catch (WebssoClientException e) {
            this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                    Error.ISSUER));
            throw e;
        } catch (Exception e) {
            this.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                    Error.ISSUER));
            throw new WebssoClientException(this.getValidationResult().getSubstatus(), e);
        }
    }

    /**
     * @return the messageID
     */
    public String getMessageID() {
        return messageID;
    }

    /**
     * @param messageID
     *            the messageID to set
     */
    public void setMessageID(String messageID) {
        this.messageID = messageID;
    }

    /**
     * @return the relayState
     */
    public String getRelayState() {
        return relayState;
    }

    /**
     * @param relayState
     *            the relayState to set
     */
    public void setRelayState(String relayState) {
        this.relayState = relayState;
    }

    /**
     * @return the status
     */
    public String getStatus() {
        return status;
    }

    /**
     * @param status
     *            the status to set
     */
    public void setStatus(String status) {
        this.status = status;
    }

    /**
     * @return the subStatus
     */
    public String getSubStatus() {
        return subStatus;
    }

    /**
     * @param subStatus
     *            the subStatus to set
     */
    public void setSubStatus(String subStatus) {
        this.subStatus = subStatus;
    }

    /**
     * @return the issuerVal
     */
    public String getIssuerVal() {
        return issuerVal;
    }

    /**
     * @param issuerVal
     *            the issuerVal to set
     */
    public void setIssuerVal(String issuerVal) {
        this.issuerVal = issuerVal;
    }

    /**
     * @return the issueInstant
     */
    public DateTime getIssueInstant() {
        return issueInstant;
    }

    /**
     * @param issueInstant
     *            the issueInstant to set
     */
    public void setIssueInstant(DateTime issueInstant) {
        this.issueInstant = issueInstant;
    }

    /**
     * @return the destination
     */
    public String getDestination() {
        return destination;
    }

    /**
     * @param destination
     *            the destination to set
     */
    public void setDestination(String destination) {
        this.destination = destination;
    }

    /**
     * @return the messageData
     */
    public MessageData getMessageData() {
        return messageData;
    }

    /**
     * @param messageData
     *            the messageData to set
     */
    public void setMessageData(MessageData messageData) {
        this.messageData = messageData;
    }

    /**
     * @return the validationResult
     */
    public ValidationResult getValidationResult() {
        return validationResult;
    }

    /**
     * @param validationResult
     *            the validationResult to set
     */
    public void setValidationResult(ValidationResult validationResult) {
        this.validationResult = validationResult;
    }

    /**
     * @return reference of samlDom
     */
    public Document getSamlDom() {
        return samlDom;
    }

    /**
     * @param samlDom
     *            the samlDom to set
     */
    public void setSamlDom(Document samlDom) {
        this.samlDom = samlDom;
    }

    /**
     * @return the sessionIndex
     */
    public String getSessionIndex() {
        return sessionIndex;
    }

    /**
     * @param sessionIndex
     *            the sessionIndex to set
     */
    public void setSessionIndex(String sessionIndex) {
        this.sessionIndex = sessionIndex;
    }

    /**
     * @return the request
     */
    public HttpServletRequest getRequest() {
        return request;
    }

    /**
     * @param request
     *            the request to set
     */
    public void setRequest(HttpServletRequest request) {
        this.request = request;
    }

    /**
     * @return the metadataSettings
     */
    public MetadataSettings getMetadataSettings() {
        return metadataSettings;
    }

    /**
     * @param metadataSettings
     *            the metadataSettings to set
     */
    public void setMetadataSettings(MetadataSettings metadataSettings) {
        this.metadataSettings = metadataSettings;
    }

    public boolean isIdpInitiated() {
        return isIdpInitiated;
    }

}
