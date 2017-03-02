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
package com.vmware.identity.websso.client.endpoint;

import java.io.IOException;

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.LogoutRequest;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.LogoutProcessor;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MessageType;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SPConfiguration;
import com.vmware.identity.websso.client.SamlNames;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SharedUtils;
import com.vmware.identity.websso.client.SloRequestSettings;
import com.vmware.identity.websso.client.SubjectData;

/**
 * SLO endpoint component responsible for constructing logout request to the
 * IDP.
 *
 */
@Component
public class SloRequestSender {
    private static final Logger logger = LoggerFactory.getLogger(SloRequestSender.class);

    @Autowired
    private MetadataSettings metadataSettings;

    @Autowired
    private MessageStore messageStore;

    @Autowired
    private LogoutProcessor logoutProcessor;

    public void setMetadataSettings(MetadataSettings metadataSettings) {
        this.metadataSettings = metadataSettings;
    }

    public MetadataSettings getMetadataSettings() {
        return metadataSettings;
    }

    public void setMessageStore(MessageStore messageStore) {
        this.messageStore = messageStore;
    }

    public MessageStore getMessageStore() {
        return messageStore;
    }

    public void setLogoutProcessor(LogoutProcessor logoutProcessor) {
        this.logoutProcessor = logoutProcessor;
    }

    public LogoutProcessor getLogoutProcessor() {
        return logoutProcessor;
    }

    /**
     * Entry point for sending SLO request to IDP. This function results a HTTP
     * redirect with logout parameters attached to the url used in the redirect.
     *
     * @param requestSettings
     *            SSO request settings
     * @param response
     *            HTTP response servlet used for redirecting the user browser
     *            signing on request..
     * @throws IOException
     */
    public void sendRequest(SloRequestSettings requestSettings, HttpServletResponse response) throws IOException {

        Validate.notNull(requestSettings, "requestSettings");
        Validate.notNull(response, "response");
        Validate.notNull(getMessageStore(), "MessageStore");
        Validate.notNull(getMetadataSettings(), "MetadataSettings");

        String redirectUrl = getRequestUrl(requestSettings);

        if (redirectUrl != null) {
            SharedUtils.SetNoCacheHeader(response);
            response.sendRedirect(redirectUrl);
        } else {
            if (SamlUtils.isIdpSupportSLO(getMetadataSettings(), requestSettings)) {
                logger.error("Failed to send out the SLO request!");
            } else {
                logger.warn(String.format(
                        "SLO end point does not exist for IDP: %s, SLO request is not sent.", requestSettings.getIDPAlias()));
            }
        }
    }

    /**
     * Create logout request url with SloRerequestSettings object.
     *
     * @param requestSettings
     *            SSO request settings
     * @return url if succeed null if failed
     */
    public String getRequestUrl(SloRequestSettings requestSettings) {
        return getRequestUrl(requestSettings,null);
    }


    /**
     * Create logout request url with SloRerequestSettings object.
     *
     * @param requestSettings
     *            SSO request settings
     * @param reqID
     * @return url if succeed null if failed
     */
    public String getRequestUrl(SloRequestSettings requestSettings, String reqID) {

        Validate.notNull(requestSettings);
        Validate.notEmpty(reqID, "regID");

        String redirectUrl = null;

        try {
            logger.info("Producing redirect url");
            LogoutRequest samlRequest = createRequest(requestSettings, reqID);
            if (samlRequest != null) {
                logger.info("SP LogoutRequest is created.");

                redirectUrl = createRequestUrlFromLogoutRequest(samlRequest, requestSettings);
            }
            return redirectUrl;
        } catch (Exception e) {
            logger.error("Failed to create logout request url! Exception = ", e);
            logoutProcessor.internalError(e, null, null);
        }
        return redirectUrl;
    }

    /**
     * Create redirect url from LogoutRequest obj.
     *
     * @param samlRequest
     * @param requestSettings
     * @return
     */
    private String createRequestUrlFromLogoutRequest(LogoutRequest samlRequest, SloRequestSettings requestSettings) {

        String retval = null;
        logger.info("createRequestUrlFromLogoutRequest, IDP: " + requestSettings.getIDPAlias());
        Validate.notNull(samlRequest, "samlRequest can't be null");
        Validate.notNull(requestSettings, "SloRequestSettings can't be null");

        try {
            SPConfiguration spConfig = getMetadataSettings().getSPConfiguration(requestSettings.getSPAlias());
            if (spConfig == null) {
                throw new IllegalArgumentException("service provider setting unavailable for "
                        + requestSettings.getSPAlias());
            }

            String issuer = SamlUtils.getSpSloLocation(spConfig, SamlNames.HTTP_REDIRECT);
            Validate.notEmpty(issuer, "issuer");

            // 2 create an instance for SamlUtils
            SamlUtils samlUtils = new SamlUtils(null, // cert. not used.
                    spConfig.getSigningPrivateKey(), spConfig.getSigningAlgorithm(), null, // check
                                                                                           // algorithm.
                                                                                           // not
                                                                                           // used.
                    issuer);

            // 1. encode request into a string

            // encode the LogoutResponse obj as a query string parameter for
            // redirect binding.
            String encodedRequest = SamlUtils.encodeSAMLObject(samlRequest, true);

            // 2. calculate the message that needs to be signed
            Boolean isSigned = requestSettings.isSigned();
            if (isSigned) {
                Validate.notNull(spConfig.getSigningAlgorithm(), "signing algorithm");
                Validate.notNull(spConfig.getSigningPrivateKey(), "signing key");
            }

            String toBeSigned = SamlUtils.generateRedirectUrlQueryStringParameters(encodedRequest, null,
                    requestSettings.getRelayState(), isSigned ? spConfig.getSigningAlgorithm() : null, null);
            logger.info("Relay State: " + requestSettings.getRelayState());
            if (toBeSigned == null || toBeSigned.isEmpty()) {
                logger.warn("Message to be signed is null or empty");
            }

            retval = samlRequest.getDestination();

            // 3. calculate signature
            if (isSigned) {
                String signature = samlUtils.signMessage(toBeSigned);

                // 4. combine everything into a redirect URL. same as step 2
                // except now we attach signature.
                String queryString = SamlUtils.generateRedirectUrlQueryStringParameters(encodedRequest, null,
                        requestSettings.getRelayState(), spConfig.getSigningAlgorithm(), signature);
                retval += "?" + queryString;
            } else { // not signed. attach only the unsigned query string.
                retval += "?" + toBeSigned;
            }

        } catch (Exception e) {
            retval = null;
        }

        return retval;

    }

    /**
     * Construct LogoutRequest from requestSettings
     *
     * @param requestSettings
     * @param reqID   optional requestID. The ID will be generated if not provided by caller.
     * @return openSaml LogoutRequest
     */
    private LogoutRequest createRequest(SloRequestSettings requestSettings, String reqID) {

        LogoutRequest request = null;

        logger.info("generateRequest for IDP: " + requestSettings.getIDPAlias());

        try {
            // 1 Some validation of required information
            IDPConfiguration idpConfig = getMetadataSettings().getIDPConfiguration(requestSettings.getIDPAlias());
            Validate.notNull(idpConfig, "IDPConfiguration not found for " + requestSettings.getIDPAlias());

            SPConfiguration spConfig = getMetadataSettings().getSPConfiguration(requestSettings.getSPAlias());

            if (spConfig == null) {
                throw new IllegalArgumentException("service provider setting unavailable for "
                        + requestSettings.getSPAlias());
            }

            // issuer is always entity id instead of slo service location.
            String issuer = spConfig.getEntityID();

            // 2 create an instance for SamlUtils
            SamlUtils samlUtils = new SamlUtils(null, // cert. not used.
                    null, // signing key not used.
                    null, // sign algorithm. not used.
                    null, // check algorithm. not used.
                    issuer);

            String destination = SamlUtils.getIdpSloLocation(idpConfig, SamlNames.HTTP_REDIRECT);

            // if IDP's SLO end point does not exist, return null LogoutRequest.
            if (destination == null) {
                logger.warn(String.format("SLO end point does not exist for IDP: %s.", idpConfig.getAlias()));
                return null;
            }

            // 3. create openSaml LogoutRequest object
            request = samlUtils.createSamlLogoutRequest(
                    reqID,
                    destination, requestSettings.getNameIDFormat(), requestSettings.getSubject(),
                    requestSettings.getSessionIndex());

            // 4. create a logout message in message store.
            Message message = new Message(MessageType.LOGOUT_REQUEST, request.getID(), requestSettings.getRelayState(),
                    request.getIssueInstant(), request.getIssuer().getValue(), destination, // target
                    null, // status
                    null, // substatus
                    requestSettings.getSessionIndex(), new SubjectData(requestSettings.getNameIDFormat(),
                            requestSettings.getSubject()), // MessageDatda
                    null,// tag
                    false); //Idp_initiated
            getMessageStore().add(message);

        } catch (Exception e) {
            logger.error("Caught exception while generating request " + e.toString() + ", will ignore the request.");
            return null;
        }

        return request;
    }
}
