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

package com.vmware.identity.openidconnect.protocol;

import java.net.URI;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.ScopeValue;

/**
 * @author Yehia Zayour
 */
public final class TokenRequest extends ProtocolRequest {
    private final URI uri;
    private final AuthorizationGrant authzGrant;
    private final Scope scope;
    private final SolutionUserAssertion solutionUserAssertion;
    private final ClientAssertion clientAssertion;
    private final CorrelationID correlationId;
    private final ClientID clientId;

    public TokenRequest(
            URI uri,
            AuthorizationGrant authzGrant,
            Scope scope,
            SolutionUserAssertion solutionUserAssertion,
            ClientAssertion clientAssertion,
            ClientID clientId,
            CorrelationID correlationId) {
        Validate.notNull(uri, "uri");
        Validate.notNull(authzGrant, "authzGrant");
        // nullable scope
        // nullable solutionUserAssertion
        // nullable clientAssertion
        // nullable clientId
        // nullable correlationId

        this.uri = uri;
        this.authzGrant = authzGrant;
        this.scope = scope;
        this.solutionUserAssertion = solutionUserAssertion;
        this.clientAssertion = clientAssertion;
        this.correlationId = correlationId;

        if (this.clientAssertion != null) {
            this.clientId = new ClientID(this.clientAssertion.getIssuer().getValue());
        } else if (clientId != null) {
            this.clientId = clientId;
        } else {
            this.clientId = null;
        }
    }

    public AuthorizationGrant getAuthorizationGrant() {
        return this.authzGrant;
    }

    public Scope getScope() {
        return this.scope;
    }

    public SolutionUserAssertion getSolutionUserAssertion() {
        return this.solutionUserAssertion;
    }

    public ClientAssertion getClientAssertion() {
        return this.clientAssertion;
    }

    public CorrelationID getCorrelationID() {
        return this.correlationId;
    }

    public ClientID getClientID() {
        return this.clientId;
    }

    @Override
    public HttpRequest toHttpRequest() {
        return HttpRequest.createPostRequest(this.uri, toParameters());
    }

    private Map<String, String> toParameters() {
        Map<String, String> result = new HashMap<String, String>();

        result.putAll(this.authzGrant.toParameters());
        if (this.scope != null) {
            result.put("scope", this.scope.toString());
        }
        if (this.solutionUserAssertion != null) {
            result.put("solution_user_assertion", this.solutionUserAssertion.serialize());
        }

        if (this.clientAssertion != null) {
            result.put("client_assertion", this.clientAssertion.serialize());
            result.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
        } else if (this.clientId != null) {
            result.put("client_id", this.clientId.getValue());
        }

        if (this.correlationId != null) {
            result.put("correlation_id", this.correlationId.getValue());
        }

        return result;
    }

    public static TokenRequest parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");

        Map<String, String> parameters = httpRequest.getParameters();

        AuthorizationGrant authzGrant = AuthorizationGrant.parse(parameters);

        Scope scope = null;
        if (authzGrant.getGrantType() == GrantType.AUTHORIZATION_CODE || authzGrant.getGrantType() == GrantType.REFRESH_TOKEN) {
            // ignore scope, ignore instead of reject because kubectl will send scope in refresh_token request
        } else {
            if (parameters.containsKey("scope")) {
                scope = Scope.parse(ParameterMapUtils.getString(parameters, "scope"));
            } else {
                throw new ParseException("missing scope parameter");
            }
        }

        SolutionUserAssertion solutionUserAssertion = null;
        if (parameters.containsKey("solution_user_assertion")) {
            solutionUserAssertion = SolutionUserAssertion.parse(parameters);
        }

        ClientAssertion clientAssertion = null;
        if (parameters.containsKey("client_assertion")) {
            clientAssertion = ClientAssertion.parse(parameters);
        }

        ClientID clientId = null;
        if (parameters.get("client_id") != null) {
            clientId = new ClientID(ParameterMapUtils.getString(parameters, "client_id"));
        }

        CorrelationID correlationId = null;
        if (parameters.containsKey("correlation_id")) {
            correlationId = new CorrelationID(ParameterMapUtils.getString(parameters, "correlation_id"));
        }

        validate(authzGrant, scope, solutionUserAssertion, clientAssertion, clientId);

        return new TokenRequest(
                httpRequest.getURI(),
                authzGrant,
                scope,
                solutionUserAssertion,
                clientAssertion,
                clientId,
                correlationId);
    }

    private static void validate(
            AuthorizationGrant authzGrant,
            Scope scope,
            SolutionUserAssertion solutionUserAssertion,
            ClientAssertion clientAssertion,
            ClientID clientId) throws ParseException {
        if (clientAssertion != null && solutionUserAssertion != null) {
            throw new ParseException("client_assertion and solution_user_assertion in the same request is not allowed");
        }

        if (clientAssertion != null && clientId != null && !Objects.equals(clientAssertion.getIssuer().getValue(), clientId.getValue())) {
            throw new ParseException("client_assertion issuer must match client_id");
        }

        GrantType grantType = authzGrant.getGrantType();

        if (grantType == GrantType.CLIENT_CREDENTIALS && scope.contains(ScopeValue.OFFLINE_ACCESS)) {
            throw new ParseException(ErrorObject.invalidScope("refresh token (offline_access) is not allowed for client credentials grant"));
        }

        if (grantType == GrantType.SOLUTION_USER_CREDENTIALS && scope.contains(ScopeValue.OFFLINE_ACCESS)) {
            throw new ParseException(ErrorObject.invalidScope("refresh token (offline_access) is not allowed for solution user credentials grant"));
        }

        if (grantType == GrantType.SOLUTION_USER_CREDENTIALS && solutionUserAssertion == null) {
            throw new ParseException("solution_user_assertion parameter is required for solution user credentials grant");
        }

        if (grantType == GrantType.CLIENT_CREDENTIALS && clientAssertion == null) {
            throw new ParseException("client_assertion parameter is required for client credentials grant");
        }

        if (grantType == GrantType.AUTHORIZATION_CODE && clientAssertion == null) {
            throw new ParseException("client_assertion parameter is required for authz code grant");
        }
    }
}