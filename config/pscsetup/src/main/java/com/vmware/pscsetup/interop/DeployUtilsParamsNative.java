/**
 *
 * Copyright 2014 VMware, Inc.  All rights reserved.
 */

package com.vmware.pscsetup.interop;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Structure;

public class DeployUtilsParamsNative extends Structure {
	public String pszHostname;
	public String pszDomainName;
	public String pszPassword;
	public int dir_svc_mode;
	public String pszServer;
	public String pszSite;

	public DeployUtilsParamsNative(String hostname, String domainName,
			String password, int mode, String server, String site) {

		this.pszHostname = hostname;
		this.pszDomainName = domainName;
		this.pszPassword = password;
		this.dir_svc_mode = mode;
		this.pszServer = server;
		this.pszSite = site;
		write();
	}

	@Override
	protected List<String> getFieldOrder() {
		return Arrays.asList(new String[] { "pszHostname", "pszDomainName",
				"pszPassword", "dir_svc_mode", "pszServer", "pszSite" });
	}

}
