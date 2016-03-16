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
import com.vmware.identity.rest.idm.data.ResourceServerDTO;

/**
 * The {@code ResourceServerResource} is effectively a container that gathers all of
 * the commands related to the Resource Servers for a tenant. These commands
 * are all gathered under the {@value #RESOURCE_SERVER_URI} endpoint on the REST server.
 *
 * @author Yehia Zayour
 */
public class ResourceServerResource extends ClientResource {

    private static final String RESOURCE_SERVER_URI = "/idm/tenant/%s/resourceserver";
    private static final String RESOURCE_SERVER_NAME_URI = RESOURCE_SERVER_URI + "/%s";

    private static final String RESOURCE_SERVER_POST_URI = "/idm/post/tenant/%s/resourceserver";
    private static final String RESOURCE_SERVER_NAME_POST_URI = RESOURCE_SERVER_POST_URI + "/%s";

    public ResourceServerResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Register a new Resource Server.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to register the resource server with.
     * @param resourceServerDTO the content of the resource server being registered.
     * @return the newly registered resource server.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ResourceServerDTO register(String tenant, ResourceServerDTO resourceServerDTO) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RESOURCE_SERVER_URI, tenant);
        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), resourceServerDTO);
        return execute(parent.getClient(), post, ResourceServerDTO.class);
    }

    /**
     * Request the list of Resource Servers associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to request the relying parties from.
     * @return a list of relying parties.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<ResourceServerDTO> getAll(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RESOURCE_SERVER_POST_URI, tenant);
        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, ResourceServerDTO.class);
    }

    /**
     * Request a specific Resource Server.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to request the resource server from.
     * @param name the name of the resource server to request.
     * @return the requested resource server.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ResourceServerDTO get(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RESOURCE_SERVER_NAME_POST_URI, tenant, name);
        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, ResourceServerDTO.class);
    }

    /**
     * Update a Resource Server.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the resource server.
     * @param name the name of the resource server being updated.
     * @param resourceServerDTO the contents used to perform the update.
     * @return the newly updated resource server.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ResourceServerDTO update(String tenant, String name, ResourceServerDTO resourceServerDTO) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RESOURCE_SERVER_NAME_URI, tenant, name);
        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), resourceServerDTO);
        return execute(parent.getClient(), put, ResourceServerDTO.class);
    }

    /**
     * Delete a Resource Server.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the resource server.
     * @param name the name of the resource server being deleted.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RESOURCE_SERVER_NAME_URI, tenant, name);
        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }
}
