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
package com.vmware.identity.rest.idm.client;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLContext;

import org.apache.http.impl.client.CloseableHttpClient;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.HostRetriever;

/**
 * The {@code AfdClient} class exposes all of the basic functionality of the RESTful IDM server.
 * The client is broken up into several subcomponents of related functionality.
 *
 * For example, to set up a client with a simple host over HTTPS, a no-op hostname verifier,
 * and a trust strategy that always trusts the server's certificates:
 * <p><blockquote><pre>
 *      HostRetriever host = new SimpleHostRetriever("10.0.0.1", true);
 *      IdmClient client = new IdmClient(host,
 *          NoopHostnameVerifier.INSTANCE,
 *          new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {
 *
 *              {@literal @}Override
 *              public boolean isTrusted(X509Certificate[] chain, String authType)
 *                      throws CertificateException {
 *                  return true;
 *              }
 *          }).build());
 * </pre></blockquote>
 */
public class IdmClient extends BaseClient {

    private ServerResource server;
    private TenantResource tenant;
    private CertificateResource certificate;
    private ExternalIDPResource externalIdp;
    private GroupResource group;
    private IdentityProviderResource providers;
    private OidcClientResource oidcClient;
    private ResourceServerResource resourceServer;
    private RelyingPartyResource relyingParty;
    private SolutionUserResource solutionUser;
    private UserResource user;
    private DiagnosticsResource diagnostics;

    /**
     * @see BaseClient#BaseClient(String)
     */
    public IdmClient(String host) {
        super(host);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, int)
     */
    public IdmClient(String host, int port) {
        super(host, port);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, boolean)
     */
    public IdmClient(String host, boolean secure) {
        super(host, secure);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, int, boolean)
     */
    public IdmClient(String host, int port, boolean secure) {
        super(host, port, secure);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, HostnameVerifier, SSLContext)
     */
    public IdmClient(String host, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, verifier, sslContext);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, int, HostnameVerifier, SSLContext)
     */
    public IdmClient(String host, int port, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, port, verifier, sslContext);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(HostRetriever)
     */
    public IdmClient(HostRetriever host) {
        super(host);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(HostRetriever, CloseableHttpClient)
     */
    public IdmClient(HostRetriever host, CloseableHttpClient client) {
        super(host, client);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(HostRetriever, HostnameVerifier, SSLContext)
     */
    public IdmClient(HostRetriever host, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, verifier, sslContext);
        initResources();
    }

    /**
     * Get the subresource containing all of the commands related to a tenant
     *
     * @return the subresource containing all of the tenant commands.
     */
    public TenantResource tenant() {
        return tenant;
    }

    /**
     * Get the subresource containing all of the commands related to the server.
     *
     * @return the subresource containing all of the server commands.
     */
    public ServerResource server() {
        return server;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * certificates.
     *
     * @return the subresource containing all of the certificate commands.
     */
    public CertificateResource certificate() {
        return certificate;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * groups.
     *
     * @return the subresource containing all of the group commands.
     */
    public GroupResource group() {
        return group;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * external identity providers.
     *
     * @return the subresource containing all of the external identity provider commands.
     */
    public ExternalIDPResource externalIdp() {
        return externalIdp;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * identity providers.
     *
     * @return the subresource containing all of the identity provider commands.
     */
    public IdentityProviderResource provider() {
        return providers;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * OpenID Connect (OIDC) clients.
     *
     * @return the subresource containing all of the OIDC commands.
     */
    public OidcClientResource oidcClient() {
        return oidcClient;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's resource servers.
     *
     * @return the subresource containing all of the resource servers commands.
     */
    public ResourceServerResource resourceServer() {
        return resourceServer;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * Secure Token Service (STS) relying parties.
     *
     * @return the subresource containing all of the relying party commands.
     */
    public RelyingPartyResource relyingParty() {
        return relyingParty;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * solution users.
     *
     * @return the subresource containing all of the solution user commands.
     */
    public SolutionUserResource solutionUser() {
        return solutionUser;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * users.
     *
     * @return the subresource containing all of the user commands.
     */
    public UserResource user() {
        return user;
    }

    /**
     * Get the subresource containing all of the commands related to a tenant's
     * diagnostics.
     *
     * @return the subresource containing all of the diagnostics commands.
     */
    public DiagnosticsResource diagnostics() {
        return diagnostics;
    }

    private void initResources() {
        server = new ServerResource(this);
        tenant = new TenantResource(this);
        certificate = new CertificateResource(this);
        externalIdp = new ExternalIDPResource(this);
        group = new GroupResource(this);
        providers = new IdentityProviderResource(this);
        oidcClient = new OidcClientResource(this);
        resourceServer = new ResourceServerResource(this);
        relyingParty = new RelyingPartyResource(this);
        solutionUser = new SolutionUserResource(this);
        user = new UserResource(this);
        diagnostics = new DiagnosticsResource(this);
    }

}
