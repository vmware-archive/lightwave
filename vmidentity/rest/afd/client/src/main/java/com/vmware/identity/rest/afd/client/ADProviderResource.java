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

import static com.vmware.identity.rest.core.client.RequestExecutor.execute;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpPost;

import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinInfoDTO;
import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinRequestDTO;
import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;
import com.vmware.identity.rest.core.data.CredentialsDTO;

/**
 * The {@code ADProviderResource} is effectively a container that gathers all of the
 * commands related to Active Directory. These commands are all gathered under the
 * {@value #AD_PROVIDER_URI_STRING} endpoint on the REST server.
 */
public class ADProviderResource extends ClientResource {

    private static final String AD_PROVIDER_URI_STRING = "/afd/provider/ad";
    private static final String AD_PROVIDER_POST_URI_STRING = "/afd/post/provider/ad";

    public ADProviderResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Request that the IDM server join to an Active Directory domain.
     *
     * <p><b>Required Role:</b> {@code administrator}
     *
     * @param joinRequest the join request.
     * @return the join status after performing the request.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ActiveDirectoryJoinInfoDTO join(ActiveDirectoryJoinRequestDTO joinRequest) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), AD_PROVIDER_URI_STRING);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), joinRequest);
        return execute(parent.getClient(), post, ActiveDirectoryJoinInfoDTO.class);
    }

    /**
     * Request that the IDM server leave an Active Directory domain.
     *
     * <p><b>Required Role:</b> {@code administrator}
     *
     * @param credentials the credentials necessary for performing the leave.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void leave(CredentialsDTO credentials) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), AD_PROVIDER_URI_STRING);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken(), credentials);
        execute(parent.getClient(), delete);
    }

    /**
     * Request the IDM server's current Active Directory join status.
     *
     * <p><b>Required Role:</b> {@code user}
     *
     * @return the current join status of the server.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ActiveDirectoryJoinInfoDTO getStatus() throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), AD_PROVIDER_POST_URI_STRING);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, ActiveDirectoryJoinInfoDTO.class);
    }

}
