/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.io.Serializable;

public class OidcConfig implements Serializable {

    private String issuerType;
    private String clientId;
    private String clientSecret;
    private String metadataURI;
    private String jwksURI;
    private String tokenRedirectURI;
    private String authorizeRedirectURI;
    private String logoutURI;
    private String postLogoutURI;
    private String redirectURI;

    /**
     * The Issuer Type is one of the recognized identifiers for recognized 3rd party IDPs
     * @return the Issuer Type
     */
    public String getIssuerType() {
        return issuerType;
    }

    /**
     * Sets the Issuer Type
     * @param issuerType Issuer Type for recognized 3rd party IDP
     * @return the current object
     */
    public OidcConfig setIssuerType(String issuerType) {
        if (issuerType != null) {
            ValidateUtil.validateNotEmpty(issuerType, "Issuer Type");
        }
        this.issuerType = issuerType;
        return this;
    }

    /**
     * Get the Client Id for the current system configured with the 3rd party IDP
     * @return Client Id
     */
    public String getClientId() {
        return clientId;
    }

    /**
     * Set the Client Id for the current system configured with the 3rd party IDP
     * @param clientId The Client Id
     * @return the current object
     */
    public OidcConfig setClientId(String clientId) {
        if (clientId != null) {
          ValidateUtil.validateNotEmpty(clientId, "Client ID");
        }
        this.clientId = clientId;
        return this;
    }

    /**
     * The Client Secret associated with the Client Id in the 3rd party IDP
     * @return The Client Secret
     */
    public String getClientSecret() {
        return clientSecret;
    }

    /**
     * Set the Client Secret associated with the Client Id in the 3rd party IDP
     * @param clientSecret The Client Secret
     * @return the current object
     */
    public OidcConfig setClientSecret(String clientSecret) {
        if (clientSecret != null) {
          ValidateUtil.validateNotEmpty(clientSecret, "Client Secret");
        }
        this.clientSecret = clientSecret;
        return this;
    }

    /**
     * Set the Metadata URI for the 3rd party IDP
     * @return the current object
     */
    public String getMetadataURI() {
        return metadataURI;
    }

    /**
     * Get the Metadata URI for the 3rd party IDP
     * @param metadataURI Metadata URI to be configured
     * @return the current object
     */
    public OidcConfig setMetadataURI(String metadataURI) {
        if (metadataURI != null) {
          ValidateUtil.validateURI(metadataURI, "Metadata URI");
        }
        this.metadataURI = metadataURI;
        return this;
    }

    /**
     * Get the Token Redirect URI corresponding to the Token Endpoint in the 3rd party IDP
     * @return the Token Redirect URI
     */
    public String getTokenRedirectURI() {
        return tokenRedirectURI;
    }

    /**
     * Set the Token Redirect URI corresponding to the Token Endpoint in the 3rd party IDP
     * @param tokenRedirectURI The Token Redirect URI to be configured
     * @return the current object
     */
    public OidcConfig setTokenRedirectURI(String tokenRedirectURI) {
        if (tokenRedirectURI != null) {
          ValidateUtil.validateURI(tokenRedirectURI, "Token Redirect URI");
        }
        this.tokenRedirectURI = tokenRedirectURI;
        return this;
    }

    /**
     * Get the JWKS Endpoint URI for the 3rd party IDP. This provides the public key that the 3rd party IDP uses to
     * sign its OAUTH tokens.
     * @return The JWKS URI
     */
    public String getJwksURI() {
        return jwksURI;
    }

    /**
     * Set the JWKS Endpoint
     * @param jwksURI The JWKS URI to be configured
     * @return the current object
     */
    public OidcConfig setJwksURI(String jwksURI) {
        if (jwksURI != null) {
          ValidateUtil.validateURI(jwksURI, "JWKS URI");
        }
        this.jwksURI = jwksURI;
        return this;
    }

    /**
     * Get the Authorization Redirect URI for the 3rd party IDP. This corresponds to the Login Endpoint.
     * @return The Authorization Redirect URI
     */
    public String getAuthorizeRedirectURI() {
        return authorizeRedirectURI;
    }

    /**
     * Set the Authorization Redirect URI for the 3rd party IDP. This corresponds to the Login Endpoint.
     * @param authorizeRedirectURI Authorization Redirect URI
     * @return the current object
     */
    public OidcConfig setAuthorizeRedirectURI(String authorizeRedirectURI) {
        if (authorizeRedirectURI != null) {
          ValidateUtil.validateURI(authorizeRedirectURI, "Authorization Redirect URI");
        }
        this.authorizeRedirectURI = authorizeRedirectURI;
        return this;
    }

    /**
     * Get the Logout URI corresponding to the 3rd party IDP
     * @return the Logout URI
     */
    public String getLogoutURI() {
        return logoutURI;
    }

    /**
     * Set the Logout URI corresponding to the 3rd party IDP
     * @param logoutURI The Logout URI
     * @return the current object
     */
    public OidcConfig setLogoutURI(String logoutURI) {
        if (logoutURI != null) {
          ValidateUtil.validateURI(logoutURI, "Logout URI");
        }
        this.logoutURI = logoutURI;
        return this;
    }

    /**
     * Get the Post Logout URI corresponding to the 3rd party IDP
     * @return the Post Logout URI
     */
    public String getPostLogoutURI() {
        return postLogoutURI;
    }

    /**
     * Set the post logout URI corresponding to the 3rd party IDP
     * @param postLogoutURI The Post Logout URI to be configured
     * @return the current object
     */
    public OidcConfig setPostLogoutURI(String postLogoutURI) {
        if (postLogoutURI != null) {
          ValidateUtil.validateURI(postLogoutURI, "Post Logout URI");
        }
        this.postLogoutURI = postLogoutURI;
        return this;
    }

    /**
     * Get the Client Redirect URI configured with the Client Id
     * @return The Redirect URI
     */
    public String getRedirectURI() {
        return redirectURI;
    }

    /**
     * Set the Redirect URI for the current system configured with the Client Id
     * @param redirectURI The Redirect URI
     * @return the current object
     */
    public OidcConfig setRedirectURI(String redirectURI) {
        if (redirectURI != null) {
          ValidateUtil.validateURI(redirectURI, "Redirect URI");
        }
        this.redirectURI = redirectURI;
        return this;
    }
}
