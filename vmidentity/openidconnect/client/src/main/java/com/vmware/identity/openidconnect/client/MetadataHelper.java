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

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.jwk.JWKSet;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.ErrorObjectMapper;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.ProviderMetadataMapper;
import com.vmware.identity.openidconnect.protocol.URIUtils;

/**
 * OIDC Metadata helper class
 *
 * @author Jun Sun
 */
public final class MetadataHelper {
    private final URI metadataURI;
    private final KeyStore keyStore;

    private MetadataHelper(Builder builder) {
        this.metadataURI = builder.metadataURI;
        this.keyStore = builder.keyStore;
    }

    /**
     * Builder for MetadataHelper
     */
    public static class Builder {
        private URI metadataURI;
        private KeyStore keyStore;
        private final String domainControllerFQDN;
        private int domainControllerPort = OIDCClientUtils.DEFAULT_OP_PORT;
        private String tenant;

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
            if (this.tenant != null) {
                sb.append(this.tenant);
                sb.append("/");
            }
            sb.append(".well-known/openid-configuration");

            try {
                this.metadataURI = URIUtils.parseURI(sb.toString());
            } catch (ParseException e) {
                throw new IllegalArgumentException("Failed to build metadata endpoint URI: " + e.getMessage(), e);
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
        HttpRequest httpRequest = HttpRequest.createGetRequest(this.metadataURI);
        HttpResponse httpResponse = OIDCClientUtils.sendSecureRequest(httpRequest, this.keyStore);
        ProviderMetadata providerMetadata = parseMetadataResponse(httpResponse);
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
        HttpRequest httpRequest = HttpRequest.createGetRequest(providerMetadata.getJWKSetURI());
        HttpResponse httpResponse = OIDCClientUtils.sendSecureRequest(httpRequest, this.keyStore);
        JWKSet providerJWKSet = parseAuthorizationServerJWKSetResponse(httpResponse);

        return providerJWKSet;
    }

    private ProviderMetadata parseMetadataResponse(HttpResponse httpResponse) throws OIDCClientException, OIDCServerException {
        Validate.notNull(httpResponse, "httpResponse");

        try {
            verifyHttpResponse(httpResponse);

            ProviderMetadata providerMetadata = ProviderMetadataMapper.parse(httpResponse.getJsonContent());
            return providerMetadata;
        } catch (ParseException e) {
            throw new OIDCClientException("Metadata response parse failed: " + e.getMessage(), e);
        }
    }

    private JWKSet parseAuthorizationServerJWKSetResponse(HttpResponse httpResponse) throws OIDCClientException, OIDCServerException {
        Validate.notNull(httpResponse, "httpResponse");

        try {
            verifyHttpResponse(httpResponse);

            JWKSet jwkSet = JWKSet.parse(httpResponse.getJsonContent());
            return jwkSet;
        } catch (java.text.ParseException e) {
            throw new OIDCClientException("Authorization server JWK set parse failed: " + e.getMessage(), e);
        }
    }

    private void verifyHttpResponse(HttpResponse httpResponse) throws OIDCServerException, OIDCClientException {
        Validate.notNull(httpResponse, "httpResponse");

        if (httpResponse.getJsonContent() == null) {
            throw new OIDCClientException("expecting json http response");
        }

        if (httpResponse.getStatusCode() != StatusCode.OK) {
            ErrorObject errorObject;
            try {
                errorObject = ErrorObjectMapper.parse(httpResponse.getJsonContent(), httpResponse.getStatusCode());
            } catch (ParseException e) {
                throw new OIDCClientException("failed to parse ErrorObject", e);
            }
            throw new OIDCServerException(errorObject);
        }
    }
}
