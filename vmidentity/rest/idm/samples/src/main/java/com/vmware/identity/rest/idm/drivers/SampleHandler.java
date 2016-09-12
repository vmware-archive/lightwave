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
import java.nio.file.Files;
import java.nio.file.Paths;

import org.apache.log4j.Logger;

import com.vmware.identity.rest.idm.samples.Config;

/**
 * Abstract class to handle the calling of samples. Calls the appropriate method for sample given command line arguments.
 * 
 * @author abapat
 *
 */
public abstract class SampleHandler {
	protected Logger log;
	protected final String tenant = Config.getProperty(Config.TENANT);

	/**
	 * Parses payload argument from command line and returns the JSON.
	 * 
	 * @param payload the argument from command line, can be a file path or JSON string.
	 * @return the JSON object corresponding the payload.
	 */
	protected String parsePayload(String payload) {
		String json = null;
		if (payload != null) {
			try {
				json = (payload.indexOf("{") == -1) ? new String(Files.readAllBytes(Paths.get(payload))) : payload;
			} catch (IOException e) {
				log.fatal("Error in parsing payload: " + payload, e);
				System.exit(1);
			}
		}
		return json;
	}

	/**
	 * Returns what type of Sample this handler calls.
	 * 
	 * @return String representing which sample this handler manages.
	 */
	public abstract String getType();

	/**
	 * Calls the appropriate sample method given an operation and payload.
	 * 
	 * @param operation the operation to perform, from command line argument.
	 * @param payload the payload or argument to the operation.
	 */
	public abstract void callSample(String operation, String payload);

}
