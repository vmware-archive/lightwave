/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.openidconnect.sample;

/**
 * @author Jun Sun
 */
public class RelyingPartyConfig {

    private String opFQDN;
    private String opListeningPort;
    private String rpListeningPort;
    private String regInfoDir;
    private String clientPrefix;
    private String adminUsername;
    private String adminPassword;
    private String tenant;
    private String rpHaEnabled;
    private String rpVecsEnabled;

    public String getOpFQDN() {
        return this.opFQDN;
    }

    public void setOpFQDN(String opFQDN) {
        this.opFQDN = opFQDN;
    }

    public String getOpListeningPort() {
        return this.opListeningPort;
    }

    public void setOpListeningPort(String opListeningPort) {
        this.opListeningPort = opListeningPort;
    }

    public String getRpListeningPort() {
        return this.rpListeningPort;
    }

    public void setRpListeningPort(String rpListeningPort) {
        this.rpListeningPort = rpListeningPort;
    }

    public String getRegInfoDir() {
        return this.regInfoDir;
    }

    public void setRegInfoDir(String regInfoDir) {
        this.regInfoDir = regInfoDir;
    }

    public String getClientPrefix() {
        return this.clientPrefix;
    }

    public void setClientPrefix(String clientPrefix) {
        this.clientPrefix = clientPrefix;
    }

    public String getAdminUsername() {
        return this.adminUsername;
    }

    public void setAdminUsername(String adminUsername) {
        this.adminUsername = adminUsername;
    }

    public String getAdminPassword() {
        return this.adminPassword;
    }

    public void setAdminPassword(String adminPassword) {
        this.adminPassword = adminPassword;
    }

    public String getTenant() {
        return this.tenant;
    }

    public void setTenant(String tenant) {
        this.tenant = tenant;
    }

    public String getRpHaEnabled() {
        return this.rpHaEnabled;
    }

    public void setRpHaEnabled(String rpHaEnabled) {
        this.rpHaEnabled = rpHaEnabled;
    }

    public String getRpVecsEnabled() {
        return this.rpVecsEnabled;
    }

    public void setRpVecsEnabled(String rpVecsEnabled) {
        this.rpVecsEnabled = rpVecsEnabled;
    }

    public String getRpInfoFile() {
        return this.regInfoDir + "/rp.info";
    }

    public String getOpPublickeyFile() {
        return this.regInfoDir + "/op.publickey";
    }

    public String getRpPrivatekeyFile() {
        return this.regInfoDir + "/rp.privatekey";
    }

    public String getRpCertificateFile() {
        return this.regInfoDir + "/rp.certificate";
    }

    public String getRpListeningPortFile() {
        return this.regInfoDir + "/rp.port";
    }
}
