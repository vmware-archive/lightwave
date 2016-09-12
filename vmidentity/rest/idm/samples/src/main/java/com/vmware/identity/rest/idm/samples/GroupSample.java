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
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.idm.client.GroupResource;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;

/**
 * Samples for using {@code GroupResource}. The GroupResource is a container that gathers all of the commands related to a tenant's groups
 * 
 * @author abapat
 *
 */
public class GroupSample extends SampleBase {
	private GroupResource resource;

	/**
	 * Initializes IDM client and GroupResource.
	 * 
	 * @throws KeyManagementException if an error occurs when making SSL request.
	 * @throws NoSuchAlgorithmException if an error occurs when making SSL request.
	 * @throws KeyStoreException if an error occurs building making SSL request.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws ClientException if a client side error occurs.
	 * @throws IOException if an error occurs when reading response.
	 */
	public GroupSample()
			throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
		super();
		resource = new GroupResource(client);
	}

	/**
	 * Request a specific group.
	 *
	 * <p>
	 * <b>Required Role:</b> {@code user}.
	 *
	 * @param name the name of the group to request.
	 * @param tenant the name of the tenant to request the group from.
	 * @return the group retrieved from the tenant.
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws WebApplicationException in the event of an application error.
	 * @throws HttpException if there was a generic error with the remote call.
	 * @throws IOException if there was an error with the IO stream.
	 */
	public GroupDTO getGroup(String name, String tenant)
			throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
		return resource.get(tenant, name, DOMAIN);
	}

	/**
	 * Request the parents of a group.
	 *
	 * <p>
	 * <b>Required Role:</b> {@code user}.
	 *
	 * @param name the name of the group to retrieve the parents of.
	 * @param tenant the name of the tenant containing the group.
	 * @return a list of the group's parent groups.
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws WebApplicationException in the event of an application error.
	 * @throws HttpException if there was a generic error with the remote call.
	 * @throws IOException if there was an error with the IO stream.
	 */
	public List<GroupDTO> getParents(String name, String tenant)
			throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
		return resource.getParents(tenant, name, DOMAIN);
	}

	/**
	 * Request the members of a group.
	 *
	 * <p>
	 * <b>Required Role:</b> {@code user}.
	 *
	 * @param name the name of the group to get the members of.
	 * @param tenant the name of the tenant containing the group.
	 * @param type the type of members to get.
	 * @param limit the maximum number of members to retrieve.
	 * @return the result of the search query.
	 * @throws ClientException if a client side error occurs.
	 * @throws ClientProtocolException in case of an http protocol error.
	 * @throws WebApplicationException in the event of an application error.
	 * @throws HttpException if there was a generic error with the remote call.
	 * @throws IOException if there was an error with the IO stream.
	 */
	public SearchResultDTO getMembers(String name, String tenant, MemberType type, Integer limit)
			throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
		return resource.getMembers(tenant, name, DOMAIN, type, limit);
	}

}
