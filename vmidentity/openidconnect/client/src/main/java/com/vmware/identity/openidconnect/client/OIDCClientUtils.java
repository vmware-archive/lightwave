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

import java.io.IOException;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.nio.charset.UnsupportedCharsetException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.interfaces.RSAPublicKey;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;
import org.apache.http.HttpEntity;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.entity.ContentType;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.RSAKey;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.ErrorCode;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.ScopeValue;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.AuthorizationGrant;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.ClientAssertion;
import com.vmware.identity.openidconnect.protocol.ClientCredentialsGrant;
import com.vmware.identity.openidconnect.protocol.GSSTicketGrant;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.JSONUtils;
import com.vmware.identity.openidconnect.protocol.SecurIDGrant;
import com.vmware.identity.openidconnect.protocol.SolutionUserAssertion;
import com.vmware.identity.openidconnect.protocol.SolutionUserCredentialsGrant;
import com.vmware.identity.openidconnect.protocol.TokenErrorResponse;
import com.vmware.identity.openidconnect.protocol.TokenRequest;
import com.vmware.identity.openidconnect.protocol.TokenResponse;
import com.vmware.identity.openidconnect.protocol.TokenSuccessResponse;

/**
 * Utils for OIDC client library
 *
 * @author Jun Sun
 * @author Yehia Zayour
 */
class OIDCClientUtils {
    static final int DEFAULT_OP_PORT = 443;
    static final int HTTP_CLIENT_TIMEOUT_MILLISECS = 60 * 1000; // set HTTP Client timeout to be 60 seconds.

    static HttpResponse sendSecureRequest(HttpRequest httpRequest, KeyStore keyStore) throws OIDCClientException, SSLConnectionException {
        Validate.notNull(httpRequest, "httpRequest");
        Validate.notNull(keyStore, "keyStore");

        TrustManagerFactory trustManagerFactory;
        SSLContext sslContext;
        try {
            trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            trustManagerFactory.init(keyStore);
            sslContext = SSLContext.getInstance("SSL");
            sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
        } catch (NoSuchAlgorithmException | KeyStoreException | KeyManagementException e) {
            throw new SSLConnectionException("Failed to build SSL Context: " + e.getMessage(), e);
        }

        return sendSecureRequest(httpRequest, sslContext);
    }

    static HttpResponse sendSecureRequest(HttpRequest httpRequest, SSLContext sslContext) throws OIDCClientException, SSLConnectionException {
        Validate.notNull(httpRequest, "httpRequest");
        Validate.notNull(sslContext, "sslContext");

        RequestConfig config = RequestConfig.custom()
                .setConnectTimeout(HTTP_CLIENT_TIMEOUT_MILLISECS)
                .setConnectionRequestTimeout(HTTP_CLIENT_TIMEOUT_MILLISECS)
                .setSocketTimeout(HTTP_CLIENT_TIMEOUT_MILLISECS)
                .build();

        CloseableHttpClient client = HttpClients.custom()
                .setSSLContext(sslContext)
                .setDefaultRequestConfig(config)
                .build();

        CloseableHttpResponse closeableHttpResponse = null;

        try {
            HttpRequestBase httpTask = httpRequest.toHttpTask();
            closeableHttpResponse = client.execute(httpTask);

            int statusCodeInt = closeableHttpResponse.getStatusLine().getStatusCode();
            StatusCode statusCode;
            try {
                statusCode = StatusCode.parse(statusCodeInt);
            } catch (ParseException e) {
                throw new OIDCClientException("failed to parse status code", e);
            }
            JSONObject jsonContent = null;
            HttpEntity httpEntity = closeableHttpResponse.getEntity();
            if (httpEntity != null) {
                ContentType contentType;
                try {
                    contentType = ContentType.get(httpEntity);
                } catch (UnsupportedCharsetException | org.apache.http.ParseException e) {
                    throw new OIDCClientException("Error in setting content type in HTTP response.");
                }
                if (!StandardCharsets.UTF_8.equals(contentType.getCharset())) {
                    throw new OIDCClientException("unsupported charset: " + contentType.getCharset());
                }
                if (!ContentType.APPLICATION_JSON.getMimeType().equalsIgnoreCase(contentType.getMimeType())) {
                    throw new OIDCClientException("unsupported mime type: " + contentType.getMimeType());
                }
                String content = EntityUtils.toString(httpEntity);
                try {
                    jsonContent = JSONUtils.parseJSONObject(content);
                } catch (ParseException e) {
                    throw new OIDCClientException("failed to parse json response", e);
                }
            }

            closeableHttpResponse.close();
            client.close();

            return HttpResponse.createJsonResponse(statusCode, jsonContent);
        } catch (IOException e) {
            throw new OIDCClientException("IOException caught in HTTP communication:" + e.getMessage(), e);
        }
    }

    static RSAPublicKey convertJWKSetToRSAPublicKey(JWKSet jwkSet) throws OIDCClientException {
        if (jwkSet == null || jwkSet.getKeys() == null || jwkSet.getKeys().size() != 1) {
            throw new OIDCClientException("Invalid JWK set.");
        }

        try {
            RSAKey rsaKey = (RSAKey) jwkSet.getKeys().get(0);
            return rsaKey.toRSAPublicKey();
        } catch (JOSEException e) {
            throw new OIDCClientException("Extract RSA public key from RSA key failed: " + e.getMessage(), e);
        }
    }

    static ClientAssertion createClientAssertion(
            ClientID clientId,
            HolderOfKeyConfig holderOfKeyConfig,
            URI endpointURI) throws OIDCClientException {
        Validate.notNull(clientId, "clientId");
        Validate.notNull(holderOfKeyConfig, "holderOfKeyConfig");
        Validate.notNull(endpointURI, "endpointURI");

        try {
            Date issueTime = new Date();
            return new ClientAssertion(
                    holderOfKeyConfig.getClientPrivateKey(),
                    new JWTID(),
                    clientId,
                    endpointURI,
                    issueTime);
        } catch (JOSEException e) {
            throw new OIDCClientException("failed to sign client_assertion", e);
        }
    }

    static SolutionUserAssertion createSolutionUserAssertion(
            HolderOfKeyConfig holderOfKeyConfig,
            URI tokenEndpointURI) throws OIDCClientException {
        Validate.notNull(holderOfKeyConfig, "holderOfKeyConfig");
        Validate.notNull(tokenEndpointURI, "tokenEndpointURI");

        try {
            Date issueTime = new Date();
            return new SolutionUserAssertion(
                    holderOfKeyConfig.getClientPrivateKey(),
                    new JWTID(),
                    holderOfKeyConfig.getClientCertificate().getSubjectDN().getName(),
                    tokenEndpointURI,
                    issueTime);
        } catch (JOSEException e) {
            throw new OIDCClientException("failed to sign solution_user_assertion", e);
        }
    }

    static TokenRequest buildTokenRequest(
            AuthorizationGrant grant,
            TokenSpec tokenSpec,
            URI tokenEndpointURI,
            ClientID clientId,
            ClientAuthenticationMethod clientAuthenticationMethod,
            HolderOfKeyConfig holderOfKeyConfig) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(grant, "grant");
        Validate.notNull(tokenSpec, "tokenSpec");
        Validate.notNull(tokenEndpointURI, "tokenEndpointURI");

        Scope scope = OIDCClientUtils.buildScopeFromTokenSpec(tokenSpec);

        SolutionUserAssertion solutionUserAssertion = null;
        ClientAssertion clientAssertion = null;

        if (clientAuthenticationMethod == ClientAuthenticationMethod.PRIVATE_KEY_JWT) {
            clientAssertion = createClientAssertion(clientId, holderOfKeyConfig, tokenEndpointURI);
        } else if (holderOfKeyConfig != null) {
            solutionUserAssertion = createSolutionUserAssertion(holderOfKeyConfig, tokenEndpointURI);
        }

        if (grant instanceof SolutionUserCredentialsGrant && solutionUserAssertion == null) {
            throw new OIDCClientException("Solution user credentials grant requires an non-null solution user assertion.");
        }

        if (grant instanceof ClientCredentialsGrant && clientAssertion == null) {
            throw new OIDCClientException("Client credentials grant requires an non-null client assertion.");
        }

        return new TokenRequest(
                tokenEndpointURI,
                grant,
                tokenSpec == TokenSpec.EMPTY ? null : scope,
                solutionUserAssertion,
                clientAssertion,
                clientId,
                new CorrelationID());
    }

    static HttpResponse buildAndSendTokenRequest(
            AuthorizationGrant grant,
            TokenSpec tokenSpec,
            URI tokenEndpointURI,
            ClientID clientId,
            ClientAuthenticationMethod clientAuthenticationMethod,
            HolderOfKeyConfig holderOfKeyConfig,
            KeyStore keyStore) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(grant, "grant");
        Validate.notNull(tokenSpec, "tokenSpec");
        Validate.notNull(tokenEndpointURI, "tokenEndpointURI");
        Validate.notNull(keyStore, "keyStore");

        TokenRequest tokenRequest = buildTokenRequest(
                grant,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig);

        return OIDCClientUtils.sendSecureRequest(tokenRequest.toHttpRequest(), keyStore);
    }

    static OIDCTokens parseTokenResponse(
            HttpResponse httpResponse,
            RSAPublicKey providerPublicKey,
            Issuer issuer,
            ClientID clientId,
            long clockToleranceInSeconds) throws OIDCClientException, TokenValidationException, OIDCServerException {
        Validate.notNull(httpResponse, "httpResponse");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(issuer, "issuer");

        TokenResponse tokenResponse;
        try {
            tokenResponse = TokenResponse.parse(httpResponse);
        } catch (ParseException e) {
            throw new OIDCClientException("Parse token response failed: " + e.getMessage(), e);
        }

        if (tokenResponse instanceof TokenSuccessResponse) {
            TokenSuccessResponse tokenSuccessResponse = (TokenSuccessResponse) tokenResponse;
            IDToken idToken = IDToken.build(
                    tokenSuccessResponse.getIDToken(),
                    providerPublicKey,
                    issuer,
                    clientId,
                    clockToleranceInSeconds);
            AccessToken accessToken = new AccessToken(tokenSuccessResponse.getAccessToken().serialize());
            RefreshToken refreshToken = tokenSuccessResponse.getRefreshToken() == null ?
                    null :
                    new RefreshToken(tokenSuccessResponse.getRefreshToken());
            return new OIDCTokens(idToken, accessToken, refreshToken);
        } else {
            TokenErrorResponse tokenErrorResponse = (TokenErrorResponse) tokenResponse;
            throw new OIDCServerException(tokenErrorResponse.getErrorObject());
        }
    }

    static Scope buildScopeFromTokenSpec(TokenSpec tokenSpec) throws OIDCClientException {
        Validate.notNull(tokenSpec, "tokenSpec");

        Set<ScopeValue> scopeValueSet = new HashSet<ScopeValue>();
        scopeValueSet.add(ScopeValue.OPENID);
        if (tokenSpec.isRefreshTokenRequested()) {
            scopeValueSet.add(ScopeValue.OFFLINE_ACCESS);
        }

        if (tokenSpec.idTokenGroupsRequested() == GroupMembershipType.FULL) {
            scopeValueSet.add(ScopeValue.ID_TOKEN_GROUPS);
        } else if (tokenSpec.idTokenGroupsRequested() == GroupMembershipType.FILTERED) {
            scopeValueSet.add(ScopeValue.ID_TOKEN_GROUPS_FILTERED);
        }

        if (tokenSpec.accessTokenGroupsRequested() == GroupMembershipType.FULL) {
            scopeValueSet.add(ScopeValue.ACCESS_TOKEN_GROUPS);
        } else if (tokenSpec.accessTokenGroupsRequested() == GroupMembershipType.FILTERED) {
            scopeValueSet.add(ScopeValue.ACCESS_TOKEN_GROUPS_FILTERED);
        }

        if (tokenSpec.getResourceServers() != null) {
            for (String resourceServer : tokenSpec.getResourceServers()) {
                ScopeValue scopeValue;
                try {
                    scopeValue = ScopeValue.parse(resourceServer);
                } catch (ParseException e) {
                    throw new OIDCClientException("failed to parse scope value", e);
                }
                scopeValueSet.add(scopeValue);
            }
        }

        return new Scope(scopeValueSet);
    }

    static boolean isValidGssResponse(ErrorObject errorObject) {
        String[] parts = errorObject.getDescription().split(":");
        return errorObject.getErrorCode() == ErrorCode.INVALID_GRANT
                && parts.length == 3
                && parts[0].equals("gss_continue_needed");
    }

    static boolean isSecurIDNextPasscode(ErrorObject errorObject) {
        String[] parts = errorObject.getDescription().split(":");
        return
                errorObject.getErrorCode() == ErrorCode.INVALID_GRANT &&
                parts.length == 2 &&
                parts[0].equals("securid_next_code_required");
    }

    static HttpResponse negotiateGssResponse(
            GSSNegotiationHandler gssNegotiationHandler,
            TokenSpec tokenSpec,
            URI tokenEndpointURI,
            ClientID clientId,
            ClientAuthenticationMethod clientAuthenticationMethod,
            HolderOfKeyConfig holderOfKeyConfig,
            KeyStore keyStore,
            String contextId) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {

        // set initial gss ticket to null
        byte[] gssTicket = gssNegotiationHandler.negotiate(null);

        HttpResponse httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                new GSSTicketGrant(contextId, gssTicket),
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore);

        while (httpResponse.getStatusCode() != StatusCode.OK) {
            try {
                TokenErrorResponse response = TokenErrorResponse.parse(httpResponse);
                ErrorObject errorObject = response.getErrorObject();
                if (OIDCClientUtils.isValidGssResponse(errorObject)) {
                    String[] parts = errorObject.getDescription().split(":");

                    if (parts[1].equals(contextId)) {
                        // send a new gss ticket
                        httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                                new GSSTicketGrant(
                                        parts[1],
                                        gssNegotiationHandler.negotiate(Base64Utils.decodeToBytes(parts[2]))),
                                tokenSpec,
                                tokenEndpointURI,
                                clientId,
                                clientAuthenticationMethod,
                                holderOfKeyConfig,
                                keyStore);
                    } else {
                        throw new OIDCClientException("Context Id received does not match.");
                    }
                } else {
                    throw new OIDCServerException(errorObject);
                }
            } catch (ParseException e) {
                throw new OIDCClientException("Parse token response failed: " + e.getMessage(), e);
            }
        }

        return httpResponse;
    }

    static HttpResponse handleSecurIDMultiLeggedGrant(
            String username,
            String passcode,
            SecurIDRetriever securIdRetriever,
            TokenSpec tokenSpec,
            URI tokenEndpointURI,
            ClientID clientId,
            ClientAuthenticationMethod clientAuthenticationMethod,
            HolderOfKeyConfig holderOfKeyConfig,
            KeyStore keyStore) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {

        HttpResponse httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                new SecurIDGrant(username, passcode, null /* sessionId */),
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore);

        while (httpResponse.getStatusCode() != StatusCode.OK) {
            TokenErrorResponse response;
            try {
                response = TokenErrorResponse.parse(httpResponse);
            } catch (ParseException e) {
                throw new OIDCClientException("Parse token response failed: " + e.getMessage(), e);
            }
            ErrorObject errorObject = response.getErrorObject();
            if (OIDCClientUtils.isSecurIDNextPasscode(errorObject)) {
                String[] parts = errorObject.getDescription().split(":");
                String sessionId = Base64Utils.decodeToString(parts[1]);
                httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                        new SecurIDGrant(username, securIdRetriever.getNextPasscode(username), sessionId),
                        tokenSpec,
                        tokenEndpointURI,
                        clientId,
                        clientAuthenticationMethod,
                        holderOfKeyConfig,
                        keyStore);
            } else {
                throw new OIDCServerException(errorObject);
            }
        }

        return httpResponse;
    }
}
