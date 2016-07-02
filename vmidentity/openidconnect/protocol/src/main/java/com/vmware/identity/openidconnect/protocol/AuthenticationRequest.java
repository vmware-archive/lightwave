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

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.ResponseType;
import com.vmware.identity.openidconnect.common.ResponseTypeValue;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.ScopeValue;
import com.vmware.identity.openidconnect.common.State;

/**
 * @author Yehia Zayour
 */
public final class AuthenticationRequest extends ProtocolRequest {
    private final URI uri;
    private final ResponseType responseType;
    private final ResponseMode responseMode;
    private final ClientID clientId;
    private final URI redirectUri;
    private final Scope scope;
    private final State state;
    private final Nonce nonce;
    private final ClientAssertion clientAssertion;
    private final CorrelationID correlationId;

    public AuthenticationRequest(
            URI uri,
            ResponseType responseType,
            ResponseMode responseMode,
            ClientID clientId,
            URI redirectUri,
            Scope scope,
            State state,
            Nonce nonce,
            ClientAssertion clientAssertion,
            CorrelationID correlationId) {
        Validate.notNull(uri, "uri");
        Validate.notNull(responseType, "responseType");
        Validate.notNull(responseMode, "responseMode");
        Validate.notNull(clientId, "clientId");
        Validate.notNull(redirectUri, "redirectUri");
        Validate.notNull(scope, "scope");
        Validate.notNull(state, "state");
        Validate.notNull(nonce, "nonce");
        // nullable clientAssertion
        // nullable correlationId

        this.uri = uri;
        this.responseType = responseType;
        this.responseMode = responseMode;
        this.clientId = clientId;
        this.redirectUri = redirectUri;
        this.scope = scope;
        this.state = state;
        this.nonce = nonce;
        this.clientAssertion = clientAssertion;
        this.correlationId = correlationId;
    }

    public ResponseType getResponseType() {
        return this.responseType;
    }

    public ResponseMode getResponseMode() {
        return this.responseMode;
    }

    public ClientID getClientID() {
        return this.clientId;
    }

    public URI getRedirectURI() {
        return this.redirectUri;
    }

    public Scope getScope() {
        return this.scope;
    }

    public State getState() {
        return this.state;
    }

    public Nonce getNonce() {
        return this.nonce;
    }

    public ClientAssertion getClientAssertion() {
        return this.clientAssertion;
    }

    public CorrelationID getCorrelationID() {
        return this.correlationId;
    }

    @Override
    public HttpRequest toHttpRequest() {
        return HttpRequest.createGetRequest(this.uri, toParameters());
    }

    private Map<String, String> toParameters() {
        Map<String, String> result = new HashMap<String, String>();

        result.put("response_type", this.responseType.toString());
        result.put("response_mode", this.responseMode.getValue());
        result.put("client_id", this.clientId.getValue());
        result.put("redirect_uri", this.redirectUri.toString());
        result.put("scope", this.scope.toString());
        result.put("state", this.state.getValue());
        result.put("nonce", this.nonce.getValue());
        if (this.clientAssertion != null) {
            result.put("client_assertion", this.clientAssertion.serialize());
        }
        if (this.correlationId != null) {
            result.put("correlation_id", this.correlationId.getValue());
        }

        return result;
    }

    public static AuthenticationRequest parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");

        Map<String, String> parameters = httpRequest.getParameters();

        ClientID clientId = null;
        ResponseMode responseMode = null;
        URI redirectUri = null;
        State state = null;

        try {
            clientId = new ClientID(ParameterMapUtils.getString(parameters, "client_id"));
            responseMode = ResponseMode.parse(ParameterMapUtils.getString(parameters, "response_mode"));
            redirectUri = ParameterMapUtils.getURI(parameters, "redirect_uri");
            state = State.parse(ParameterMapUtils.getString(parameters, "state"));

            ResponseType responseType = ResponseType.parse(ParameterMapUtils.getString(parameters, "response_type"));
            Scope scope = Scope.parse(ParameterMapUtils.getString(parameters, "scope"));
            Nonce nonce = new Nonce(ParameterMapUtils.getString(parameters, "nonce"));

            ClientAssertion clientAssertion = null;
            if (parameters.containsKey("client_assertion")) {
                clientAssertion = ClientAssertion.parse(parameters);
            }

            CorrelationID correlationId = null;
            if (parameters.containsKey("correlation_id")) {
                correlationId = new CorrelationID(ParameterMapUtils.getString(parameters, "correlation_id"));
            }

            validate(responseType, responseMode, clientId, scope, clientAssertion);

            return new AuthenticationRequest(
                    httpRequest.getURI(),
                    responseType,
                    responseMode,
                    clientId,
                    redirectUri,
                    scope,
                    state,
                    nonce,
                    clientAssertion,
                    correlationId);
        } catch (com.vmware.identity.openidconnect.common.ParseException e) {
            throw new ParseException(e.getErrorObject(), clientId, responseMode, redirectUri, state, e);
        }
    }

    private static void validate(
            ResponseType responseType,
            ResponseMode responseMode,
            ClientID clientId,
            Scope scope,
            ClientAssertion clientAssertion) throws com.vmware.identity.openidconnect.common.ParseException {
        if (responseMode == ResponseMode.QUERY && responseType.contains(ResponseTypeValue.ID_TOKEN)) {
            throw new com.vmware.identity.openidconnect.common.ParseException("response_mode=query is not allowed for implicit flow");
        }

        if (responseMode == ResponseMode.FRAGMENT && responseType.contains(ResponseTypeValue.AUTHORIZATION_CODE)) {
            throw new com.vmware.identity.openidconnect.common.ParseException("response_mode=fragment is not allowed for authz code flow");
        }

        if (responseType.contains(ResponseTypeValue.ID_TOKEN) && scope.contains(ScopeValue.OFFLINE_ACCESS)) {
            String message = "refresh token (offline_access) is not allowed for this grant_type";
            throw new com.vmware.identity.openidconnect.common.ParseException(ErrorObject.invalidScope(message));
        }

        if (clientAssertion != null && !clientAssertion.getIssuer().getValue().equals(clientId.getValue())) {
            throw new com.vmware.identity.openidconnect.common.ParseException(ErrorObject.invalidClient("client_assertion issuer must match client_id"));
        }
    }

    public static class ParseException extends Exception {
        private static final long serialVersionUID = 1L;

        private final ErrorObject errorObject;
        private final ClientID clientId;
        private final ResponseMode responseMode;
        private final URI redirectUri;
        private final State state;

        private ParseException(
                ErrorObject errorObject,
                ClientID clientId,
                ResponseMode responseMode,
                URI redirectUri,
                State state,
                Throwable cause) {
            super(errorObject.getDescription(), cause);
            this.errorObject = errorObject;
            this.clientId = clientId;
            this.responseMode = responseMode;
            this.redirectUri = redirectUri;
            this.state = state;
        }

        public ErrorObject getErrorObject() {
            return this.errorObject;
        }

        public ClientID getClientID() {
            return this.clientId;
        }

        public URI getRedirectURI() {
            return this.redirectUri;
        }

        public AuthenticationErrorResponse createAuthenticationErrorResponse(boolean isAjaxRequest) {
            AuthenticationErrorResponse response = null;
            if (this.responseMode != null && this.redirectUri != null && this.state != null) {
                response = new AuthenticationErrorResponse(
                        this.responseMode,
                        this.redirectUri,
                        this.state,
                        isAjaxRequest,
                        this.errorObject);
            }
            return response;
        }
    }
}