/*
 *  Copyright (c) 2012-2017 VMware, Inc.  All Rights Reserved.
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

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;
import com.vmware.identity.rest.idm.data.FederatedIdpDTO;
import org.apache.commons.codec.binary.Base64;
import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;

import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URLEncoder;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import static com.vmware.identity.rest.core.client.RequestExecutor.execute;
import static com.vmware.identity.rest.core.client.RequestExecutor.executeAndReturnList;
import static com.vmware.identity.rest.core.client.URIFactory.buildParameters;
import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

/**
 * The {@code FederatedIDPResource} is effectively a container that gathers all of the
 * commands related to a tenant's federated identity providers. These commands are all
 * gathered under the {@value #FEDERATED_IDP_URI} endpoint on the REST server.
 */
public class FederatedIDPResource extends ClientResource {

    private static final String FEDERATED_IDP_URI = "/idm/tenant/%s/federation";
    private static final String FEDERATED_IDP_REGISTRATION_URI = FEDERATED_IDP_URI + "/json";
    private static final String FEDERATED_IDP_NAME_URI = FEDERATED_IDP_URI + "/%s";

    public FederatedIDPResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Register an external identity provider.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to register the external identity provider for.
     * @param federatedIdp the federated identity provider to register.
     * @return the federated identity provider that was registered.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public FederatedIdpDTO
    register(
            String tenant,
            FederatedIdpDTO federatedIdp
    ) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), FEDERATED_IDP_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), federatedIdp);
        return execute(parent.getClient(), post, FederatedIdpDTO.class);
    }

    /**
     * Register an federated identity provider(ADFS, Shibboleth etc) with IDP provided metadata.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to register the external identity provider for.
     * @param metadata the JSON configuration data provided by federated identity provider (ADFS, Shibboleth, etc) to register.
     * @return the federated identity provider that was registered.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.Identity
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public FederatedIdpDTO
    register(
            String tenant,
            String metadata
    ) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), FEDERATED_IDP_REGISTRATION_URI, tenant);
        HttpPost post = RequestFactory.createPostRequestWithFormEncodedString(
                                            uri,
                                            parent.getToken(),
                                            URLEncoder.encode(
                                                    Base64.encodeBase64String(metadata.getBytes("UTF-8")),
                                                    "UTF-8"
                                            )
        );
        return execute(parent.getClient(), post, FederatedIdpDTO.class);
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
    public List<FederatedIdpDTO>
    getAll(
            String tenant
    ) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), FEDERATED_IDP_URI, tenant);

        HttpGet request = RequestFactory.createGetRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), request, FederatedIdpDTO.class);
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
    public FederatedIdpDTO
    get(
            String tenant,
            String entityId
    ) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(
                    parent.getHostRetriever(),
                    FEDERATED_IDP_NAME_URI,
                    tenant,
                    Base64.encodeBase64String(entityId.getBytes("UTF-8"))
                    );

        HttpGet request = RequestFactory.createGetRequest(uri, parent.getToken());
        return execute(parent.getClient(), request, FederatedIdpDTO.class);
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
    public void
    delete(
            String tenant,
            String entityId
    ) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        delete(tenant, Base64.encodeBase64String(entityId.getBytes("UTF-8")), false);
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
    public void
    delete(
            String tenant,
            String entityId,
            boolean removeJitUsers
    ) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
    	Map<String, Object> params = buildParameters("remove", String.valueOf(removeJitUsers));
    	URI uri = buildURI(
    	            parent.getHostRetriever(),
                    FEDERATED_IDP_NAME_URI,
                    tenant,
                    Base64.encodeBase64String(entityId.getBytes("UTF-8")),
                    params
                    );
        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }
}
