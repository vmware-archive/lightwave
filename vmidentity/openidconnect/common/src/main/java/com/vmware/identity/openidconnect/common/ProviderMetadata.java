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

    public ProviderMetadata(
            Issuer issuer,
            URI authorizationEndpointURI,
            URI tokenEndpointURI,
            URI endSessionEndpointURI,
            URI jwkSetURI) {
        Validate.notNull(issuer, "issuer");
        Validate.notNull(authorizationEndpointURI, "authorizationEndpointURI");
        Validate.notNull(tokenEndpointURI, "tokenEndpointURI");
        Validate.notNull(endSessionEndpointURI, "endSessionEndpointURI");
        Validate.notNull(jwkSetURI, "jwkSetURI");

        this.issuer = issuer;
        this.authorizationEndpointURI = authorizationEndpointURI;
        this.tokenEndpointURI = tokenEndpointURI;
        this.endSessionEndpointURI = endSessionEndpointURI;
        this.jwkSetURI = jwkSetURI;
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
}