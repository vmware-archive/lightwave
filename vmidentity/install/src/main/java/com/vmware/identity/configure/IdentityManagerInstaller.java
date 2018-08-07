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
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Paths;
import java.nio.file.Path;
import java.nio.file.attribute.GroupPrincipal;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.UserPrincipal;
import java.nio.file.attribute.UserPrincipalLookupService;

import com.sun.jna.Native;
import org.apache.commons.lang.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.installer.ReleaseUtil;

import static java.nio.file.StandardCopyOption.REPLACE_EXISTING;

public class IdentityManagerInstaller implements IPlatformComponentInstaller {
    private static final String ID = "vmware-identity-manager";
    private static final String Name = "VMware Identity Manager";
    private static final String Description = "VMware Identity Manager";
    private static final Logger log = LoggerFactory
            .getLogger(IdentityManagerInstaller.class);
    private static final String jniDispatchLib = "/com/sun/jna/linux-x86-64/libjnidispatch.so";
    private static final String jniDispatchExportPath = "/vmware-sts/conf/libjnidispatch.so";

    private static final String lightwaveGroup = "lightwave";
    private static final String lightwaveUser = "lightwave";

    private String hostnameURL = null;
    private boolean isLightwave = false;
    private boolean initialized = false;
    private boolean setReverseProxy = false;
    private final VmIdentityParams params;

    public IdentityManagerInstaller(boolean setReverseProxy, VmIdentityParams installParams) {
        this.params = installParams;
        if (!this.params.isUpgradeMode()) {
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

        log.info("Writing host info to Registry");

        writeHostinfo();

        log.info("Configuring registry setting for IDM Complete");

        extractJniLib();
    }

    @Override
    public void upgrade() {
        String hostname = null;
        try {
            hostname = HostnameReader.readHostNameFromRegistry();
        } catch (Exception e) {
            log.warn("Failed to read hostname registry.", e);
        }

        extractJniLib();

        // set hostname registry with hostname file only if it is not set
        // for backward compatibility
        if (StringUtils.isEmpty(hostname) && isHostnameFilePresent()) {
            try {
                this.hostnameURL = readHostnameFromFile();
            } catch (Exception ex) {
                log.warn("Failed to read hostname file.", ex);
            }
            try {
                if (this.hostnameURL == null) {
                    initialize();
                }
                writeHostinfo();
                removeHostnameFile();
            } catch (Exception ex){
                log.error("Unable to write host info for IDM upgrade.", ex);
            }
        }

        try {
            log.info("Configuring registry setting for IDM");
            InstallerUtils.getInstallerHelper().configRegistry();
        } catch (Exception ex) {
            log.error("Unable to configure registry for IDM upgrade.", ex);
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
            this.hostnameURL = this.params.getEndpoint();
            if (StringUtils.isEmpty(this.hostnameURL)) {
                this.hostnameURL = VmAfClientUtil.getHostnameURL();
            }

            try {
                this.isLightwave = ReleaseUtil.isLightwave();
            } catch (IOException e) {
                throw new IllegalStateException(String.format("Failed to check whether the installation is Lightwave: %s", e.getMessage()));
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

    private void extractJniLib() {
        try {
            File jna = Native.extractFromResourcePath(jniDispatchLib);
            UserPrincipalLookupService lookupService = FileSystems.getDefault().getUserPrincipalLookupService();
            GroupPrincipal group = lookupService.lookupPrincipalByGroupName(lightwaveGroup);
            UserPrincipal user = lookupService.lookupPrincipalByName(lightwaveUser);

            String ssoHome = InstallerUtils.getInstallerHelper().getSSOHomePath();
            Path exportPath = Paths.get(ssoHome, jniDispatchExportPath);

            Files.copy(jna.toPath(), exportPath, REPLACE_EXISTING);
            Files.getFileAttributeView(exportPath, PosixFileAttributeView.class, LinkOption.NOFOLLOW_LINKS).setGroup(group);
            Files.getFileAttributeView(exportPath, PosixFileAttributeView.class, LinkOption.NOFOLLOW_LINKS).setOwner(user);
        } catch (Exception e) {
            log.error("Failed to extract jnidispatch lib", e);
        }
    }

}
