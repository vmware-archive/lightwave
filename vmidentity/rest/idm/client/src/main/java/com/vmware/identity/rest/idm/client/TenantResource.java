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
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.data.attributes.SearchType;
import com.vmware.identity.rest.idm.data.attributes.TenantConfigType;

/**
 * The {@code TenantResource} is effectively a container that gathers all of
 * the commands related to a system's tenants. These commands are all gathered
 * under the {@value #TENANT_URI} endpoint on the REST server.
 */
public class TenantResource extends ClientResource {

    private static final String TENANT_URI = "/idm/tenant";
    private static final String TENANT_NAME_URI = TENANT_URI + "/%s";

    private static final String TENANT_POST_URI = "/idm/post/tenant";
    private static final String TENANT_NAME_POST_URI = TENANT_POST_URI + "/%s";

    public TenantResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Create a new tenant.
     *
     * <p><b>Required Role:</b> {@code system administrator}.
     *
     * @param tenant the new tenant to be created.
     * @return the newly created tenant.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public TenantDTO create(TenantDTO tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), TENANT_URI);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), tenant);
        return execute(parent.getClient(), post, TenantDTO.class);
    }

    /**
     * Request a specific tenant.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to retrieve.
     * @return the requested tenant.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public TenantDTO get(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), TENANT_NAME_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, TenantDTO.class);
    }

    /**
     * Delete a tenant.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to delete.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), TENANT_NAME_URI, tenant);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

    /**
     * Request a tenant's full configuration details.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to retrieve the details from.
     * @return the configuration details of the tenant.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public TenantConfigurationDTO getConfig(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        return getConfig(tenant, null);
    }

    /**
     * Request specific components of a tenant's configuration details.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to retrieve the details from.
     * @param configTypes a list of the types of configuration details to retrieve.
     * @return the requested configuration details of the tenant.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public TenantConfigurationDTO getConfig(String tenant, List<TenantConfigType> configTypes) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        Map<String, Object> params = buildParameters("type", configTypes);
        URI uri = buildURI(parent.getHostRetriever(), TENANT_NAME_POST_URI + "/config", tenant, params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, TenantConfigurationDTO.class);
    }

    /**
     * Update a tenant's configuration.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to update.
     * @param config the configuration to update the tenant with.
     * @return the newly configured tenant's configuration details.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public TenantConfigurationDTO updateConfig(String tenant, TenantConfigurationDTO config) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), TENANT_NAME_URI + "/config", tenant);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), config);
        return execute(parent.getClient(), put, TenantConfigurationDTO.class);
    }

    /**
     * Search by name for members within a tenant's domain.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to search.
     * @param domain the domain to search through.
     * @param query the search string (e.g. a name)
     * @return the results of the search.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public SearchResultDTO search(String tenant, String domain, String query) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        return search(tenant, domain, query, null, null, null);
    }

    /**
     * Search for members within a tenant's domain.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to search.
     * @param domain the domain to search through.
     * @param query the search string related to the {@code searchBy} parameter.
     * @param memberType the type of members to search for.
     * @param searchBy the type of search to perform.
     * @param limit the maximum number of results for the search.
     * @return the results of the search.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public SearchResultDTO search(String tenant, String domain, String query, MemberType memberType, SearchType searchBy, Integer limit) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        Map<String, Object> params = buildParameters("domain", domain,
                "query", query,
                "type", memberType,
                "searchBy", searchBy,
                "limit", limit);
        URI uri = buildURI(parent.getHostRetriever(), TENANT_NAME_POST_URI + "/search", tenant, params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, SearchResultDTO.class);
    }

}
