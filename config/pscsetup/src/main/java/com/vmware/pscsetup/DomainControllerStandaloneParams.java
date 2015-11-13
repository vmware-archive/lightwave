/**
 *
 * Copyright 2014 VMware, Inc.  All rights reserved.
 */

package com.vmware.pscsetup;

public class DomainControllerStandaloneParams {
    private String hostname;
    private String password;
    private String site;
    private String domainName;

    public String getDomainName() {
	return domainName;
    }

    public void setDomainName(String domainName) {
	this.domainName = domainName;
    }

    public String getHostname() {
	return hostname;
    }

    public void setHostname(String hostname) {
	this.hostname = hostname;
    }

    public String getPassword() {
	return password;
    }

    public void setPassword(String password) {
	this.password = password;
    }

    public String getSite() {
	return site;
    }

    public void setSite(String site) {
	this.site = site;
    }
}
