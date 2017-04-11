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
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;

/**
 * @author Yehia Zayour
 */
@Controller
public class AuthenticationController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(AuthenticationController.class);

    @Autowired
    private CasIdmClient idmClient;

    @Autowired
    private AuthorizationCodeManager authzCodeManager;

    @Autowired
    private SessionManager sessionManager;

    @Autowired
    private MessageSource messageSource;

    public AuthenticationController() {
    }

    // for unit tests
    AuthenticationController(
            CasIdmClient idmClient,
            AuthorizationCodeManager authzCodeManager,
            SessionManager sessionManager,
            MessageSource messageSource) {
        this.idmClient = idmClient;
        this.authzCodeManager = authzCodeManager;
        this.sessionManager = sessionManager;
        this.messageSource = messageSource;
    }

    @RequestMapping(
            value = { Endpoints.BASE + Endpoints.AUTHENTICATION + "/{tenant:.*}", Endpoints.AUTHENTICATION_CAC_RPROXY + "/{tenant:.*}", Endpoints.AUTHENTICATION_CAC_TOMCAT + "/{tenant:.*}" },
            method = { RequestMethod.GET, RequestMethod.POST })
    public ModelAndView authenticate(
            Model model,
            Locale locale,
            HttpServletRequest request,
            HttpServletResponse response,
            @PathVariable("tenant") String tenant) throws IOException {
        ModelAndView page;
        HttpResponse httpResponse;
        IDiagnosticsContextScope context = null;

        try {
            HttpRequest httpRequest = HttpRequest.from(request);
            context = DiagnosticsContextFactory.createContext(LoggerUtils.getCorrelationID(httpRequest).getValue(), tenant);

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
            httpResponse = result.getRight();
        } catch (Exception e) {
            ErrorObject errorObject = ErrorObject.serverError(String.format("unhandled %s: %s", e.getClass().getName(), e.getMessage()));
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            page = null;
            httpResponse = HttpResponse.createErrorResponse(errorObject);
        } finally {
            if (context != null) {
                context.close();
            }
        }

        if (httpResponse != null) {
            httpResponse.applyTo(response);
        }
        return page;
    }
}
