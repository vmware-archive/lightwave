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

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.samples.UserSample;

/**
 * Class for handling calls to UserSample from command line.
 *
 * @author abapat
 *
 */
public class UserSampleHandler extends SampleHandler {
	private UserSample sample;

	/**
	 * Initializes UserSample and logger.
	 */
	public UserSampleHandler() {
		log = Logger.getLogger(getClass().getName());
		try {
			sample = new UserSample();
		} catch (KeyManagementException | NoSuchAlgorithmException | KeyStoreException | ClientException | IOException e) {
			log.fatal("Error occured when initializing UserSample", e);
		}
	}

	@Override
	public String getType() {
		return "user";
	}

	@Override
	public void callSample(String operation, String json) {
		String payload = parsePayload(json);
		try {
			if (operation.equalsIgnoreCase("create") || operation.equalsIgnoreCase("update")) {
				Gson g = new GsonBuilder().create();
				UserDTO user = g.fromJson(payload, UserDTO.class);
				if (operation.equalsIgnoreCase("create")) {
					log.info("Creating User (Not Implemented): " + payload);
				} else {
					log.info("Updating User (Not Implemented): " + payload);
				}
				log.info(user.toPrettyString());
			} else if (operation.equalsIgnoreCase("read") || operation.equalsIgnoreCase("delete")) {
				JSONObject JSON = new JSONObject(payload);
				if (operation.equalsIgnoreCase("read")) {
					log.info("Getting User: " + payload);
					UserDTO user = sample.getUser(JSON.getString("name"), JSON.getString("domain"), tenant);
					log.info(user.toPrettyString());
				} else {
					log.info("Deleting User (Not Implemented): " + payload);
				}
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
