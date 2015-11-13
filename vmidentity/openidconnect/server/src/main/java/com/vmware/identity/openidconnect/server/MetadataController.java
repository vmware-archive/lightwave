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
import java.net.URISyntaxException;
import java.util.ArrayList;

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
import com.nimbusds.oauth2.sdk.ErrorObject;
import com.nimbusds.oauth2.sdk.id.Issuer;
import com.nimbusds.openid.connect.sdk.ResponseMode;
import com.nimbusds.openid.connect.sdk.op.OIDCProviderMetadata;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * @author Jun Sun
 */
@Controller
public class MetadataController {

    // Logger
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(MetadataController.class);

    // IDM client
    @Autowired
    private IdmClient idmClient;

    // Constructor for Spring MVC
    public MetadataController() {
    }

    // Constructor for unit test
    public MetadataController(IdmClient idmClient) {
        this.idmClient = idmClient;
    }

    // Metadata handler for default tenant
    @RequestMapping(value = "/.well-known/openid-configuration", method = RequestMethod.GET)
    public void metadata(
            HttpServletRequest httpServletRequest,
            HttpServletResponse httpServletResponse) throws IOException {

        TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(this.idmClient);

        String defaultTenant;
        try {
            defaultTenant = tenantInfoRetriever.getDefaultTenantName();
        } catch (ServerException e) {
            String message = e.getErrorObject().getDescription();
            logger.error(message, e);
            httpServletResponse.sendError(e.getErrorObject().getHTTPStatusCode(), message);
            return;
        }

        metadata(
                httpServletRequest,
                httpServletResponse,
                defaultTenant);
    }

    // Metadata handler for tenant
    @RequestMapping(value = "/{tenant:.*}/.well-known/openid-configuration", method = RequestMethod.GET)
    public void  metadata(
            HttpServletRequest httpServletRequest,
            HttpServletResponse httpServletResponse,
            @PathVariable("tenant") String tenant) throws IOException {

        try {
            TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(this.idmClient);
            TenantInformation tenantInformation = tenantInfoRetriever.retrieveTenantInfo(tenant);
            Issuer issuer = tenantInformation.getIssuer();

            URI jwkSetURI = new URI(Shared.replaceLast(issuer.getValue(), tenant, "jwks/" + tenant));
            URI authzEndpoint = new URI(Shared.replaceLast(issuer.getValue(), tenant, "oidc/authorize/" + tenant));
            URI tokenEndpoint = new URI(Shared.replaceLast(issuer.getValue(), tenant, "token/" + tenant));
            URI endSessionEndpoint = new URI(Shared.replaceLast(issuer.getValue(), tenant, "logout/" + tenant));

            OIDCProviderMetadata oidcProviderMetadata = new OIDCProviderMetadata(
                    issuer,
                    Profile.SUBJECT_TYPES,
                    jwkSetURI);

            // apply default values
            oidcProviderMetadata.applyDefaults();

            // Set endpoints
            oidcProviderMetadata.setAuthorizationEndpointURI(authzEndpoint);
            oidcProviderMetadata.setTokenEndpointURI(tokenEndpoint);
            oidcProviderMetadata.setEndSessionEndpointURI(endSessionEndpoint);

            // Set scopes
            oidcProviderMetadata.setScopes(Profile.SCOPES);

            // Set response types
            oidcProviderMetadata.setResponseTypes(Profile.RESPONSE_TYPES);

            // Set response modes to an empty list
            oidcProviderMetadata.setResponseModes(new ArrayList<ResponseMode>());

            // Set id token signing alg
            oidcProviderMetadata.setIDTokenJWSAlgs(Profile.ID_TOKEN_JWS_ALGS);

            // Set grant types
            oidcProviderMetadata.setGrantTypes(Profile.GRANT_TYPES);

            // Set token endpoint authentication methods
            oidcProviderMetadata.setTokenEndpointAuthMethods(Profile.TOKEN_ENDPOINT_AUTH_METHODS);

            // Set token endpoint JWS algorithms
            oidcProviderMetadata.setTokenEndpointJWSAlgs(Profile.TOKEN_ENDPOINT_JWS_ALGS);

            // Set claims
            oidcProviderMetadata.setClaims(Profile.CLAIMS);

            // Set no support for request uri parameter
            oidcProviderMetadata.setSupportsRequestURIParam(false);

            Shared.writeJSONResponse(
                    httpServletResponse,
                    HttpServletResponse.SC_OK,
                    oidcProviderMetadata.toJSONObject());
        } catch (URISyntaxException e) {
            logger.error("URISyntaxException Exception in creating OIDC metadata response.", e);
            ErrorObject errorObject = new ErrorObject(
                    "metadata_endpoint_error",
                    "URISyntaxException Exception in creating OIDC metadata response.");
            Shared.writeJSONResponse(httpServletResponse,
                    HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                    errorObject.toJSONObject());
        } catch (ServerException e) {
            logger.error("exception in retrieving tenant info", e);
            ErrorObject errorObject = e.getErrorObject();
            Shared.writeJSONResponse(httpServletResponse,
                    errorObject.getHTTPStatusCode(),
                    errorObject.toJSONObject());
        } catch (Exception e) {
            logger.error("Exception in creating OIDC metadata response.", e);
            ErrorObject errorObject = new ErrorObject(
                    "metadata_endpoint_error",
                    "Exception in creating OIDC metadata response.");
            Shared.writeJSONResponse(httpServletResponse,
                    HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                    errorObject.toJSONObject());
        }
    }

    // JWKs handler for default tenant
    @RequestMapping(value = "/jwks", method = RequestMethod.GET)
    public void jwks(
            HttpServletRequest httpServletRequest,
            HttpServletResponse httpServletResponse) throws IOException {

        TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(this.idmClient);

        String defaultTenant;
        try {
            defaultTenant = tenantInfoRetriever.getDefaultTenantName();
        } catch (ServerException e) {
            String message = e.getErrorObject().getDescription();
            logger.error(message, e);
            httpServletResponse.sendError(e.getErrorObject().getHTTPStatusCode(), message);
            return;
        }

        jwks(
                httpServletRequest,
                httpServletResponse,
                defaultTenant);
    }

    // JWKs handler for tenant
    @RequestMapping(value = "/jwks/{tenant:.*}", method = RequestMethod.GET)
    public void jwks(
            HttpServletRequest httpServletRequest,
            HttpServletResponse httpServletResponse,
            @PathVariable("tenant") String tenant) throws IOException {

        try {
            TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(this.idmClient);
            TenantInformation tenantInformation = tenantInfoRetriever.retrieveTenantInfo(tenant);

            RSAKey rsaKey = new RSAKey(
                    tenantInformation.getPublicKey(),
                    KeyUse.SIGNATURE,
                    null,
                    JWSAlgorithm.RS256,
                    null,
                    null,
                    null,
                    null);

            JWKSet jwks = new JWKSet(rsaKey);

            Shared.writeJSONResponse(httpServletResponse, HttpServletResponse.SC_OK, jwks.toJSONObject());
        } catch (ServerException e) {
            logger.error("exception in retrieving tenant info", e);
            ErrorObject errorObject = e.getErrorObject();
            Shared.writeJSONResponse(httpServletResponse,
                    errorObject.getHTTPStatusCode(),
                    errorObject.toJSONObject());
        } catch (Exception e) {
            logger.error("Exception in creating OIDC JSON Web Key set response.", e);
            ErrorObject errorObject = new ErrorObject(
                    "jwks_endpoint_error",
                    "Exception in creating OIDC JSON Web Key set response.");
            Shared.writeJSONResponse(httpServletResponse,
                    HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                    errorObject.toJSONObject());
        }
    }
}
