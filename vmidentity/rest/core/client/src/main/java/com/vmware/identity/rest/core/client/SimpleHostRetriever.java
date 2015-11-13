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

import org.apache.http.client.utils.URIBuilder;

/**
 * The {@code SimpleHostRetriever} takes a single hostname and uses that
 * while constructing the request URIs.
 *
 * @see HostRetriever
 */
public class SimpleHostRetriever extends HostRetriever {

    private final String host;

    /**
     * Construct a {@code SimpleHostRetriever} with a hostname. Connections
     * will be assumed to not be over HTTPS and use the default port.
     *
     * @param host the name of the host.
     */
    public SimpleHostRetriever(String host) {
        this(host, NO_PORT, false);
    }

    /**
     * Construct a {@code SimpleHostRetriever} with a hostname and port. Connections
     * will be assumed to not be over HTTPS.
     *
     * @param host the name of the host.
     * @param port the port for the host.
     */
    public SimpleHostRetriever(String host, int port) {
        this(host, port, false);
    }

    /**
     * Construct a {@code SimpleHostRetriever} with a hostname and security setting.
     * Connections will use the default port.
     *
     * @param host the name of the host.
     * @param secure whether to perform the request over HTTPS or not.
     */
    public SimpleHostRetriever(String host, boolean secure) {
        this(host, NO_PORT, secure);
    }

    /**
     * Construct a {@code SimpleHostRetriever} with a hostname, port, and security setting.
     *
     * @param host the name of the host.
     * @param port the port for the host.
     * @param secure whether to perform the request over HTTPS or not.
     */
    public SimpleHostRetriever(String host, int port, boolean secure) {
        super(port, secure);
        this.host = host;
    }

    /**
     * Get the hostname used by this retriever.
     *
     * @return the hostname.
     */
    public String getHost() {
        return host;
    }

    @Override
    public URIBuilder getURIBuilder() {
        URIBuilder builder = new URIBuilder()
            .setScheme(getScheme())
            .setHost(host);

        if (hasPort()) {
            builder.setPort(port);
        }

        return builder;
    }

}
