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
public class LogoutController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(LogoutController.class);

    @Autowired
    private CasIdmClient idmClient;

    @Autowired
    private SessionManager sessionManager;

    public LogoutController() {
    }

    // for unit tests
    LogoutController(CasIdmClient idmClient, SessionManager sessionManager) {
        this.idmClient = idmClient;
        this.sessionManager = sessionManager;
    }

    // for unit tests
    void setSessionManager(SessionManager sessionManager) {
        this.sessionManager = sessionManager;
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.LOGOUT + "/{tenant:.*}", method = { RequestMethod.GET, RequestMethod.POST })
    public void logout(
            HttpServletRequest request,
            HttpServletResponse response,
            @PathVariable("tenant") String tenant) throws IOException {
        HttpResponse httpResponse;
        IDiagnosticsContextScope context = null;

        try {
            HttpRequest httpRequest = HttpRequest.from(request);
            context = DiagnosticsContextFactory.createContext(LoggerUtils.getCorrelationID(httpRequest).getValue(), tenant);

            LogoutRequestProcessor p = new LogoutRequestProcessor(
                    this.idmClient,
                    this.sessionManager,
                    httpRequest,
                    tenant);
            httpResponse = p.process();
        } catch (Exception e) {
            ErrorObject errorObject = ErrorObject.serverError(String.format("unhandled %s: %s", e.getClass().getName(), e.getMessage()));
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createErrorResponse(errorObject);
        } finally {
            if (context != null) {
                context.close();
            }
        }

        httpResponse.applyTo(response);
    }
}
