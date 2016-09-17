/**
 *
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.configure;

public class VmIdentityParams {
    private String hostname;
    private String username;
    private String password;
    private String domainName;
    private boolean isUpgrade = false;
    private boolean isStartService = false;
    private String backupDir;
    private String vmidentityConfFile;

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

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public void SetUpgradeMode() {
        this.isUpgrade = true;
    }

    public boolean isUpgradeMode() {
        return this.isUpgrade;
    }

    public void setBackupDir(String path) {
        this.backupDir = path;
    }

    public void setVmIdentityConf(String filepath) {
        this.vmidentityConfFile = filepath;
    }

    public String getBackupDir() {
        return this.backupDir;
    }

    public String getVmIdentityConf() {
        return this.vmidentityConfFile;
    }
    public void setServiceStart() {
        isStartService = true;
    }
    public boolean getServiceStart() {
        return isStartService;
    }
}
