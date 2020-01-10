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

import org.slf4j.LoggerFactory;

import org.json.JSONException;
import org.json.JSONObject;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;
import com.vmware.identity.rest.idm.samples.SolutionUserSample;

/**
 * Class for handling calling samples for SolutionUser from command line.
 * 
 * @author abapat
 *
 */
public class SolutionUserSampleHandler extends SampleHandler {
	private SolutionUserSample sample;

	/**
	 * Initializes SolutionUserSample and logger.
	 */
	public SolutionUserSampleHandler() {
		logger = LoggerFactory.getLogger(getClass().getName());
		try {
			sample = new SolutionUserSample();
		} catch (KeyManagementException | NoSuchAlgorithmException | KeyStoreException | ClientException | IOException e) {
			logger.error("Error occured when initializing SolutionUserSample", e);
		}
	}

	@Override
	public String getType() {
		return "solutionuser";
	}

	@Override
	public void callSample(String operation, String json) {
		String payload = parsePayload(json);
		try {
			JSONObject JSON = new JSONObject(payload);
			if (operation.equalsIgnoreCase("read")) {
				logger.info("Getting Solution User: " + payload);
				SolutionUserDTO u = sample.getSoutionUser(JSON.getString("name"), tenant);
				logger.info(u.toPrettyString());
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
