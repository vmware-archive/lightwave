/*
 *
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
 *
 */

package com.vmware.identity.idm;

import java.io.Serializable;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.List;
import java.util.UUID;

//The semantic of OIDC client fields can be found in the following links:
//OAUTH: https://tools.ietf.org/html/rfc6749#section-2
//OIDC: http://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata
public class OIDCClient implements Serializable {

    private static final long serialVersionUID = 1L;

    // Client information fields
    private final String clientId;

    // Client meta data
    private final List<String> redirectUris;
    private final String tokenEndpointAuthMethod;
    private final String tokenEndpointAuthSigningAlg;
    private final String idTokenSignedResponseAlg;
    private final List<String> postLogoutRedirectUris;
    private final String logoutUri;
    private final String certSubjectDN;
    private final long authnRequestClientAssertionLifetimeMS;

    private OIDCClient(Builder oidcClientBuilder) {

        // Client information fields
        this.clientId = oidcClientBuilder.clientId;

        // Client meta data
        this.redirectUris = oidcClientBuilder.redirectUris;
        this.tokenEndpointAuthMethod = oidcClientBuilder.tokenEndpointAuthMethod;
        this.tokenEndpointAuthSigningAlg = oidcClientBuilder.tokenEndpointAuthSigningAlg;
        this.idTokenSignedResponseAlg = oidcClientBuilder.idTokenSignedResponseAlg;
        this.postLogoutRedirectUris = oidcClientBuilder.postLogoutRedirectUris;
        this.logoutUri = oidcClientBuilder.logoutUri;
        this.certSubjectDN = oidcClientBuilder.certSubjectDN;
        this.authnRequestClientAssertionLifetimeMS = oidcClientBuilder.authnRequestClientAssertionLifetimeMS;
    }

    public String getClientId() {
        return this.clientId;
    }

    public List<String> getRedirectUris() {
        return this.redirectUris;
    }

    public String getTokenEndpointAuthMethod() {
        return this.tokenEndpointAuthMethod;
    }

    public String getTokenEndpointAuthSigningAlg() {
        return this.tokenEndpointAuthSigningAlg;
    }

    public String getIdTokenSignedResponseAlg() {
        return this.idTokenSignedResponseAlg;
    }

    public List<String> getPostLogoutRedirectUris() {
        return this.postLogoutRedirectUris;
    }

    public String getLogoutUri() {
        return this.logoutUri;
    }

    public String getCertSubjectDN() {
        return this.certSubjectDN;
    }

    public long getAuthnRequestClientAssertionLifetimeMS() {
        return this.authnRequestClientAssertionLifetimeMS;
    }

    // The builder class with additional data validation and defaults setting
    public static class Builder {

        // Client information fields
        private final String clientId;

        // Client meta data
        private List<String> redirectUris;
        private String tokenEndpointAuthMethod;
        private String tokenEndpointAuthSigningAlg;
        private String idTokenSignedResponseAlg;
        private List<String> postLogoutRedirectUris;
        private String logoutUri;
        private String certSubjectDN;
        private long authnRequestClientAssertionLifetimeMS;

        public Builder(String clientId) {
            // create client id as an UUID string if it is null.
            if (clientId == null) {
                this.clientId = UUID.randomUUID().toString();
            } else {
                this.clientId = clientId;
            }
        }

        public Builder redirectUris(List<String> redirectUris) {
            this.redirectUris = redirectUris;
            return this;
        }

        public Builder tokenEndpointAuthMethod(String tokenEndpointAuthMethod) {
            this.tokenEndpointAuthMethod = tokenEndpointAuthMethod;
            return this;
        }

        public Builder tokenEndpointAuthSigningAlg(String tokenEndpointAuthSigningAlg) {
            this.tokenEndpointAuthSigningAlg = tokenEndpointAuthSigningAlg;
            return this;
        }

        public Builder idTokenSignedResponseAlg(String idTokenSignedResponseAlg) {
            this.idTokenSignedResponseAlg = idTokenSignedResponseAlg;
            return this;
        }

        public Builder postLogoutRedirectUris(List<String> postLogoutRedirectUris) {
            this.postLogoutRedirectUris = postLogoutRedirectUris;
            return this;
        }

        public Builder logoutUri(String logoutUri) {
            this.logoutUri = logoutUri;
            return this;
        }

        public Builder certSubjectDN(String certSubjectDN) {
            this.certSubjectDN = certSubjectDN;
            return this;
        }

        public Builder authnRequestClientAssertionLifetimeMS(long authnRequestClientAssertionLifetimeMS) {
            this.authnRequestClientAssertionLifetimeMS = authnRequestClientAssertionLifetimeMS;
            return this;
        }

        public OIDCClient build() {

            // Validate, set defaults and check data.
            validateAndSetDefaults();
            checkClientRestriction();

            return new OIDCClient(this);
        }

        // Validate and set OIDC meta data defaults
        // Refer OIDC SDK code for string representation of each fields
        private void validateAndSetDefaults() {
            if (this.redirectUris == null || this.redirectUris.size() == 0) {
                throw new IllegalArgumentException("Invalid client metadata: "
                        + "Redirect URI is required.");
            } else {
                for (String uri : this.redirectUris) {
                    if (!isValidUri(uri)) {
                        throw new IllegalArgumentException("Invalid redirect URI: " + uri);
                    }
                }
            }

            if (this.postLogoutRedirectUris != null) {
                for (String uri : this.postLogoutRedirectUris) {
                    if (!isValidUri(uri)) {
                        throw new IllegalArgumentException("Invalid post logout redirect URI: " + uri);
                    }
                }
            }

            if (this.logoutUri != null) {
                if (!isValidUri(this.logoutUri)) {
                    throw new IllegalArgumentException("Invalid logout URI: " + this.logoutUri);
                }
            }

            if (this.authnRequestClientAssertionLifetimeMS < 0) {
                throw new IllegalArgumentException("negative authnRequestClientAssertionLifetimeMS: " + this.authnRequestClientAssertionLifetimeMS);
            }

            if (this.idTokenSignedResponseAlg == null) {
                this.idTokenSignedResponseAlg = "RS256";
            }

            if (this.tokenEndpointAuthMethod == null) {
                this.tokenEndpointAuthMethod = "none";
            }

            if (this.tokenEndpointAuthSigningAlg == null) {
                this.tokenEndpointAuthSigningAlg = "RS256";
            }
        }

        // Check VMware profile
        private void checkClientRestriction() {
            if (!this.tokenEndpointAuthMethod.equals("none") && !this.tokenEndpointAuthMethod.equals("private_key_jwt")) {
                throw new IllegalArgumentException("Invalid client metadata: "
                        + "Token endpoint authentication method must be either private_key_jwt or none.");
            }

            if (this.tokenEndpointAuthMethod.equals("private_key_jwt") && (this.certSubjectDN == null || this.certSubjectDN.isEmpty())) {
                throw new IllegalArgumentException("Invalid client metadata: "
                        + "Certificate subject distinguished name can not be null or empty if token endpoint authentication method is private_key_jwt.");
            }

            // If using PRIVATE_KEY_JWT, check RS256 is used and only one RS256 key provided.
            if (this.tokenEndpointAuthMethod.equals("private_key_jwt") && !this.tokenEndpointAuthSigningAlg.equals("RS256")) {
                throw new IllegalArgumentException("Invalid client metadata: "
                        + "Token endpoint authentication signing algorithm must be RS256 if token endpoint authentication method is private_key_jwt.");
            }

            if (!this.idTokenSignedResponseAlg.equals("RS256")) {
                throw new IllegalArgumentException("Invalid client metadata: "
                        + "Id token signed response algorithm must be RS256.");
            }
        }

        private boolean isValidUri(String uriString) {
            URI uri;

            try {
                uri = new URI(uriString);
            } catch (URISyntaxException e) {
                return false;
            }

            if (uri.getScheme() == null) {
                return false;
            }

            return true;
        }
    }
}
