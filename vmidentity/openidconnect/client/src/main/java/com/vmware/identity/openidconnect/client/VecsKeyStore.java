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

package com.vmware.identity.openidconnect.client;

import java.io.IOException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;

import com.vmware.provider.VecsLoadStoreParameter;

/**
 * VECS Key Store
 *
 * @author Jun Sun
 */
class VecsKeyStore {
    private static KeyStore instance = null;

    private VecsKeyStore() {
    };

    static KeyStore getInstance() {
        if (instance == null) {
            synchronized (VecsKeyStore.class) {
                if (instance == null) {
                    try {
                        instance = KeyStore.getInstance("VKS");
                        instance.load(new VecsLoadStoreParameter("TRUSTED_ROOTS"));
                    } catch (KeyStoreException | NoSuchAlgorithmException | CertificateException | IOException e) {
                        throw new IllegalStateException("Load VECS key store failed. Check if VECS is supported on this client.");
                    }
                }
            }
        }
        return instance;
    }
}
