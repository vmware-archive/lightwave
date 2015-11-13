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

package com.vmware.identity.openidconnect.server;

import java.io.IOException;
import java.util.Locale;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang3.tuple.Pair;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.servlet.ModelAndView;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.HttpRequest;

/**
 * @author Yehia Zayour
 */
@Controller
public class AuthorizationController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(AuthorizationController.class);

    @Autowired
    private IdmClient idmClient;

    @Autowired
    private AuthorizationCodeManager authzCodeManager;

    @Autowired
    private SessionManager sessionManager;

    @Autowired
    private MessageSource messageSource;

    public AuthorizationController() {
    }

    // for unit tests
    AuthorizationController(
            IdmClient idmClient,
            AuthorizationCodeManager authzCodeManager,
            SessionManager sessionManager,
            MessageSource messageSource) {
        this.idmClient = idmClient;
        this.authzCodeManager = authzCodeManager;
        this.sessionManager = sessionManager;
        this.messageSource = messageSource;
    }

    // for unit tests
    void setAuthorizationCodeManager(AuthorizationCodeManager authzCodeManager) {
        this.authzCodeManager = authzCodeManager;
    }

    // for unit tests
    void setSessionManager(SessionManager sessionManager) {
        this.sessionManager = sessionManager;
    }

    @RequestMapping(value = "/oidc/authorize", method = { RequestMethod.GET, RequestMethod.POST })
    public ModelAndView authorize(
            Model model,
            Locale locale,
            HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        return authorize(model, locale, request, response, null);
    }

    @RequestMapping(value = "/oidc/authorize/{tenant:.*}", method = { RequestMethod.GET, RequestMethod.POST })
    public ModelAndView authorize(
            Model model,
            Locale locale,
            HttpServletRequest request,
            HttpServletResponse response,
            @PathVariable("tenant") String tenant) throws IOException {
        ModelAndView page = null;

        IDiagnosticsContextScope context = null;

        try {
            HttpRequest httpRequest = HttpRequest.create(request);
            context = DiagnosticsContextFactory.createContext(CorrelationID.get(httpRequest).getValue(), tenant);

            AuthenticationRequestProcessor p = new AuthenticationRequestProcessor(
                    this.idmClient,
                    this.authzCodeManager,
                    this.sessionManager,
                    this.messageSource,
                    model,
                    locale,
                    httpRequest,
                    tenant);
            Pair<ModelAndView, HttpResponse> result = p.process();
            page = result.getLeft();
            HttpResponse httpResponse = result.getRight();
            if (httpResponse != null) {
                httpResponse.applyTo(response);
            }
        } catch (Exception e) {
            logger.error("unhandled exception", e);
            response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, "unhandled exception: " + e.getClass().getName());
        } finally {
            if (context != null) {
                context.close();
            }
        }

        return page;
    }
}
