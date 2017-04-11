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
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPublicKey;
import java.util.Date;
import java.util.UUID;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.PersonUserAssertionSigner;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.ResponseType;
import com.vmware.identity.openidconnect.common.ResponseTypeValue;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest;
import com.vmware.identity.openidconnect.protocol.AuthorizationCodeGrant;
import com.vmware.identity.openidconnect.protocol.AuthorizationGrant;
import com.vmware.identity.openidconnect.protocol.ClientAssertion;
import com.vmware.identity.openidconnect.protocol.ClientCredentialsGrant;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.LogoutRequest;
import com.vmware.identity.openidconnect.protocol.PasswordGrant;
import com.vmware.identity.openidconnect.protocol.PersonUserAssertion;
import com.vmware.identity.openidconnect.protocol.PersonUserCertificateGrant;
import com.vmware.identity.openidconnect.protocol.RefreshTokenGrant;
import com.vmware.identity.openidconnect.protocol.SolutionUserCredentialsGrant;
import com.vmware.identity.openidconnect.protocol.URIUtils;

/**
 * OIDC Client
 *
 * @author Jun Sun
 * @author Yehia Zayour
 */
public final class OIDCClient {

    private final URI authorizationEndpointURI;
    private final URI tokenEndpointURI;
    private final URI endSessionEndpointURI;
    private final RSAPublicKey providerPublicKey;
    private final Issuer issuer;

    private final ClientID clientId;
    private final ClientAuthenticationMethod clientAuthenticationMethod;
    private final HolderOfKeyConfig holderOfKeyConfig;
    private final HighAvailabilityConfig highAvailabilityConfig;
    private final long clockToleranceInSeconds;
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
        this.providerPublicKey = clientConfig.getConnectionConfig().getProviderPublicKey();
        this.issuer = clientConfig.getConnectionConfig().getIssuer();

        this.clientId = clientConfig.getClientId();
        this.clientAuthenticationMethod = clientConfig.getClientAuthenticationMethod();
        this.holderOfKeyConfig = clientConfig.getHolderOfKeyConfig();
        this.highAvailabilityConfig = clientConfig.getHighAvailabilityConfig();
        this.clockToleranceInSeconds = clientConfig.getClockToleranceInSeconds();
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
        Validate.notNull(state, "state");
        Validate.notNull(nonce, "nonce");

        if (responseType.contains(ResponseTypeValue.AUTHORIZATION_CODE)) {
            if (this.clientAuthenticationMethod != ClientAuthenticationMethod.PRIVATE_KEY_JWT) {
                throw new OIDCClientException("ClientAuthenticationMethod.PRIVATE_KEY_JWT is required when response type is code.");
            }
            if (responseMode != ResponseMode.QUERY && responseMode != ResponseMode.FORM_POST) {
                throw new OIDCClientException("Only 'QUERY' or 'FORM_POST' response mode is supported when response type is code.");
            }
        } else if (responseType.contains(ResponseTypeValue.ID_TOKEN)) {
            if (responseMode != ResponseMode.FRAGMENT && responseMode != ResponseMode.FORM_POST) {
                throw new OIDCClientException("Only 'FRAGMENT' or 'FORM_POST' response mode is supported when response type includes id_token.");
            }
        }

        URI authorizationEndpointURI = this.authorizationEndpointURI;
        if (highAvailabilityEnabled()) {
            String domainController = getAvailableDomainController();
            authorizationEndpointURI = URIUtils.changeHostComponent(authorizationEndpointURI, domainController);
        }

        Scope scope = OIDCClientUtils.buildScopeFromTokenSpec(tokenSpec);

        ClientAssertion clientAssertion = null;
        if (this.clientAuthenticationMethod == ClientAuthenticationMethod.PRIVATE_KEY_JWT) {
            clientAssertion = OIDCClientUtils.createClientAssertion(this.clientId, this.holderOfKeyConfig, authorizationEndpointURI);
        }

        AuthenticationRequest authenticationRequest = new AuthenticationRequest(
                authorizationEndpointURI,
                responseType,
                responseMode,
                this.clientId,
                redirectEndpointURI,
                scope,
                state,
                nonce,
                clientAssertion,
                new CorrelationID());
        return authenticationRequest.toHttpRequest().getURI();
    }

    OIDCTokens acquireTokens(
            AuthorizationGrant authorizationGrant,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(authorizationGrant, "authorizationGrant");
        Validate.notNull(tokenSpec, "tokenSpec");
        if ((authorizationGrant instanceof AuthorizationCodeGrant || authorizationGrant instanceof RefreshTokenGrant) && tokenSpec != TokenSpec.EMPTY) {
            throw new IllegalArgumentException("tokenSpec must be TokenSpec.EMPTY for authz code and refresh token grants");
        }

        HttpResponse httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                authorizationGrant,
                tokenSpec,
                getTokenEndpointURI(),
                this.clientId,
                this.clientAuthenticationMethod,
                this.holderOfKeyConfig,
                this.keyStore);

        return OIDCClientUtils.parseTokenResponse(
                httpResponse,
                this.providerPublicKey,
                this.issuer,
                this.clientId,
                this.clockToleranceInSeconds);
    }

    /**
     * tokens by smart card certificate that represents a PersonUser
     *
     * @param personUserCertificate     smart card cert
     * @param signer                    client-implemented interface that signs an object with the smart card private key
     * @param tokenSpec                 Specification of tokens requested.
     */
    public OIDCTokens acquireTokensByPersonUserCertificate(
            X509Certificate personUserCertificate,
            PersonUserAssertionSigner signer,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(personUserCertificate, "personUserCertificate");
        Validate.notNull(signer, "signer");
        Validate.notNull(tokenSpec, "tokenSpec");

        Date issueTime = new Date(); // now
        URI tokenEndpointURI = getTokenEndpointURI();

        PersonUserAssertion personUserAssertion;
        try {
            personUserAssertion = new PersonUserAssertion(
                    signer,
                    new JWTID(),
                    personUserCertificate.getSubjectDN().getName(),
                    tokenEndpointURI,
                    issueTime);
        } catch (JOSEException e) {
            throw new OIDCClientException("failed to construct PersonUserAssertion", e);
        }

        return acquireTokens(new PersonUserCertificateGrant(personUserCertificate, personUserAssertion), tokenSpec);
    }

    /**
     * Get tokens by GSSNegotiationHandler which handles multi-legged GSSTicketGrant
     *
     * @param gssNegotiationHandler     client-implemented interface that provides us with the next gss ticket
     * @param tokenSpec                 Specification of tokens requested.
     * @return                          OIDC Tokens.
     * @throws OIDCClientException      Client side exception.
     * @throws OIDCServerException      Server side exception.
     * @throws TokenValidationException Token validation exception.
     * @throws SSLConnectionException   SSL connection exception.
     */
    public OIDCTokens acquireTokensByGSS(
            GSSNegotiationHandler gssNegotiationHandler,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(gssNegotiationHandler, "gssNegotiationHandler");
        Validate.notNull(tokenSpec, "tokenSpec");

        HttpResponse httpResponse = OIDCClientUtils.negotiateGssResponse(
                gssNegotiationHandler,
                tokenSpec,
                getTokenEndpointURI(),
                this.clientId,
                this.clientAuthenticationMethod,
                this.holderOfKeyConfig,
                this.keyStore,
                UUID.randomUUID().toString());

        return OIDCClientUtils.parseTokenResponse(
                httpResponse,
                this.providerPublicKey,
                this.issuer,
                this.clientId,
                this.clockToleranceInSeconds);
    }

    /**
     * Get tokens by SecurIDRetriever which handles multi-legged SecurIDGrant
     *
     * @param usename
     * @param passcode
     * @param securIdRetriever          client-implemented class that provides us with the next RSA SecurID passcode
     * @param tokenSpec                 Specification of tokens requested.
     * @return                          OIDC Tokens.
     * @throws OIDCClientException      Client side exception.
     * @throws OIDCServerException      Server side exception.
     * @throws TokenValidationException Token validation exception.
     * @throws SSLConnectionException   SSL connection exception.
     */
    public OIDCTokens acquireTokensBySecurID(
            String username,
            String passcode,
            SecurIDRetriever securIdRetriever,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notEmpty(username, "username");
        Validate.notEmpty(passcode, "passcode");
        Validate.notNull(securIdRetriever, "securIdRetriever");
        Validate.notNull(tokenSpec, "tokenSpec");

        HttpResponse httpResponse = OIDCClientUtils.handleSecurIDMultiLeggedGrant(
                username,
                passcode,
                securIdRetriever,
                tokenSpec,
                getTokenEndpointURI(),
                this.clientId,
                this.clientAuthenticationMethod,
                this.holderOfKeyConfig,
                this.keyStore);

        return OIDCClientUtils.parseTokenResponse(
                httpResponse,
                this.providerPublicKey,
                this.issuer,
                this.clientId,
                this.clockToleranceInSeconds);
    }

    public OIDCTokens acquireTokensByPassword(
            String username,
            String password,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notEmpty(username, "username");
        Validate.notEmpty(password, "password");
        Validate.notNull(tokenSpec, "tokenSpec");
        return acquireTokens(new PasswordGrant(username, password), tokenSpec);
    }

    public OIDCTokens acquireTokensByRefreshToken(
            RefreshToken refreshToken) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(refreshToken, "refreshToken");
        return acquireTokens(new RefreshTokenGrant(refreshToken.getRefreshToken()), TokenSpec.EMPTY);
    }

    public OIDCTokens acquireTokensByAuthorizationCode(
            AuthorizationCode code,
            URI redirectUri) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(code, "code");
        Validate.notNull(redirectUri, "redirectUri");
        return acquireTokens(new AuthorizationCodeGrant(code, redirectUri), TokenSpec.EMPTY);
    }

    public OIDCTokens acquireTokensBySolutionUserCredentials(
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(tokenSpec, "tokenSpec");
        Validate.notNull(this.holderOfKeyConfig, "this.holderOfKeyConfig");
        return acquireTokens(new SolutionUserCredentialsGrant(), tokenSpec);
    }

    public OIDCTokens acquireTokensByClientCredentials(
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(tokenSpec, "tokenSpec");
        Validate.isTrue(
                this.clientAuthenticationMethod == ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                "this.clientAuthenticationMethod == ClientAuthenticationMethod.PRIVATE_KEY_JWT");
        return acquireTokens(new ClientCredentialsGrant(), tokenSpec);
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

        LogoutRequest logoutRequest = buildLogoutRequest(postLogoutRedirectEndpointURI, idToken, state);
        return logoutRequest.toHttpRequest().getURI();
    }

    /**
     * Build a logout request html form (client returns form that is auto-submitted to the Authorization Server)
     *
     * @param postLogoutRedirectEndpointURI     Post logout URI.
     * @param idToken                           ID token received from a previous request.
     * @param state                             State value used in a logout request, it is optional.
     * @return                                  Logout request html form.
     * @throws OIDCClientException              Client side exception.
     */
    public String buildLogoutRequestHtmlForm(
            URI postLogoutRedirectEndpointURI,
            IDToken idToken,
            State state) throws OIDCClientException {
        Validate.notNull(postLogoutRedirectEndpointURI, "postLogoutRedirectEndpointURI");
        Validate.notNull(idToken, "idToken");

        LogoutRequest logoutRequest = buildLogoutRequest(postLogoutRedirectEndpointURI, idToken, state);
        return logoutRequest.toHtmlForm();
    }

    private LogoutRequest buildLogoutRequest(
            URI postLogoutRedirectEndpointURI,
            IDToken idToken,
            State state) throws OIDCClientException {
        URI endSessionEndpointURI = this.endSessionEndpointURI;
        if (highAvailabilityEnabled()) {
            String domainController = getAvailableDomainController();
            endSessionEndpointURI = URIUtils.changeHostComponent(endSessionEndpointURI, domainController);
        }

        ClientAssertion clientAssertion = null;
        if (this.clientAuthenticationMethod == ClientAuthenticationMethod.PRIVATE_KEY_JWT) {
            clientAssertion = OIDCClientUtils.createClientAssertion(this.clientId, this.holderOfKeyConfig, endSessionEndpointURI);
        }

        return new LogoutRequest(
                endSessionEndpointURI,
                idToken.getIDToken(),
                postLogoutRedirectEndpointURI,
                state,
                clientAssertion,
                new CorrelationID());
    }

    private URI getTokenEndpointURI() throws OIDCClientException {
        URI tokenEndpointURI = this.tokenEndpointURI;
        if (highAvailabilityEnabled()) {
            String domainController = getAvailableDomainController();
            tokenEndpointURI = URIUtils.changeHostComponent(tokenEndpointURI, domainController);
        }
        return tokenEndpointURI;
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
