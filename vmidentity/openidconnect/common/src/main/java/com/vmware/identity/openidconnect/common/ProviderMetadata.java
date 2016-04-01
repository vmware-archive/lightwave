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

import net.minidev.json.JSONObject;

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

    public JSONObject toJSONObject() {
        JSONObject json = new JSONObject();

        json.put("issuer",                  this.issuer.getValue());
        json.put("authorization_endpoint",  this.authorizationEndpointURI.toString());
        json.put("token_endpoint",          this.tokenEndpointURI.toString());
        json.put("end_session_endpoint",    this.endSessionEndpointURI.toString());
        json.put("jwks_uri",                this.jwkSetURI.toString());

        return json;
    }

    public static ProviderMetadata parse(JSONObject json) throws ParseException {
        Validate.notNull(json, "json");

        URI issuerURI                   = JSONUtils.getURI(json, "issuer");
        URI authorizationEndpointURI    = JSONUtils.getURI(json, "authorization_endpoint");
        URI tokenEndpointURI            = JSONUtils.getURI(json, "token_endpoint");
        URI endSessionEndpointURI       = JSONUtils.getURI(json, "end_session_endpoint");
        URI jwkSetURI                   = JSONUtils.getURI(json, "jwks_uri");

        return new ProviderMetadata(
                new Issuer(issuerURI.toString()),
                authorizationEndpointURI,
                tokenEndpointURI,
                endSessionEndpointURI,
                jwkSetURI);
    }
}