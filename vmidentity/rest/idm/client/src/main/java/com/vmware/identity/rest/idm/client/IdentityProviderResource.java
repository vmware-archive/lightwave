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
import static com.vmware.identity.rest.core.client.URIFactory.buildParameters;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.IOException;
import java.net.URI;
import java.util.List;
import java.util.Map;

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
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;

/**
 * The {@code IdentityProviderResource} is effectively a container that gathers all of
 * the commands related to a tenant's identity providers. These commands are all
 * gathered under the {@value #PROVIDER_URI} endpoint on the REST server.
 */
public class IdentityProviderResource extends ClientResource {

    private static final String PROVIDER_URI = "/idm/tenant/%s/providers";
    private static final String PROVIDER_NAME_URI = PROVIDER_URI + "/%s";

    private static final String PROVIDER_POST_URI = "/idm/post/tenant/%s/providers";
    private static final String PROVIDER_NAME_POST_URI = PROVIDER_POST_URI + "/%s";

    public IdentityProviderResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Create a new identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to create an identity provider for.
     * @param provider the provider to create.
     * @return the newly created provider.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public IdentityProviderDTO create(String tenant, IdentityProviderDTO provider) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        return create(tenant, provider, false);
    }

    /**
     * Probe an identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant that will perform the probe.
     * @param provider the provider to probe.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void probe(String tenant, IdentityProviderDTO provider) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        create(tenant, provider, true);
    }

    /**
     * Request the list of identity providers associated with a tenant.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to request the identity providers from.
     * @return a list of identity providers.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<IdentityProviderDTO> getAll(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), PROVIDER_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, IdentityProviderDTO.class);
    }

    /**
     * Request a specific identity provider.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to request the identity provider from.
     * @param name the name of the identity provider to request.
     * @return the requested identity provider.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public IdentityProviderDTO get(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), PROVIDER_NAME_POST_URI, tenant, name);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, IdentityProviderDTO.class);
    }

    /**
     * Update an identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant the identity provider is associated with.
     * @param name the name of the identity provider to update.
     * @param provider the contents of the update.
     * @return the updated identity provider.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public IdentityProviderDTO update(String tenant, String name, IdentityProviderDTO provider) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), PROVIDER_NAME_URI, tenant, name);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), provider);
        return execute(parent.getClient(), put, IdentityProviderDTO.class);
    }

    /**
     * Delete an identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to delete the identity provider from.
     * @param name the name of the identity provider to delete.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), PROVIDER_NAME_URI, tenant, name);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

    private IdentityProviderDTO create(String tenant, IdentityProviderDTO provider, boolean probe) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        Map<String, Object> params = buildParameters("probe", String.valueOf(probe));
        URI uri = buildURI(parent.getHostRetriever(), PROVIDER_URI, tenant, params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), provider);
        return execute(parent.getClient(), post, IdentityProviderDTO.class);
    }

}
