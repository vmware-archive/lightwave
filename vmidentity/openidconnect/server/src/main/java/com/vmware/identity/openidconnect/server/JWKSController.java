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
import java.util.ArrayList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.KeyUse;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jose.util.Base64;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.HttpResponse;

/**
 * @author Jun Sun
 * @author Yehia Zayour
 */
@Controller
public class JWKSController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(JWKSController.class);

    @Autowired
    private CasIdmClient idmClient;

    // Constructor for Spring MVC
    public JWKSController() {
    }

    // Constructor for unit test
    public JWKSController(CasIdmClient idmClient) {
        this.idmClient = idmClient;
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.JWKS, method = RequestMethod.GET)
    public void jwks(
            HttpServletRequest httpServletRequest,
            HttpServletResponse httpServletResponse) throws IOException {
        jwks(httpServletRequest, httpServletResponse, null);
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.JWKS + "/{tenant:.*}", method = RequestMethod.GET)
    public void jwks(
            HttpServletRequest httpServletRequest,
            HttpServletResponse httpServletResponse,
            @PathVariable("tenant") String tenant) throws IOException {
        HttpResponse httpResponse;

        try {
            TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(this.idmClient);
            if (tenant == null) {
                tenant = tenantInfoRetriever.getDefaultTenantName();
            }
            TenantInfo tenantInfo = tenantInfoRetriever.retrieveTenantInfo(tenant);

            List<Base64> x5c = new ArrayList<Base64>(1);
            x5c.add(Base64.encode(tenantInfo.getCertificate().getEncoded()));
            RSAKey rsaKey = new RSAKey(
                    tenantInfo.getPublicKey(),
                    KeyUse.SIGNATURE,
                    null,
                    JWSAlgorithm.RS256,
                    null,
                    null,
                    null,
                    x5c);
            JWKSet jwks = new JWKSet(rsaKey);

            httpResponse = HttpResponse.createJsonResponse(StatusCode.OK, jwks.toJSONObject());
        } catch (ServerException e) {
            ErrorObject errorObject = e.getErrorObject();
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
        } catch (Exception e) {
            ErrorObject errorObject = ErrorObject.serverError(String.format("unhandled %s: %s", e.getClass().getName(), e.getMessage()));
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
        }

        httpResponse.applyTo(httpServletResponse);
    }
}
