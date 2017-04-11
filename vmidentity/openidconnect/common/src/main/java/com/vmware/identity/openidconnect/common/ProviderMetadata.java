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

package com.vmware.identity.openidconnect.common;

import java.net.URI;
import java.util.List;

import org.apache.commons.lang3.Validate;

/**
 * @author Yehia Zayour
 * @author Jun Sun
 */
public final class ProviderMetadata {
    private final Issuer issuer;
    private final URI authorizationEndpointURI;
    private final URI tokenEndpointURI;
    private final URI endSessionEndpointURI;
    private final URI jwkSetURI;
    private final List<String> responseTypesSupported;
    private final List<String> subjectTypesSupported;
    private final List<String> idTokenSigningAlgorithmValuesSupported;

    private ProviderMetadata(Builder builder) {
        Validate.notNull(builder);
        Validate.notNull(builder.issuer, "builder.issuer");
        Validate.notNull(builder.authorizationEndpointURI, "builder.authorizationEndpointURI");
        Validate.notNull(builder.tokenEndpointURI, "builder.tokenEndpointURI");
        Validate.notNull(builder.endSessionEndpointURI, "builder.endSessionEndpointURI");
        Validate.notNull(builder.jwkSetURI, "builder.jwkSetURI");
        Validate.notEmpty(builder.responseTypesSupported, "builder.responseTypesSupported");
        Validate.notEmpty(builder.subjectTypesSupported, "builder.subjectTypesSupported");
        Validate.notEmpty(builder.idTokenSigningAlgorithmValuesSupported, "builder.idTokenSigningAlgorithmValuesSupported");

        this.issuer                     = builder.issuer;
        this.authorizationEndpointURI   = builder.authorizationEndpointURI;
        this.tokenEndpointURI           = builder.tokenEndpointURI;
        this.endSessionEndpointURI      = builder.endSessionEndpointURI;
        this.jwkSetURI                  = builder.jwkSetURI;
        this.responseTypesSupported     = builder.responseTypesSupported;
        this.subjectTypesSupported      = builder.subjectTypesSupported;
        this.idTokenSigningAlgorithmValuesSupported = builder.idTokenSigningAlgorithmValuesSupported;
    }

    public Issuer getIssuer() {
        return this.issuer;
    }

    public URI getAuthorizationEndpointURI() {
        return this.authorizationEndpointURI;
    }

    public URI getTokenEndpointURI() {
        return this.tokenEndpointURI;
    }

    public URI getEndSessionEndpointURI() {
        return this.endSessionEndpointURI;
    }

    public URI getJWKSetURI() {
        return this.jwkSetURI;
    }

    public List<String> getResponseTypesSupported() {
        return this.responseTypesSupported;
    }

    public List<String> getSubjectTypesSupported() {
        return this.subjectTypesSupported;
    }

    public List<String> getIDTokenSigningAlgorithmValuesSupported() {
        return this.idTokenSigningAlgorithmValuesSupported;
    }

    public static class Builder {
        private Issuer issuer;
        private URI authorizationEndpointURI;
        private URI tokenEndpointURI;
        private URI endSessionEndpointURI;
        private URI jwkSetURI;
        private List<String> responseTypesSupported;
        private List<String> subjectTypesSupported;
        private List<String> idTokenSigningAlgorithmValuesSupported;

        public Builder(Issuer issuer) {
            Validate.notNull(issuer, "issuer");
            this.issuer = issuer;
        }

        public Builder authorizationEndpointURI(URI authorizationEndpointURI) {
            Validate.notNull(authorizationEndpointURI, "authorizationEndpointURI");
            this.authorizationEndpointURI = authorizationEndpointURI;
            return this;
        }

        public Builder tokenEndpointURI(URI tokenEndpointURI) {
            Validate.notNull(tokenEndpointURI, "tokenEndpointURI");
            this.tokenEndpointURI = tokenEndpointURI;
            return this;
        }

        public Builder endSessionEndpointURI(URI endSessionEndpointURI) {
            Validate.notNull(endSessionEndpointURI, "endSessionEndpointURI");
            this.endSessionEndpointURI = endSessionEndpointURI;
            return this;
        }

        public Builder jwkSetURI(URI jwkSetURI) {
            Validate.notNull(jwkSetURI, "jwkSetURI");
            this.jwkSetURI = jwkSetURI;
            return this;
        }

        public Builder responseTypesSupported(List<String> responseTypesSupported) {
            Validate.notEmpty(responseTypesSupported, "responseTypesSupported");
            this.responseTypesSupported = responseTypesSupported;
            return this;
        }

        public Builder subjectTypesSupported(List<String> subjectTypesSupported) {
            Validate.notEmpty(subjectTypesSupported, "subjectTypesSupported");
            this.subjectTypesSupported = subjectTypesSupported;
            return this;
        }

        public Builder idTokenSigningAlgorithmValuesSupported(List<String> idTokenSigningAlgorithmValuesSupported) {
            Validate.notEmpty(idTokenSigningAlgorithmValuesSupported, "idTokenSigningAlgorithmValuesSupported");
            this.idTokenSigningAlgorithmValuesSupported = idTokenSigningAlgorithmValuesSupported;
            return this;
        }

        public ProviderMetadata build() {
            return new ProviderMetadata(this);
        }
    }
}