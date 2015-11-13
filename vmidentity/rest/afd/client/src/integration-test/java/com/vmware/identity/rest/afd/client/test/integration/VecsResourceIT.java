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
package com.vmware.identity.rest.afd.client.test.integration;

import static org.junit.Assert.assertFalse;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.conn.ssl.TrustStrategy;
import org.apache.http.ssl.SSLContextBuilder;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.afd.client.AfdClient;
import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.HostRetriever;
import com.vmware.identity.rest.core.client.SimpleHostRetriever;
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.test.integration.IntegrationTestProperties;
import com.vmware.identity.rest.core.client.test.integration.util.TokenFactory;
import com.vmware.identity.rest.core.data.CertificateDTO;

public class VecsResourceIT {

    private static AfdClient client;
    private static IntegrationTestProperties properties;

    @BeforeClass
    public static void setup() throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        properties = new IntegrationTestProperties();
        HostRetriever hostRetriever = new SimpleHostRetriever(properties.getHost(), true);
        client = new AfdClient(hostRetriever,
                NoopHostnameVerifier.INSTANCE,
                new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {

                    @Override
                    public boolean isTrusted(X509Certificate[] chain, String authType)
                            throws CertificateException {
                        return true;
                    }
                }).build());

        String adminUPN = UPNUtil.buildUPN(properties.getSystemAdminUsername(), properties.getSystemDomain());
        String token = TokenFactory.getAccessToken(properties.getHost(), properties.getSystemTenant(), adminUPN, properties.getSystemAdminPassword());
        AccessToken accessToken = new AccessToken(token, AccessToken.Type.JWT);
        client.setToken(accessToken);
    }

    @Test
    public void testGetSSLCertificates() throws ClientProtocolException, ClientException, HttpException, IOException {
        List<CertificateDTO> certs = client.vecs().getSSLCertificates();

        assertFalse(certs.isEmpty());
    }

}
