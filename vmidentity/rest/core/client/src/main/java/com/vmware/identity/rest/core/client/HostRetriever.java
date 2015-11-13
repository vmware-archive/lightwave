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

import com.vmware.identity.rest.core.client.exceptions.ClientException;

/**
 * This abstract class describes a set of operations required for retrieving
 * the URI that will be used during all HTTP requests.
 */
public abstract class HostRetriever {

    public static final int NO_PORT = -1;
    public static final int DEFAULT_HTTP_PORT = 80;
    public static final int DEFAULT_HTTPS_PORT = 443;

    public static final String HTTP_SCHEME = "http";
    public static final String HTTPS_SCHEME = "https";

    protected final boolean secure;
    protected final int port;

    /**
     * Construct an {@code HostRetriever} with a port and whether
     * the connection should be secure or not.
     *
     * @param port
     * @param secure
     */
    protected HostRetriever(int port, boolean secure) {
        this.secure = secure;
        this.port = port;
    }

    /**
     * Get a {@link URIBuilder} with the {@code scheme}, {@code host}, and optionally,
     * {@code port} already configured.
     *
     * @return a {@link URIBuilder} with the endpoint already preloaded.
     * @throws ClientException if an error occurred while building the URI.
     */
    public abstract URIBuilder getURIBuilder() throws ClientException;

    /**
     * Check if the URI will be built using HTTPS or not.
     *
     * @return {@code true} if the URI will use HTTPS or {@code false} otherwise.
     */
    public boolean isSecure() {
        return secure;
    }

    /**
     * Get the scheme used for building the URI, based on whether the connection
     * should be secure or not.
     *
     * @return the scheme used for building the URI.
     */
    public String getScheme() {
        return secure ? HTTPS_SCHEME : HTTP_SCHEME;
    }

    /**
     * Get the port associated with the URI.
     *
     * @return the port number or {@value NO_PORT} if there is no port associated
     * with the retriever.
     */
    public int getPort() {
        return port;
    }

    /**
     * Check if the retriever has a port associated with it or not.
     *
     * @return {@code true} if there is a port associated with the retriever,
     * {@code false} otherwise.
     */
    public boolean hasPort() {
        return port != NO_PORT;
    }

}
