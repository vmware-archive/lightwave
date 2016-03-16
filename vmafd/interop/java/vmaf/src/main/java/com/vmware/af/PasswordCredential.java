/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


package com.vmware.af;

public class PasswordCredential {
	
	private String _username;
	private String _password;
	
	public PasswordCredential(String username, String password)
	{
		_username = username;
		_password = password;
	}
	
	public String getUserName()
	{
		return _username;
	}
	
	public String getPassword()
	{
		return _password;
	}
}
