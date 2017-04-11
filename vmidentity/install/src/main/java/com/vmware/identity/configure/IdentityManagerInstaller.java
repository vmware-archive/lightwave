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
package com.vmware.identity.configure;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.installer.ReleaseUtil;

public class IdentityManagerInstaller implements IPlatformComponentInstaller {
    private static final String ID = "vmware-identity-manager";
    private static final String Name = "VMware Identity Manager";
    private static final String Description = "VMware Identity Manager";
    private static final Logger log = LoggerFactory
            .getLogger(IdentityManagerInstaller.class);

    private String hostnameURL = null;
    private boolean isLightwave = false;
    private boolean initialized = false;
    private boolean setReverseProxy = false;

    public IdentityManagerInstaller(boolean setReverseProxy, boolean isUpgrade) {
        if (!isUpgrade) {
            this.setReverseProxy = setReverseProxy;
        }
    }

    @Override
    public void install() throws Exception {
        initialize();

        if(setReverseProxy){
            log.info("Setting reverse proxy port");
		    setReverseProxyPort();
        }

        log.info("Configuring registry setting for IDM");

        InstallerUtils.getInstallerHelper().configRegistry();

        log.info("Writing host info to Regisry");

        writeHostinfo();

        log.info("Configuring registry setting for IDM Complete");
    }

    @Override
    public void upgrade() {
        if (isHostnameFilePresent()) {
            try {
                this.hostnameURL = readHostnameFromFile();
                if (this.hostnameURL == null) {
                    initialize();
                }
            } catch (Exception ex) {
              this.hostnameURL = VmAfClientUtil.getHostnameURL();
            }
            try {
                writeHostinfo();
                removeHostnameFile();
            } catch (Exception ex){

            }
        }


    }

    @Override
    public void uninstall() {
        // TODO Auto-generated method stub

    }

    @Override
    public void migrate() {

    }

    private void initialize() {
        if (!initialized) {
            String hostnameURL = VmAfClientUtil.getHostnameURL();

            this.hostnameURL = hostnameURL;

            try {
                this.isLightwave = ReleaseUtil.isLightwave();
            } catch (IOException e) {
                throw new IllegalStateException(String.format("Failed to check whehter the installation is Lightwave: %s", e.getMessage()));
            }

            initialized = true;
        }
    }

    private void writeHostinfo() throws IdentityManagerInstallerException {
        Validate.validateNotEmpty(hostnameURL, "Host name URL");

        HostinfoWriter writer = new HostinfoWriter(hostnameURL, isLightwave);
        try {
            writer.write();
        } catch (HostinfoCreationFailedException e) {
            e.printStackTrace();
            throw new IdentityManagerInstallerException(
                    "Failed to create hostinfo file", e);
        }
    }

    private void setReverseProxyPort() {
        VmAfClientUtil.setReverseProxyPort(InstallerUtils.REVERSE_PROXY_PORT);
    }

    @Override
    public PlatformInstallComponent getComponentInfo() {
        return new PlatformInstallComponent(ID, Name, Description);
    }

    private boolean isHostnameFilePresent() {
        String configPath = InstallerUtils.getInstallerHelper().getConfigFolderPath();
        if (!configPath.endsWith(File.separator)){
            configPath += File.separator;
        }
        String hostnameFile = configPath + "hostname.txt";
        return (new File(hostnameFile).exists());
    }

    private String readHostnameFromFile() throws IOException{
        String hostname = null;
        String configPath = InstallerUtils.getInstallerHelper().getConfigFolderPath();
        if (!configPath.endsWith(File.separator)){
            configPath += File.separator;
        }
        String hostnameFile = configPath + "hostname.txt";
        BufferedReader br = null;
        FileInputStream fr = null;
        try {
            fr = new FileInputStream(hostnameFile);
            br = new BufferedReader(new InputStreamReader(fr));
            String str = br.readLine().trim();
            if (str != null)
                hostname  = str;
        } catch (Exception ex){

        } finally {

            if (br != null){
                br.close();
            }
            if (fr != null) {
                fr.close();
            }
        }
        return hostname;
    }

    private void removeHostnameFile() {
        String configPath = InstallerUtils.getInstallerHelper().getConfigFolderPath();
        if (!configPath.endsWith(File.separator)){
            configPath += File.separator;
        }
        File hostnameFile = new File(configPath + "hostname.txt");
        try {
            hostnameFile.delete();
        } catch (Exception ex){

        }
    }

}
