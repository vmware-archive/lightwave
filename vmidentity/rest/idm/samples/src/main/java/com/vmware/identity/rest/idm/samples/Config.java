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
import java.io.InputStream;
import java.util.Properties;

import org.apache.log4j.Logger;

/**
 * Class for reading properties from a configuration file. Used for getting configuration details of server such as tenant, hostname of server,
 * domain, and admin credentials. These details are used to get the access token and make api calls.
 * 
 * @author abapat
 *
 */
public class Config {
	private static Properties properties = null;
	private static Logger log = Logger.getLogger(Config.class);

	public static final String DOMAIN = "system.domain";
	public static final String TENANT = "system.tenant";
	public static final String HOSTNAME = "system.hostname";
	public static final String ADMIN_USERNAME = "admin.username";
	public static final String ADMIN_PASSWORD = "admin.password";

	private static final String CONFIG_FILENAME = "config.properties";

	private static Properties get() throws IOException {
		Properties configProperties = new Properties();
		try (InputStream in = Config.class.getClassLoader().getResourceAsStream(CONFIG_FILENAME)) {
			configProperties.load(in);
			return configProperties;
		}
	}

	/**
	 * Gets a property from the config file given the name of the field
	 * 
	 * @param name
	 * @return String property associated with name
	 */
	public static String getProperty(String name) {
		try {
			if (properties == null) {
				properties = get();
			}
			return properties.getProperty(name);
		} catch (IOException e) {
			log.fatal("Error when getting property: " + name, e);
			return null;
		}
	}
}
