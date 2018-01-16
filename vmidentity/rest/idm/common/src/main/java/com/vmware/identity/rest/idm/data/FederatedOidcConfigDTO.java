/*
 *  Copyright (c) 2017 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.rest.idm.data;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;

import java.net.URI;
import java.net.URISyntaxException;
import java.security.PublicKey;

/**
 * The {@code FederatedOidcConfigDTO} represents OIDC Configuration used for OIDC Federation
 *
 * @author Sriram Nambakam
 */
@JsonDeserialize(builder=FederatedOidcConfigDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(JsonInclude.Include.NON_EMPTY)
public class FederatedOidcConfigDTO {

    private String issuerType;
    private String clientId;
    private String clientSecret;
    private String tokenEndpoint;
    private String authorizeEndpoint;
    private String logoutEndpoint;
    private String postLogoutEndpoint;
    private String jwksEndpoint;
    private String metadataEndpoint;
    private String redirectURL;

    public FederatedOidcConfigDTO(
            String issuerType,
            String clientId,
            String clientSecret,
            String tokenEndpoint,
            String authorizeEndpoint,
            String logoutEndpoint,
            String postLogoutEndpoint,
            String jwksEndpoint,
            String metadataEndpoint,
            String redirectURL
    ) {
        this.issuerType = issuerType;
        this.clientId = clientId;
        this.clientSecret = clientSecret;
        this.tokenEndpoint = tokenEndpoint;
        this.authorizeEndpoint = authorizeEndpoint;
        this.logoutEndpoint = logoutEndpoint;
        this.postLogoutEndpoint = postLogoutEndpoint;
        this.jwksEndpoint = jwksEndpoint;
        this.metadataEndpoint = metadataEndpoint;
        this.redirectURL = redirectURL;
    }

    public String getIssuerType() { return issuerType; }

    public String getClientId() {
        return clientId;
    }

    public String getClientSecret() {
        return clientSecret;
    }

    public String getTokenEndpoint() {
        return tokenEndpoint;
    }

    public String getLogoutEndpoint() {
        return logoutEndpoint;
    }

    public String getAuthorizeEndpoint() {
        return authorizeEndpoint;
    }

    public String getPostLogoutEndpoint() {
        return postLogoutEndpoint;
    }

    public String getJwksEndpoint() {
        return jwksEndpoint;
    }

    public String getMetadataEndpoint() {
        return metadataEndpoint;
    }

    public String getRedirectURL() {
        return redirectURL;
    }

    /**
     * Creates an instance of the {@link FederatedOidcConfigDTO.Builder} class.
     *
     * @return a new {@code FederatedOidcConfigDTO.Builder}.
     */
    public static FederatedOidcConfigDTO.Builder builder() {
        return new FederatedOidcConfigDTO.Builder();
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for
     * usage when constructing the object from its JSON string and thus only accepts
     * content for the canonical fields of the JSON representation. Other constructors
     * may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown=true)
    @JsonPOJOBuilder
    public static class Builder {
        private String issuerType;
        private String clientId;
        private String clientSecret;
        private String tokenEndpoint;
        private String authorizeEndpoint;
        private String logoutEndpoint;
        private String postLogoutEndpoint;
        private String jwksEndpoint;
        private String metadataEndpoint;
        private String redirectURL;

        public FederatedOidcConfigDTO.Builder withIssuerType(String issuerType) {
            if (issuerType != null) {
                validateNotEmpty(issuerType, "Issuer Type");
            }
            this.issuerType = issuerType;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withClientId(String clientId) {
            if (clientId != null) {
                validateNotEmpty(clientId, "Client ID");
            }
            this.clientId = clientId;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withClientSecret(String clientSecret) {
            if (clientSecret != null) {
                validateNotEmpty(clientSecret, "Client Secret");
            }
            this.clientSecret = clientSecret;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withTokenEndpoint(String tokenEndpoint) {
            if (tokenEndpoint != null) {
                validateURI(tokenEndpoint, "Token Endpoint");
            }
            this.tokenEndpoint = tokenEndpoint;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withAuthorizeEndpoint(String authorizeEndpoint) {
            if (authorizeEndpoint != null) {
                validateURI(authorizeEndpoint, "Authorize Endpoint");
            }
            this.authorizeEndpoint = authorizeEndpoint;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withLogoutEndpoint(String logoutEndpoint) {
            if (logoutEndpoint != null) {
                validateURI(logoutEndpoint, "Logout Endpoint");
            }
            this.logoutEndpoint = logoutEndpoint;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withPostLogoutEndpoint(String postLogoutEndpoint) {
            if (postLogoutEndpoint != null) {
                validateURI(postLogoutEndpoint, "Post Logout Endpoint");
            }
            this.postLogoutEndpoint = postLogoutEndpoint;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withMetadataEndpoint(String metadataEndpoint) {
            if (metadataEndpoint != null) {
                validateURI(metadataEndpoint, "Metadata Endpoint");
            }
            this.metadataEndpoint = metadataEndpoint;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withJwksEndpoint(String jwksEndpoint) {
            if (jwksEndpoint != null) {
                validateURI(jwksEndpoint, "JWKS Endpoint");
            }
            this.jwksEndpoint = jwksEndpoint;
            return this;
        }

        public FederatedOidcConfigDTO.Builder withRedirectURL(String redirectURL) {
            if (redirectURL != null) {
                validateURI(redirectURL, "Redirect URL");
            }
            this.redirectURL = redirectURL;
            return this;
        }

        public FederatedOidcConfigDTO build() {
            return new FederatedOidcConfigDTO(
                            issuerType,
                            clientId,
                            clientSecret,
                            tokenEndpoint,
                            authorizeEndpoint,
                            logoutEndpoint,
                            postLogoutEndpoint,
                            jwksEndpoint,
                            metadataEndpoint,
                            redirectURL
            );
        }

        private static void validateURI(String uri, String fieldName) {
            validateNotEmpty(uri, fieldName);
            try {
                URI test_uri = new URI(uri);
            } catch (URISyntaxException e) {
                throw new IllegalArgumentException(
                    String.format("Error: An invalid value [%s] was specified for field -%s", uri, fieldName)
                );
            }
        }

        private static void validateNotEmpty(String uri, String fieldName) {
            if (uri == null || uri.isEmpty()) {
                throw new IllegalArgumentException(
                    String.format("Error: An empty string value was specified for field - %s ", fieldName)
                );
            }
        }
    }
}
