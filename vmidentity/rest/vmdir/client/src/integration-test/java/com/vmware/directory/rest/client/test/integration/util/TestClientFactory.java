package com.vmware.directory.rest.client.test.integration.util;

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

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.HostRetriever;
import com.vmware.identity.rest.core.client.SimpleHostRetriever;
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.test.integration.util.TokenFactory;

public class TestClientFactory {

    /**
     * Create an VmdirClient with the given parameters.
     *
     * @param host address of the remote server
     * @param tenant name of the tenant
     * @param username username in UPN format
     * @param password password
     * @return IdmClient
     * @throws IOException
     * @throws ClientException
     * @throws ClientProtocolException
     * @throws KeyStoreException
     * @throws NoSuchAlgorithmException
     * @throws KeyManagementException
     */
    public static VmdirClient createClient(String host, String tenant, String username, String password) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        HostRetriever hostRetriever = new SimpleHostRetriever(host, true);
        VmdirClient client = new VmdirClient(hostRetriever,
                NoopHostnameVerifier.INSTANCE,
                new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {

                    @Override
                    public boolean isTrusted(X509Certificate[] chain, String authType)
                            throws CertificateException {
                        return true;
                    }
                }).build());

        String token = TokenFactory.getAccessToken(host, tenant, username, password);

        client.setToken(new AccessToken(token, AccessToken.Type.JWT));
        return client;
    }

    public static VmdirClient createClient(String host, String tenant, String username, String domain, String password) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        return createClient(host, tenant, UPNUtil.buildUPN(username, domain), password);
    }

}
