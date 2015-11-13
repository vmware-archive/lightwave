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

package com.vmware.identity.openidconnect.client;

import java.io.Serializable;
import java.net.URI;
import java.util.List;

import org.apache.commons.lang3.Validate;

/**
 * OIDC server meta data. This contains meta data from query server metadata endpoint.
 *
 * @author Jun Sun
 */
public class ProviderMetadata implements Serializable {

    private static final long serialVersionUID = 1L;

    private final Issuer issuer;
    private final URI authorizationEndpointURI;
    private final URI tokenEndpointURI;
    private final URI registrationEndpointURI;
    private final URI endSessionEndpoint;
    private final URI jwkSetURI;
    private final Scope scope;
    private final List<ResponseType> responseTypes;
    private final List<ResponseMode> responseModes;
    private final List<GrantType> grantTypes;
    private final List<SubjectType> subjectTypes;
    private final List<ClientAuthenticationMethod> tokenEndpointAuthMethods;
    private final List<JWSAlgorithm> tokenEndpointJWSAlgs;
    private final List<JWSAlgorithm> idTokenJWSAlgs;
    private final List<String> claims;
    private boolean requestURIParamSupported = true;

    ProviderMetadata(
            Issuer issuer,
            URI authorizationEndpointURI,
            URI tokenEndpointURI,
            URI registrationEndpointURI,
            URI endSessionEndpoint,
            URI jwkSetURI,
            Scope scope,
            List<ResponseType> responseTypes,
            List<ResponseMode> responseModes,
            List<GrantType> grantTypes,
            List<SubjectType> subjectTypes,
            List<ClientAuthenticationMethod> tokenEndpointAuthMethods,
            List<JWSAlgorithm> tokenEndpointJWSAlgs,
            List<JWSAlgorithm> idTokenJWSAlgs,
            List<String> claims,
            boolean requestURIParamSupported) {
        Validate.notNull(issuer, "issuer");
        Validate.notNull(authorizationEndpointURI, "authorizationEndpointURI");
        Validate.notNull(tokenEndpointURI, "tokenEndpointURI");
        Validate.notNull(endSessionEndpoint, "endSessionEndpoint");
        Validate.notNull(jwkSetURI, "jwkSetURI");
        Validate.notEmpty(responseTypes, "responseTypes");
        Validate.notEmpty(subjectTypes, "subjectTypes");
        Validate.notEmpty(idTokenJWSAlgs, "idTokenJWSAlgs");

        this.issuer = issuer;
        this.authorizationEndpointURI = authorizationEndpointURI;
        this.tokenEndpointURI = tokenEndpointURI;
        this.registrationEndpointURI = registrationEndpointURI;
        this.endSessionEndpoint = endSessionEndpoint;
        this.jwkSetURI = jwkSetURI;
        this.scope = scope;
        this.responseTypes = responseTypes;
        this.responseModes = responseModes;
        this.grantTypes = grantTypes;
        this.subjectTypes = subjectTypes;
        this.tokenEndpointAuthMethods = tokenEndpointAuthMethods;
        this.tokenEndpointJWSAlgs = tokenEndpointJWSAlgs;
        this.idTokenJWSAlgs = idTokenJWSAlgs;
        this.claims = claims;
        this.requestURIParamSupported = requestURIParamSupported;
    }

    /**
     * Get issuer
     *
     * @return                                  OIDC server issuer
     */
    public Issuer getIssuer() {
        return this.issuer;
    }

    /**
     * Get authorization endpoint URI
     *
     * @return                                  OIDC server authorization endpoint URI
     */
    public URI getAuthorizationEndpointURI() {
        return this.authorizationEndpointURI;
    }

    /**
     * Get token endpoint URI
     *
     * @return                                  OIDC server token endpoint URI
     */
    public URI getTokenEndpointURI() {
        return this.tokenEndpointURI;
    }

    /**
     * Get registration endpoint URI
     *
     * @return                                  OIDC server registration endpoint URI
     */
    public URI getRegistrationEndpointURI() {
        return this.registrationEndpointURI;
    }

    /**
     * Get logout endpoint URI
     *
     * @return                                  OIDC server logout endpoint URI
     */
    public URI getEndSessionEndpoint() {
        return this.endSessionEndpoint;
    }

    /**
     * Get JWK set URI
     *
     * @return                                  OIDC server JWK set URI
     */
    public URI getJWKSetURI() {
        return this.jwkSetURI;
    }

    /**
     * Get scope
     *
     * @return                                  OIDC server supported scope values
     */
    public Scope getScope() {
        return this.scope;
    }

    /**
     * Get response types
     *
     * @return                                  OIDC server supported response types
     */
    public List<ResponseType> getRts() {
        return this.responseTypes;
    }

    /**
     * Get response modes
     *
     * @return                                  OIDC server supported response modes
     */
    public List<ResponseMode> getRms() {
        return this.responseModes;
    }

    /**
     * Get grant types
     *
     * @return                                  OIDC server supported grant types
     */
    public List<GrantType> getGts() {
        return this.grantTypes;
    }

    /**
     * Get subject types
     *
     * @return                                  OIDC server supported subject types
     */
    public List<SubjectType> getSubjectTypes() {
        return this.subjectTypes;
    }

    /**
     * Get client authentication methods
     *
     * @return                                  OIDC server supported client authentication methods
     */
    public List<ClientAuthenticationMethod> getTokenEndpointAuthMethods() {
        return this.tokenEndpointAuthMethods;
    }

    /**
     * Get token endpoint JWS algorithms
     *
     * @return                                  OIDC server supported token endpoint JWS algorithms
     */
    public List<JWSAlgorithm> getTokenEndpointJWSAlgs() {
        return this.tokenEndpointJWSAlgs;
    }

    /**
     * Get Id token signing JWS algorithms
     *
     * @return                                  OIDC server supported Id token signing JWS algorithms
     */
    public List<JWSAlgorithm> getIdTokenJWSAlgs() {
        return this.idTokenJWSAlgs;
    }

    /**
     * Get claims
     *
     * @return                                  OIDC server supported claims
     */
    public List<String> getClaims() {
        return this.claims;
    }

    /**
     * Get request URI parameters supported flag
     *
     * @return                                  Boolean about whether OIDC server supports Request URI Parameters
     */
    public boolean isRequestURIParamSupported() {
        return this.requestURIParamSupported;
    }
}
