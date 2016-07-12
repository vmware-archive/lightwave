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

import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.ProviderMetadata;

/**
 * Server connection configuration
 *
 * @author Jun Sun
 */
public final class ConnectionConfig {
    private final URI authorizationEndpointURI;
    private final URI tokenEndpointURI;
    private final URI endSessionEndpointURI;
    private final Issuer issuer;
    private final RSAPublicKey providerPublicKey;
    private final KeyStore keyStore;

    /**
     * Constructor
     *
     * @param providerMetadata          OIDC server metadata from querying metadata endpoint. It contains server endpoints URI,
     *                                  issuer and other server supported capability configurations.
     * @param providerPublicKey         OIDC server public key. This key is used to validate token signatures.
     */
    public ConnectionConfig(ProviderMetadata providerMetadata, RSAPublicKey providerPublicKey) {
        this(providerMetadata, providerPublicKey, VecsKeyStore.getInstance());
    }

    /**
     * Constructor
     *
     * @param providerMetadata          OIDC server metadata from querying metadata endpoint. It contains server endpoints URI,
     *                                  issuer and other server supported capability configurations.
     * @param providerPublicKey         OIDC server public key. This key is used to validate token signatures.
     * @param keyStore                  Key store which contains server SSL certificate.
     */
    public ConnectionConfig(ProviderMetadata providerMetadata, RSAPublicKey providerPublicKey, KeyStore keyStore) {
        Validate.notNull(providerMetadata, "providerMetadata");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(keyStore, "keyStore");

        this.authorizationEndpointURI = providerMetadata.getAuthorizationEndpointURI();
        this.tokenEndpointURI = providerMetadata.getTokenEndpointURI();
        this.endSessionEndpointURI = providerMetadata.getEndSessionEndpointURI();
        this.issuer = providerMetadata.getIssuer();
        this.providerPublicKey = providerPublicKey;
        this.keyStore = keyStore;
    }

    /**
     * Get authorization endpoint URI
     *
     * @return                          Authorization endpoint URI
     */
    public URI getAuthorizationEndpointURI() {
        return this.authorizationEndpointURI;
    }

    /**
     * Get token endpoint URI
     *
     * @return                          Token endpoint URI
     */
    public URI getTokenEndpointURI() {
        return this.tokenEndpointURI;
    }

    /**
     * Get logout endpoint URI
     *
     * @return                          Logout endpoint URI
     */
    public URI getEndSessionEndpointURI() {
        return this.endSessionEndpointURI;
    }

    /**
     * Get OIDC server issuer
     *
     * @return                          OIDC server issuer
     */
    public Issuer getIssuer() {
        return this.issuer;
    }

    /**
     * Get OIDC server public key
     *
     * @return                          OIDC server public key
     */
    public RSAPublicKey getProviderPublicKey() {
        return this.providerPublicKey;
    }

    /**
     * Get key store which contains OIDC server SSL certificate
     *
     * @return                          Key store which contains OIDC server SSL certificate
     */
    public KeyStore getKeyStore() {
        return this.keyStore;
    }
}
