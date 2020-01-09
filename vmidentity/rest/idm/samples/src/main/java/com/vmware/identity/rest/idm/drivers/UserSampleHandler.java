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

import org.json.JSONException;
import org.json.JSONObject;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import org.slf4j.LoggerFactory;

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
		logger = LoggerFactory.getLogger(getClass().getName());
		try {
			sample = new UserSample();
		} catch (KeyManagementException | NoSuchAlgorithmException | KeyStoreException | ClientException | IOException e) {
			logger.error("Error occured when initializing UserSample", e);
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
					logger.info("Creating User (Not Implemented): " + payload);
				} else {
					logger.info("Updating User (Not Implemented): " + payload);
				}
				logger.info(user.toPrettyString());
			} else if (operation.equalsIgnoreCase("read") || operation.equalsIgnoreCase("delete")) {
				JSONObject JSON = new JSONObject(payload);
				if (operation.equalsIgnoreCase("read")) {
					logger.info("Getting User: " + payload);
					UserDTO user = sample.getUser(JSON.getString("name"), JSON.getString("domain"), tenant);
					logger.info(user.toPrettyString());
				} else {
					logger.info("Deleting User (Not Implemented): " + payload);
				}
			} else {
				logger.error("Invalid command: " + operation);
			}
		} catch (JSONException e) {
			logger.error("Error when parsing payload", e);
		} catch (Exception e) {
			logger.error("Error when calling sample", e);
		}

	}

}
