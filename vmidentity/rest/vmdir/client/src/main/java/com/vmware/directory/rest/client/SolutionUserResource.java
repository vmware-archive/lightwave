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
package com.vmware.directory.rest.client;

import static com.vmware.identity.rest.core.client.RequestExecutor.execute;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;

import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;

/**
 * The {@code SolutionUserResource} is effectively a container that gathers all of
 * the commands related to a tenant's solution users. These commands
 * are all gathered under the {@value #SOLUTION_USERS_URI} endpoint on the REST server.
 */
public class SolutionUserResource extends ClientResource {

    private static final String SOLUTION_USERS_URI = "/vmdir/tenant/%s/solutionusers";
    private static final String SOLUTION_USERS_NAME_URI = SOLUTION_USERS_URI + "/%s";

    private static final String SOLUTION_USERS_POST_URI = "/vmdir/post/tenant/%s/solutionusers";
    private static final String SOLUTION_USERS_NAME_POST_URI = SOLUTION_USERS_POST_URI + "/%s";

    public SolutionUserResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Create a new solution user.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to create the solution user on.
     * @param user the contents of the solution user being created.
     * @return the newly created solution user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public SolutionUserDTO create(String tenant, SolutionUserDTO user) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), SOLUTION_USERS_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), user);
        return execute(parent.getClient(), post, SolutionUserDTO.class);
    }

    /**
     * Request a specific solution user.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to request the solution user from.
     * @param name the name of the solution user.
     * @return the requested solution user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public SolutionUserDTO get(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), SOLUTION_USERS_NAME_POST_URI, tenant, name);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, SolutionUserDTO.class);
    }

    /**
     * Update a solution user.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the solution user.
     * @param name the name of the solution user to update.
     * @param solutionUser the contents to update the solution user with.
     * @return the newly updated solution user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public SolutionUserDTO update(String tenant, String name, SolutionUserDTO solutionUser) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), SOLUTION_USERS_NAME_URI, tenant, name);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), solutionUser);
        return execute(parent.getClient(), put, SolutionUserDTO.class);
    }

    /**
     * Delete a solution user.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the solution user.
     * @param name the name of the solution user to delete.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), SOLUTION_USERS_NAME_URI, tenant, name);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

}