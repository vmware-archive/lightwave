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
package com.vmware.identity.rest.core.client;

import java.io.Closeable;
import java.io.IOException;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLContext;

import org.apache.http.client.HttpClient;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;

public class BaseClient implements Closeable{

    private final ReadWriteLock tokenLock;
    private AccessToken token;
    private HostRetriever host;
    private CloseableHttpClient client;

    /**
     * Constructs a client with a {@code host}. This will use the default
     * {@link HttpClient} and a {@link SimpleHostRetriever}. The port is assumed to be
     * the default port (-1).
     *
     * @see HttpClients#createDefault()
     * @param host the hostname of the remote REST server.
     */
    protected BaseClient(String host) {
        this(new SimpleHostRetriever(host), HttpClients.createDefault());
    }


    /**
     * Constructs a client with a {@code host} and {@code port}.
     * This will use the default {@link HttpClient} and a {@link SimpleHostRetriever}.
     *
     * @see HttpClients#createDefault()
     * @param host the hostname of the remote REST server.
     * @param port a flag indicating whether the connection is secure or not.
     */
    protected BaseClient(String host, int port) {
        this(new SimpleHostRetriever(host, port), HttpClients.createDefault());
    }

    /**
     * Constructs a client with a {@code host} and {@code secure} flag.
     * This will use the default {@link HttpClient} and a {@link SimpleHostRetriever}.
     * The port is assumed to be the default port (-1).
     *
     * @see HttpClients#createDefault()
     * @param host the hostname of the remote REST server.
     * @param secure a flag indicating whether the connection is secure or not.
     */
    protected BaseClient(String host, boolean secure) {
        this(new SimpleHostRetriever(host, secure), HttpClients.createDefault());
    }

    /**
     * Constructs a client with a {@code host}, {@code port}
     * and {@code secure} flag. This will use the default {@link HttpClient}
     * and a {@link SimpleHostRetriever}.
     *
     * @see HttpClients#createDefault()
     * @param host the hostname of the remote REST server.
     * @param port the port to use when connecting to the remote REST server.
     * @param secure a flag indicating whether the connection is secure or not.
     */
    protected BaseClient(String host, int port, boolean secure) {
        this(new SimpleHostRetriever(host, port, secure), HttpClients.createDefault());
    }

    /**
     * Constructs a client with a variety of parameters. The {@code host} parameter
     * is loaded into a {@link SimpleHostRetriever} using the default port (-1), assuming that the
     * connection will be secure.
     *
     * @param host the hostname of the remote REST server.
     * @param verifier a {@link HostnameVerifier} for use when verifying the SSL
     *  certificate hostname.
     * @param sslContext a {@link SSLContext} that defines the trust relationship
     *  with the SSL certificates.
     */
    protected BaseClient(String host, HostnameVerifier verifier, SSLContext sslContext) {
        this(new SimpleHostRetriever(host, true), verifier, sslContext);
    }

    /**
     * Constructs a client with a variety of parameters. The {@code host} and
     * {@code port} parameters are loaded into a {@link SimpleHostRetriever} with the
     * assumption that the connection will be secure.
     *
     * @param host the hostname of the remote REST server.
     * @param port the port to use when connecting to the remote REST server.
     * @param verifier a {@link HostnameVerifier} for use when verifying the SSL
     *  certificate hostname.
     * @param sslContext a {@link SSLContext} that defines the trust relationship
     *  with the SSL certificates.
     */
    protected BaseClient(String host, int port, HostnameVerifier verifier, SSLContext sslContext) {
        this(new SimpleHostRetriever(host, port, true), verifier, sslContext);
    }

    /**
     * Constructs a client with a variety of parameters. The {@code host} and
     * {@code port} parameters are loaded into a {@link SimpleHostRetriever} with the
     * assumption that the connection will be secure.
     *
     * @param host the hostname of the remote REST server.
     * @param port the port to use when connecting to the remote REST server.
     * @param verifier a {@link HostnameVerifier} for use when verifying the SSL
     *  certificate hostname.
     * @param sslContext a {@link SSLContext} that defines the trust relationship
     *  with the SSL certificates.
     *  @param token access token that authorize service calls
     */
    protected BaseClient(String host, int port, HostnameVerifier verifier, SSLContext sslContext, AccessToken token) {
        this.host = new SimpleHostRetriever(host, port, true);
        this.client = HttpClients.custom()
                .setSSLHostnameVerifier(verifier)
                .setSSLContext(sslContext)
                .build();
        this.token = token;
        this.tokenLock = new ReentrantReadWriteLock();
    }

    /**
     * Constructs a client with a given {@link HostRetriever}.
     *
     * @param host a {@link HostRetriever} that will supply the URLs for the REST server.
     */
    protected BaseClient(HostRetriever host) {
        this(host, HttpClients.createDefault());
    }

    /**
     * Constructs a client with a given {@link HostRetriever} and a
     * {@link CloseableHttpClient}. Using this constructor allows for the greatest
     * degree of customization against the underlying {@link HttpClient}.
     *
     * @param host a {@link HostRetriever} that will supply the URLs for the REST server.
     * @param client a {@link CloseableHttpClient} that will be used to perform all REST calls.
     */
    protected BaseClient(HostRetriever host, CloseableHttpClient client) {
        this.host = host;
        this.client = client;
        this.tokenLock = new ReentrantReadWriteLock();
    }

    /**
     * Constructs a client with a given {@link HostRetriever},
     * {@link HostnameVerifier}, and {@link SSLContext}.
     *
     * @param host a {@link HostRetriever} that will supply the URLs for the REST server.
     * @param verifier a {@link HostnameVerifier} for use when verifying the SSL
     *  certificate hostname.
     * @param sslContext a {@link SSLContext} that defines the trust relationship
     *  with the SSL certificates.
     */
    protected BaseClient(HostRetriever host, HostnameVerifier verifier, SSLContext sslContext) {
        this(host, HttpClients.custom()
                .setSSLHostnameVerifier(verifier)
                .setSSLContext(sslContext)
                .build());
    }

    /**
     * Get the {@link HostRetriever} in use by this client.
     *
     * @return the {@link HostRetriever} used by this client.
     */
    public HostRetriever getHostRetriever() {
        return host;
    }

    /**
     * Get the {@link CloseableHttpClient} in use by this client.
     *
     * @return the {@link CloseableHttpClient} used by this client.
     */
    public CloseableHttpClient getClient() {
        return client;
    }

    /**
     * Set the {@link AccessToken} in use by this client. This access token
     * controls the level of access granted to the caller. These tokens can be
     * obtained through STS (SAML tokens) or OIDC (JWT tokens). For the purposes
     * of the client, the token can be considered a simple string.
     *
     * @param token the token to use for the client.
     */
    public void setToken(AccessToken token) {
        tokenLock.writeLock().lock();
        this.token = token;
        tokenLock.writeLock().unlock();
    }

    /**
     * Get the {@link AccessToken} in use by this client.
     *
     * @return the {@link AccessToken} used by this client.
     */
    public AccessToken getToken() {
        tokenLock.readLock().lock();
        AccessToken token = this.token;
        tokenLock.readLock().unlock();
        return token;
    }


    /**
     * Deallocates all the resources held by http client.
     */
    @Override
    public void close() {
        try {
            if(client != null) {
                client.close();
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to deallocate client held resources");
        }
    }

}
