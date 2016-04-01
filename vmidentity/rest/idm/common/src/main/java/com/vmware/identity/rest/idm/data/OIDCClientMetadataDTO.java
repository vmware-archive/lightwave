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
            Long authnRequestClientAssertionLifetimeMS) {
        this.redirectUris = redirectUris;
        this.tokenEndpointAuthMethod = tokenEndpointAuthMethod;
        this.postLogoutRedirectUris = postLogoutRedirectUris;
        this.logoutUri = logoutUri;
        this.certSubjectDN = certSubjectDN;
        this.authnRequestClientAssertionLifetimeMS = authnRequestClientAssertionLifetimeMS;
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

    public Long getAuthnRequestClientAssertionLifetimeMS() {
        return this.authnRequestClientAssertionLifetimeMS;
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

        public OIDCClientMetadataDTO build() {
            return new OIDCClientMetadataDTO(
                    redirectUris,
                    tokenEndpointAuthMethod,
                    postLogoutRedirectUris,
                    logoutUri,
                    certSubjectDN,
                    authnRequestClientAssertionLifetimeMS);
        }
    }

}
