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
import com.vmware.identity.rest.idm.data.ServerDetailsDTO;
import com.vmware.identity.rest.idm.data.attributes.ComputerType;

/**
 * The {@code ServerResource} is effectively a container that gathers all of
 * the commands related to the operation of the IDM server or the machine as
 * a whole. These commands are all gathered under the {@value #SERVER_URI} endpoint on the REST server.
 */
public class ServerResource extends ClientResource {

    @SuppressWarnings("unused")
    private static final String SERVER_URI = "/idm/server";
    private static final String SERVER_URI_POST = "/idm/post/server";

    public ServerResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Request the list of all computers associated with the machine.
     *
     * <p><b>Required Role:</b> {@code system configuration}.
     *
     * @param type the type of computers to retrieve.
     * @return a list of servers.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<ServerDetailsDTO> getComputers(ComputerType type) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        Map<String, Object> params = buildParameters("type", type);
        URI uri = buildURI(parent.getHostRetriever(), SERVER_URI_POST + "/computers", params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, ServerDetailsDTO.class);
    }

}
