/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.domainmanager;

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 03/15/13
 * Time: 10:59 PM
 */
public class DomainControllerInfo
{
	public String domainName; // domain name in format of 'vmware.com' or domain netbios name 'VMWAREM'
	public String domainIpAddress;
	public String domainFQDN;
	public String domainDnsForestName;
	public String domainNetBiosName;

	public DomainControllerInfo(String domainName, String domainNetBiosName, String domainAddress, String domainFQDN, String domainDnsForestName)
	{
		this.domainName = normalizeName(domainName);
		this.domainNetBiosName = normalizeName(domainNetBiosName);
		this.domainIpAddress = normalizeName(domainAddress);
		this.domainFQDN = normalizeName(domainFQDN);
		this.domainDnsForestName = normalizeName(domainDnsForestName);
	}

	// strip out leading '\\\\' for domainAddress and domainDnsName if there is any
	private String normalizeName(String name)
	{
		return (name == null || name.length() == 0) ? name : name.substring(name.lastIndexOf('\\')+1);
	}
}

