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
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;

/**
 * The {@code RelyingPartyResource} is effectively a container that gathers all of
 * the commands related to the SAML 2.0 Relying Parties for a tenant. These commands
 * are all gathered under the {@value #RELYING_PARTY_URI} endpoint on the REST server.
 */
public class RelyingPartyResource extends ClientResource {

    private static final String RELYING_PARTY_URI = "/idm/tenant/%s/relyingparty";
    private static final String RELYING_PARTY_NAME_URI = RELYING_PARTY_URI + "/%s";

    private static final String RELYING_PARTY_POST_URI = "/idm/post/tenant/%s/relyingparty";
    private static final String RELYING_PARTY_NAME_POST_URI = RELYING_PARTY_POST_URI + "/%s";

    public RelyingPartyResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Register a new Relying Party.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to register the relying party with.
     * @param relyingParty the content of the relying party being registered.
     * @return the newly registered relying party.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public RelyingPartyDTO register(String tenant, RelyingPartyDTO relyingParty) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RELYING_PARTY_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), relyingParty);
        return execute(parent.getClient(), post, RelyingPartyDTO.class);
    }

    /**
     * Register a new Relying Party.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to register the relying party with.
     * @param relyingPartyConfigData the XML formatted content of the relying party being registered.
     * @return the newly registered relying party.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public RelyingPartyDTO register(String tenant, String relyingPartyConfigData) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RELYING_PARTY_URI, tenant);

        HttpPost post = RequestFactory.createPostRequestWithXml(uri, parent.getToken(), relyingPartyConfigData);
        return execute(parent.getClient(), post, RelyingPartyDTO.class);
    }

    /**
     * Request the list of Relying Parties associated with a tenant.
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
    public List<RelyingPartyDTO> getAll(String tenant) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RELYING_PARTY_POST_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, RelyingPartyDTO.class);
    }

    /**
     * Request a specific Relying Party.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant to request the relying party from.
     * @param name the name of the relying party to request.
     * @return the requested relying party.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public RelyingPartyDTO get(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RELYING_PARTY_NAME_POST_URI, tenant, name);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, RelyingPartyDTO.class);
    }

    /**
     * Update a Relying Party.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the relying party.
     * @param name the name of the relying party being updated.
     * @param relyingParty the contents used to perform the update.
     * @return the newly updated relying party.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public RelyingPartyDTO update(String tenant, String name, RelyingPartyDTO relyingParty) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RELYING_PARTY_NAME_URI, tenant, name);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), relyingParty);
        return execute(parent.getClient(), put, RelyingPartyDTO.class);
    }

    /**
     * Delete a Relying Party.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the relying party.
     * @param name the name of the relying party being deleted.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String name) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), RELYING_PARTY_NAME_URI, tenant, name);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

}
