/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.*;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.*;
import com.vmware.identity.openidconnect.protocol.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.security.NoSuchProviderException;

@Controller
public class FederationTokenController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederationTokenController.class);

    @Autowired
    private CasIdmClient idmClient;

    @Autowired
    private FederatedIdentityProcessor cspProcessor;

    @RequestMapping(
            value = Endpoints.BASE + Endpoints.FEDERATION,
            method = RequestMethod.GET
    )
    public void federate(
            HttpServletRequest request,
            HttpServletResponse response
    ) throws IOException {
        HttpResponse httpResponse;
        try {
            final FederationRelayState state = FederationRelayState.build(request.getParameter("state"));
            final IDPConfig idpConfig = findFederatedIDP(state.getIssuer()); // External IDP corresponding to Issuer
            final OidcConfig oidcConfig = idpConfig.getOidcConfig();
            if (oidcConfig == null) {
                throw new ServerException(ErrorObject.invalidRequest("no oidc configuration found"));
            }
            final FederatedIdentityProcessor processor = findProcessor(oidcConfig.getIssuerType());
            httpResponse = processor.processRequest(request, state, idpConfig);
        } catch (Exception e) {
            ErrorObject errorObject = ErrorObject.invalidRequest(
                    String.format("unhandled %s: %s", e.getClass().getName(), e.getMessage()));
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
        }
        httpResponse.applyTo(response);
    }

    private IDPConfig findFederatedIDP(String issuer) throws Exception {
        String systemTenantName = idmClient.getSystemTenant();
        IDPConfig result = idmClient.getExternalIdpConfigForTenant(systemTenantName, issuer);
        if (result == null) {
            throw new ServerException(ErrorObject.invalidRequest("no federated idp config found"));
        }
        if (!result.getProtocol().equals(IDPConfig.IDP_PROTOCOL_OAUTH_2_0)) {
            ErrorObject errorObject = ErrorObject.serverError(
                String.format("Failed to find federated OIDC IDP config for issuer: %s", issuer)
            );
            LoggerUtils.logFailedRequest(logger, errorObject);
            throw new ServerException(errorObject);
        }
        return result;
    }

    private FederatedIdentityProcessor findProcessor(String issuerType) throws Exception {
        if (issuerType == null || !issuerType.equals("csp")) {
            throw new NoSuchProviderException("Error: Unsupported Issuer Type - " + issuerType);
        }
        return cspProcessor;
    }
}
