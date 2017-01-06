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

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.samlservice.LogoutState;
import com.vmware.identity.samlservice.ProcessingFilter;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.impl.SamlServiceImpl;
import com.vmware.identity.session.SessionManager;

/**
 * Single Logout (SLO) service controller
 * This accepts both LogoutRequest and LogoutResponse messages
 *  over HTTP Redirect binding
 *
 */
@Controller
public final class SloController {

    @Autowired
    private MessageSource messageSource;

    @Autowired
    private ProcessingFilter<LogoutState> processor;

    @Autowired
    private SessionManager sessionManager;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SloController.class);

    /**
     * Handle SAML LogoutRequest/LogoutResponse
     */
    @RequestMapping(value = "/websso/SAML2/SLO/{tenant:.*}", method = RequestMethod.GET)
    public void slo(Locale locale, @PathVariable(value = "tenant") String tenant, Model model
        , HttpServletRequest request, HttpServletResponse response) throws IOException {
        logger.info("Welcome to Single Logout request/response handler! " +
                "The client locale is "+ locale.toString() + ", tenant is " + tenant);

        LogoutState logoutState = new LogoutState(request, response
            , sessionManager, locale, messageSource );

        try {
            try {
                logoutState.parseRequestForTenant(tenant, processor);
            } catch (IllegalStateException e) {
            }
            if (logoutState.getValidationResult().isValid()) {
                logoutState.process(tenant, processor);
            }
            String redirectUrl = null;
            boolean externalAuthenticated = false;
            if (logoutState.getValidationResult().isValid()) {
                externalAuthenticated = logoutState.checkIsExternalAuthenticated();
            }
            if (logoutState.needLogoutRequest() &&  !externalAuthenticated) {
                //send slo request to all non-initiating relying parties
                SamlServiceImpl.sendSLORequestsToOtherParticipants(tenant, logoutState);
            }

            /*
             * For external authenticated session, response will be sent out
             * when response from IDP is received.
             */
            if (logoutState.needLogoutResponse() && !externalAuthenticated) {

                redirectUrl = SamlServiceImpl.buildResponseUrl(tenant, logoutState);

                if (redirectUrl != null) {
                    response.sendRedirect(redirectUrl);
                } else {
                    if (!logoutState.getValidationResult().isValid()) {
                        SamlServiceImpl.sendLogoutError(locale, response, logoutState, messageSource);
                    } else {
                        // SLO end point does not exist, do nothing.
                        logger.warn("SLO end point for initiating service provider does not exist, no logout response is sent.");
                    }
                }
            }
            else if (!logoutState.getValidationResult().isValid()) {
                SamlServiceImpl.sendLogoutError(locale, response, logoutState, messageSource);
            }
         } catch (Exception e) {
             logger.error("Exception in sending out single log out messages " +
                     "to service providers."+e.getMessage(), e);
        }

        model.addAttribute("tenant", tenant);
    }

    /**
     * Handle SAML LogoutRequest/LogoutResponse for default tenant
     */
    @RequestMapping(value = "/websso/SAML2/SLO", method = RequestMethod.GET)
    public void sloDefaultTenant(Locale locale, Model model, HttpServletRequest request
        , HttpServletResponse response) throws IOException {
        logger.info("Welcome to Single Logout request/response handler! " +
                "The client locale is "+ locale.toString() + ", DEFAULT tenant");

        slo(locale, Shared.getDefaultTenant(), model, request, response);
    }

    /**
     * Handle request sent with a wrong binding
     */
    @RequestMapping(value = "/websso/SAML2/SLO/{tenant:.*}")
    public void sloError(Locale locale, @PathVariable(value = "tenant") String tenant
        , HttpServletResponse response) throws IOException {
        logger.info("SLO binding error! The client locale is "+ locale.toString() + ", tenant is " + tenant);

        sloDefaultTenantBindingError(locale, response);
    }

    /**
     * Handle default tenant request sent with a wrong binding
     */
    @RequestMapping(value = "/websso/SAML2/SLO")
    public void sloDefaultTenantBindingError(Locale locale, HttpServletResponse response) throws IOException {
        logger.info("SLO binding error! The client locale is "+ locale.toString() + ", DEFAULT tenant");

        // use validation result code to return error to client
        ValidationResult vr = new ValidationResult(
                HttpServletResponse.SC_BAD_REQUEST, "BadRequest", "Binding");
        String message = vr.getMessage(messageSource, locale);
        response.sendError(vr.getResponseCode(), message);
        logger.info("Responded with ERROR " + vr.getResponseCode()
                + ", message " + message);
    }


    public MessageSource getMessageSource() {
        return messageSource;
    }

    public void setMessageSource(MessageSource ms) {
        messageSource = ms;
    }

    /**
     * @return the sessionManager
     */
    public SessionManager getSessionManager() {
        return sessionManager;
    }

    /**
     * @param sessionManager the sessionManager to set
     */
    public void setSessionManager(SessionManager sessionManager) {
        this.sessionManager = sessionManager;
    }

    /**
     * @return the processor
     */
    public ProcessingFilter<LogoutState> getProcessor() {
        return processor;
    }

    /**
     * @param processor the processor to set
     */
    public void setProcessor(ProcessingFilter<LogoutState> processor) {
        this.processor = processor;
    }

}
