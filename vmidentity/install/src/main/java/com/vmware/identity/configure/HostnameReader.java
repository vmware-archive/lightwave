package com.vmware.identity.configure;

import com.vmware.identity.installer.ReleaseUtil;

/*
 *  Copyright (c) 2016 VMware, Inc.  All Rights Reserved.
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


import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegValueType;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;
import com.vmware.identity.interop.registry.RegistryValueType;
import java.io.File;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.io.IOException;

public class HostnameReader {
    private static final String CONFIG_IDENTITY_ROOT_KEY = "Software\\VMware\\Identity\\Configuration";
    private static final String HOST_NAME_KEY = "Hostname";
    private static final String PORT_NUMBER_KEY_VSPHERE = "StsLocalTcPort"; // 7444
    private static final String PORT_NUMBER_KEY_LIGHTWAVE = "StsTcPort"; // 443
    private String hostname;

    private static String read(String key) throws Exception {
        String value = null;
        IRegistryKey registryRootKey = null;

        try {

            IRegistryAdapter registryAdpater = RegistryAdapterFactory.getInstance().getRegistryAdapter();
            registryRootKey = registryAdpater.openRootKey((int) RegKeyAccess.KEY_READ);
            if (registryRootKey == null) {
                new NullPointerException("Unable to open Root Key");
            }
            value = registryAdpater.getStringValue(registryRootKey, CONFIG_IDENTITY_ROOT_KEY, key, true);
        } finally {
            if (registryRootKey != null)
                registryRootKey.close();
        }
        return value;
    }

    public static String readHostName()  throws Exception {
        String hostname = null;
        try {
            hostname = read(HOST_NAME_KEY);
        } catch (Exception ex){
            System.out.println("Failed to read hostname from registry");
        }
        if (hostname == null ){
            hostname = readHostnameFromFile();
        }
        return hostname;
    }

    /**
     * The HTTPS ports for STS varies based on release.
     * <li> In lightwave, STS secure port is 443 </li>
     * <li> In Vsphere release, STS secure port is 7444 </li>
     */
    public static String readPortNumber() throws Exception {
        if(ReleaseUtil.isLightwave()) {
            return read(PORT_NUMBER_KEY_LIGHTWAVE);
        } else {
            return read(PORT_NUMBER_KEY_VSPHERE);
        }
    }

    private static String readHostnameFromFile() throws IOException{
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
            System.out.println("failed to read hostname.txt file");
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
}
