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
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.charset.UnsupportedCharsetException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.InvalidKeySpecException;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import org.apache.commons.lang3.Validate;
import org.apache.http.HttpEntity;
import org.apache.http.NameValuePair;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpDelete;
import org.apache.http.client.methods.HttpEntityEnclosingRequestBase;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.util.EntityUtils;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.JWSHeader;
import com.nimbusds.jose.JWSSigner;
import com.nimbusds.jose.crypto.RSASSASigner;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jose.util.Base64;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.ErrorObject;
import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.SerializeException;
import com.nimbusds.oauth2.sdk.auth.PrivateKeyJWT;
import com.nimbusds.oauth2.sdk.http.CommonContentTypes;
import com.nimbusds.oauth2.sdk.http.HTTPRequest;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.oauth2.sdk.id.JWTID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenErrorResponse;
import com.vmware.identity.openidconnect.common.TokenRequest;
import com.vmware.identity.openidconnect.common.TokenSuccessResponse;

/**
 * Utils for OIDC client library
 *
 * @author Jun Sun
 */
class OIDCClientUtils {
    static final int DEFAULT_OP_PORT = 443;
    static final String DEFAULT_TENANT = "vsphere.local";
    static final int HTTP_CLIENT_TIMEOUT_MILLISECS = 60 * 1000; // set HTTP Client timeout to be 60 seconds.

    static HTTPResponse sendSecureRequest(HTTPRequest httpRequest, KeyStore keyStore) throws OIDCClientException, SSLConnectionException {
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

    static HTTPResponse sendSecureRequest(HTTPRequest httpRequest, SSLContext sslContext) throws OIDCClientException, SSLConnectionException {
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

        if (httpRequest.getURL() == null) {
            throw new OIDCClientException("URL is null in HTTP request.");
        }

        HTTPResponse httpResponse;
        try {
            if (HTTPRequest.Method.GET.equals(httpRequest.getMethod())) {
                StringBuilder sb = new StringBuilder(httpRequest.getURL().toString());
                if (httpRequest.getQuery() != null) {
                    sb.append('?');
                    sb.append(httpRequest.getQuery());
                }
                HttpGet httpGet = new HttpGet(sb.toString());
                httpGet.setHeader("Authorization", httpRequest.getAuthorization());
                closeableHttpResponse = client.execute(httpGet);
            } else if (HTTPRequest.Method.POST.equals(httpRequest.getMethod()) || HTTPRequest.Method.PUT.equals(httpRequest.getMethod())){
                HttpEntityEnclosingRequestBase httpTask = null;
                if (HTTPRequest.Method.POST.equals(httpRequest.getMethod())) {
                    httpTask = new HttpPost(httpRequest.getURL().toString());
                } else {
                    httpTask = new HttpPut(httpRequest.getURL().toString());
                }
                httpTask.setHeader("Authorization", httpRequest.getAuthorization());

                if (CommonContentTypes.APPLICATION_URLENCODED.match(httpRequest.getContentType())) {
                    List<NameValuePair> urlParameters = new ArrayList<NameValuePair>();
                    for (String key : httpRequest.getQueryParameters().keySet()) {
                        urlParameters.add(new BasicNameValuePair(key, httpRequest.getQueryParameters().get(key)));
                    }
                    httpTask.setEntity(new UrlEncodedFormEntity(urlParameters, CommonContentTypes.APPLICATION_URLENCODED.getParameter("charset")));
                } else if (CommonContentTypes.APPLICATION_JSON.match(httpRequest.getContentType())) {
                    httpTask.setEntity(new StringEntity(httpRequest.getQuery(), CommonContentTypes.APPLICATION_JSON.getParameter("charset")));
                } else {
                    throw new OIDCClientException("Unsupported content type in HTTP request.");
                }
                httpTask.setHeader("Content-Type", httpRequest.getContentType().toString());

                closeableHttpResponse = client.execute(httpTask);
            } else if (HTTPRequest.Method.DELETE.equals(httpRequest.getMethod())){
                HttpDelete httpDelete = new HttpDelete(httpRequest.getURL().toString());
                httpDelete.setHeader("Authorization", httpRequest.getAuthorization());
                closeableHttpResponse = client.execute(httpDelete);
            } else {
                throw new OIDCClientException("Unsupported HTTP method in HTTP request.");
            }

            httpResponse = new HTTPResponse(closeableHttpResponse.getStatusLine().getStatusCode());
            HttpEntity httpEntity = closeableHttpResponse.getEntity();
            if (httpEntity != null) {
                try {
                    httpResponse.setContentType(ContentType.get(httpEntity).toString());
                } catch (UnsupportedCharsetException | ParseException | org.apache.http.ParseException e) {
                    throw new OIDCClientException("Error in setting content type in HTTP response.");
                }
                httpResponse.setContent(EntityUtils.toString(httpEntity));
            }

            closeableHttpResponse.close();
            client.close();
        } catch (IOException e) {
            throw new OIDCClientException("IOException caught in HTTP communication:" + e.getMessage(), e);
        }

        return httpResponse;
    }

    static RSAPublicKey convertJWKSetToRSAPublicKey(JWKSet jwkSet) throws OIDCClientException {
        if (jwkSet == null || jwkSet.getKeys() == null || jwkSet.getKeys().size() != 1) {
            throw new OIDCClientException("Invalid JWK set.");
        }

        try {
            RSAKey rsaKey = (RSAKey) jwkSet.getKeys().get(0);
            return rsaKey.toRSAPublicKey();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new OIDCClientException("Extract RSA public key from RSA key failed: " + e.getMessage(), e);
        }
    }

    static SignedJWT createAssertion(
            ClientID clientId,
            HolderOfKeyConfig holderOfKeyConfig,
            String tokenEndpoint) throws JOSEException {
        Validate.notNull(holderOfKeyConfig, "holderOfKeyConfig");
        Validate.notNull(tokenEndpoint, "tokenEndpoint");

        final long jwtLifeTime = 2 * 60 * 1000L;
        Date now = new Date();

        String tokenClass = (clientId == null) ?  TokenClass.SOLUTION_ASSERTION.getName() : TokenClass.CLIENT_ASSERTION.getName();
        String tokenIssuer = (clientId == null) ? holderOfKeyConfig.getClientCertificate().getSubjectDN().getName() : clientId.getValue();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", tokenClass);
        claimsSet.setClaim("token_type", com.vmware.identity.openidconnect.common.TokenType.BEARER.getName());
        claimsSet.setJWTID((new JWTID()).toString());
        claimsSet.setIssuer(tokenIssuer);
        claimsSet.setSubject(tokenIssuer);
        claimsSet.setAudience(tokenEndpoint);
        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + jwtLifeTime));

        SignedJWT signedJwt = new SignedJWT(new JWSHeader(JWSAlgorithm.RS256), claimsSet);
        JWSSigner signer = new RSASSASigner(holderOfKeyConfig.getClientPrivateKey());
        signedJwt.sign(signer);
        return signedJwt;
    }

    static TokenRequest buildTokenRequest(
            AuthorizationGrant grant,
            TokenSpec tokenSpec,
            URI tokenEndpointURI,
            ClientID clientId,
            HolderOfKeyConfig holderOfKeyConfig) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        Validate.notNull(grant, "grant");
        Validate.notNull(tokenSpec, "tokenSpec");
        Validate.notNull(tokenEndpointURI, "tokenEndpointURI");

        Scope scope = OIDCClientUtils.buildScopeFromTokenSpec(tokenSpec);
        List<String> scopeList = scope.getScopeList();

        SignedJWT solutionAssertion = null;
        PrivateKeyJWT clientAssertion = null;

        if (tokenSpec.getTokenType().equals(TokenType.HOK)) {
            if (holderOfKeyConfig == null) {
                throw new OIDCClientException("Holder of key configuation can not be null if HOK token is requested.");
            }

            try {
                SignedJWT signedJWT = createAssertion(
                        clientId,
                        holderOfKeyConfig,
                        tokenEndpointURI.toString());
                if (clientId == null) {
                    solutionAssertion = signedJWT;
                } else {
                    clientAssertion = new PrivateKeyJWT(signedJWT);
                }
            } catch (JOSEException e) {
                throw new OIDCClientException("Build Assertion JWT token failed: " + e.getMessage(), e);
            }
        }

        if (grant instanceof SolutionUserCredentialsGrant && solutionAssertion == null) {
            throw new OIDCClientException("Solution user credentials grant requires an non-null solution assertion.");
        }

        if (grant instanceof ClientCredentialsGrant && clientAssertion == null) {
            throw new OIDCClientException("Client credentials grant requires an non-null client assertion.");
        }

        if (grant instanceof RefreshTokenGrant || grant instanceof AuthorizationCodeGrant) {
            // refresh token and authorization code grant requires a null scope
            scopeList = null;
        }

        return TokenRequest.create(
                tokenEndpointURI,
                grant.toNimbusAuthorizationGrant(),
                com.nimbusds.oauth2.sdk.Scope.parse(scopeList),
                solutionAssertion,
                clientAssertion,
                new CorrelationID());
    }

    static HTTPResponse buildAndSendTokenRequest(
            AuthorizationGrant grant,
            TokenSpec tokenSpec,
            URI tokenEndpointURI,
            ClientID clientId,
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
                holderOfKeyConfig);

        try {
            return OIDCClientUtils.sendSecureRequest(tokenRequest.toHTTPRequest(), keyStore);
        } catch (SerializeException e) {
            throw new OIDCClientException("Convert token request to HTTP request failed: " + e.getMessage(), e);
        }
    }

    static OIDCTokens parseTokenResponse(
            HTTPResponse httpResponse,
            TokenSpec tokenSpec,
            RSAPublicKey providerPublicKey,
            ClientID clientId,
            Issuer issuer) throws OIDCClientException, TokenValidationException, OIDCServerException {
        Validate.notNull(httpResponse, "httpResponse");
        Validate.notNull(tokenSpec, "tokenSpec");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(issuer, "issuer");

        if (httpResponse.getStatusCode() == 200) {
            try {
                TokenSuccessResponse response = TokenSuccessResponse.parse(httpResponse);
                IDToken idToken = null;
                RefreshToken refreshToken = null;
                AccessToken accessToken = null;

                idToken = IDToken.build(
                        response.getIDToken().getSignedJWT(),
                        providerPublicKey,
                        clientId,
                        issuer);

                if (response.getAccessToken() != null) {
                    accessToken = new AccessToken(response.getAccessToken().getValue());
                }

                if (response.getRefreshToken() != null) {
                    refreshToken = new RefreshToken(response.getRefreshToken().getValue());
                }

                return new OIDCTokens(
                        accessToken,
                        idToken,
                        refreshToken);
            } catch (ParseException e) {
                throw new OIDCClientException("Parse token response failed: " + e.getMessage(), e);
            }
        } else {
            try {
                TokenErrorResponse response = TokenErrorResponse.parse(httpResponse);
                ErrorObject errorObject = response.getErrorObject();
                throw new OIDCServerException(errorObject.getCode(), errorObject.getDescription());
            } catch (ParseException e) {
                throw new OIDCClientException("Parse token response failed: " + e.getMessage(), e);
            }
        }
    }

    static Scope buildScopeFromTokenSpec(TokenSpec tokenSpec) throws OIDCClientException {
        Validate.notNull(tokenSpec, "tokenSpec");

        List<String> scopeList = new ArrayList<String>();
        scopeList.add("openid");
        if (tokenSpec.isRefreshTokenRequested()) {
            scopeList.add("offline_access");
        }
        if (tokenSpec.isIdTokenGroupsRequested()) {
            scopeList.add("id_groups");
        }
        if (tokenSpec.isAccessTokenGroupsRequested()) {
            scopeList.add("at_groups");
        }
        if (tokenSpec.getResouceServers() != null) {
            for (String resourceServer : tokenSpec.getResouceServers()) {
                if (!resourceServer.startsWith("rs_")) {
                    throw new OIDCClientException("Resource server name should start with prefix \"rs_\".");
                }
            }
            scopeList.addAll(tokenSpec.getResouceServers());
        }
        if (tokenSpec.getAdditionalScopeValues() != null) {
            scopeList.addAll(tokenSpec.getAdditionalScopeValues());
        }
        Scope scope = new Scope(scopeList);
        return scope;
    }

    static boolean isValidGssResponse(ErrorObject errorObject) {
        String[] parts = errorObject.getDescription().split(":");
        return OAuth2Error.INVALID_GRANT.getCode().equals(errorObject.getCode())
                && parts.length == 3
                && parts[0].equals("gss_continue_needed");
    }

    static HTTPResponse negotiateGssResponse(
            NegotiationHandler negotiationHandler,
            TokenSpec tokenSpec,
            URI tokenEndpointURI,
            ClientID clientId,
            HolderOfKeyConfig holderOfKeyConfig,
            KeyStore keyStore,
            String contextId) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {

        // set initial gss ticket to null
        byte[] gssTicket = negotiationHandler.negotiate(null);

        HTTPResponse httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                new GssTicketGrant(contextId, gssTicket),
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore);

        while (httpResponse.getStatusCode() != 200) {
            try {
                TokenErrorResponse response = TokenErrorResponse.parse(httpResponse);
                ErrorObject errorObject = response.getErrorObject();
                if (OIDCClientUtils.isValidGssResponse(errorObject)) {
                    String[] parts = errorObject.getDescription().split(":");

                    if (parts[1].equals(contextId)) {
                        // send a new gss ticket
                        httpResponse = OIDCClientUtils.buildAndSendTokenRequest(
                                new GssTicketGrant(
                                        parts[1],
                                        negotiationHandler.negotiate(new Base64(parts[2]).decode())),
                                tokenSpec,
                                tokenEndpointURI,
                                clientId,
                                holderOfKeyConfig,
                                keyStore);
                    } else {
                        throw new OIDCClientException("Context Id received does not match.");
                    }
                } else {
                    throw new OIDCServerException(errorObject.getCode(), errorObject.getDescription());
                }
            } catch (ParseException e) {
                throw new OIDCClientException("Parse token response failed: " + e.getMessage(), e);
            }
        }

        return httpResponse;
    }

    static URI changeUriHostComponent(URI uri, String host) throws OIDCClientException {
        Validate.notNull(uri, "uri");
        Validate.notEmpty(host, "host");

        URI result;
        try {
            URL oldUrl = uri.toURL();
            URL newUrl = new URL(oldUrl.getProtocol(), host, oldUrl.getPort(), oldUrl.getFile());
            result = new URI(newUrl.toString());
        } catch (MalformedURLException | URISyntaxException e) {
            throw new OIDCClientException("uri/url syntax exception", e);
        }
        return result;
    }

    static URL buildBaseUrl(String domainControllerFQDN, int domainControllerPort) {
        StringBuilder sb = new StringBuilder();
        sb.append("https://");
        sb.append(domainControllerFQDN);
        sb.append(":");
        sb.append(String.valueOf(domainControllerPort));
        try {
            return new URL(sb.toString());
        } catch (MalformedURLException e) {
            throw new IllegalArgumentException("Failed to build Admin server base URL: " + e.getMessage(), e);
        }
    }

    static URL buildEndpointUrl(URL baseUrl, String path) {
        StringBuilder sb = new StringBuilder(baseUrl.toString());
        sb.append(path);
        try {
            return new URL(sb.toString());
        } catch (MalformedURLException e) {
            throw new IllegalArgumentException("Failed to build endpoint URL: " + e.getMessage(), e);
        }
    }
}
