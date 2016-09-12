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
package com.vmware.identity.rest.idm.drivers;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;

import org.apache.log4j.Logger;
import org.json.JSONException;
import org.json.JSONObject;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.samples.GroupSample;

/**
 * Class for handling calls to GroupSample from command line.
 * 
 * @author abapat
 *
 */
public class GroupSampleHandler extends SampleHandler {
	private GroupSample sample;

	/**
	 * Initializes GroupSample and logger.
	 */
	public GroupSampleHandler() {
		log = Logger.getLogger(getClass().getName());
		try {
			sample = new GroupSample();
		} catch (KeyManagementException | NoSuchAlgorithmException | KeyStoreException | ClientException | IOException e) {
			log.fatal("Error occured when initializing GroupSample", e);
		}
	}

	@Override
	public String getType() {
		return "group";
	}

	/**
	 * Parses String member type and returns corresponding MemberType enum.
	 * 
	 * @param memberType String.
	 * @return corresponding MemberType enum.
	 */
	private MemberType getMemberType(String memberType) {
		MemberType type = null;
		if (memberType.equalsIgnoreCase("group")) {
			type = MemberType.GROUP;
		} else if (memberType.equalsIgnoreCase("user")) {
			type = MemberType.USER;
		} else if (memberType.equalsIgnoreCase("solutionuser")) {
			type = MemberType.SOLUTIONUSER;
		} else if (memberType.equalsIgnoreCase("all")) {
			type = MemberType.ALL;
		} else {
			log.fatal("Invalid member type: " + memberType);
		}
		return type;
	}

	@Override
	public void callSample(String operation, String json) {
		String payload = parsePayload(json);
		try {
			JSONObject JSON = new JSONObject(payload);
			if (operation.equalsIgnoreCase("read")) {
				log.info("Getting Group: " + payload);
				GroupDTO group = sample.getGroup(JSON.getString("name"), tenant);
				log.info(group.toPrettyString());
			} else if (operation.equalsIgnoreCase("getmembers")) {
				log.info("Getting Members: " + payload);
				SearchResultDTO s = sample.getMembers(JSON.getString("name"), tenant, getMemberType(JSON.getString("type").toLowerCase()),
						JSON.getInt("limit"));
				log.info(s.toPrettyString());
			} else {
				log.fatal("Invalid command: " + operation);
			}
		} catch (JSONException e) {
			log.fatal("Error when parsing payload", e);
		} catch (Exception e) {
			log.fatal("Error when calling sample", e);
		}

	}
}
