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
 */

package com.vmware.lightwaveui;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.io.IOException;

public class ServerHelper {

	public String getHostname() throws FileNotFoundException,UnsupportedEncodingException, IOException {
		FileInputStream fis = new FileInputStream("/etc/vmware-sso/hostname.txt");
		BufferedReader reader = null;
		try{
			reader = new BufferedReader(new InputStreamReader(fis, "UTF-8"));
			String line = reader.readLine();
			return line;
		}
		finally{
			if(reader != null)
			reader.close();
		}
		
	}
}
