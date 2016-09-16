/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
 *
 */
package com.vmware.identity.rest.idm.samples;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;

import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.idm.client.UserResource;
import com.vmware.identity.rest.idm.data.PasswordDetailsDTO;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.data.UserDetailsDTO;

/**
 * Samples for using {@code UserResource}. The User Resource is a container that gathers all of the commands related to a tenant's users.
 *
 * @author abapat
 *
 */
public class UserSample extends SampleBase {
    private UserResource resource;

    /**
     * Initializes IDM client and UserResource.
     *
     * @throws KeyManagementException if an error occurs when making SSL request.
     * @throws NoSuchAlgorithmException if an error occurs when making SSL request.
     * @throws KeyStoreException if an error occurs building making SSL request.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws ClientException if a client side error occurs.
     * @throws IOException if an error occurs when reading response.
     */
    public UserSample()
            throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        super();
        resource = new UserResource(client);
    }

    public UserDTO createUser(UserDTO user, String tenant)
            throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
        return resource.create(tenant, user);
    }

    /**
     * Request a specific user.
     *
     * <p>
     * <b>Required Role:</b> {@code user}.
     *
     * @see UPNUtil#buildUPN(String, String)
     * @param name the name of the user to request.
     * @param domain the domain of the user to request.
     * @param tenant the tenant to retrieve the user from.
     * @return the requested user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public UserDTO getUser(String name, String domain, String tenant)
            throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
        return resource.get(tenant, name, domain);
    }

    /**
     * Delete a user.
     *
     * <p>
     * <b>Required Role:</b> {@code administrator}.
     *
     * @param name the name of the user.
     * @param domain the domain of the user.
     * @param tenant the name of the tenant containing the user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public void deleteUser(String name, String domain, String tenant)
            throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
        resource.delete(tenant, name, domain);
    }

    /**
     * Update a user.
     *
     * <p>
     * <b>Required Role:</b> {@code administrator}.
     *
     * @param name the name for the user.
     * @param firstName the first name of the user.
     * @param lastName the last name of the user.
     * @param email the email of the user.
     * @param password the password of the user.
     * @param description the description of the user.
     * @param domain the domain of the user.
     * @param tenant the name of the tenant containing the user.
     * @return the newly updated user.
     * @throws ClientException if a client side error occurs.
     * @throws ClientProtocolException in case of an http protocol error.
     * @throws WebApplicationException in the event of an application error.
     * @throws HttpException if there was a generic error with the remote call.
     * @throws IOException if there was an error with the IO stream.
     */
    public UserDTO updateUser(String name, String firstName, String lastName, String email, String password, String description, String domain,
            String tenant) throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
        UserDTO user = generateUser(name, firstName, lastName, email, domain, password, description);
        return resource.update(tenant, name, domain, user);
    }

    public UserDTO updateUser(UserDTO user, String tenant)
            throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
        return resource.update(tenant, user.getName(), user.getDomain(), user);
    }

    private UserDTO generateUser(String name, String firstName, String lastName, String email, String domain, String password, String description) {
        return new UserDTO.Builder().withName(name).withDomain(domain).withDetails(generateUserDetails(firstName, lastName, email, description))
                .withPasswordDetails(generatePasswordDetails(password)).withLocked(false).withDisabled(false).build();
    }

    private UserDetailsDTO generateUserDetails(String firstName, String lastName, String email, String description) {
        return new UserDetailsDTO.Builder().withFirstName(firstName).withLastName(lastName).withDescription(description).withEmail(email).build();
    }

    private PasswordDetailsDTO generatePasswordDetails(String password) {
        return new PasswordDetailsDTO.Builder().withPassword(password).build();
    }

}
