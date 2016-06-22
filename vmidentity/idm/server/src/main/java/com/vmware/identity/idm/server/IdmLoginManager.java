/*
 *
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
 *
 */

package com.vmware.identity.idm.server;

import java.io.File;
import java.io.IOException;
import java.rmi.RemoteException;
import java.security.NoSuchAlgorithmException;

import org.apache.commons.lang.SystemUtils;

import com.vmware.identity.idm.CommonUtil;
import com.vmware.identity.idm.IIdentityManager;
import com.vmware.identity.idm.ILoginManager;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;

public class IdmLoginManager implements ILoginManager {

    private static final String SECRET_FILE = "idmlogin.txt";
    private static final String WIN_REG_PATH = "SOFTWARE\\VMware, Inc.\\VMware Identity Services";
    private static final String WIN_REG_KEY = "ConfigPath";
    private static final String KEYS = "\\keys\\";
    private static final String LINUX_FILE_PATH = "/etc/vmware-sso/keys/";
    private IdentityManager server = null;
    private final String serverHash;

    public IdmLoginManager(IdentityManager server) throws NoSuchAlgorithmException, IOException {
            this.server = server;
            serverHash = CommonUtil.Computesha256Hash(getSecretFilePrivate());
            ValidateUtil.validateNotEmpty(serverHash, "Server Hash");
        }

    @Override
    public IIdentityManager Login(String hash) throws RemoteException,
    SecurityException {
        if (serverHash.equals(hash)) {
                return this.server;
        } else {
                throw new SecurityException(
                        "Access denied. Invalid Authentication Attempt.");
        }
    }

    @Override
    public File getSecretFile() throws RemoteException {
        return getSecretFilePrivate();
    }

    private File getSecretFilePrivate() {
        if (SystemUtils.IS_OS_LINUX) {
            return getSecretFileLinux();
        } else {
            return getSecretFileWindows();
        }
    }

    private File getSecretFileWindows() {

        IRegistryAdapter regAdapter = RegistryAdapterFactory.getInstance()
                .getRegistryAdapter();

        IRegistryKey rootKey = regAdapter
                .openRootKey((int) RegKeyAccess.KEY_READ);
        try {
            String configPath = regAdapter.getStringValue(rootKey, WIN_REG_PATH,
                    WIN_REG_KEY, false);
            return new File(configPath.concat(KEYS).concat(SECRET_FILE));
        } finally {
            if(rootKey != null)
            rootKey.close();
        }
    }

    private File getSecretFileLinux() {
        String filePath = LINUX_FILE_PATH;
        return new File(filePath.concat(SECRET_FILE));
    }
}
