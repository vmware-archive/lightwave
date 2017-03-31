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
import java.net.URI;
import java.util.Arrays;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.ProviderMetadataMapper;
import com.vmware.identity.openidconnect.protocol.URIUtils;

/**
 * @author Jun Sun
 * @author Yehia Zayour
 */
@Controller
public class MetadataController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(MetadataController.class);

    private static final List<String> RESPONSE_TYPES_SUPPORTED = Arrays.asList("code", "id_token", "token id_token");
    private static final List<String> SUBJECT_TYPES_SUPPORTED = Arrays.asList("public");
    private static final List<String> ID_TOKEN_SIGNING_ALGORITHM_VALUES_SUPPORTED = Arrays.asList("RS256");

    @Autowired
    private CasIdmClient idmClient;

    // Constructor for Spring MVC
    public MetadataController() {
    }

    // Constructor for unit test
    public MetadataController(CasIdmClient idmClient) {
        this.idmClient = idmClient;
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.METADATA, method = RequestMethod.GET)
    public void metadata(
            HttpServletRequest httpServletRequest,
            HttpServletResponse httpServletResponse) throws IOException {
        metadata(httpServletRequest, httpServletResponse, null);
    }

    @RequestMapping(value = Endpoints.BASE + "/{tenant:.*}" + Endpoints.METADATA, method = RequestMethod.GET)
    public void metadata(
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
            tenant = tenantInfo.getName(); // use tenant name as it appears in directory
            Issuer issuer = tenantInfo.getIssuer();
            ProviderMetadata providerMetadata = new ProviderMetadata.Builder(issuer).
                        authorizationEndpointURI(endpointURI(issuer, tenant, Endpoints.AUTHENTICATION)).
                        tokenEndpointURI(endpointURI(issuer, tenant, Endpoints.TOKEN)).
                        endSessionEndpointURI(endpointURI(issuer, tenant, Endpoints.LOGOUT)).
                        jwkSetURI(endpointURI(issuer, tenant, Endpoints.JWKS)).
                        responseTypesSupported(RESPONSE_TYPES_SUPPORTED).
                        subjectTypesSupported(SUBJECT_TYPES_SUPPORTED).
                        idTokenSigningAlgorithmValuesSupported(ID_TOKEN_SIGNING_ALGORITHM_VALUES_SUPPORTED).build();

            httpResponse = HttpResponse.createJsonResponse(StatusCode.OK, ProviderMetadataMapper.toJSONObject(providerMetadata));
        } catch (ParseException e) {
            ErrorObject errorObject = e.getErrorObject();
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
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

    private static URI endpointURI(Issuer issuer, String tenant, String endpoint) throws ParseException {
        return URIUtils.parseURI(replaceLast(issuer.getValue(), "/" + tenant, endpoint + "/" + tenant));
    }

    private static String replaceLast(String string, String from, String to) {
        int lastIndex = string.lastIndexOf(from);
        if (lastIndex < 0) {
            return string;
        }
        String tail = string.substring(lastIndex).replaceFirst(from, to);
        return string.substring(0, lastIndex) + tail;
    }
}
