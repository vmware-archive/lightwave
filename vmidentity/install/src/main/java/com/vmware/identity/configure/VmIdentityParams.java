/**
 *
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.configure;

public class VmIdentityParams {
    private String hostname;
    private String hostnameType;
    private String subjectAltName;
    private String username;
    private String password;
    private String domainName;
    private boolean isUpgrade = false;
    private String backupDir;
    private String vmidentityConfFile;
    private boolean isMigration = false;
    private String sourceVersion;
    private String sourcePlatform;
    private String exportFolder;
    private boolean isSetHostnameMode = false;
    private boolean isSetCheckSTSHealth = false;

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

    public String getHostnameType() {
        return hostnameType;
    }

    public void setHostnameType(String hostnameType) {
        this.hostnameType = hostnameType;
    }

    public void setSubjectAltName(String subject) {
        this.subjectAltName = subject;
    }

    public String getSubjectAltName() {
        return subjectAltName;
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

    public void setMigrationMode(boolean migration) {
        isMigration = migration;
    }

    public boolean isMigrationMode() {
        return isMigration;
    }

    public void setSourceVersion (String version) {
        sourceVersion = version;
    }

    public String getSourceVersion() { return sourceVersion; }

    public void setSourcePlatform(String platform) {
        sourcePlatform = platform;
    }

    public String getSourcePlatform() {
        return sourcePlatform;
    }

    public void setExportFolder(String folder) {
        this.exportFolder = folder;
    }

    public String getExportFolder() {
        return exportFolder;
    }

    public void setHostNameMode(boolean mode) {
        isSetHostnameMode= mode;
    }

    public boolean isHostnameMode() {
        return isSetHostnameMode;
    }

    public void setCheckSTSHealth(boolean mode) {
        isSetCheckSTSHealth = mode;
    }

    public boolean isCheckSTSHealth() {
        return isSetCheckSTSHealth;
    }
 }
