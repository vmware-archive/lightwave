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
import java.util.Locale;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.commons.lang.Validate;

import org.opensaml.saml2.core.StatusCode;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.websso.client.Error;
import com.vmware.identity.websso.client.LogonProcessor;
import com.vmware.identity.websso.client.LogonProcessorEx;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MessageType;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SsoValidationState;
import com.vmware.identity.websso.client.ValidationResult;
import com.vmware.identity.websso.client.ValidationState;

/**
 * SsoResponseListener is a @Controller which is used to receive HTTP Post
 * responses from the IDP.
 */
@Controller
public class SsoResponseListener {

    private static final Logger logger = LoggerFactory.getLogger(SsoResponseListener.class);

    @Autowired
    private MetadataSettings metadataSettings;

    @Autowired
    private MessageStore messageStore;

    @Autowired
    private LogonProcessor logonProcessor;

    private boolean assertionMustBeSigned = true;

    /**
     * Deprecated mapping point without locale parameter. Will remove once NGC changes to call the
     * new locale-capable function directly.
     *
     *
     *
     * Process SAML Authentication Response from IDP. It maps to http 'post'
     * request with response embedded in the post. The following information is
     * returned: base 64 coded response decoded response NameIDFormat used
     * decoded relay state. It also log a message in the message store and
     * notify callbacks.
     *
     * @param request
     *            http request passed into the call
     * @param ssoResponseMapping
     *            client defined string bean
     * @return void
     * @throws IOException
     */
    @Deprecated
    public void consumeResponse( @PathVariable(value = "tenant") final String tenant, /*
                                                                                      * make
                                                                                      * compiler
                                                                                      * happy
                                                                                      */
            HttpServletRequest request, HttpServletResponse httpResponse) {
        consumeResponse(null, tenant,request, httpResponse);
    }
    /**
     * Process SAML Authentication Response from IDP. It maps to http 'post'
     * request with response embedded in the post. The following information is
     * returned: base 64 coded response decoded response NameIDFormat used
     * decoded relay state. It also log a message in the message store and
     * notify callbacks.
     * @Locale locale
     *           Client machine Browser locale
     * @param request
     *            http request passed into the call
     * @param ssoResponseMapping
     *            client defined string bean
     * @return void
     * @throws IOException
     */
    @RequestMapping(value = "/SsoClient/SSO/{tenant:.*}", method = RequestMethod.POST)
    public void consumeResponse(Locale locale, @PathVariable(value = "tenant") final String tenant, /*
                                                                                      * make
                                                                                      * compiler
                                                                                      * happy
                                                                                      */
            HttpServletRequest request, HttpServletResponse httpResponse) {

        logger.info("You have POST'ed to " + "Websso client library!");
        Validate.notNull(request);
        ValidationState validator = new SsoValidationState(request, this);

        try {

            validator.validate();

            // create Message, add to message store
            Message message = this.createMessage(validator);

            // send the message via callback functions.
            if (validator.getStatus().equals(StatusCode.SUCCESS_URI)) {
                authenticationSuccess(message, locale, tenant, request, httpResponse);
            } else {
                authenticationError(message, locale,tenant, request, httpResponse);
            }

        } catch (Exception e) {
            logger.error("Authentication Exception:", e);
            if (validator.getValidationResult().isValid()) {
                validator.setValidationResult(new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                        Error.BAD_RESPONSE, e.getMessage()));
                internalError(e,locale,tenant,request,httpResponse);
            } else {
                Message errorMessage = this.createMessage(validator);
                authenticationError(errorMessage, locale, tenant, request, httpResponse);
            }

        }
    }

    private void authenticationError(Message message, Locale locale, String tenant, HttpServletRequest request, HttpServletResponse httpResponse) {
        if ((this.logonProcessor instanceof LogonProcessorEx) ) {
            ((LogonProcessorEx) this.logonProcessor).authenticationError(message,locale, tenant, request,httpResponse);
        } else {
            this.logonProcessor.authenticationError(message,request,httpResponse);
        }
    }
    private void internalError(Exception exception, Locale locale, String tenant, HttpServletRequest request, HttpServletResponse httpResponse) {
        if ((this.logonProcessor instanceof LogonProcessorEx) ) {
            ((LogonProcessorEx) this.logonProcessor).internalError(exception,locale, tenant, request,httpResponse);
        } else {
            this.logonProcessor.internalError(exception,request,httpResponse);
        }
    }

    private void authenticationSuccess(Message message, Locale locale, String tenant, HttpServletRequest request, HttpServletResponse httpResponse) {
        if ((this.logonProcessor instanceof LogonProcessorEx) ) {
            ((LogonProcessorEx) this.logonProcessor).authenticationSuccess(message,locale, tenant, request,httpResponse);
        } else {
            this.logonProcessor.authenticationSuccess(message,request,httpResponse);
        }
    }

    private Message createMessage(ValidationState validator) {

        Message message = new Message(MessageType.AUTHN_RESPONSE, validator.getMessageID(), validator.getRelayState(),
                validator.getIssueInstant(), validator.getIssuerVal(), validator.getDestination(),
                validator.getStatus(), validator.getSubStatus(), validator.getSessionIndex(),
                validator.getMessageData(), null, validator.isIdpInitiated());
        message.setValidationResult(validator.getValidationResult());
        return message;

    }

    public void setMetadataSettings(MetadataSettings metadataSettings) {
        this.metadataSettings = metadataSettings;
    }

    public MetadataSettings getMetadataSettings() {
        return this.metadataSettings;
    }

    public void setMessageStore(MessageStore messageStore) {
        this.messageStore = messageStore;
    }

    public MessageStore getMessageStore() {
        return this.messageStore;
    }

    public void setLogonProcessor(LogonProcessor logonProcessor) {
        this.logonProcessor = logonProcessor;
    }

    /**
     * @return the assertionMustBeSigned
     */
    public boolean isAssertionMustBeSigned() {
        return this.assertionMustBeSigned;
    }

    /**
     * The default value is true.
     * Note: In production deployment we always sign assertion. This setter allows testing code to test without signing.
     *
     * @param assertionMustBeSigned
     *            the assertionMustBeSigned to set
     */
    public void setAssertionMustBeSigned(boolean assertionMustBeSigned) {
        this.assertionMustBeSigned = assertionMustBeSigned;
    }

}
