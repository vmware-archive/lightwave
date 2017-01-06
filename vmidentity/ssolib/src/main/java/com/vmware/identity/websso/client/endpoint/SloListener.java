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
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.StatusCode;
import org.opensaml.xml.io.MarshallingException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.websso.client.Error;
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
import com.vmware.identity.websso.client.SloRequestValidationState;
import com.vmware.identity.websso.client.SloResponseValidationState;
import com.vmware.identity.websso.client.SubjectData;
import com.vmware.identity.websso.client.ValidationResult;
import com.vmware.identity.websso.client.ValidationState;
import com.vmware.identity.websso.client.WebssoClientException;

/**
 * SloListener is a @Controller which is used to receive HTTP Redirect logout
 * requests and responses from the IDP.
 *
 *
 */
@Controller
public class SloListener {

    private static final Logger logger = LoggerFactory
            .getLogger(SloListener.class);

    private ValidationState validationState;
    private String tenant;

    @Autowired
    private MetadataSettings metadataSettings;

    @Autowired
    private MessageStore messageStore;

    @Autowired
    private LogoutProcessor logoutProcessor;

    public void setMetadataSettings(final MetadataSettings metadataSettings) {
        this.metadataSettings = metadataSettings;
    }

    public MetadataSettings getMetadataSettings() {
        return this.metadataSettings;
    }

    public void setMessageStore(final MessageStore messageStore) {
        this.messageStore = messageStore;
    }

    public MessageStore getMessageStore() {
        return this.messageStore;
    }

    public void setLogoutProcessor(final LogoutProcessor logoutProcessor) {
        this.logoutProcessor = logoutProcessor;
    }

    public LogoutProcessor getLogoutProcessor() {
        return this.logoutProcessor;
    }

    /**
     * Handle SAML LogoutResponse and LogoutRequest at service provide side With
     * logout response received, the method validate and call relevant callback
     * to notify the client code. With logout request received, the method
     * validate notify client code via callback "logoutRequest" and post a
     * response via redirect back to the issuer of the request.
     *
     * @param tenant
     *            service provider alias that can be use to identify
     *            SPConfiguration.
     * @param request
     *            http request captured
     * @param httpResponse
     *            http response obj for sending response.
     * @throws IOException
     * @throws Exception
     */
    @RequestMapping(value = "/SsoClient/SLO/{tenant:.*}", method = RequestMethod.GET)
    public void slo(@PathVariable(value = "tenant") final String tenant,
            final HttpServletRequest request,
            final HttpServletResponse httpResponse) {

        logger.info("You sent a GET message to " + "Websso client library! ");
        LogoutProcessor processor = this.getLogoutProcessor();
        Validate.notNull(processor, "LogoutProcessor is not set");
        this.setTenant(tenant);

        if (request != null) {
            String samlResponseStr = request
                    .getParameter(SamlUtils.SAML_RESPONSE_PARAMETER);
            String samlRequestStr = request
                    .getParameter(SamlUtils.SAML_REQUEST_PARAMETER);

            if (samlResponseStr != null) {
                this.sloResponse(request, httpResponse);
            } else if (samlRequestStr != null) {
                this.sloRequest(request, httpResponse);
            } else {
                logger.error("Null request or response.");
                ValidationState validator = new SloRequestValidationState(
                        request, this);
                this.validationState = validator;

                validator.setValidationResult(new ValidationResult(
                        HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                        "No request string!"));

                Message errorMessage = this.createMessage(validator, true);

                processor.logoutError(errorMessage, request, httpResponse);
            }
        }

    }

    /**
     * Helper function handle SAML LogoutRequest only. A response is send out as
     * result of successful processing of the request and a return from client
     * callback.
     *
     * @param request
     *            httprequest sent to this URL
     * @param response
     * @throws Exception. Succeeded
     *             if no exception.
     */
    private void sloRequest(final HttpServletRequest request,
            final HttpServletResponse httpResponse) {

        // dependencies validations.
        logger.info("Logout request received: ");
        LogoutProcessor processor = this.getLogoutProcessor();
        ValidationState validator = new SloRequestValidationState(request, this);
        this.validationState = validator;

        try {
            validator.validate();

            // create Message, add to message store
            Message message = this.createMessage(validator, false);
            this.getMessageStore().add(message);

            // send the message via callback functions.
            processor.logoutRequest(message, request, httpResponse,
                    this.getTenant());

        } catch (Exception error) {
            logger.error("Logout request validation failed.", error);

            if (validator.getValidationResult().isValid()) {
                validator.setValidationResult(new ValidationResult(
                        HttpServletResponse.SC_BAD_REQUEST, Error.BAD_REQUEST,
                        error.getMessage()));
            }
            Message errorMessage = this.createMessage(validator, true);
            processor.logoutError(errorMessage, request, httpResponse);
            return;
        }
        // Got a valid slo request. Now create and send out a response to
        // requester.
        try {
            String redirectUrl = this.createResponseUrl(
                    validator.getMessageID(),
                    validator.getIssuerVal(), // target
                    (SubjectData) validator.getMessageData(),
                    validator.getRelayState());
            logger.info("SP SAML Response URL is " + redirectUrl);
            if (validator.getValidationResult().isValid()) {
                if (redirectUrl != null) {
                    SharedUtils.SetNoCacheHeader(httpResponse);
                    httpResponse.sendRedirect(redirectUrl);
                } else {
                    // redirectUrl can be null if IDP's SLO end point does not exist.
                    logger.warn(String.format(
                            "SLO end point does not exist for IDP: %s, SLO response is not sent.", validator.getIssuerVal()));
                }
            }
        } catch (Exception error) {
            // This is an error on client side in producing the response. Report
            // it to user
            processor.internalError(error, request, httpResponse);
        }
    }

    /**
     * Return response URL for redirecting.
     *
     * @param inResponseTo
     * @param idpUrl
     * @return url string. return null if failed
     * @throws IOException
     * @throws MarshallingException
     * @throws WebssoClientException
     * @throws NoSuchAlgorithmException
     * @throws Exception
     */
    private String createResponseUrl(final String inResponseTo,
            final String idpUrl, final SubjectData subject, String relayState)
            throws MarshallingException, IOException, WebssoClientException,
            NoSuchAlgorithmException {
        String retval = null;

        LogoutResponse samlResponse = this.createResponse(inResponseTo, idpUrl);
        if (samlResponse != null) {
            logger.info("SAML Response is created ");

            retval = this.createResponseUrlFromLogoutResponse(samlResponse,
                    idpUrl, relayState);
        }
        return retval;
    }

    /**
     * create signed response URL from LougoutResponse
     *
     * @param samlResponse
     *            LougoutResponse object where the url is create from.
     * @param idpUrl
     * @return response URL
     * @throws IOException
     * @throws MarshallingException
     * @throws WebssoClientException
     * @throws NoSuchAlgorithmException
     */
    private String createResponseUrlFromLogoutResponse(
            final LogoutResponse samlResponse, final String idpUrl,
            String relayState) throws MarshallingException, IOException,
            WebssoClientException, NoSuchAlgorithmException {

        logger.info("generateResponseUrl to IDP," + idpUrl);

        String retval = null;
        Validate.notNull(samlResponse, "Null LogoutResponse object.");
        Validate.notNull(idpUrl, "idpUrl can not be null");

        // 1. encode response into a string

        // encode the LougoutResponse obj as a query string
        // parameter for redirect binding.
        String encodedResponse = SamlUtils.encodeSAMLObject(samlResponse, true);

        SPConfiguration spConfig = this.getMetadataSettings()
                .getSPConfiguration(this.getTenant());

        String signingAlgo = spConfig.getSigningAlgorithm();
        PrivateKey key = spConfig.getSigningPrivateKey();

        if (null == key && null == signingAlgo) {
            throw new WebssoClientException(
                    "SP Configuration does not have signing key/algorithm set.");
        }

        // 2. calculate the message that needs to be signed
        // First, we need to find the SP configuration that this response issuer

        String toBeSigned = SamlUtils.generateRedirectUrlQueryStringParameters(
                null, encodedResponse, relayState,
                spConfig.getSigningAlgorithm(), // algoname
                null); // signature

        // 3. sign the response

        SamlUtils samlUtils = new SamlUtils(null, // cert. not used.
                key, signingAlgo, null, // check algorithm. not used.
                SamlUtils.getSpSloLocation(spConfig, // Issuer
                        SamlNames.HTTP_REDIRECT));

        String signature = samlUtils.signMessage(toBeSigned);

        // 4. combine everything into a redirect URL
        retval = samlResponse.getDestination();

        String queryString = SamlUtils
                .generateRedirectUrlQueryStringParameters(null,
                        encodedResponse, relayState, signingAlgo, signature);

        retval = retval + "?" + queryString;
        logger.info("Generated URL " + retval);

        return retval;

    }

    /**
     * Construct and initialize a LogoutResponse object.
     *
     * @param inResponseTo
     *            slo request ID.
     * @param idpUrl
     *            required
     * @param inResponseTo
     *            optional
     * @return LogoutResponse created.
     * @throws WebssoClientException
     */
    private LogoutResponse createResponse(final String inResponseTo,
            final String idpUrl) throws WebssoClientException {

        LogoutResponse retval = null;

        logger.info("generate SLO response for IDP: " + idpUrl);

        if (this.validationState.getValidationResult().getResponseCode() == HttpServletResponse.SC_OK) {
            Validate.notNull(idpUrl, "idpUrl");

            IDPConfiguration idpConfig = this.getMetadataSettings()
                    .getIDPConfigurationByEntityID(idpUrl);

            if (idpConfig == null) {
                throw new WebssoClientException("Uknown IDP configuration. IDP entity ID = : "+idpUrl);
            }
            try {
                // we should reply with Saml response
                String destination = SamlUtils.getIdpSloLocation(idpConfig,
                        SamlNames.HTTP_REDIRECT);

                // if IDP's SLO end point does not exist, return null LogoutResponse.
                if (destination == null) {
                    logger.warn(String.format("SLO end point does not exist for IDP: %s.", idpConfig.getAlias()));
                    return null;
                }

                String issuerValue = this.getIssuerVal(this.tenant);

                // get slo request validation result.
                ValidationResult valResult = this.validationState
                        .getValidationResult();
                retval = SamlUtils.createSamlLogoutResponse(issuerValue,
                        inResponseTo, // inResponseTo
                        destination, // destination
                        valResult.getStatus(), valResult.getSubstatus(), null);
            } catch (Exception e) {
                logger.error("Caught exception while generating response.", e);
                throw new WebssoClientException(e);
            }
        }
        return retval;
    }

    /**
     * @param tenant
     *            organization aliase.
     * @return
     * @throws WebssoClientException
     */
    private String getIssuerVal(final String tenant)
            throws WebssoClientException {

        if (tenant == null) {
            return null;
        }
        SPConfiguration sp = this.getMetadataSettings().getSPConfiguration(
                tenant);
        Validate.notNull(sp, "service provider setting unavailable for "
                + tenant);
        if (sp == null) {
            throw new IllegalArgumentException(
                    "service provider setting unavailable for " + tenant);
        }
        return sp.getEntityID();
    }

    /**
     * Helper function process slo LogoutResponse only
     *
     * @param request
     *            http request captured
     * @param httpResponse
     *            http response obj for sending response. * @throws Exception
     */
    private void sloResponse(final HttpServletRequest request,
            final HttpServletResponse httpResponse) {

        // dependencies validations.
        logger.info("Logout response received: ");
        LogoutProcessor processor = this.getLogoutProcessor();
        ValidationState validator = new SloResponseValidationState(request,
                this);
        this.validationState = validator;

        try {
            validator.validate();

            // log Message
            Message message = this.createMessage(validator, true);

            // send the message via callback functions.
            if (validator.getStatus().equals(StatusCode.SUCCESS_URI)) {
                this.getLogoutProcessor().logoutSuccess(message, request,
                        httpResponse);
            } else {
                logger.error("Received Logout Response status is a 'Logout Failure'");
                this.getLogoutProcessor().logoutError(message, request,
                        httpResponse);
            }
        } catch (Exception error) {
            logger.error("Logout Response validation failed. Exception:", error);

            if (validator.getValidationResult().isValid()) {
                validator.setValidationResult(new ValidationResult(
                        HttpServletResponse.SC_BAD_REQUEST, Error.BAD_RESPONSE,
                        error.getMessage()));
            }
            Message errorMessage = this.createMessage(validator, true);
            processor.logoutError(errorMessage, request, httpResponse);

        }
    }

    private Message createMessage(ValidationState validator, Boolean isResponse) {
        // TODO optimization opportunity: I could parameterize MessageType and
        // put this function in base class
        String status = validator.getStatus();
        String substatus = validator.getSubStatus();
        String relayState = validator.getRelayState();
        String sessionId = validator.getSessionIndex();
        Message message = new Message(isResponse ? MessageType.LOGOUT_RESPONSE
                : MessageType.LOGOUT_REQUEST, validator.getMessageID(),
                relayState, validator.getIssueInstant(),
                validator.getIssuerVal(), validator.getDestination(), status,
                substatus, sessionId, validator.getMessageData(), null, validator.isIdpInitiated());
        message.setValidationResult(validator.getValidationResult());
        logger.info("Logout response/request validation result:");
        logger.info(
                "Status: %s  Substatus: %s SessionIndex:%s RelayState:%s  ",
                status, substatus, sessionId, relayState);
        return message;

    }

    /**
     * @return the alias name of tenant
     */
    public String getTenant() {
        return this.tenant;
    }

    /**
     * @param tenant
     *            the alias name of tenant to set
     */
    public void setTenant(String tenant) {
        this.tenant = tenant;
    }

}
