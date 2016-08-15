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

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.PasswordResetRequestDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.ClientResource;
import com.vmware.identity.rest.core.client.RequestFactory;
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.methods.HttpDeleteWithBody;

/**
 * The {@code UserResource} is effectively a container that gathers all of
 * the commands related to a tenant's users. These commands are all gathered
 * under the {@value #USER_URI} endpoint on the REST server.
 */
public class UserResource extends ClientResource {

    private static final String USER_URI = "/vmdir/tenant/%s/users";
    private static final String USER_NAME_URI = USER_URI + "/%s";
    private static final String USER_NAME_PASSWORD_URI = USER_NAME_URI + "/password";

    private static final String USER_POST_URI = "/vmdir/post/tenant/%s/users";
    private static final String USER_NAME_POST_URI = USER_POST_URI + "/%s";
    private static final String USER_NAME_GROUPS_POST_URI = USER_NAME_POST_URI + "/groups";

    public UserResource(BaseClient parent) {
        super(parent);
    }

    /**
     * Create a new user.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the tenant to create the user on.
     * @param user the user to be created.
     * @return the newly created user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public UserDTO create(String tenant, UserDTO user) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        URI uri = buildURI(parent.getHostRetriever(), USER_URI, tenant);

        HttpPost post = RequestFactory.createPostRequest(uri, parent.getToken(), user);
        return execute(parent.getClient(), post, UserDTO.class);
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

    /**
     * Update a user.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the user.
     * @param name the name of the user.
     * @param domain the domain of the user.
     * @param user the contents of the user to update.
     * @return the newly updated user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public UserDTO update(String tenant, String name, String domain, UserDTO user) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        URI uri = buildURI(parent.getHostRetriever(), USER_NAME_URI, tenant, upn);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), user);
        return execute(parent.getClient(), put, UserDTO.class);
    }

    /**
     * Update a user's password.
     *
     * <p><b>Required Role:</b> {@code user}.
     *
     * @param tenant the name of the tenant containing the user.
     * @param name the name of the user.
     * @param domain the domain of the user.
     * @param currentPassword the user's current password.
     * @param newPassword the new password to set for the user.
     * @return the newly updated user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public UserDTO updatePassword(String tenant, String name, String domain, String currentPassword, String newPassword) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        PasswordResetRequestDTO reset = new PasswordResetRequestDTO(currentPassword, newPassword);

        URI uri = buildURI(parent.getHostRetriever(), USER_NAME_PASSWORD_URI, tenant, upn);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), reset);
        return execute(parent.getClient(), put, UserDTO.class);
    }

    /**
     * Reset a user's password.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the user.
     * @param name the name of the user.
     * @param domain the domain of the user.
     * @param newPassword the new password to set for the user.
     * @return the newly updated user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public UserDTO resetPassword(String tenant, String name, String domain, String newPassword) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        PasswordResetRequestDTO reset = new PasswordResetRequestDTO(null, newPassword);

        URI uri = buildURI(parent.getHostRetriever(), USER_NAME_PASSWORD_URI, tenant, upn);

        HttpPut put = RequestFactory.createPutRequest(uri, parent.getToken(), reset);
        return execute(parent.getClient(), put, UserDTO.class);
    }

    /**
     * Delete a user.
     *
     * <p><b>Required Role:</b> {@code administrator}.
     *
     * @param tenant the name of the tenant containing the user.
     * @param name the name of the user.
     * @param domain the domain of the user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void delete(String tenant, String name, String domain) throws ClientException, ClientProtocolException, WebApplicationException, HttpException, IOException {
        String upn = UPNUtil.buildUPN(name, domain);
        URI uri = buildURI(parent.getHostRetriever(), USER_NAME_URI, tenant, upn);

        HttpDeleteWithBody delete = RequestFactory.createDeleteRequest(uri, parent.getToken());
        execute(parent.getClient(), delete);
    }

}
