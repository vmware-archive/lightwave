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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

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
public class TokenController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TokenController.class);

    @Autowired
    private CasIdmClient idmClient;

    @Autowired
    private AuthorizationCodeManager authzCodeManager;

    public TokenController() {
    }

    // for unit tests
    TokenController(CasIdmClient idmClient, AuthorizationCodeManager authzCodeManager) {
        this.idmClient = idmClient;
        this.authzCodeManager = authzCodeManager;
    }

    // for unit tests
    void setAuthorizationCodeManager(AuthorizationCodeManager authzCodeManager) {
        this.authzCodeManager = authzCodeManager;
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.TOKEN, method = RequestMethod.POST, consumes = "application/x-www-form-urlencoded")
    public void acquireTokens(
            HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        acquireTokens(request, response, null);
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.TOKEN + "/{tenant:.*}", method = RequestMethod.POST, consumes = "application/x-www-form-urlencoded")
    public void acquireTokens(
            HttpServletRequest request,
            HttpServletResponse response,
            @PathVariable("tenant") String tenant) throws IOException {
        HttpResponse httpResponse;
        IDiagnosticsContextScope context = null;

        try {
            if (request.getQueryString() != null && !request.getQueryString().isEmpty()) {
                ErrorObject errorObject = ErrorObject.invalidRequest("query parameters are not allowed at token endpoint");
                LoggerUtils.logFailedRequest(logger, errorObject);
                httpResponse = HttpResponse.createJsonResponse(errorObject);
            } else {
                HttpRequest httpRequest = HttpRequest.from(request);
                context = DiagnosticsContextFactory.createContext(LoggerUtils.getCorrelationID(httpRequest).getValue(), tenant);

                TokenRequestProcessor p = new TokenRequestProcessor(
                        this.idmClient,
                        this.authzCodeManager,
                        httpRequest,
                        tenant);
                httpResponse = p.process();
            }
        } catch (Exception e) {
            ErrorObject errorObject = ErrorObject.serverError(String.format("unhandled %s: %s", e.getClass().getName(), e.getMessage()));
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
        } finally {
            if (context != null) {
                context.close();
            }
        }

        httpResponse.applyTo(response);
    }
}
