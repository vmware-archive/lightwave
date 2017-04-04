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
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.UserDTO;

/**
 * The {@code UserResource} is effectively a container that gathers all of
 * the commands related to a tenant's users. These commands are all gathered
 * under the {@value #USER_URI} endpoint on the REST server.
 */
public class UserResource extends ClientResource {

    private static final String USER_POST_URI = "/idm/post/tenant/%s/users";
    private static final String USER_NAME_POST_URI = USER_POST_URI + "/%s";
    private static final String USER_NAME_GROUPS_POST_URI = USER_NAME_POST_URI + "/groups";

    public UserResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Request a specific user.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @see UPNUtil#buildUPN(String, String)
     * @param tenant the tenant to retrieve the user from.
     * @param name the name of the user to request.
     * @param domain the domain of the user to request.
     * @return the requested user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public UserDTO get(String tenant, String name, String domain) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        URI uri = buildURI(parent.getHostRetriever(), USER_NAME_POST_URI, tenant, upn);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, UserDTO.class);
    }

    /**
     * Request the list of groups that a user is a member of.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant containing the user.
     * @param name the name of the user.
     * @param domain the domain of the user.
     * @param nested whether to perform a nested group search or not.
     * @return the list of groups that a user is a member of.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<GroupDTO> getGroups(String tenant, String name, String domain, boolean nested) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        Map<String, Object> params = buildParameters("nested", String.valueOf(nested));
        URI uri = buildURI(parent.getHostRetriever(), USER_NAME_GROUPS_POST_URI, tenant, upn, params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, GroupDTO.class);
    }

}
