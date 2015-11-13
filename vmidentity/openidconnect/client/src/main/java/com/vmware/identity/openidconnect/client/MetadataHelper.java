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

import java.net.MalformedURLException;
import java.net.URL;
import java.security.KeyStore;
import java.security.interfaces.RSAPublicKey;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.http.HTTPRequest;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.openid.connect.sdk.op.OIDCProviderMetadata;

/**
 * OIDC Metadata helper class
 *
 * @author Jun Sun
 */
public class MetadataHelper {
    private final URL metadataURL;
    private final KeyStore keyStore;

    private MetadataHelper(Builder builder) {
        this.metadataURL = builder.metadataURL;
        this.keyStore = builder.keyStore;
    }

    /**
     * Builder for MetadataHelper
     */
    public static class Builder {
        private URL metadataURL;
        private KeyStore keyStore;
        private final String domainControllerFQDN;
        private int domainControllerPort = OIDCClientUtils.DEFAULT_OP_PORT;
        private String tenant = OIDCClientUtils.DEFAULT_TENANT;

        /**
         * Constructor
         *
         * @param domainControllerFQDN      Domain controller FQDN which runs service.
         */
        public Builder(String domainControllerFQDN) {
            Validate.notEmpty(domainControllerFQDN, "domainControllerFQDN");

            this.domainControllerFQDN = domainControllerFQDN;
        }

        /**
         * Set domain controller port
         *
         * @param domainControllerPort      Domain controller Port which runs service.
         * @return Builder
         */
        public Builder domainControllerPort(int domainControllerPort) {
            Validate.isTrue(domainControllerPort > 0, "domainControllerPort");

            this.domainControllerPort = domainControllerPort;
            return this;
        }

        /**
         * Set tenant
         *
         * @param tenant                    Tenant.
         * @return Builder
         */
        public Builder tenant(String tenant) {
            Validate.notEmpty(tenant, "tenant");

            this.tenant = tenant;
            return this;
        }

        /**
         * Set key store
         *
         * @param keyStore                  Key store which contains server SSL certificate.
         * @return Builder
         */
        public Builder keyStore(KeyStore keyStore) {
            Validate.notNull(keyStore, "keyStore");

            this.keyStore = keyStore;
            return this;
        }

        /**
         * Build MetadataHelper
         *
         * @return MetadataHelper
         */
        public MetadataHelper build() {
            if (this.keyStore == null) {
                this.keyStore = VecsKeyStore.getInstance();
            }

            StringBuilder sb = new StringBuilder();
            sb.append("https://");
            sb.append(this.domainControllerFQDN);
            sb.append(":");
            sb.append(String.valueOf(this.domainControllerPort));
            sb.append("/openidconnect/");
            sb.append(this.tenant);
            sb.append("/.well-known/openid-configuration");

            try {
                this.metadataURL = new URL(sb.toString());
            } catch (MalformedURLException e) {
                throw new IllegalArgumentException("Failed to build metadata endpoint URL: " + e.getMessage(), e);
            }

            return new MetadataHelper(this);
        }
    }

    /**
     * Get OIDC server meta data.
     *
     * @return                          OIDC server metadata. It contains server endpoints URI, issuer and other server supported capability configurations.
     * @throws OIDCClientException      Client side exception.
     * @throws OIDCServerException      Server side exception.
     * @throws SSLConnectionException   SSL connection exception.
     */
    public ProviderMetadata getProviderMetadata() throws OIDCClientException, OIDCServerException, SSLConnectionException {
        HTTPRequest httpRequest = null;
        httpRequest = new HTTPRequest(HTTPRequest.Method.GET, this.metadataURL);
        HTTPResponse httpResponse = OIDCClientUtils.sendSecureRequest(httpRequest, this.keyStore);
        OIDCProviderMetadata oidcProviderMetadata = parseMetadataResponse(httpResponse);

        List<ResponseType> responseTypes = new ArrayList<ResponseType>();
        for (com.nimbusds.oauth2.sdk.ResponseType rt : oidcProviderMetadata.getResponseTypes()) {
            Set<ResponseValue> rvs = new HashSet<ResponseValue>();
            for (Iterator<com.nimbusds.oauth2.sdk.ResponseType.Value> iter = rt.iterator(); iter.hasNext(); ) {
                rvs.add(ResponseValue.getResponseValue(iter.next().getValue()));
            }
            responseTypes.add(new ResponseType(rvs));
        }

        List<ResponseMode> responseModes = new ArrayList<ResponseMode>();
        for (com.nimbusds.openid.connect.sdk.ResponseMode rm : oidcProviderMetadata.getResponseModes()) {
            responseModes.add(ResponseMode.getResponseMode(rm.getValue()));
        }

        List<GrantType> grantTypes = new ArrayList<GrantType>();
        for (com.nimbusds.oauth2.sdk.GrantType gt : oidcProviderMetadata.getGrantTypes()) {
            grantTypes.add(GrantType.getGrantType(gt.getValue()));
        }

        List<SubjectType> subjectTypes = new ArrayList<SubjectType>();
        for (com.nimbusds.openid.connect.sdk.SubjectType st : oidcProviderMetadata.getSubjectTypes()) {
            subjectTypes.add(SubjectType.getSubjectType(st.toString()));
        }

        List<ClientAuthenticationMethod> tokenEndpointAuthMethods = new ArrayList<ClientAuthenticationMethod>();
        for (com.nimbusds.oauth2.sdk.auth.ClientAuthenticationMethod cam : oidcProviderMetadata.getTokenEndpointAuthMethods()) {
            tokenEndpointAuthMethods.add(ClientAuthenticationMethod.getClientAuthenticationMethod(cam.getValue()));
        }

        List<JWSAlgorithm> tokenEndpointJWSAlgs = new ArrayList<JWSAlgorithm>();
        for (com.nimbusds.jose.JWSAlgorithm ja : oidcProviderMetadata.getTokenEndpointJWSAlgs()) {
            tokenEndpointJWSAlgs.add(JWSAlgorithm.getJWSAlgorithm(ja.getName()));
        }

        List<JWSAlgorithm> idTokenJWSAlgs = new ArrayList<JWSAlgorithm>();
        for (com.nimbusds.jose.JWSAlgorithm ja : oidcProviderMetadata.getIDTokenJWSAlgs()) {
            idTokenJWSAlgs.add(JWSAlgorithm.getJWSAlgorithm(ja.getName()));
        }

        ProviderMetadata providerMetadata = new ProviderMetadata(
                new Issuer(oidcProviderMetadata.getIssuer().getValue()),
                oidcProviderMetadata.getAuthorizationEndpointURI(),
                oidcProviderMetadata.getTokenEndpointURI(),
                oidcProviderMetadata.getRegistrationEndpointURI(),
                oidcProviderMetadata.getEndSessionEndpointURI(),
                oidcProviderMetadata.getJWKSetURI(),
                new Scope(oidcProviderMetadata.getScopes().toStringList()),
                responseTypes,
                responseModes,
                grantTypes,
                subjectTypes,
                tokenEndpointAuthMethods,
                tokenEndpointJWSAlgs,
                idTokenJWSAlgs,
                oidcProviderMetadata.getClaims(),
                oidcProviderMetadata.supportsRequestURIParam());
        return providerMetadata;
    }

    /**
     * Get OIDC server tenant public key.
     *
     * @param providerMetadata          OIDC server metadata from querying metadata endpoint. It contains server endpoints URI,
     *                                  issuer and other server supported capability configurations.
     * @return                          OIDC server public key. This key is used to validate token signatures.
     * @throws OIDCClientException      Client side exception.
     * @throws OIDCServerException      Server side exception.
     * @throws SSLConnectionException   SSL connection exception.
     */
    public RSAPublicKey getProviderRSAPublicKey(ProviderMetadata providerMetadata) throws OIDCClientException, OIDCServerException, SSLConnectionException {
        JWKSet providerJWKSet = getProviderJWKSet(providerMetadata);

        return OIDCClientUtils.convertJWKSetToRSAPublicKey(providerJWKSet);
    }

    private JWKSet getProviderJWKSet(ProviderMetadata providerMetadata) throws OIDCClientException, OIDCServerException, SSLConnectionException {
        HTTPRequest httpRequest = null;
        try {
            httpRequest = new HTTPRequest(HTTPRequest.Method.GET, providerMetadata.getJWKSetURI().toURL());
        } catch (MalformedURLException e) {
            throw new OIDCClientException("URL conversion exception: " + e.getMessage(), e);
        }
        HTTPResponse httpResponse = OIDCClientUtils.sendSecureRequest(httpRequest, this.keyStore);
        JWKSet providerJWKSet = parseAuthorizationServerJWKSetResponse(httpResponse);

        return providerJWKSet;
    }

    private OIDCProviderMetadata parseMetadataResponse(HTTPResponse httpResponse) throws OIDCClientException, OIDCServerException {
        Validate.notNull(httpResponse, "httpResponse");

        try {
            verifyHTTPResponse(httpResponse);

            OIDCProviderMetadata oidcProviderMetadata = OIDCProviderMetadata.parse(httpResponse.getContentAsJSONObject());
            return oidcProviderMetadata;
        } catch (ParseException e) {
            throw new OIDCClientException("Metadata response parse failed: " + e.getMessage(), e);
        }
    }

    private JWKSet parseAuthorizationServerJWKSetResponse(HTTPResponse httpResponse) throws OIDCClientException, OIDCServerException {
        Validate.notNull(httpResponse, "httpResponse");

        try {
            verifyHTTPResponse(httpResponse);

            JWKSet jwkSet = JWKSet.parse(httpResponse.getContentAsJSONObject());
            return jwkSet;
        } catch (ParseException | java.text.ParseException e) {
            throw new OIDCClientException("Authorization server JWK set parse failed: " + e.getMessage(), e);
        }
    }

    private void verifyHTTPResponse(HTTPResponse httpResponse) throws OIDCServerException, OIDCClientException {
        Validate.notNull(httpResponse, "httpResponse");

        if (httpResponse.getStatusCode() != HTTPResponse.SC_OK) {
            JSONObject jsonObject = null;
            try {
                jsonObject = httpResponse.getContentAsJSONObject();
            } catch (ParseException e) {
                throw new OIDCClientException("Parse HTTP response failed: " + e.getMessage(), e);
            }
            throw new OIDCServerException((String) jsonObject.get("error"), (String) jsonObject.get("error_description"));
        }
    }
}
