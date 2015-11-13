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

import org.opensaml.saml2.core.Response;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
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
                if (requestState.isProxying()) {
                    return;
                }
            }
            requestState.addResponseHeaders(response);
            Response samlResponse = requestState.generateResponseForTenant(
                    tenant, token);
            if (samlResponse == null) {
                // use validation result code to return redirect or error to
                // client
                ValidationResult vr = requestState.getValidationResult();
                if (vr.needsLogonView()) {
                	requestState.setNeedLoginView(true);
                } else if (vr.isRedirect()) {
					response.sendRedirect(vr.getStatus());
					logger.error("SSO Request responded with REDIRECT {} Substatus code: ",
					        vr.getResponseCode(),vr.getSubstatus());
				} else {
					String message = vr.getMessage(messageSource, locale);
                    response.addHeader(
                            Shared.RESPONSE_ERROR_HEADER,
                            Shared.encodeString(message));
                    response.sendError(vr.getResponseCode(), message);
                    logger.error("Sending error to browser. ERROR: {}, message ",vr.getResponseCode(), message);
                }
            } else {
                String samlResponseForm = requestState
                        .generateResponseFormForTenant(samlResponse, tenant);
				if (samlResponseForm == null || samlResponseForm.isEmpty()) {
					logger.error("Invalid SAML Response Form");
				}
                // write response
                Shared.sendResponse(response, Shared.HTML_CONTENT_TYPE, samlResponseForm);
            }
        } catch (IOException e) {
            logger.error("Caught IO exception " + e.toString());
        }
    }
}
