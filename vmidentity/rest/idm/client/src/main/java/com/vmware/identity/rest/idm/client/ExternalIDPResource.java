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

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;

/**
 * The {@code ExternalIDPResource} is effectively a container that gathers all of the
 * commands related to a tenant's external identity providers. These commands are all
 * gathered under the {@value #EXTERNAL_IDP_URI} endpoint on the REST server.
 */
public class ExternalIDPResource extends ClientResource {

    private static final String EXTERNAL_IDP_URI = "/idm/tenant/%s/externalidp";
    private static final String EXTERNAL_IDP_NAME_URI = EXTERNAL_IDP_URI + "/%s";

    private static final String EXTERNAL_IDP_POST_URI = "/idm/post/tenant/%s/externalidp";
    private static final String EXTERNAL_IDP_NAME_POST_URI = EXTERNAL_IDP_POST_URI + "/%s";

    public ExternalIDPResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Register an external identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to register the external identity provider for.
     * @param externalidp the external identity provider to register.
     * @return the external identity provider that was registered.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ExternalIDPDTO register(String tenant, ExternalIDPDTO externalidp) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), EXTERNAL_IDP_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), externalidp);
        return execute(parent.getClient(), post, ExternalIDPDTO.class);
    }

    /**
     * Register an external identity provider(ADFS, Shibboleth etc) with IDP provided metadata.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to register the external identity provider for.
     * @param metadata the XML configuration data provided by external identity provider (ADFS, Shibboleth, etc) to register.
     * @return the external identity provider that was registered.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ExternalIDPDTO register(String tenant, String metadata) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), EXTERNAL_IDP_URI, tenant);

        HttpPost post = RequestFactory.createPostRequestWithXml(uri, parent.getToken(), metadata);
        return execute(parent.getClient(), post, ExternalIDPDTO.class);
    }

    /**
     * Request the list of external identity providers associated with a .
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to request the external identity providers from.
     * @return a list of external identity providers.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<ExternalIDPDTO> getAll(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), EXTERNAL_IDP_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, ExternalIDPDTO.class);
    }

    /**
     * Request a specific external identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to fetch the request identity provider from.
     * @param entityId the entity identifier for the external identity provider to request.
     * @return the requested external identity provider.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public ExternalIDPDTO get(String tenant, String entityId) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), EXTERNAL_IDP_NAME_POST_URI, tenant, entityId);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, ExternalIDPDTO.class);
    }

    /**
     * Delete a specific external identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to delete the external identity provider from.
     * @param entityId the entity identifier for the external identity provider.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String entityId) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        delete(tenant, entityId, false);
    }

    /**
     * Delete a specific external identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to delete the external identity provider from.
     * @param entityId the entity identifier for the external identity provider.
     * @param removeJitUsers true to remove jit users associated with the external IDP
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String entityId, boolean removeJitUsers) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
    	Map<String, Object> params = buildParameters("remove", String.valueOf(removeJitUsers));
    	URI uri = buildURI(parent.getHostRetriever(), EXTERNAL_IDP_NAME_URI, tenant, entityId, params);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }
}
