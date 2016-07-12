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
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;

/**
 * The {@code GroupResource} is effectively a container that gathers all of the
 * commands related to a tenant's groups. These commands are all gathered under
 * the {@value #GROUPS_URI} endpoint on the REST server.
 */
public class GroupResource extends ClientResource {

    private static final String GROUPS_POST_URI = "/idm/post/tenant/%s/groups";
    private static final String GROUPS_NAME_POST_URI = GROUPS_POST_URI + "/%s";
    private static final String GROUPS_NAME_MEMBERS_POST_URI = GROUPS_NAME_POST_URI + "/members";
    private static final String GROUPS_NAME_PARENTS_POST_URI = GROUPS_NAME_POST_URI + "/parents";

    public GroupResource(BaseClient client) {
        super(client);
    }

    /**
     * Request a specific group.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant to request the group from.
     * @param name the name of the group to request.
     * @param domain the domain of the group to request.
     * @return the group retrieved from the tenant.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public GroupDTO get(String tenant, String name, String domain) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        URI uri = buildURI(parent.getHostRetriever(), GROUPS_NAME_POST_URI, tenant, upn);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, GroupDTO.class);
    }

    /**
     * Request the members of a group.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant containing the group.
     * @param name the name of the group to get the members of.
     * @param domain the domain of the group to request.
     * @param type the type of members to get.
     * @param limit the maximum number of members to retrieve.
     * @return the result of the search query.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public SearchResultDTO getMembers(String tenant, String name, String domain, MemberType type, Integer limit) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        Map<String, Object> params = buildParameters("type", type, "limit", limit);
        URI uri = buildURI(parent.getHostRetriever(), GROUPS_NAME_MEMBERS_POST_URI, tenant, upn, params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return execute(parent.getClient(), post, SearchResultDTO.class);
    }

    /**
     * Request the parents of a group.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant containing the group.
     * @param name the name of the group to retrieve the parents of.
     * @param domain the domain of the group to request.
     * @return a list of the group's parent groups.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public List<GroupDTO> getParents(String tenant, String name, String domain) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        Map<String, Object> params = buildParameters("nested", String.valueOf(false));
        URI uri = buildURI(parent.getHostRetriever(), GROUPS_NAME_PARENTS_POST_URI, tenant, upn, params);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken());
        return executeAndReturnList(parent.getClient(), post, GroupDTO.class);
    }

}
