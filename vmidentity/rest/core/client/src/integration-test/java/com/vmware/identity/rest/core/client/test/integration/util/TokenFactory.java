/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * *********************************************************************/
package com.vmware.identity.rest.core.client.test.integration.util;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import org.apache.http.client.ClientProtocolException;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.conn.ssl.TrustStrategy;
import org.apache.http.ssl.SSLContextBuilder;

import com.vmware.identity.rest.core.client.exceptions.ClientException;

public class TokenFactory {

    public static String getAccessToken(String host, String tenant, String username, String password) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        OIDCClient client = new OIDCClient(host,
                NoopHostnameVerifier.INSTANCE,
                new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {

                    @Override
                    public boolean isTrusted(X509Certificate[] chain, String authType)
                            throws CertificateException {
                        return true;
                    }
                }).build());

        return client.getAccessToken(tenant, username, password);
    }

}
