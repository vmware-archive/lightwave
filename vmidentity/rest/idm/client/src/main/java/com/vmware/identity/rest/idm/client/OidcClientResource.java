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

import static com.vmware.identity.rest.core.client.RequestExecutor.execute;
import static com.vmware.identity.rest.core.client.RequestExecutor.executeAndReturnList;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;

/**
 * The {@code OidcClientResource} is effectively a container that gathers all of
 * the commands related to the Open ID Connect (OIDC) clients for a tenant. These commands
 * are all gathered under the {@value #OIDC_CLIENT_URI} endpoint on the REST server.
 */
public class OidcClientResource extends ClientResource {

    private static final String OIDC_CLIENT_URI = "/idm/tenant/%s/oidcclient";
    private static final String OIDC_CLIENT_ID_URI = OIDC_CLIENT_URI + "/%s";

    private static final String OIDC_CLIENT_POST_URI = "/idm/post/tenant/%s/oidcclient";
    private static final String OIDC_CLIENT_ID_POST_URI = OIDC_CLIENT_POST_URI + "/%s";

    public OidcClientResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Register a new Open ID Connect client.
     *
     * <p><b>Required Role:</b> {@code TrustedUser}.
     *
     * @param tenant the name of the tenant to register the client with.
     * @param metadata the metadata necessary for registering the client.
     * @return the newly registered client.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public OIDCClientDTO register(String tenant, OIDCClientMetadataDTO metadata) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), OIDC_CLIENT_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), metadata);
        return execute(parent.getClient(), post, OIDCClientDTO.class);
    }

    /**
     * Request the list of Open ID Connect clients associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code TrustedUser}.
     *
     * @param tenant the name of the tenant to request the clients from.
     * @return a list of OIDC clients.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<OIDCClientDTO> getAll(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), OIDC_CLIENT_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, OIDCClientDTO.class);
    }

    /**
     * Request a specific Open ID Connect client.
     *
     * <p><b>Required Role:</b> {@code TrustedUser}.
     *
     * @param tenant the name of the tenant to request the client from.
     * @param clientId the identifier of the client to request.
     * @return the requested client.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public OIDCClientDTO get(String tenant, String clientId) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), OIDC_CLIENT_ID_POST_URI, tenant, clientId);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, OIDCClientDTO.class);
    }

    /**
     * Update an Open ID Connect client.
     *
     * <p><b>Required Role:</b> {@code TrustedUser}.
     *
     * @param tenant the name of the tenant containing the client.
     * @param clientId the identifier of the client being updated.
     * @param metadata the metadata contents used to update the client.
     * @return the newly updated client.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public OIDCClientDTO update(String tenant, String clientId, OIDCClientMetadataDTO metadata) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), OIDC_CLIENT_ID_URI, tenant, clientId);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), metadata);
        return execute(parent.getClient(), put, OIDCClientDTO.class);
    }

    /**
     * Delete an Open ID Connect client.
     *
     * <p><b>Required Role:</b> {@code TrustedUser}.
     *
     * @param tenant the name of the tenant containing the client.
     * @param clientId the identifier of the client being deleted.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String clientId) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), OIDC_CLIENT_ID_URI, tenant, clientId);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

}
