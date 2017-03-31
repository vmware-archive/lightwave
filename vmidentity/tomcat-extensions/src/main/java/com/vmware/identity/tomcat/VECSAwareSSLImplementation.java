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
package com.vmware.identity.tomcat;

import java.io.IOException;

import java.security.KeyStore;
import org.apache.juli.logging.Log;
import org.apache.juli.logging.LogFactory;
import org.apache.tomcat.util.net.SSLHostConfigCertificate;
import org.apache.tomcat.util.net.SSLUtil;
import org.apache.tomcat.util.net.jsse.JSSEImplementation;
import org.apache.tomcat.util.net.jsse.JSSEUtil;

import com.vmware.provider.VecsLoadStoreParameter;

public class VECSAwareSSLImplementation extends JSSEImplementation {

	private static final Log log = LogFactory.getLog(VECSAwareSSLImplementation.class);

    @Override
    public SSLUtil getSSLUtil(SSLHostConfigCertificate certificate) {

        try {
            KeyStore ks = getKeystore(certificate);
            certificate.setCertificateKeystore(ks);
        } catch(Exception ex){
            log.error(ex.getStackTrace().toString());
        }
        return new JSSEUtil(certificate);
    }

    /*
     * Gets the SSL server's keystore - VECS.
     */
    protected KeyStore getKeystore(SSLHostConfigCertificate certificate)
            throws IOException {

        String type = certificate.getCertificateKeystoreType();
        String keystoreName = certificate.getCertificateKeystoreFile();
        try {
            if (type == null || type.isEmpty()) {
                throw new IOException(
                    "keystore type must be provided");
      }
            if ("VKS".equalsIgnoreCase(type)) {

                if (keystoreName == null || keystoreName.isEmpty()) {
                    throw new IOException(
                        "keystore file must specify the keystore name");
                }

                KeyStore ks = null;
                String provider = certificate.getCertificateKeystoreProvider();

                if (provider == null || provider.isEmpty()) {
                    ks = KeyStore.getInstance(type);
                } else {
                    ks = KeyStore.getInstance(type, provider);
                }

                VecsLoadStoreParameter params = new VecsLoadStoreParameter(keystoreName);
                ks.load(params);
                resetCertificateKeyStoreFile(certificate);
                return ks;

            } else {
                resetCertificateKeyStoreFile(certificate);
                return certificate.getCertificateKeystore();
            }
        } catch (Exception ex) {
            throw new IOException(
                    "Failed to load keystore " + keystoreName, ex);
        }
    }

    /**
     * Keystorefile contains the store name. Once store name is used to load the
     * vecs keystore, reset the certificate keystorefile.
     * @param certificate
     */
    private void resetCertificateKeyStoreFile(SSLHostConfigCertificate certificate){
        certificate.setCertificateKeystoreFile("");
    }
}
