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
import java.util.Map;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.AuthorizationCodeGrant;
import com.nimbusds.oauth2.sdk.AuthorizationGrant;
import com.nimbusds.oauth2.sdk.ClientCredentialsGrant;
import com.nimbusds.oauth2.sdk.GrantType;
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.SerializeException;
import com.nimbusds.oauth2.sdk.auth.ClientAuthentication;
import com.nimbusds.oauth2.sdk.auth.PrivateKeyJWT;
import com.nimbusds.oauth2.sdk.http.HTTPRequest;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.oauth2.sdk.util.URLUtils;

/**
 * @author Yehia Zayour
 */
public class TokenRequest extends com.nimbusds.oauth2.sdk.TokenRequest {
    private final SignedJWT solutionAssertion;
    private final CorrelationID correlationId;

    public TokenRequest(
            URI uri,
            AuthorizationGrant authzGrant,
            Scope scope,
            CorrelationID correlationId) {
        super(uri, authzGrant, scope);

        Validate.notNull(uri, "uri");
        Validate.notNull(authzGrant, "authzGrant");

        this.solutionAssertion = null;
        this.correlationId = correlationId;
    }

    public TokenRequest(
            URI uri,
            AuthorizationGrant authzGrant,
            Scope scope,
            SignedJWT solutionAssertion,
            CorrelationID correlationId) {
        super(uri, authzGrant, scope);

        Validate.notNull(uri, "uri");
        Validate.notNull(authzGrant, "authzGrant");
        Validate.notNull(solutionAssertion, "solutionAssertion");

        this.solutionAssertion = solutionAssertion;
        this.correlationId = correlationId;
    }

    public TokenRequest(
            URI uri,
            AuthorizationGrant authzGrant,
            Scope scope,
            PrivateKeyJWT privateKeyJwt,
            CorrelationID correlationId) {
        super(uri, privateKeyJwt, authzGrant, scope);

        Validate.notNull(uri, "uri");
        Validate.notNull(authzGrant, "authzGrant");
        Validate.notNull(privateKeyJwt, "clientAuthn");

        this.solutionAssertion = null;
        this.correlationId = correlationId;
    }

    public static TokenRequest create(
            URI uri,
            AuthorizationGrant authzGrant,
            Scope scope,
            SignedJWT solutionAssertion,
            PrivateKeyJWT privateKeyJwt,
            CorrelationID correlationId) {
        Validate.notNull(uri, "uri");
        Validate.notNull(authzGrant, "authzGrant");
        Validate.isTrue(solutionAssertion == null || privateKeyJwt == null, "one of solutionAssertion and privateKeyJwt should be null");

        TokenRequest tokenRequest = null;
        if (privateKeyJwt != null) {
            tokenRequest = new TokenRequest(uri, authzGrant, scope, privateKeyJwt, correlationId);
        } else if (solutionAssertion != null) {
            tokenRequest = new TokenRequest(uri, authzGrant, scope, solutionAssertion, correlationId);
        } else {
            tokenRequest = new TokenRequest(uri, authzGrant, scope, correlationId);
        }
        return tokenRequest;
    }

    public SignedJWT getSolutionAssertion() {
        return this.solutionAssertion;
    }

    public CorrelationID getCorrelationID() {
        return this.correlationId;
    }

    public SignedJWT getClientAssertion() {
        SignedJWT clientAssertion = null;
        ClientAuthentication clientAuthn = super.getClientAuthentication();
        if (clientAuthn != null && clientAuthn instanceof PrivateKeyJWT) {
            clientAssertion = ((PrivateKeyJWT) clientAuthn).getClientAssertion();
        }
        return clientAssertion;
    }

    @Override
    public ClientID getClientID() {
        ClientID clientId = null;
        ClientAuthentication clientAuthn = super.getClientAuthentication();
        if (clientAuthn != null && clientAuthn instanceof PrivateKeyJWT) {
            clientId = ((PrivateKeyJWT) clientAuthn).getClientID();
        }
        return clientId;
    }

    @Override
    public HTTPRequest toHTTPRequest() throws SerializeException {
        HTTPRequest httpRequest = super.toHTTPRequest();

        if (this.solutionAssertion != null || this.correlationId != null) {
            Map<String, String> parameters = httpRequest.getQueryParameters();
            if (this.solutionAssertion != null) {
                parameters.put("solution_assertion", this.solutionAssertion.serialize());
            }
            if (this.correlationId != null) {
                parameters.put("correlation_id", this.correlationId.getValue());
            }
            httpRequest.setQuery(URLUtils.serializeParameters(parameters));
        }

        return httpRequest;
    }

    // we defined two extension grant types: GssTicketGrant and SolutionUserCredentialsGrant, nimbus SDK cannot parse them
    public static TokenRequest parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");

        Map<String, String> parameters = httpRequest.getParameters();

        String grantTypeString = parameters.get("grant_type");
        if (grantTypeString == null) {
            throw new ParseException("missing grant_type parameter");
        }
        GrantType grantType = GrantType.parse(grantTypeString);

        AuthorizationGrant authzGrant;
        if (grantType.equals(SolutionUserCredentialsGrant.GRANT_TYPE)) {
            authzGrant = SolutionUserCredentialsGrant.parse(parameters);
        } else if (grantType.equals(GssTicketGrant.GRANT_TYPE)) {
            authzGrant = GssTicketGrant.parse(parameters);
        } else {
            authzGrant = AuthorizationGrant.parse(parameters);
        }

        if (parameters.get("client_id") != null) {
            throw new ParseException("client_id parameter is not allowed, send client_assertion instead");
        }

        Scope scope = Scope.parse(parameters.get("scope"));

        PrivateKeyJWT privateKeyJwt = null;
        if (parameters.containsKey("client_assertion")) {
            privateKeyJwt = PrivateKeyJWT.parse(parameters);
        }

        SignedJWT solutionAssertion = null;
        String solutionAssertionString = parameters.get("solution_assertion");
        if (solutionAssertionString != null) {
            try {
                solutionAssertion = SignedJWT.parse(solutionAssertionString);
            } catch (java.text.ParseException e) {
                throw new ParseException("failed to parse solution_assertion parameter");
            }
        }

        CorrelationID correlationId = null;
        String correlationIdString = parameters.get("correlation_id");
        if (!StringUtils.isBlank(correlationIdString)) {
            correlationId = new CorrelationID(correlationIdString);
        }

        if (privateKeyJwt != null && solutionAssertion != null) {
            throw new ParseException("client_assertion and solution_assertion in the same request is not allowed");
        }

        if (grantType.equals(SolutionUserCredentialsGrant.GRANT_TYPE) && solutionAssertion == null) {
            throw new ParseException("solution_assertion parameter is required for solution user credentials flow");
        }

        if (grantType.equals(ClientCredentialsGrant.GRANT_TYPE) && privateKeyJwt == null) {
            throw new ParseException("client_assertion parameter is required for client credentials flow");
        }

        if (grantType.equals(AuthorizationCodeGrant.GRANT_TYPE) && privateKeyJwt == null) {
            throw new ParseException("client_assertion parameter is required for authz code flow");
        }

        return TokenRequest.create(httpRequest.getRequestUri(), authzGrant, scope, solutionAssertion, privateKeyJwt, correlationId);
    }
}
