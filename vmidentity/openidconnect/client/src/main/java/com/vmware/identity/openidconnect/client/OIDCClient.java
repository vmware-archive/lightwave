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

import java.net.URI;
import java.security.KeyStore;
import java.security.interfaces.RSAPublicKey;
import java.util.Set;
import java.util.UUID;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.SerializeException;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.openid.connect.sdk.OIDCResponseTypeValue;
import com.vmware.identity.openidconnect.common.AuthenticationRequest;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.LogoutRequest;

/**
 * OIDC Client
 *
 * @author Jun Sun
 */
public class OIDCClient {

    private final URI authorizationEndpointURI;
    private final URI tokenEndpointURI;
    private final URI endSessionEndpointURI;
    private final Issuer issuer;
    private final RSAPublicKey providerPublicKey;

    private final ClientID clientId;
    private final HolderOfKeyConfig holderOfKeyConfig;
    private final HighAvailabilityConfig highAvailabilityConfig;
    private final KeyStore keyStore;

    /**
     * Constructor
     *
     * @param clientConfig              Configuration required in creating a client
     */
    public OIDCClient(ClientConfig clientConfig) {
        Validate.notNull(clientConfig, "clientConfig");

        this.authorizationEndpointURI = clientConfig.getConnectionConfig().getAuthorizationEndpointURI();
        this.tokenEndpointURI = clientConfig.getConnectionConfig().getTokenEndpointURI();
        this.endSessionEndpointURI = clientConfig.getConnectionConfig().getEndSessionEndpointURI();
        this.issuer = clientConfig.getConnectionConfig().getIssuer();
        this.providerPublicKey = clientConfig.getConnectionConfig().getProviderPublicKey();

        this.clientId = clientConfig.getClientId();
        this.holderOfKeyConfig = clientConfig.getHolderOfKeyConfig();
        this.highAvailabilityConfig = clientConfig.getHighAvailabilityConfig();
        this.keyStore = clientConfig.getConnectionConfig().getKeyStore();
    }

    /**
     * Build an authentication request URI
     *
     * @param redirectEndpointURI       URI where OIDC server will sends response to.
     * @param responseType              Response type specifying whether code or id token (optionally with access token) is requested.
     * @param responseMode              Response mode. For code flow, it should be FORM_POST; For implicit flow, it can be either FORM_POST or FRAGMENT.
     * @param tokenSpec                 Specification of tokens requested.
     * @param state                     State value used in a request, it is optional.
     * @param nonce                     Nonce value used in a request, it is optional.
     * @return                          Authentication request URI.
     * @throws OIDCClientException      Client side exception.
     */
    public URI buildAuthenticationRequestURI(
            URI redirectEndpointURI,
            ResponseType responseType,
            ResponseMode responseMode,
            TokenSpec tokenSpec,
            State state,
            Nonce nonce) throws OIDCClientException {
        Validate.notNull(redirectEndpointURI, "redirectEndpointURI");
        Validate.notNull(responseType, "responseType");
        Validate.notNull(responseMode, "responseMode");
        Validate.notNull(tokenSpec, "tokenSpec");

        com.nimbusds.oauth2.sdk.ResponseType nimbusResponseType = new com.nimbusds.oauth2.sdk.ResponseType();
        Set<ResponseValue> responseTypeSet = responseType.getResponseTypeSet();
        if (responseTypeSet.size() == 1 && responseTypeSet.contains(ResponseValue.CODE)) {
            if (!tokenSpec.getTokenType().equals(TokenType.HOK)) {
                throw new OIDCClientException("Only HOK token is supported when response type is code.");
            }
            if (!responseMode.equals(ResponseMode.QUERY) && !responseMode.equals(ResponseMode.FORM_POST)) {
                throw new OIDCClientException("Only 'QUERY' or 'FORM_POST' response mode is supported when response type is code.");
            }
            nimbusResponseType.add(com.nimbusds.oauth2.sdk.ResponseType.Value.CODE);
        } else if (responseTypeSet.contains(ResponseValue.ID_TOKEN)) {
            if (!tokenSpec.getTokenType().equals(TokenType.BEARER)) {
                throw new OIDCClientException("Only Bearer token is supported when response type includes id_token.");
            }
            if (!responseMode.equals(ResponseMode.FRAGMENT) && !responseMode.equals(ResponseMode.FORM_POST)) {
                throw new OIDCClientException("Only 'FRAGMENT' or 'FORM_POST' response mode is supported when response type includes id_token.");
            }
            if (nonce == null) {
                throw new OIDCClientException("Nonce is required for OIDC implicit flow.");
            }
            nimbusResponseType.add(OIDCResponseTypeValue.ID_TOKEN);
            if (responseTypeSet.size() == 1) {
                // do nothing
            } else if (responseTypeSet.size() == 2 && responseTypeSet.contains(ResponseValue.TOKEN)) {
                nimbusResponseType.add(com.nimbusds.oauth2.sdk.ResponseType.Value.TOKEN);
            } else {
                throw new OIDCClientException("The requested response type is not supported.");
            }
        } else {
            throw new OIDCClientException("The requested response type is not supported.");
        }

        URI authorizationEndpointURI = this.authorizationEndpointURI;
        if (highAvailabilityEnabled()) {
            String domainController = getAvailableDomainController();
            authorizationEndpointURI = OIDCClientUtils.changeUriHostComponent(authorizationEndpointURI, domainController);
        }

        Scope scope = OIDCClientUtils.buildScopeFromTokenSpec(tokenSpec);

        SignedJWT clientAssertion = null;
        if (this.holderOfKeyConfig != null) {
            try {
                clientAssertion = OIDCClientUtils.createAssertion(this.clientId, this.holderOfKeyConfig, authorizationEndpointURI.toString());
            } catch (JOSEException e) {
                throw new OIDCClientException("failed to construct client_assertion parameter");
            }
        }

        AuthenticationRequest authenticationRequest = new AuthenticationRequest(
                authorizationEndpointURI,
                nimbusResponseType,
                com.nimbusds.openid.connect.sdk.ResponseMode.parse(responseMode.getValue()),
                new com.nimbusds.oauth2.sdk.id.ClientID(this.clientId.getValue()),
                redirectEndpointURI,
                com.nimbusds.oauth2.sdk.Scope.parse(scope.getScopeList()),
                (state == null) ? null : new com.nimbusds.oauth2.sdk.id.State(state.getValue()),
                (nonce == null) ? null : new com.nimbusds.openid.connect.sdk.Nonce(nonce.getValue()),
                clientAssertion,
                new CorrelationID());

        try {
            return authenticationRequest.toURI();
        } catch (SerializeException e) {
            throw new OIDCClientException("Build authentication request URI failed: " + e.getMessage(), e);
        }
    }

    /**
     * Get tokens by grant
     *
     * @param authorizationGrant        Authorization grant. It can be one of the following:
     *                                  PasswordCredentialsGrant, GssGrant, SolutionUserCredentialsGrant,
     *                                  ClientCredentialsGrant, RefreshTokenGrant, AuthorizationCodeGrant
     * @param tokenSpec                 Specification of tokens requested.
     * @return                          OIDC Tokens.
     * @throws OIDCClientException      Client side exception.
     * @throws OIDCServerException      Server side exception.
     * @throws TokenValidationException Token validation exception.
     * @throws SSLConnectionException   SSL connection exception.
     */
    public OIDCTokens acquireTokens(
            AuthorizationGrant authorizationGrant,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(authorizationGrant, "authorizationGrant");
        Validate.notNull(tokenSpec, "tokenSpec");

        URI tokenEndpointURI = this.tokenEndpointURI;
        if (highAvailabilityEnabled()) {
            String domainController = getAvailableDomainController();
            tokenEndpointURI = OIDCClientUtils.changeUriHostComponent(tokenEndpointURI, domainController);
        }

        HTTPResponse httpResponse = null;

        if (authorizationGrant instanceof GssGrant) {
            httpResponse = OIDCClientUtils.negotiateGssResponse(
                    ((GssGrant) authorizationGrant).getNegotiationHandler(),
                    tokenSpec,
                    tokenEndpointURI,
                    this.clientId,
                    this.holderOfKeyConfig,
                    this.keyStore,
                    UUID.randomUUID().toString());
        } else {
            httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                    authorizationGrant,
                    tokenSpec,
                    tokenEndpointURI,
                    this.clientId,
                    this.holderOfKeyConfig,
                    this.keyStore);
        }

        OIDCTokens oidcTokens = OIDCClientUtils.parseTokenResponse(
                httpResponse,
                tokenSpec,
                this.providerPublicKey,
                this.clientId,
                this.issuer);
        return oidcTokens;
    }

    /**
     * Build a logout request URI
     *
     * @param postLogoutRedirectEndpointURI     Post logout URI.
     * @param idToken                           ID token received from a previous request.
     * @param state                             State value used in a logout request, it is optional.
     * @return                                  Logout request URI.
     * @throws OIDCClientException              Client side exception.
     */
    public URI buildLogoutRequestURI(
            URI postLogoutRedirectEndpointURI,
            IDToken idToken,
            State state) throws OIDCClientException {
        Validate.notNull(postLogoutRedirectEndpointURI, "postLogoutRedirectEndpointURI");
        Validate.notNull(idToken, "idToken");

        URI endSessionEndpointURI = this.endSessionEndpointURI;
        if (highAvailabilityEnabled()) {
            String domainController = getAvailableDomainController();
            endSessionEndpointURI = OIDCClientUtils.changeUriHostComponent(endSessionEndpointURI, domainController);
        }

        SignedJWT clientAssertion = null;
        if (this.holderOfKeyConfig != null) {
            try {
                clientAssertion = OIDCClientUtils.createAssertion(this.clientId, this.holderOfKeyConfig, endSessionEndpointURI.toString());
            } catch (JOSEException e) {
                throw new OIDCClientException("failed to construct client_assertion parameter");
            }
        }

        LogoutRequest logoutRequest = new LogoutRequest(
                endSessionEndpointURI,
                new com.vmware.identity.openidconnect.common.IDToken(idToken.getSignedJWT()),
                postLogoutRedirectEndpointURI,
                (state == null) ? null : new com.nimbusds.oauth2.sdk.id.State(state.getValue()),
                clientAssertion,
                new CorrelationID());

        try {
            return logoutRequest.toURI();
        } catch (SerializeException e) {
            throw new OIDCClientException("Build logout request URI failed: " + e.getMessage(), e);
        }
    }

    private boolean highAvailabilityEnabled() {
        return (this.highAvailabilityConfig != null);
    }

    private String getAvailableDomainController() throws OIDCClientException {
        String result;
        try (ClientDCCache session = this.highAvailabilityConfig.getClientDCCacheFactory().createSession()) {
            result = session.getAvailableDC();
        }
        return result;
    }
}
