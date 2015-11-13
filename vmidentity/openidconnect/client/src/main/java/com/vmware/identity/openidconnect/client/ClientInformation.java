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

import java.io.Serializable;
import java.net.URI;
import java.util.Set;

import org.apache.commons.lang3.Validate;

/**
 * Client information. This contains meta data from client registration.
 *
 * @author Jun Sun
 */
public class ClientInformation implements Serializable {

    private static final long serialVersionUID = 1L;

    private final ClientID clientId;
    private final Set<URI> redirectUris;
    private final ClientAuthenticationMethod tokenEndpointAuthMethod;
    private final Set<URI> postLogoutRedirectUris;
    private final URI logoutUri;
    private final String certSubjectDN;

    ClientInformation(
            ClientID clientId,
            Set<URI> redirectUris,
            ClientAuthenticationMethod tokenEndpointAuthMethod,
            Set<URI> postLogoutRedirectUris,
            URI logoutUri,
            String certSubjectDN) {
        Validate.notNull(clientId, "clientId");
        Validate.notEmpty(redirectUris, "redirectUris");
        Validate.notNull(tokenEndpointAuthMethod, "tokenEndpointAuthMethod");
        if (ClientAuthenticationMethod.PRIVATE_KEY_JWT.equals(tokenEndpointAuthMethod)) {
            Validate.notEmpty(certSubjectDN, "certSubjectDN");
        }

        this.clientId = clientId;
        this.redirectUris = redirectUris;
        this.tokenEndpointAuthMethod = tokenEndpointAuthMethod;
        this.postLogoutRedirectUris = postLogoutRedirectUris;
        this.logoutUri = logoutUri;
        this.certSubjectDN = certSubjectDN;
    }

    /**
     * Get client id
     *
     * @return                                  Client id
     */
    public ClientID getClientId() {
        return this.clientId;
    }

    /**
     * Get redirect URIs
     *
     * @return                                  Redirect URIs
     */
    public Set<URI> getRedirectUris() {
        return this.redirectUris;
    }

    /**
     * Get client authentication method
     *
     * @return                                  Client authentication method
     */
    public ClientAuthenticationMethod getTokenEndpointAuthMethod() {
        return this.tokenEndpointAuthMethod;
    }

    /**
     * Get post logout redirect URIs
     *
     * @return                                  Post logout redirect URIs
     */
    public Set<URI> getPostLogoutRedirectUris() {
        return this.postLogoutRedirectUris;
    }

    /**
     * Get logout URI
     *
     * @return                                  Logout URI
     */
    public URI getLogoutUri() {
        return this.logoutUri;
    }

    /**
     * Get certificate subject DN
     *
     * @return                                  Certificate subject DN
     */
    public String getCertSubjectDN() {
        return this.certSubjectDN;
    }
}
