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
package com.vmware.identity.rest.afd.client;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLContext;

import org.apache.http.impl.client.CloseableHttpClient;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.HostRetriever;

/**
 * The {@code AfdClient} class exposes all of the basic functionality of the RESTful AFD server.
 * The client is broken up into several subcomponents of related functionality.
 *
 * For example, to set up a client with a simple host over HTTPS, a no-op hostname verifier,
 * and a trust strategy that always trusts the server's certificates:
 * <p><blockquote><pre>
 *      HostRetriever host = new SimpleHostRetriever("10.0.0.1", true);
 *      AfdClient client = new AfdClient(host,
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
public class AfdClient extends BaseClient {

    private ADProviderResource adProvider;
    private VecsResource vecs;

    /**
     * @see BaseClient#BaseClient(String)
     */
    public AfdClient(String host) {
        super(host);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, int)
     */
    public AfdClient(String host, int port) {
        super(host, port);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, boolean)
     */
    public AfdClient(String host, boolean secure) {
        super(host, secure);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, int, boolean)
     */
    public AfdClient(String host, int port, boolean secure) {
        super(host, port, secure);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, HostnameVerifier, SSLContext)
     */
    public AfdClient(String host, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, verifier, sslContext);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(String, int, HostnameVerifier, SSLContext)
     */
    public AfdClient(String host, int port, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, port, verifier, sslContext);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(HostRetriever)
     */
    public AfdClient(HostRetriever host) {
        super(host);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(HostRetriever, CloseableHttpClient)
     */
    public AfdClient(HostRetriever host, CloseableHttpClient client) {
        super(host, client);
        initResources();
    }

    /**
     * @see BaseClient#BaseClient(HostRetriever, HostnameVerifier, SSLContext)
     */
    public AfdClient(HostRetriever host, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, verifier, sslContext);
        initResources();
    }

    /**
     * Get the subresource containing all of the commands related to Active Directory.
     *
     * @return the subresource containing all of the Active Directory commands.
     */
    public ADProviderResource activeDirectory() {
        return adProvider;
    }

    /**
     * Get the subresource containing all of the commands related to VECS.
     *
     * @return the subresource containing all of the VECS commands.
     */
    public VecsResource vecs() {
        return vecs;
    }

    private void initResources() {
        adProvider = new ADProviderResource(this);
        vecs = new VecsResource(this);
    }

}
