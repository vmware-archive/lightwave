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

import static com.vmware.identity.rest.core.client.RequestExecutor.executeAndReturnList;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpGet;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.data.CertificateDTO;

/**
 * The {@code VecsResource} is effectively a container that gathers all of the
 * commands related to VECS. These commands are all gathered under the
 * {@value #VECS_URI_STRING} endpoint on the REST server.
 */
public class VecsResource extends ClientResource {

    private static final String VECS_URI_STRING = "/afd/vecs";
    private static final String VECS_SSL_URI_STRING = VECS_URI_STRING + "/ssl";

    public VecsResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Request the SSL certificates used by the remote server. Note that this call
     * is something of a catch-22 as the SSL certificates are technically needed for
     * the trust manager before performing the call in the first place.
     *
     * <p><b>Required Role:</b> {@code anonymous}
     *
     * @return a list of certificates used for SSL by the remote server.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was an error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<CertificateDTO> getSSLCertificates() throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), VECS_SSL_URI_STRING);

        HttpGet get = RequestFactory.createGetRequest(uri);
        return executeAndReturnList(parent.getClient(), get, CertificateDTO.class);
    }

}
