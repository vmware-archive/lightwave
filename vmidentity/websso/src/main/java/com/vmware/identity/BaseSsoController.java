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

import java.io.IOException;
import java.util.Locale;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.AuthnRequest;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.SAMLResponseSender;
import com.vmware.identity.samlservice.SAMLResponseSenderFactory;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.SAMLAuthnResponseSenderFactory;
import com.vmware.identity.session.SessionManager;

/**
 * Base class for all SSO controllers with some common protected methods
 *
 */
public abstract class BaseSsoController {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(BaseSsoController.class);

    /**
     * Core SSO request processing
     *
     * @param locale
     *            User locale
     * @param tenant
     *            Tenant name, not null (resolve default tenant name prior to
     *            the call)
     * @param request
     *            SSO request
     * @param response
     *            Response object
     * @param authenticator
     *            authenticator to use, will be different for different
     *            controllers (Kerb, UNP etc.)
     * @param messageSource
     *            message source to use
     * @param sessionManager
     *            session manager to use
     * @return boolean if this is a new session, the function returns needLoginView = true. This allows
     *       to send user a login view
     */
    protected void processSsoRequest(Locale locale, String tenant,
            HttpServletRequest request, HttpServletResponse response,
            AuthenticationFilter<AuthnRequestState> authenticator,
            AuthnRequestState requestState,
            MessageSource messageSource, SessionManager sessionManager) {

        requestState.setLocal(locale);
        requestState.setMessageSource(messageSource);
        requestState.setNeedLoginView(false);  //if this is a new session, the function returns needLoginView = true;
        try {
            try {
                requestState.parseRequestForTenant(tenant, authenticator);
            } catch (IllegalStateException e) {
                logger.error("Could not parse tenant request {}", e.toString());
            }
            Document token = null;
            if (requestState.getValidationResult().isValid()) {
                token = requestState.authenticate(tenant, authenticator);

                // if proxying the request.  We are in async state.
                // The response is handled in logonProcessor
                if (requestState.isProxying() && requestState.getValidationResult().isValid()) {
                    return;
                }
            }
            ValidationResult vr = requestState.getValidationResult();
            Validate.notNull(vr, "Null validation result.");

            requestState.addResponseHeaders(response);
            if (vr.needsLogonView()) {
                //Post login form
                requestState.setNeedLoginView(true);
                return;
            }

            SAMLResponseSenderFactory responseSenderFactory =
                    new SAMLAuthnResponseSenderFactory();

            SAMLResponseSender responseSender = responseSenderFactory.buildResponseSender
                    (tenant, response, locale,
                    null,  //for IDP initiated, no relay state in post Response to SP
                    requestState,
                    requestState.getAuthnMethod(),
                    requestState.getSessionId(),
                    requestState.getPrincipalId(),
                    messageSource,sessionManager);

            AuthnRequest authnReq = requestState.getAuthnRequest();
            String rpID = authnReq == null? null:authnReq.getIssuer().getValue();   //It is possible rpID is not available due to bad request message

            responseSender.sendResponseToRP(rpID, token);

            logger.info("End processing SP-Initiated SSO response. Session was created.");

        } catch (IOException e) {
            logger.error("Caught IO exception " + e.toString());
        }
    }
}
