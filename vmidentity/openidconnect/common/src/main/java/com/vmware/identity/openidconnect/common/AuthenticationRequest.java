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
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.ResponseType;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.SerializeException;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.oauth2.sdk.id.State;
import com.nimbusds.openid.connect.sdk.Nonce;
import com.nimbusds.openid.connect.sdk.OIDCResponseTypeValue;
import com.nimbusds.openid.connect.sdk.ResponseMode;

/**
 * @author Yehia Zayour
 */
public class AuthenticationRequest extends com.nimbusds.openid.connect.sdk.AuthenticationRequest {
    private final ResponseMode responseMode;
    private final SignedJWT clientAssertion;
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
            SignedJWT clientAssertion,
            CorrelationID correlationId) {
        super(uri, responseType, scope, clientId, redirectUri, state, nonce);

        Validate.notNull(uri, "uri");
        Validate.notNull(responseType, "responseType");
        Validate.notNull(responseMode, "responseMode");
        Validate.notNull(clientId, "clientId");
        Validate.notNull(redirectUri, "redirectUri");
        Validate.notNull(scope, "scope");
        Validate.notNull(state, "state");
        Validate.notNull(nonce, "nonce");

        this.responseMode = responseMode;
        this.clientAssertion = clientAssertion;
        this.correlationId = correlationId;
    }

    public ResponseMode getResponseMode() {
        return this.responseMode;
    }

    public SignedJWT getClientAssertion() {
        return this.clientAssertion;
    }

    public CorrelationID getCorrelationID() {
        return this.correlationId;
    }

    @Override
    public Map<String, String> toParameters() throws SerializeException {
        Map<String, String> result = super.toParameters();
        result.put("response_mode", this.responseMode.getValue());
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
        com.nimbusds.openid.connect.sdk.AuthenticationRequest nimbusAuthnRequest = com.nimbusds.openid.connect.sdk.AuthenticationRequest.parse(httpRequest.getRequestUri(), parameters);

        String responseModeString = parameters.get("response_mode");
        if (StringUtils.isBlank(responseModeString)) {
            throw new ParseException("missing response_mode parameter");
        }

        ResponseMode responseMode = ResponseMode.parse(responseModeString);
        if (responseMode.equals(ResponseMode.QUERY) && nimbusAuthnRequest.getResponseType().contains(OIDCResponseTypeValue.ID_TOKEN)) {
            throw new ParseException("response_mode=query is not allowed for implicit flow");
        }
        if (responseMode.equals(ResponseMode.FRAGMENT) && nimbusAuthnRequest.getResponseType().contains(ResponseType.Value.CODE)) {
            throw new ParseException("response_mode=fragment is not allowed for authz code flow");
        }

        SignedJWT clientAssertion = null;
        String clientAssertionString = parameters.get("client_assertion");
        if (clientAssertionString != null) {
            try {
                clientAssertion = SignedJWT.parse(clientAssertionString);
            } catch (java.text.ParseException e) {
                throw new ParseException("failed to parse client_assertion parameter: " + e.getMessage());
            }
        }

        CorrelationID correlationId = null;
        String correlationIdString = parameters.get("correlation_id");
        if (!StringUtils.isBlank(correlationIdString)) {
            correlationId = new CorrelationID(correlationIdString);
        }

        if (nimbusAuthnRequest.getState() == null) {
            throw new ParseException("missing state parameter");
        }

        if (nimbusAuthnRequest.getNonce() == null) {
            throw new ParseException("missing nonce parameter");
        }

        return new AuthenticationRequest(
                nimbusAuthnRequest.getEndpointURI(),
                nimbusAuthnRequest.getResponseType(),
                responseMode,
                nimbusAuthnRequest.getClientID(),
                nimbusAuthnRequest.getRedirectionURI(),
                nimbusAuthnRequest.getScope(),
                nimbusAuthnRequest.getState(),
                nimbusAuthnRequest.getNonce(),
                clientAssertion,
                correlationId);
    }
}
