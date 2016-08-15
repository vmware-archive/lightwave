/*
 *  Copyright (c) 2015 VMware, Inc.  All Rights Reserved.
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
/**
 * @author dmehta
 */
package com.vmware.identity.tomcat;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.security.KeyStore;

import org.apache.commons.lang.SystemUtils;
import org.apache.tomcat.util.net.AbstractEndpoint;
import org.apache.tomcat.util.net.jsse.JSSESocketFactory;
import com.vmware.provider.VecsLoadStoreParameter;
import org.apache.commons.lang.SystemUtils;

public class VECSAwareJSSESocketFactory extends JSSESocketFactory {

    private final AbstractEndpoint _endpoint;
    private final String store;
    private final String KEYSTORE_FILE="keystore.txt";
    private final String CONF_DIR="conf";

    public VECSAwareJSSESocketFactory(AbstractEndpoint endpoint, String store) {
        super(endpoint);
        this._endpoint = endpoint;
        this.store = store;
        System.out.println("Store name : " +this.store);
    }

    /*
     * Gets the SSL server's keystore.
     */
    @Override
    protected KeyStore getKeystore(String type, String provider, String pass)
            throws IOException {

        if ("VKS".equalsIgnoreCase(type)) {

            System.out.println("Store name in server.xml- " + store);
            String keystoreName = store;
            if (keystoreName == null || keystoreName.isEmpty()) {
                throw new IOException(
                        "keystore file must specify the keystore name");
            }

            KeyStore ks = null;
            try {
                if (provider == null || provider.isEmpty()) {
                    ks = KeyStore.getInstance(type);
                } else {
                    ks = KeyStore.getInstance(type, provider);
                }

                VecsLoadStoreParameter params = new VecsLoadStoreParameter(
                        keystoreName);
                ks.load(params);
            } catch (Exception ex) {
                throw new IOException(
                        "Failed to load keystore " + keystoreName, ex);
            }
            return ks;
        } else {
            return super.getKeystore(type, provider, pass);
        }
    }
}
