/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;

public class HostinfoWriter {
    private final String CONFIG_IDENTITY_ROOT_KEY = "Software\\VMware\\Identity\\Configuration";
    private final String HOST_NAME_KEY = "Hostname";
    private final String HOST_NAME_TYPE_KEY = "HostnameType";
    private final String IS_LIGHTWAVE_KEY = "isLightwave";
    private final String hostname;
    private final String hostnameType;
    private final boolean isLightwave;

    private static final Logger log = LoggerFactory
            .getLogger(HostinfoWriter.class);

    public HostinfoWriter(String hostname) {
        Validate.validateNotEmpty(hostname, "Hostname");

        this.hostname = hostname;
        this.hostnameType = null;
        this.isLightwave = false;
    }

    public HostinfoWriter(String hostname, boolean isLightwave) {
        Validate.validateNotEmpty(hostname, "Hostname");

        this.hostname = hostname;
        this.hostnameType = null;
        this.isLightwave = isLightwave;
    }

    public HostinfoWriter(String hostname, String hostnameType) {
        Validate.validateNotEmpty(hostname, "Hostname");
        Validate.validateNotEmpty(hostnameType, "HostnameType");

        this.hostname = hostname;
        this.hostnameType = hostnameType;
        this.isLightwave = false;
    }

    public void write() throws HostinfoCreationFailedException {

        IRegistryAdapter registryAdpater = null;
        IRegistryKey registryRootKey = null;
        IRegistryKey vmIdentityRegistryKey = null;
        try {
            registryAdpater = RegistryAdapterFactory.getInstance().getRegistryAdapter();
            registryRootKey = registryAdpater.openRootKey((int) RegKeyAccess.KEY_WRITE);
            vmIdentityRegistryKey = registryAdpater.openKey(registryRootKey, CONFIG_IDENTITY_ROOT_KEY, 0, (int) RegKeyAccess.KEY_ALL_ACCESS);
            registryAdpater.setStringValue(vmIdentityRegistryKey, HOST_NAME_KEY, this.hostname);
            if (this.hostnameType != null) {
                registryAdpater.setStringValue(vmIdentityRegistryKey, HOST_NAME_TYPE_KEY, this.hostnameType);
            }
            if (this.isLightwave) {
                registryAdpater.setIntValue(vmIdentityRegistryKey, IS_LIGHTWAVE_KEY, 1);
            }
        } catch (Exception ex) {
            throw new HostinfoCreationFailedException(ex);
        } finally {
            if(registryRootKey != null) {
                registryRootKey.close();
            }
            if(vmIdentityRegistryKey != null){
                vmIdentityRegistryKey.close();
            }
        }

    }

}
