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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.log4j.Logger;

/**
 * Class for driving IDM REST API samples. Initialized with handlers for completing command line requests.
 * 
 * @author abapat
 *
 */
public class SampleDriver {
	private static final Logger log = Logger.getLogger(SampleDriver.class);
	private List<SampleHandler> handlers;

	/**
	 * Initialize Sample Driver with a list of handlers.
	 */
	public SampleDriver() {
		handlers = new ArrayList<SampleHandler>();
		handlers.add(new CertificateSampleHandler());
		handlers.add(new GroupSampleHandler());
		handlers.add(new SolutionUserSampleHandler());
		handlers.add(new UserSampleHandler());
	}

	/**
	 * Populates options returns object.
	 * 
	 * @return Options for command line parsing
	 */
	private Options getOptions() {
		Options options = new Options();
		options.addOption(Option.builder("s").required().longOpt("sample").desc("Type of sample to run: user, group, etc.").hasArg().build());
		options.addOption(
				Option.builder("o").required().longOpt("operation").desc("Type of operation to run: create, update, etc.").hasArg().build());
		options.addOption(Option.builder("p").argName("JSON").hasArg().longOpt("payload")
				.desc("Payload or argument, if required. Can pass in path to JSON file or JSON string").build());
		return options;
	}

	/**
	 * Runs the sample corresponding to the command line arguments. There must be a handler initialized to carry out request.
	 * 
	 * @param args the command line arguments
	 */
	public void runSample(String[] args) {
		CommandLine line = null;
		try {
			CommandLineParser parser = new DefaultParser();
			line = parser.parse(getOptions(), args);

			String sample = line.getOptionValue("s");
			String operation = line.getOptionValue("o");
			String payload = line.getOptionValue("p");
			for (SampleHandler handler : handlers) {
				if (handler.getType().equalsIgnoreCase(sample)) {
					handler.callSample(operation, payload);
				}
			}
		} catch (ParseException e) {
			log.fatal("Error in parsing commands", e);
		}
	}

	public static void main(String[] args) {
		SampleDriver driver = new SampleDriver();
		driver.runSample(args);
	}
}
