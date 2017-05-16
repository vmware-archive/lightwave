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
package com.vmware.identity.rest.idm.data;

import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code OIDCClientMetadataDTO} class contains the metadata associated with
 * an {@link OIDCClientDTO}.
 *
 * @see <a href=https://tools.ietf.org/html/rfc6749#section-2>
 *  The OAuth 2.0 Authorization Framework - Client Registration
 *  </a>
 * @see <a href=http://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata>
 *  OpenID Connect Dynamic Client Registration 1.0 incorporating errata set 1 - Client Metadata
 *  </a>
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
@JsonDeserialize(builder = OIDCClientMetadataDTO.Builder.class)
public class OIDCClientMetadataDTO extends DTO {

    private final List<String> redirectUris;
    private final String tokenEndpointAuthMethod;
    private final List<String> postLogoutRedirectUris;
    private final String logoutUri;
    private final String certSubjectDN;
    private final Long authnRequestClientAssertionLifetimeMS;
    private final String clientSecret;
    private final List<String> authorities;
    private final List<String> resourceIds;
    private final List<String> scopes;
    private final List<String> autoApproveScopes;
    private final List<String> authorizedGrantTypes;
    private final Map<String, Object> additionalInformation;

    /**
     * Construct an {@code OIDCClientMetadataDTO} with its various details.
     *
     * @param redirectUris the list of redirect URIs.
     * @param tokenEndpointAuthMethod the authentication method for the token endpoint.
     * @param postLogoutRedirectUris the list of post-logout redirect URIs.
     * @param logoutUri the logout URI.
     * @param certSubjectDN the certificate subject DN.
     * @param authnRequestClientAssertionLifetimeMS lifetime of the client_assertion in authn requests
     */
    public OIDCClientMetadataDTO(
            List<String> redirectUris,
            String tokenEndpointAuthMethod,
            List<String> postLogoutRedirectUris,
            String logoutUri,
            String certSubjectDN,
            Long authnRequestClientAssertionLifetimeMS,
            String clientSecret,
            List<String> authorities,
            List<String> resourceIds,
            List<String> scopes,
            List<String> autoApproveScopes,
            List<String> authorizedGrantTypes,
            Map<String, Object> additionalInformation) {
        this.redirectUris = redirectUris;
        this.tokenEndpointAuthMethod = tokenEndpointAuthMethod;
        this.postLogoutRedirectUris = postLogoutRedirectUris;
        this.logoutUri = logoutUri;
        this.certSubjectDN = certSubjectDN;
        this.authnRequestClientAssertionLifetimeMS = authnRequestClientAssertionLifetimeMS;
        this.clientSecret = clientSecret;
        this.authorities = authorities;
        this.resourceIds = resourceIds;
        this.scopes = scopes;
        this.autoApproveScopes = autoApproveScopes;
        this.authorizedGrantTypes = authorizedGrantTypes;
        this.additionalInformation = additionalInformation;
    }

    /**
     * Get the list of redirect URIs for the OIDC Client.
     *
     * @return the list of redirect URIs.
     */
    public List<String> getRedirectUris() {
        return this.redirectUris;
    }

    /**
     * Get the authentication method for the token endpoint of the OIDC client.
     *
     * @return the authentication method for the token endpoint.
     */
    public String getTokenEndpointAuthMethod() {
        return this.tokenEndpointAuthMethod;
    }

    /**
     * Get the list of post-logout redirect URIs for the OIDC client.
     *
     * @return the list of post-logout redirect URIs.
     */
    public List<String> getPostLogoutRedirectUris() {
        return this.postLogoutRedirectUris;
    }

    /**
     * Get the logout URI for the OIDC client.
     *
     * @return the logout URI.
     */
    public String getLogoutUri() {
        return this.logoutUri;
    }

    /**
     * Get the certificate subject DN for the OIDC client.
     *
     * @return the certificate subject DN.
     */
    public String getCertSubjectDN() {
        return this.certSubjectDN;
    }

    /**
     * Get the authentication request client assertion lifetime in milliseconds.
     *
     * @return the authentication request client assertion lifetime, in milliseconds.
     */
    public Long getAuthnRequestClientAssertionLifetimeMS() {
        return this.authnRequestClientAssertionLifetimeMS;
    }

    /**
     * Get the client secret.
     *
     * @return the client secret.
     */
    public String getClientSecret() {
        return this.clientSecret;
    }

    /**
     * Get the client authorities.
     *
     * @return the client authorities.
     */
    public List<String> getAuthorities() {
        return this.authorities;
    }

    /**
     * Get the list of resources that the client can access.
     *
     * @return the list of resources that the client can access.
     */
    public List<String> getResourceIds() {
        return this.resourceIds;
    }

    /**
     * Get the scopes of the client.
     *
     * @return the list of scopes the client is granted.
     */
    public List<String> getScopes() {
        return this.scopes;
    }

    /**
     * Get the list of scopes for which the client does not require user approval.
     *
     * @return the list of scopes for which the client does not require user approval.
     */
    public List<String> getAutoApproveScopes() {
        return this.autoApproveScopes;
    }

    /**
     * Get the list of grant types for which this client is authorized.
     *
     * @return the list of grant types for which this client is authorized.
     */
    public List<String> getAuthorizedGrantTypes() {
        return this.authorizedGrantTypes;
    }

    /**
     * Get additional information not required for vanilla OAuth (JSON blob).
     *
     * @return the additional information not required for vanilla OAuth.
     */
    public Map<String, Object> getAdditionalInformation() {
        return this.additionalInformation;
    }

    /**
     * Creates an instance of the {@link OIDCClientMetadataDTO.Builder} class.
     *
     * @return a new {@code OIDCClientMetadataDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
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
        private List<String> redirectUris;
        private String tokenEndpointAuthMethod;
        private List<String> postLogoutRedirectUris;
        private String logoutUri;
        private String certSubjectDN;
        private Long authnRequestClientAssertionLifetimeMS;
        private String clientSecret;
        private List<String> authorities;
        private List<String> resourceIds;
        private List<String> scopes;
        private List<String> autoApproveScopes;
        private List<String> authorizedGrantTypes;
        private Map<String, Object> additionalInformation;

        public Builder withRedirectUris(List<String> redirectUris) {
            this.redirectUris = redirectUris;
            return this;
        }

        public Builder withTokenEndpointAuthMethod(String tokenEndpointAuthMethod) {
            this.tokenEndpointAuthMethod = tokenEndpointAuthMethod;
            return this;
        }

        public Builder withPostLogoutRedirectUris(List<String> postLogoutRedirectUris) {
            this.postLogoutRedirectUris = postLogoutRedirectUris;
            return this;
        }

        public Builder withLogoutUri(String logoutUri) {
            this.logoutUri = logoutUri;
            return this;
        }

        public Builder withCertSubjectDN(String certSubjectDN) {
            this.certSubjectDN = certSubjectDN;
            return this;
        }

        public Builder withAuthnRequestClientAssertionLifetimeMS(Long authnRequestClientAssertionLifetimeMS) {
            this.authnRequestClientAssertionLifetimeMS = authnRequestClientAssertionLifetimeMS;
            return this;
        }

        public Builder withClientSecret(String clientSecret) {
            this.clientSecret = clientSecret;
            return this;
        }

        public Builder withAuthorities(List<String> authorities) {
            this.authorities = authorities;
            return this;
        }

        public Builder withResourceIds(List<String> resourceIds) {
            this.resourceIds = resourceIds;
            return this;
        }

        public Builder withScopes(List<String> scopes) {
            this.scopes = scopes;
            return this;
        }

        public Builder withAutoApproveScopes(List<String> autoApproveScopes) {
            this.autoApproveScopes = autoApproveScopes;
            return this;
        }

        public Builder withAuthorizedGrantTypes(List<String> authorizedGrantTypes) {
            this.authorizedGrantTypes = authorizedGrantTypes;
            return this;
        }

        public Builder withAdditionalInformation(Map<String, Object> additionalInformation) {
            this.additionalInformation = additionalInformation;
            return this;
        }

        public OIDCClientMetadataDTO build() {
            return new OIDCClientMetadataDTO(
                    redirectUris,
                    tokenEndpointAuthMethod,
                    postLogoutRedirectUris,
                    logoutUri,
                    certSubjectDN,
                    authnRequestClientAssertionLifetimeMS,
                    clientSecret,
                    authorities,
                    resourceIds,
                    scopes,
                    autoApproveScopes,
                    authorizedGrantTypes,
                    additionalInformation);
        }
    }

}
